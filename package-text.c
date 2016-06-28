#include <stdio.h>

#include "package.h"

int main() {
	struct wifi_info *i = get_wifi_info();
	printf("24: %u (%u), 50: %u (%u)\n", i->c24, i->t24, i->c50, i->t50);
	struct airtime *a = get_airtime();
	printf("24: %f 50: %f\n", a->a24, a->a50);
}
