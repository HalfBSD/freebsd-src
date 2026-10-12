/* Focused TCP quota ownership and error-path regression.
 * Link against libunbound.a with --wrap=ub_event_new, --wrap=ub_event_add,
 * --wrap=recv and --wrap=writev. No production fault-injection hooks.
 */
#include "config.h"
#include "util/config_file.h"
#include "util/netevent.h"
#include "util/ub_event.h"
#include "util/net_help.h"
#include "util/tcp_conn_limit.h"
#include "services/listen_dnsport.h"
#include "sldns/sbuffer.h"
#include <sys/uio.h>
#undef NDEBUG
#include <assert.h>

static int fail_new, fail_add, fail_read, fail_write;
struct ub_event* __real_ub_event_new(struct ub_event_base*, int, short,
    void (*)(int, short, void*), void*);
int __real_ub_event_add(struct ub_event*, struct timeval*);
ssize_t __real_recv(int, void*, size_t, int);
ssize_t __real_writev(int, const struct iovec*, int);

struct ub_event*
__wrap_ub_event_new(struct ub_event_base* base, int fd, short bits,
    void (*callback)(int, short, void*), void* arg)
{
	if(fail_new) { fail_new = 0; return NULL; }
	return __real_ub_event_new(base, fd, bits, callback, arg);
}

int
__wrap_ub_event_add(struct ub_event* event, struct timeval* timeout)
{
	if(fail_add) { fail_add = 0; return -1; }
	return __real_ub_event_add(event, timeout);
}

ssize_t
__wrap_recv(int fd, void* data, size_t length, int flags)
{
	if(fail_read) { fail_read = 0; errno = EIO; return -1; }
	return __real_recv(fd, data, length, flags);
}

ssize_t
__wrap_writev(int fd, const struct iovec* vec, int count)
{
	if(fail_write) { fail_write = 0; errno = EIO; return -1; }
	return __real_writev(fd, vec, count);
}

static int
connect_client(struct sockaddr_in* address)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(fd >= 0);
	assert(connect(fd, (struct sockaddr*)address, sizeof(*address)) == 0);
	return fd;
}

static struct comm_point*
active_child(struct comm_point* parent)
{
	int i;
	for(i=0; i<parent->max_tcp_count; i++)
		if(parent->tcp_handlers[i]->fd != -1)
			return parent->tcp_handlers[i];
	return NULL;
}

int
main(void)
{
	struct config_file* cfg = config_create();
	struct tcl_list* list = tcl_list_create();
	struct comm_base* base = comm_base_create(0);
	struct sockaddr_in address;
	struct sockaddr_storage remote;
	socklen_t length = sizeof(address);
	struct comm_point* parent;
	struct comm_point* child;
	struct tcl_addr* quota;
	int listener, client, rejected, iteration;
	assert(cfg && list && base);
	assert(cfg_str2list_insert(&cfg->tcp_connection_limits,
	    strdup("127.0.0.1/32"), strdup("1")));
	assert(tcl_list_apply_cfg(list, cfg));
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	memset(&remote, 0, sizeof(remote));
	memcpy(&remote, &address, sizeof(address));
	quota = tcl_addr_lookup(list, &remote, sizeof(address));
	assert(quota && quota->count == 0);
	listener = socket(AF_INET, SOCK_STREAM, 0);
	assert(listener >= 0);
	assert(bind(listener, (struct sockaddr*)&address, sizeof(address)) == 0);
	assert(getsockname(listener, (struct sockaddr*)&address, &length) == 0);
	assert(listen(listener, 8) == 0);
	fd_set_nonblock(listener);
	parent = comm_point_create_tcp(base, listener, 2, 30000, list, 512,
	    NULL, listen_type_tcp, 0, NULL, NULL, NULL);
	assert(parent);

	/* Event allocation fails before accept: no quota acquired; retry works. */
	client = connect_client(&address);
	fail_new = 1;
	comm_point_tcp_accept_callback(listener, UB_EV_READ, parent);
	assert(!fail_new && quota->count == 0 && parent->cur_tcp_count == 0);
	comm_point_tcp_accept_callback(listener, UB_EV_READ, parent);
	child = active_child(parent);
	assert(child && child->tcl_addr == quota && quota->count == 1);
	assert(parent->tcl_addr == NULL);
	rejected = connect_client(&address);
	comm_point_tcp_accept_callback(listener, UB_EV_READ, parent);
	assert(quota->count == 1 && parent->cur_tcp_count == 1);
	assert(parent->tcl_addr == NULL);
	close(rejected);
	comm_point_drop_reply(&child->repinfo);
	assert(quota->count == 0 && child->tcl_addr == NULL);
	comm_point_close(child); /* Repeated close must not release twice. */
	assert(quota->count == 0);
	close(client);

	/* Registration failure after quota acquisition must reclaim the handler. */
	client = connect_client(&address);
	fail_add = 1;
	comm_point_tcp_accept_callback(listener, UB_EV_READ, parent);
	assert(!fail_add && quota->count == 0 && parent->cur_tcp_count == 0);
	assert(active_child(parent) == NULL);
	close(client);

	/* Deterministic read/write failures and timeout share the release path. */
	for(iteration=0; iteration<3; iteration++) {
		client = connect_client(&address);
		comm_point_tcp_accept_callback(listener, UB_EV_READ, parent);
		child = active_child(parent);
		assert(child && quota->count == 1);
		child->tcp_do_close = 1; /* No callback is needed after reclamation. */
		if(iteration == 0) {
			fail_read = 1;
			comm_point_tcp_handle_callback(child->fd, UB_EV_READ, child);
			assert(!fail_read);
		} else if(iteration == 1) {
			sldns_buffer_set_limit(child->buffer, 1);
			child->tcp_is_reading = 0;
			fail_write = 1;
			comm_point_tcp_handle_callback(child->fd, UB_EV_WRITE, child);
			assert(!fail_write);
		} else {
			comm_point_tcp_handle_callback(child->fd, UB_EV_TIMEOUT, child);
		}
		assert(quota->count == 0 && parent->cur_tcp_count == 0);
		assert(child->tcl_addr == NULL && child->fd == -1);
		comm_point_close(child);
		assert(quota->count == 0);
		close(client);
	}

	/* Destroy the listener with an active child; the quota tree outlives it. */
	client = connect_client(&address);
	comm_point_tcp_accept_callback(listener, UB_EV_READ, parent);
	assert(quota->count == 1);
	comm_point_delete(parent);
	assert(quota->count == 0);
	close(client);
	comm_base_delete(base);
	tcl_list_delete(list);
	config_delete(cfg);
	puts("PASS TCP quota ownership, repeated close, rejected accept, initialization/read/write/timeout failure and shutdown");
	return 0;
}
