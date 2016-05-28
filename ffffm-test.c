#include <stdio.h>

#include "ffffm.h"

int main() {
	char *nexthop = ffffm_get_nexthop();
	if (nexthop)
		printf("%s\n", nexthop);
	else
		printf("argh\n");
	double airtime = ffffm_get_airtime();
	printf("Airtime: %f\n", airtime);
}
