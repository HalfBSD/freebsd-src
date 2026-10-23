#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "client.h"
#include "drm.h"
#include "evdev.h"
#include "hidraw.h"
#include "linked_list.h"
#include "log.h"
#include "protocol.h"
#include "seat.h"
#include "terminal.h"
#include "wscons.h"

static int console_close(void);

/* A seat owns the sole console; additional local sessions are unsupported. */
struct seat *seat_create(const char *seat_name) {
	struct seat *seat = calloc(1, sizeof(struct seat));
	if (seat == NULL) {
		return NULL;
	}
	linked_list_init(&seat->clients);
	seat->seat_name = strdup(seat_name);
	if (seat->seat_name == NULL) {
		free(seat);
		return NULL;
	}
	/* Restore a text/keyboard baseline after a previous daemon crash. */
	if (console_close() == -1) {
		log_error("Could not initialize console state");
		free(seat->seat_name);
		free(seat);
		return NULL;
	}
	log_infof("Created console seat %s", seat_name);
	return seat;
}

void seat_destroy(struct seat *seat) {
	assert(seat);
	while (!linked_list_empty(&seat->clients)) {
		struct client *client = (struct client *)seat->clients.next;
		assert(client->seat == seat);
		client_destroy(client);
	}
	linked_list_remove(&seat->link);
	free(seat->seat_name);
	free(seat);
}

/* Console handoff must either complete or restore the keyboard/text state. */
static int console_open(void) {
	int fd = terminal_open();
	if (fd == -1) return -1;
	if (terminal_set_keyboard(fd, false) == -1 ||
	    terminal_set_graphics(fd, true) == -1) {
		int saved = errno;
		terminal_set_graphics(fd, false);
		terminal_set_keyboard(fd, true);
		close(fd);
		errno = saved;
		return -1;
	}
	close(fd);
	return 0;
}

static int console_close(void) {
	int fd = terminal_open();
	if (fd == -1) return -1;
	int graphics = terminal_set_graphics(fd, false);
	int keyboard = terminal_set_keyboard(fd, true);
	close(fd);
	return graphics == -1 || keyboard == -1 ? -1 : 0;
}

/*
 * seat_add_client assigns a session ID to the client and adds it to the seat,
 * if allowed. The client does not open the seat, remaining closed until
 * `seat_open_client` is called.
 *
 * Fails if the client is not eligible to be added to a new seat, or if the
 * seat does not accept new clients.
 */
int seat_add_client(struct seat *seat, struct client *client) {
	if (client->seat != NULL) {
		log_error("Could not add client: client is already a member of a seat");
		errno = EBUSY;
		return -1;
	}

	if (!linked_list_empty(&seat->clients)) {
		errno = EBUSY;
		return -1;
	}
	if (client->session != -1) {
		errno = EINVAL;
		return -1;
	}
	client->session = 1;

	client->seat = seat;
	linked_list_remove(&client->link);
	linked_list_insert(&seat->clients, &client->link);

	log_infof("Added client %d to %s", client->session, seat->seat_name);

	return 0;
}

/*
 * seat_remove_client tears down the client and removes it from the seat,
 * revoking any open devices as necessary. If the client was active on the seat
 * at the time of this call, seat_activate is called to activate a new client
 * if any is eligible. The sole console is restored for non-graphical use.
 */
void seat_remove_client(struct client *client) {
	struct seat *seat = client->seat;

	linked_list_remove(&client->link);
	linked_list_init(&client->link);

	while (!linked_list_empty(&client->devices)) {
		struct seat_device *device = (struct seat_device *)client->devices.next;
		seat_close_device(client, device);
	}

	if (seat->active_client == client) {
		seat->active_client = NULL;
		if (console_close() == -1)
			log_error("Could not restore console after client removal");
	}

	client->state = CLIENT_CLOSED;
	client->seat = NULL;

	log_infof("Removed client %d from %s", client->session, seat->seat_name);
}

/*
 * seat_find_device finds an open device on the seat based on its device ID.
 */
struct seat_device *seat_find_device(struct client *client, int device_id) {
	if (device_id == 0) {
		errno = EINVAL;
		return NULL;
	}

	for (struct linked_list *elem = client->devices.next; elem != &client->devices;
	     elem = elem->next) {
		struct seat_device *seat_device = (struct seat_device *)elem;
		if (seat_device->device_id == device_id) {
			return seat_device;
		}
	}
	errno = ENOENT;
	return NULL;
}

/*
 * seat_open_device opens a device by the specified device path for the client,
 * sanitizing the path and configuring the device as necessary for usage. If
 * such a device has already been opened, the reference count is increased and
 * the device entry is reused.
 *
 * Fails if the client is not active or has exceeded its device limit, or if
 * the device type is not supported or could not be opened.
 */
struct seat_device *seat_open_device(struct client *client, const char *path) {
	struct seat *seat = client->seat;
	log_debugf("Opening device %s for client %d on %s", path, client->session, seat->seat_name);

	if (client->state != CLIENT_ACTIVE) {
		log_error("Could not open device: client is not active");
		errno = EPERM;
		return NULL;
	}
	assert(seat->active_client == client);

	char sanitized_path[PATH_MAX];
	if (realpath(path, sanitized_path) == NULL) {
		log_errorf("Could not canonicalize path %s: %s", path, strerror(errno));
		return NULL;
	}

	enum seat_device_type type;
	if (path_is_evdev(sanitized_path)) {
		type = SEAT_DEVICE_TYPE_EVDEV;
	} else if (path_is_drm(sanitized_path)) {
		type = SEAT_DEVICE_TYPE_DRM;
	} else if (path_is_wscons(sanitized_path)) {
		type = SEAT_DEVICE_TYPE_WSCONS;
	} else if (path_is_hidraw(sanitized_path)) {
		type = SEAT_DEVICE_TYPE_HIDRAW;
	} else {
		log_errorf("%s is not a supported device type ", sanitized_path);
		errno = ENOENT;
		return NULL;
	}

	int device_id = 1;
	size_t device_count = 0;
	for (struct linked_list *elem = client->devices.next; elem != &client->devices;
	     elem = elem->next) {
		struct seat_device *old_device = (struct seat_device *)elem;

		if (strcmp(old_device->path, sanitized_path) == 0) {
			old_device->ref_cnt++;
			return old_device;
		}

		if (old_device->device_id >= device_id) {
			device_id = old_device->device_id + 1;
		}
		device_count++;
	}

	if (device_count >= MAX_SEAT_DEVICES) {
		log_error("Client exceeded max seat devices");
		errno = EMFILE;
		return NULL;
	}

	int fd = open(sanitized_path, O_RDWR | O_NOCTTY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd == -1) {
		log_errorf("Could not open file: %s", strerror(errno));
		return NULL;
	}

	switch (type) {
	case SEAT_DEVICE_TYPE_DRM:
		if (drm_set_master(fd) == -1) {
			log_errorf("Could not make device fd drm master: %s", strerror(errno));
		}
		break;
	case SEAT_DEVICE_TYPE_EVDEV:
		// Nothing to do here
		break;
	case SEAT_DEVICE_TYPE_WSCONS:
		// Nothing to do here
		break;
	case SEAT_DEVICE_TYPE_HIDRAW:
		// Nothing to do here
		break;
	default:
		log_errorf("Invalid seat device type: %d", type);
		abort();
	}

	struct seat_device *device = calloc(1, sizeof(struct seat_device));
	if (device == NULL) {
		log_errorf("Allocation failed: %s", strerror(errno));
		close(fd);
		errno = ENOMEM;
		return NULL;
	}

	device->path = strdup(sanitized_path);
	if (device->path == NULL) {
		log_errorf("Allocation failed: %s", strerror(errno));
		close(fd);
		free(device);
		errno = ENOMEM;
		return NULL;
	}

	device->ref_cnt = 1;
	device->type = type;
	device->fd = fd;
	device->device_id = device_id;
	device->active = true;
	linked_list_insert(&client->devices, &device->link);

	return device;
}

/*
 * seat_deactivate_device revokes access to the device so that the client can
 * no longer use it for privileged actions. Depending on the device type, the
 * client may be required to reopen the device to use it again.
 */
static int seat_deactivate_device(struct seat_device *seat_device) {
	if (!seat_device->active) {
		return 0;
	}
	switch (seat_device->type) {
	case SEAT_DEVICE_TYPE_DRM:
		if (drm_drop_master(seat_device->fd) == -1) {
			log_errorf("Could not revoke drm master on device fd: %s", strerror(errno));
			return -1;
		}
		break;
	case SEAT_DEVICE_TYPE_EVDEV:
		if (evdev_revoke(seat_device->fd) == -1) {
			log_errorf("Could not revoke evdev on device fd: %s", strerror(errno));
			return -1;
		}
		break;
	case SEAT_DEVICE_TYPE_HIDRAW:
		if (hidraw_revoke(seat_device->fd) == -1) {
			log_errorf("Could not revoke hidraw on device fd: %s", strerror(errno));
			return -1;
		}
		break;
	case SEAT_DEVICE_TYPE_WSCONS:
		// Nothing to do here
		break;
	default:
		log_errorf("Invalid seat device type: %d", seat_device->type);
		abort();
	}
	seat_device->active = false;
	return 0;
}

/*
 * seat_close_device reduces the reference count for the device. If it reaches
 * zero, the device is deactivated, closed and removed.
 */
void seat_close_device(struct client *client, struct seat_device *seat_device) {
	log_debugf("Closing device %s for client %d on %s", seat_device->path, client->session,
		   client->seat->seat_name);

	seat_device->ref_cnt--;
	if (seat_device->ref_cnt > 0) {
		return;
	}

	// The caller might be closing devices in error handling. As we cannot
	// fail anyway, let's ensure we do not clobber errno for caller
	// convenience.
	int stored_errno = errno;

	linked_list_remove(&seat_device->link);
	if (seat_device->fd != -1) {
		seat_deactivate_device(seat_device);
		close(seat_device->fd);
	}
	free(seat_device->path);
	free(seat_device);

	errno = stored_errno;
}

/*
 * seat_activate_device re-activates the device for reuse after deactivation.
 * It fails if the device cannot be reused.
 */
static int seat_activate_device(struct seat_device *seat_device) {
	if (seat_device->active) {
		return 0;
	}
	switch (seat_device->type) {
	case SEAT_DEVICE_TYPE_DRM:
		if (drm_set_master(seat_device->fd) == -1) {
			log_errorf("Could not make device fd drm master: %s", strerror(errno));
		}
		seat_device->active = true;
		break;
	case SEAT_DEVICE_TYPE_EVDEV:
		errno = EINVAL;
		return -1;
	case SEAT_DEVICE_TYPE_HIDRAW:
		errno = EINVAL;
		return -1;
	case SEAT_DEVICE_TYPE_WSCONS:
		// Nothing to do here
		break;
	default:
		log_errorf("Invalid seat device type: %d", seat_device->type);
		abort();
	}

	return 0;
}

/*
 * seat_open_client makes the client active. The client must be a disabled or
 * new member of the seat, and the seat must not have an active client.
 * The sole console is configured for a graphical session.
 */
int seat_open_client(struct seat *seat, struct client *client) {
	assert(client->seat == seat);
	if (client->state != CLIENT_NEW) {
		log_error("Could not enable client: client is not new");
		errno = EALREADY;
		return -1;
	}

	if (seat->active_client != NULL) {
		log_error("Could not enable client: seat already has an active client");
		errno = EBUSY;
		return -1;
	}

	if (console_open() == -1) {
		log_error("Could not hand console to client");
		return -1;
	}

	for (struct linked_list *elem = client->devices.next; elem != &client->devices;
	     elem = elem->next) {
		struct seat_device *device = (struct seat_device *)elem;
		if (seat_activate_device(device) == -1) {
			log_errorf("Could not activate %s: %s", device->path, strerror(errno));
		}
	}

	if (client_send_enable_seat(client) == -1) {
		log_error("Could not send enable signal to client");
		for (struct linked_list *elem = client->devices.next; elem != &client->devices;
		     elem = elem->next) {
			struct seat_device *device = (struct seat_device *)elem;
			seat_deactivate_device(device);
		}
		console_close();
		return -1;
	}

	client->state = CLIENT_ACTIVE;
	seat->active_client = client;
	log_infof("Opened client %d on %s", client->session, seat->seat_name);
	return 0;
}
