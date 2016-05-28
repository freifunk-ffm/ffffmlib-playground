CFLAGS=-fPIC -Os
INSTALL=install -D

all: ffffm ffffm.so ffffm-respondd.so ffffm-lua.so

ffffm.so: LDLIBS+=-luci
ffffm: LDLIBS+=-luci
ffffm-lua.so: CFLAGS+=$(shell pkg-config --cflags lua5.1)
ffffm-lua.so: LDLIBS+=$(shell pkg-config --libs lua5.1) -L. ffffm.so

%.so: %.c
	$(CC) -shared -fPIC $(CFLAGS) $(LDLIBS) $< -o $@

install: ffffm.so ffffm-lua.so ffffm-respondd.so
	$(INSTALL) ffffm.so $(PREFIX)/lib/ffffm.so
	$(INSTALL) ffffm-lua.so $(PREFIX)/usr/lib/lua/ffffm.so
	$(INSTALL) ffffm-respondd.so $(PREFIX)/lib/gluon/respondd/ffffm.so
