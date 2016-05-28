#include "ffffm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *gatewayfile = "/sys/kernel/debug/batman_adv/bat0/gateways";

// https://github.com/freifunk-gluon/gluon/blob/d2b74b4cf048ecb8706809021332ed3e7c72b2f3/package/gluon-mesh-batman-adv-core/src/respondd.c
char *ffffm_get_nexthop() {
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

int main() {
	char *nexthop = ffffm_get_nexthop();
	if (nexthop)
		printf("%s\n", nexthop);
	else
		printf("argh\n");
}
