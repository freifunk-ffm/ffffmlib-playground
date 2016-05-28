CFLAGS=-fPIC

all: ffffm ffffm.so ffffm-respondd.so

%.so: %.c
	$(CC) -shared $(CFLAGS) $< -o $@
