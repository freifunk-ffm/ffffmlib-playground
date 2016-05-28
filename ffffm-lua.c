#include <stdlib.h>

#define LUA_LIB

#include "lua.h"
#include "lauxlib.h"

#include "ffffm.h"

static int get_nexthop(lua_State *L) {
	char *n = ffffm_get_nexthop();

	if (n) {
		lua_pushstring(L, n);
		free(n);
		return 1;
	} else {
		return 0;
	}
}

static const luaL_Reg ffffmlib[] = {
	{"get_nexthop", get_nexthop},
	{NULL, NULL},
};

LUALIB_API int luaopen_ffffm (lua_State *L) {
	luaL_register(L, "ffffm", ffffmlib);
	return 1;
}

LUALIB_API int luaopen_lua (lua_State *L) {
	return luaopen_ffffm(L);
}
