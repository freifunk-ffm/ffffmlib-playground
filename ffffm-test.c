#include <stdio.h>

#include "ffffm.h"

int main() {
	struct ffffm_wifi_info *i = ffffm_get_wifi_info();
	printf("24: %u, 50: %u\n", i->channel_24, i->channel_50);
}
