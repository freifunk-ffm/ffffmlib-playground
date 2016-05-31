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
#include <linux/nl80211.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>

#include "ffffm.h"

const unsigned int FFFFM_INVALID_CHANNEL = 0;
const double FFFFM_INVALID_AIRTIME = -1;

static const char *gatewayfile = "/sys/kernel/debug/batman_adv/bat0/gateways";
static const char *wifi_24_dev = "radio0";
static const char *wifi_50_dev = "radio1";
static const char *airtime_interface = "client0";

struct airtime_result {
        uint32_t frequency;
        uint64_t active_time;
        uint64_t busy_time;
};

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
	long int result;

        if (!s)
		return FFFFM_INVALID_CHANNEL;

        result = strtol(s, &endptr, 10);

        if (!endptr)
		return FFFFM_INVALID_CHANNEL;
	if ('\0' != *endptr)
		return FFFFM_INVALID_CHANNEL;
	if (result > UCHAR_MAX)
		return FFFFM_INVALID_CHANNEL;
	if (result < 0)
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
        if (ctx)
                uci_free_context(ctx);
	return ret;
error:
        if(ret)
                free(ret);
	ret = NULL;
	goto end;
}

void ffffm_free_wifi_info(struct ffffm_wifi_info *i) {
	free(i);
}

static int survey_airtime_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];

        struct airtime_result *result;

	static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
		[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
	};

        result = (struct airtime_result *) arg;

        if (result->frequency)
                return NL_SKIP;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_SURVEY_INFO])
                goto abort;

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX,
			     tb[NL80211_ATTR_SURVEY_INFO],
			     survey_policy))
                goto abort;

	if (!sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]) 
                goto abort;
	if (!sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY])
                goto abort;
	if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY])
                goto abort;

        result->frequency = (uint32_t)nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]),
        result->active_time = (uint64_t)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
        result->busy_time = (uint64_t)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);

abort:
        return NL_SKIP;
}

double ffffm_get_airtime(void) {
        double ret = FFFFM_INVALID_AIRTIME;
        int ctrl, ifx, flags;
        struct nl_sock *sk = NULL;
        struct nl_msg *msg = NULL;
        enum nl80211_commands cmd;
        struct airtime_result *result = NULL;

#define CHECK(x) { if (!(x)) { printf("error on line %d\n",  __LINE__); goto error; } }

        CHECK(sk = nl_socket_alloc());
        CHECK(genl_connect(sk) >= 0);

        CHECK(ctrl = genl_ctrl_resolve(sk, NL80211_GENL_NAME));
        CHECK(result = calloc(1, sizeof(*result)));
        CHECK(nl_socket_modify_cb(
                sk, NL_CB_VALID, NL_CB_CUSTOM, survey_airtime_handler, result) == 0);
        CHECK(msg = nlmsg_alloc());
        CHECK(ifx = if_nametoindex(airtime_interface));

        cmd = NL80211_CMD_GET_SURVEY;
        flags = 0;
        flags |= NLM_F_DUMP;

        /* TODO: check return? */
        genlmsg_put(msg, 0, 0, ctrl, 0, flags, cmd, 0);

        NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifx);

        CHECK(nl_send_auto_complete(sk, msg) >= 0);
        CHECK(nl_recvmsgs_default(sk) >= 0);

	ret = ((double) result->busy_time) / result->active_time;

#undef CHECK

out:
    if (msg)
            nlmsg_free(msg);
    msg = NULL;

    if (sk)
            nl_socket_free(sk);
    sk = NULL;

    if (result)
            free(result);
    result = NULL;

    return ret;

nla_put_failure:
error:
    ret = FFFFM_INVALID_AIRTIME;
    goto out;
}
