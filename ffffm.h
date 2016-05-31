#pragma once

extern const unsigned int FFFFM_INVALID_CHANNEL;
extern const double FFFFM_INVALID_AIRTIME;

struct ffffm_wifi_info {
	unsigned char c24;
	unsigned char c50;
	unsigned char t24;
	unsigned char t50;
};

char *ffffm_get_nexthop(void);
struct ffffm_wifi_info *ffffm_get_wifi_info(void);
double ffffm_get_airtime(void);
