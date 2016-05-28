#include <string.h>

#include <json-c/json.h>

#include <respondd.h>
#include <uci.h>


#include "ffffm.h"

// https://github.com/freifunk-gluon/gluon/blob/d2b74b4cf048ecb8706809021332ed3e7c72b2f3/package/gluon-node-info/src/respondd.c
static struct json_object * get_number(struct uci_context *ctx, struct uci_section *s, const char *name) {
	const char *val = uci_lookup_option_string(ctx, s, name);
	if (!val || !*val)
		return NULL;

	char *end;
	double d = strtod(val, &end);
	if (*end)
		return NULL;

	return json_object_new_double(d);
}

// https://github.com/freifunk-gluon/gluon/blob/d2b74b4cf048ecb8706809021332ed3e7c72b2f3/package/gluon-node-info/src/respondd.c
static struct uci_section * get_first_section(struct uci_package *p, const char *type) {
	struct uci_element *e;
	uci_foreach_element(&p->sections, e) {
		struct uci_section *s = uci_to_section(e);
		if (!strcmp(s->type, type))
			return s;
	}

	return NULL;
}

//static struct get_wireless_info(void) {
//	struct uci_context *ctx = uci_alloc_context();
//	struct json_object *ret;
//
//	ctx->flags &= ~UCI_FLAG_STRICT;
//
//	struct uci_package *p;
//	if (!uci_load(ctx, "wireless", &p)) {
//		struct json_object *location = get_location(ctx, p);
//		if (location)
//			json_object_object_add(ret, "location", location);
//
//		struct json_object *owner = get_owner(ctx, p);
//		if (owner)
//			json_object_object_add(ret, "owner", owner);
//
//		json_object_object_add(ret, "system", get_system(ctx, p));
//	}
//
//	uci_free_context(ctx);
//}

static struct json_object *get_nexthop(void) {
	struct json_object *ret;
	char *nexthop_s;
	struct json_object *nexthop;

	nexthop_s = ffffm_get_nexthop();
	if (!nexthop_s)
		return NULL;

	ret = json_object_new_string(nexthop_s);
	free(nexthop_s);
	return ret;
}

static struct json_object *respondd_provider_nodeinfo(void) {
	struct json_object *ret = json_object_new_object();

	struct json_object *nexthop;

	nexthop = get_nexthop();
	if (nexthop)
		json_object_object_add(ret, "nexthop", nexthop);

	return ret;
}


const struct respondd_provider_info respondd_providers[] = {
	{"nodeinfo", respondd_provider_nodeinfo},
	{}
};
