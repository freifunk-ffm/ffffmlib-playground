#pragma once

extern const unsigned int INVALID_CHANNEL;
extern const unsigned int INVALID_TXPOWER;
extern const double INVALID_AIRTIME;

struct wifi_info {
	unsigned char c24;
	unsigned char c50;
	unsigned char t24;
	unsigned char t50;
};

struct airtime {
	double a24;
	double a50;
};

struct wifi_info *get_wifi_info(void);
struct airtime *get_airtime(void);

#define DEBUG { printf("Reached %s:%d\n", __FILE__, __LINE__); fflush(NULL); }
