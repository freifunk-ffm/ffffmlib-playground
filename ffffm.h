#pragma once

extern const unsigned int FFFFM_INVALID_CHANNEL;

struct ffffm_wifi_info {
	unsigned char channel_24;
	unsigned char channel_50;
};

char *ffffm_get_nexthop(void);
struct ffffm_wifi_info *ffffm_get_wifi_info(void);
void *ffffm_free_wifi_info(struct ffffm_wifi_info *i);
