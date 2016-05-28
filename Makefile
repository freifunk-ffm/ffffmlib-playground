CFLAGS=-fPIC -Os

all: ffffm ffffm.so ffffm-respondd.so ffffm-lua.so

ffffm.so: LDLIBS+=-luci
ffffm: LDLIBS+=-luci
ffffm-lua.so: CFLAGS+=$(shell pkg-config --cflags lua5.1)
ffffm-lua.so: LDLIBS+=$(shell pkg-config --libs lua5.1) -L.

%.so: %.c
	$(CC) -shared -fPIC $(CFLAGS) $(LDLIBS) $< -o $@
