#define LUA_LIB

#include "lua.h"
#include "lauxlib.h"

static const luaL_Reg ffffmlib[] = {
	{NULL, NULL}
};

LUALIB_API int luaopen_ffffm (lua_State *L) {
	luaL_register(L, "ffffm", ffffmlib);
	lua_pushnumber(L, 15);
	lua_setfield(L, -2, "test");
	return 1;
}

LUALIB_API int luaopen_lua (lua_State *L) {
	return luaopen_ffffm(L);
}
