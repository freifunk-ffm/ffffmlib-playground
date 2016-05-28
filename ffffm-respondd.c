#include <respondd.h>
#include <json-c/json.h>

#include "ffffm.h"

static struct json_object * respondd_provider_nodeinfo(void) {
	struct json_object *ret = json_object_new_object();

	char *nexthop_s;
	struct json_object *nexthop;

	nexthop_s = ffffm_get_nexthop();
	if (!nexthop_s)
		goto err;

	nexthop = json_object_new_string(nexthop_s);

	json_object_object_add(ret, "nexthop", nexthop);

	free(nexthop_s);
	return ret;

err:
	json_object_put(ret);
	return NULL;
}


const struct respondd_provider_info respondd_providers[] = {
	{"nodeinfo", respondd_provider_nodeinfo},
	{}
};

int main() {
#include <stdio.h>
	printf("%s", json_object_to_json_string(respondd_provider_nodeinfo()));
}
