#define _GNU_SOURCE

#include <sys/socket.h>
#include <linux/nl80211.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>

#include "ffffm.h"

const double FFFFM_INVALID_AIRTIME = -1;

static const char *airtime_interface = "client0";

struct airtime_result {
        uint32_t frequency;
        uint64_t active_time;
        uint64_t busy_time;
};

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
