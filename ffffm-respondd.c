/*
  Copyright (c) 2016, Thomas Weißschuh <freifunk@t-8ch.de>
  Copyright (c) 2016, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>

#include <json-c/json.h>

#include <respondd.h>

#include "ffffm.h"

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

static struct json_object *get_wifi_info(void) {
	struct ffffm_wifi_info *i = ffffm_get_wifi_info();
	if (!i)
		goto end;

	struct json_object *ret = json_object_new_object();
	if (!ret)
		goto end;

	if (i->channel_24 != FFFFM_INVALID_CHANNEL) {
		struct json_object *w24_c = json_object_new_int64(i->channel_24);
		if (!w24_c)
			goto end;
		json_object_object_add(ret, "channel_24", w24_c);
	}
	if (i->channel_50 != FFFFM_INVALID_CHANNEL) {
		struct json_object *w50_c = json_object_new_int64(i->channel_50);
		if (!w50_c)
			goto end;
		json_object_object_add(ret, "channel_50", w50_c);
	}
end:
	ffffm_free_wifi_info(i);
	return ret;
	
}

static struct json_object *get_airtime(void) {
	double airtime = ffffm_get_airtime();
	if (FFFFM_INVALID_AIRTIME == airtime)
		return NULL;

	return json_object_new_double(airtime);
}

static struct json_object *respondd_provider_nodeinfo(void) {
	struct json_object *ret = json_object_new_object();

	if (!ret)
		return NULL;

	struct json_object *nexthop, *wifi_info, *airtime;

	nexthop = get_nexthop();
	if (nexthop)
		json_object_object_add(ret, "nexthop", nexthop);

	wifi_info = get_wifi_info();
	if (wifi_info)
		json_object_object_add(ret, "wifi_info", wifi_info);

	airtime = get_airtime();
	if (airtime)
		json_object_object_add(ret, "airtime", airtime);

	return ret;
}


const struct respondd_provider_info respondd_providers[] = {
	{"nodeinfo", respondd_provider_nodeinfo},
	{}
};
