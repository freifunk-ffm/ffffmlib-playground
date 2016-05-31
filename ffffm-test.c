#include <stdio.h>

#include "ffffm.h"

int main() {
	double airtime = ffffm_get_airtime();
	printf("Airtime: %f\n", airtime);
}
