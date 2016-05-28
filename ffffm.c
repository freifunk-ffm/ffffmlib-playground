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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <uci.h>

#include <sys/socket.h>
#include <unl.h>
#include <linux/nl80211.h>

#include "ffffm.h"

const unsigned int FFFFM_INVALID_CHANNEL = 0;
const double FFFFM_INVALID_AIRTIME = -1;

static const char *gatewayfile = "/sys/kernel/debug/batman_adv/bat0/gateways";
static const char *wifi_24_dev = "radio0";
static const char *wifi_50_dev = "radio1";
static const char *airtime_interface = "client0";

// https://github.com/freifunk-gluon/gluon/blob/d2b74b4cf048ecb8706809021332ed3e7c72b2f3/package/gluon-mesh-batman-adv-core/src/respondd.c
char *ffffm_get_nexthop(void) {
	char * ret = NULL;
	FILE *f = fopen(gatewayfile, "r");
	if (!f)
		return NULL;

	char *line = NULL;
	size_t len = 0;

	while (getline(&line, &len, f) >= 0) {
		char addr[18];
		unsigned char metric;
		char nexthop[18];

		if(sscanf(line, "=> %17[0-9a-fA-F:] (%hhu) %17[0-9a-fA-F:]", addr, &metric, nexthop) == 3) {
			ret = strdup(nexthop);
			break;
		}
	}

	free(line);
	fclose(f);
	return ret;
}

static inline const char *lookup_option_value(
		struct uci_context *ctx, struct uci_package *p,
		const char *section_name, const char *option_name) {

	struct uci_section *s = uci_lookup_section(ctx, p, section_name);
	if (!s)
		return NULL;
	return uci_lookup_option_string(ctx, s, option_name);
}

static inline unsigned char parse_channel(const char *s) {
	char *endptr = NULL;
	long int result = strtol(s, &endptr, 10);

	if ('\0' != *endptr)
		return FFFFM_INVALID_CHANNEL;
	if (result > UCHAR_MAX)
		return FFFFM_INVALID_CHANNEL;
	
	return (unsigned char)(result % UCHAR_MAX);
}

struct ffffm_wifi_info *ffffm_get_wifi_info(void) {
	struct uci_context *ctx = uci_alloc_context();
	ctx->flags &= ~UCI_FLAG_STRICT;

	struct ffffm_wifi_info *ret = calloc(1, sizeof(&ret));

	if (!ret)
		goto end;

	struct uci_package *p;
	if (uci_load(ctx, "wireless", &p))
		goto end;

	const char *c24 = lookup_option_value(ctx, p, wifi_24_dev, "channel");
	const char *c50 = lookup_option_value(ctx, p, wifi_50_dev, "channel");

	ret->channel_24 = parse_channel(c24);
	ret->channel_50 = parse_channel(c50);
end:
	uci_free_context(ctx);
	return ret;
}

void ffffm_free_wifi_info(struct ffffm_wifi_info *i) {
	free(i);
}

double ffffm_get_airtime(void) {
	struct unl *unl = calloc(1, sizeof(*unl));
	double ret = FFFFM_INVALID_AIRTIME;

	int status = unl_genl_init(unl, NL80211_GENL_NAME);
	if (status < 0)
		goto end;

	struct nl_msg *msg = unl_genl_msg(unl, NL80211_CMD_GET_SURVEY, false);
	if (!msg)
		goto end;

	NLA_PUT_STRING(msg, NL80211_ATTR_IFNAME, airtime_interface);
	if (unl_genl_request_single(unl, msg, &msg) < 0)
		goto end;

	struct nlattr *attr = unl_find_attr(unl, msg, NL80211_ATTR_SURVEY_INFO);

	struct nlattr *tb[NL80211_SURVEY_INFO_MAX + 1];
	status = nla_parse_nested(tb, NL80211_SURVEY_INFO_MAX, attr, NULL);
	uint64_t channel_active_time = nla_get_u64(tb[NL80211_SURVEY_INFO_TIME]);
	uint64_t channel_busy_time = nla_get_u64(tb[NL80211_SURVEY_INFO_TIME_BUSY]);

	ret = ((double) channel_busy_time) / channel_active_time;

end:
nla_put_failure:
	unl_free(unl);
	nlmsg_free(msg);
	return ret;
}
