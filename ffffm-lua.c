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
	lua_pushstring(L, "channel_24");
	lua_pushinteger(L, i->channel_24);
	lua_settable(L, -3);
	lua_pushstring(L, "channel_50");
	lua_pushinteger(L, i->channel_50);
	lua_settable(L, -3);

	return 0;
}

static const luaL_Reg ffffmlib[] = {
	{"get_nexthop", get_nexthop},
	{"get_wifi_info", get_wifi_info},
	{NULL, NULL},
};

LUALIB_API int luaopen_ffffm (lua_State *L) {
	luaL_register(L, "ffffm", ffffmlib);
	return 1;
}

LUALIB_API int luaopen_lua (lua_State *L) {
	return luaopen_ffffm(L);
}
