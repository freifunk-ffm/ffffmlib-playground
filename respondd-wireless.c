#include <string.h>

#include <json-c/json.h>

#include <respondd.h>

#include "ffffm.h"

static struct json_object *get_airtime(void) {
	double airtime = ffffm_get_airtime();
	if (FFFFM_INVALID_AIRTIME == airtime)
		return NULL;

	return json_object_new_double(airtime);
}

static struct json_object *respondd_provider_statistics(void) {
	struct json_object *ret = json_object_new_object();

	if (!ret)
		return NULL;

	struct json_object *airtime;

	airtime = get_airtime();
	if (airtime)
		json_object_object_add(ret, "airtime2", airtime);

	return ret;
}

static struct json_object *respondd_provider_nodeinfo(void) {
	struct ffffm_wifi_info *i = ffffm_get_wifi_info();
	if (!i)
		goto end;

	struct json_object *ret = json_object_new_object();
        struct json_object *v;
	if (!ret)
		goto end;

	if (i->c24) {
		v = json_object_new_int64(i->c24);
		if (!v)
			goto end;
		json_object_object_add(ret, "chan2", v);
	}
	if (i->c50) {
		v = json_object_new_int64(i->c50);
		if (!v)
			goto end;
		json_object_object_add(ret, "chan5", v);
	}
	if (i->t24) {
		v = json_object_new_int64(i->t24);
		if (!v)
			goto end;
		json_object_object_add(ret, "txpower2", v);
	}
	if (i->t50) {
		v = json_object_new_int64(i->t50);
		if (!v)
			goto end;
		json_object_object_add(ret, "txpower5", v);
	}
end:
        free(i);
	return ret;
	
}


const struct respondd_provider_info respondd_providers[] = {
	{"statistics", respondd_provider_statistics},
	{"nodeinfo", respondd_provider_nodeinfo},
	{0},
};
