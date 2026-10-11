/* Deterministic boundaries for the actual shared provider-health state. */
#include "config.h"
#include "services/cache/infra.h"
#include "util/config_file.h"
#include "util/net_help.h"
#include "iterator/iter_delegpt.h"
#include <assert.h>

/* Link with --wrap=strdup to exercise the TLS name allocation failure. */
static int fail_strdup;
char* __real_strdup(const char*);
char* __wrap_strdup(const char* name)
{
	return fail_strdup ? NULL : __real_strdup(name);
}

int main(void)
{
	struct config_file* cfg = config_create();
	struct infra_cache* infra = infra_create(cfg);
	struct sockaddr_storage addr[6];
	socklen_t len;
	time_t now = 1000;
	unsigned i;
	assert(cfg && infra);
	for(i=0; i<6; i++)
		assert(ipstrtoaddr(infra_forward_addresses[i], 53, &addr[i], &len));
	((struct sockaddr_in*)&addr[0])->sin_port = htons(5353);
	assert(infra_forward_index(&addr[0]) == -1);
	((struct sockaddr_in*)&addr[0])->sin_port = htons(53);
	assert(infra_forward_mask(infra, now) == 3);
	infra_forward_result(infra, &addr[0], now, 0);
	assert(infra_forward_mask(infra, now) == 2);
	infra_forward_result(infra, &addr[1], now, 0);
	assert(infra_forward_mask(infra, now) == 12);
	assert(!infra_forward_due(infra, now+29));
	assert(infra_forward_due(infra, now+30));
	assert(!infra_forward_due(infra, now+30));
	infra_forward_probe(infra, &addr[0], now+30);
	infra_forward_probe(infra, &addr[1], now+30);
	assert(infra_forward_mask(infra, now+30) == 12);
	infra_forward_result(infra, &addr[0], now+30, 1);
	/* One good exchange recovers service but retains the backoff history. */
	assert(infra_forward_mask(infra, now+30) == 1);
	infra_forward_result(infra, &addr[0], now+30, 0);
	assert(infra->forward_health[0].retry_after == now+90);
	infra_forward_result(infra, &addr[0], now+31, 0);
	assert(infra->forward_health[0].retry_after == now+90);
	/* Two consecutive replies clear hysteresis. */
	infra_forward_result(infra, &addr[0], now+90, 1);
	infra_forward_result(infra, &addr[0], now+90, 1);
	infra_forward_result(infra, &addr[0], now+90, 0);
	assert(infra->forward_health[0].retry_after == now+120);
	now += 120;
	for(i=0; i<12; i++) {
		infra_forward_result(infra, &addr[0], now, 0);
		assert(infra->forward_health[0].retry_interval <= 300);
		now = infra->forward_health[0].retry_after;
	}
	assert(infra->forward_health[0].retry_interval == 300);
	for(i=0; i<6; i++) infra_forward_result(infra, &addr[i], now, 0);
	assert(infra_forward_mask(infra, now) == 0);
	assert(infra_forward_mask(infra, now+301) == 3);
	{
		struct delegpt* dp = delegpt_create_mlc((uint8_t*)"");
		assert(dp);
		fail_strdup = 1;
		assert(!delegpt_add_addr_mlc(dp, &addr[0], len, 0, 0,
			"fixture.test", -1));
		assert(!dp->target_list && !dp->usable_list);
		fail_strdup = 0;
		delegpt_free_mlc(dp);
	}
	infra_delete(infra);
	config_delete(cfg);
	return 0;
}
