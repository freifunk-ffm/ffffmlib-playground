#define _GNU_SOURCE

#include <stdlib.h>
#include <limits.h>

#include <uci.h>

#include "ffffm.h"

const unsigned int FFFFM_INVALID_CHANNEL = 0;

static const char *wifi_24_dev = "radio0";
static const char *wifi_50_dev = "radio1";

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

static inline unsigned char parse_txpower(const char *s) {
        return parse_channel(s);
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
	const char *t24 = lookup_option_value(ctx, p, wifi_24_dev, "txpower");
	const char *t50 = lookup_option_value(ctx, p, wifi_50_dev, "txpower");

	ret->c24 = parse_channel(c24);
	ret->c50 = parse_channel(c50);
	ret->t24 = parse_txpower(t24);
	ret->t50 = parse_txpower(t50);
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
