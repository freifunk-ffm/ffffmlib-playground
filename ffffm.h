#pragma once

extern const unsigned int FFFFM_INVALID_CHANNEL;
extern const double FFFFM_INVALID_AIRTIME;

struct ffffm_wifi_info {
	unsigned char channel_24;
	unsigned char channel_50;
};

char *ffffm_get_nexthop(void);
struct ffffm_wifi_info *ffffm_get_wifi_info(void);
void *ffffm_free_wifi_info(struct ffffm_wifi_info *i);
double ffffm_get_airtime(void);
