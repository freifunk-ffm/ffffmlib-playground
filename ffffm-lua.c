#include <stdlib.h>

#define LUA_LIB
#define LUA_COMPAT_MODULE

#include "lua.h"
#include "lauxlib.h"

#include "ffffm.h"

static int get_nexthop(lua_State *L) {
	char *n = ffffm_get_nexthop();

	if (!n)
		return 0;

	lua_pushstring(L, n);
	free(n);
	return 1;
}

static int get_wifi_info(lua_State *L) {
	struct ffffm_wifi_info *i = ffffm_get_wifi_info();

	if (!i)
		return 0;

	lua_newtable(L);

	if (i->c24) {
		lua_pushstring(L, "chan2");
		lua_pushinteger(L, i->c24);
		lua_settable(L,-3);
	}
	if (i->c50) {
		lua_pushstring(L, "chan5");
		lua_pushinteger(L, i->c50);
		lua_settable(L, -3);
	}

	return 1;
}

static int get_airtime(lua_State *L) {
	double airtime = ffffm_get_airtime();
	if (FFFFM_INVALID_AIRTIME == airtime)
		return 0;

	lua_pushnumber(L, airtime);
	return 1;
}

static const luaL_Reg ffffmlib[] = {
	{"get_nexthop", get_nexthop},
	{"get_wifi_info", get_wifi_info},
	{"get_airtime", get_airtime},
	{NULL, NULL},
};

LUALIB_API int luaopen_ffffm(lua_State *L) {
#if LUA_VERSION_NUM > 501
	luaL_newLib(L, ffffmlib);
#else
	luaL_register(L, "ffffm", ffffmlib);
#endif
	return 1;
}

LUALIB_API int luaopen_lua(lua_State *L) {
	return luaopen_ffffm(L);
}
