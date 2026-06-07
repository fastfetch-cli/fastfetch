#pragma once

#include "fastfetch.h"

#if FF_HAVE_LUA
    #if !FF_DISABLE_DLOPEN
        // Hack. LUA_API is defined as extern which prevents us from implementing the functions ourselves.
        #include <luaconf.h>
        #undef LUA_API
        #undef LUALIB_API
        #undef LUAMOD_API
        #define LUA_API static inline
        #define LUALIB_API LUA_API
        #define LUAMOD_API LUA_API

        #pragma GCC diagnostic ignored "-Wunused-function"
    #endif

    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>

    #ifndef LUA_GNAME
        #define LUA_GNAME "_G"
    #endif

    #include "common/library.h"

extern struct FFLuaData {
    #if !FF_DISABLE_DLOPEN
    FF_LIBRARY_SYMBOL(luaL_checkany)
    FF_LIBRARY_SYMBOL(luaL_loadbufferx)
    FF_LIBRARY_SYMBOL(luaL_tolstring)
    FF_LIBRARY_SYMBOL(lua_callk)
    FF_LIBRARY_SYMBOL(lua_createtable)
    FF_LIBRARY_SYMBOL(lua_error)
    FF_LIBRARY_SYMBOL(lua_gettop)
    FF_LIBRARY_SYMBOL(lua_isinteger)
    FF_LIBRARY_SYMBOL(lua_next)
    FF_LIBRARY_SYMBOL(lua_pcallk)
    FF_LIBRARY_SYMBOL(lua_pushboolean)
    FF_LIBRARY_SYMBOL(lua_pushcclosure)
    FF_LIBRARY_SYMBOL(lua_pushinteger)
    FF_LIBRARY_SYMBOL(lua_pushlstring)
    FF_LIBRARY_SYMBOL(lua_pushnil)
    FF_LIBRARY_SYMBOL(lua_pushnumber)
    FF_LIBRARY_SYMBOL(lua_pushvalue)
    FF_LIBRARY_SYMBOL(lua_rawgeti)
    FF_LIBRARY_SYMBOL(lua_rawlen)
    FF_LIBRARY_SYMBOL(lua_setfield)
    FF_LIBRARY_SYMBOL(lua_setglobal)
    FF_LIBRARY_SYMBOL(lua_seti)
    FF_LIBRARY_SYMBOL(lua_settop)
    FF_LIBRARY_SYMBOL(lua_toboolean)
    FF_LIBRARY_SYMBOL(lua_tointegerx)
    FF_LIBRARY_SYMBOL(lua_tolstring)
    FF_LIBRARY_SYMBOL(lua_tonumberx)
    FF_LIBRARY_SYMBOL(lua_type)
    #endif

    lua_State* L;
    bool inited;
} luaData;

    #if !FF_DISABLE_DLOPEN
FF_A_ALWAYS_INLINE void(lua_settop)(lua_State* L, int idx) {
    return luaData.fflua_settop(L, idx);
}

FF_A_ALWAYS_INLINE int(luaL_loadbufferx)(lua_State* L, const char* buff, size_t sz, const char* name, const char* mode) {
    return luaData.ffluaL_loadbufferx(L, buff, sz, name, mode);
}

FF_A_ALWAYS_INLINE const char*(lua_tolstring) (lua_State * L, int idx, size_t* len) {
    return luaData.fflua_tolstring(L, idx, len);
}

FF_A_ALWAYS_INLINE void(lua_createtable)(lua_State* L, int narr, int nrec) {
    return luaData.fflua_createtable(L, narr, nrec);
}

FF_A_ALWAYS_INLINE void(lua_pushinteger)(lua_State* L, lua_Integer n) {
    return luaData.fflua_pushinteger(L, n);
}

FF_A_ALWAYS_INLINE void(lua_pushnumber)(lua_State* L, lua_Number n) {
    return luaData.fflua_pushnumber(L, n);
}

FF_A_ALWAYS_INLINE void(lua_pushboolean)(lua_State* L, int b) {
    return luaData.fflua_pushboolean(L, b);
}

FF_A_ALWAYS_INLINE const char*(lua_pushlstring) (lua_State * L, const char* s, size_t len) {
    return luaData.fflua_pushlstring(L, s, len);
}

FF_A_ALWAYS_INLINE void(lua_pushvalue)(lua_State* L, int idx) {
    return luaData.fflua_pushvalue(L, idx);
}

FF_A_ALWAYS_INLINE void(lua_seti)(lua_State* L, int idx, lua_Integer n) {
    return luaData.fflua_seti(L, idx, n);
}

FF_A_ALWAYS_INLINE void(lua_pushnil)(lua_State* L) {
    return luaData.fflua_pushnil(L);
}

FF_A_ALWAYS_INLINE void(lua_setfield)(lua_State* L, int idx, const char* k) {
    return luaData.fflua_setfield(L, idx, k);
}

FF_A_ALWAYS_INLINE int(lua_pcallk)(lua_State* L, int nargs, int nresults, int errfunc, lua_KContext ctx, lua_KFunction k) {
    return luaData.fflua_pcallk(L, nargs, nresults, errfunc, ctx, k);
}

FF_A_ALWAYS_INLINE int(lua_gettop)(lua_State* L) {
    return luaData.fflua_gettop(L);
}

FF_A_ALWAYS_INLINE const char*(luaL_tolstring) (lua_State * L, int idx, size_t* len) {
    return luaData.ffluaL_tolstring(L, idx, len);
}

FF_A_ALWAYS_INLINE int(lua_error)(lua_State* L) {
    return luaData.fflua_error(L);
}

FF_A_ALWAYS_INLINE void(lua_pushcclosure)(lua_State* L, lua_CFunction fn, int n) {
    return luaData.fflua_pushcclosure(L, fn, n);
}

FF_A_ALWAYS_INLINE void(luaL_checkany)(lua_State* L, int idx) {
    return luaData.ffluaL_checkany(L, idx);
}

FF_A_ALWAYS_INLINE void(lua_callk)(lua_State* L, int nargs, int nresults, lua_KContext ctx, lua_KFunction k) {
    return luaData.fflua_callk(L, nargs, nresults, ctx, k);
}

FF_A_ALWAYS_INLINE int(lua_isinteger)(lua_State* L, int idx) {
    return luaData.fflua_isinteger(L, idx);
}

FF_A_ALWAYS_INLINE int(lua_next)(lua_State* L, int idx) {
    return luaData.fflua_next(L, idx);
}

FF_A_ALWAYS_INLINE int(lua_rawgeti)(lua_State* L, int idx, lua_Integer n) {
    return luaData.fflua_rawgeti(L, idx, n);
}

FF_A_ALWAYS_INLINE
        #if LUA_VERSION_NUM > 503
lua_Unsigned
        #else
size_t
        #endif
    (lua_rawlen)(lua_State* L, int idx) {
    return luaData.fflua_rawlen(L, idx);
}

FF_A_ALWAYS_INLINE void(lua_setglobal)(lua_State* L, const char* name) {
    return luaData.fflua_setglobal(L, name);
}

FF_A_ALWAYS_INLINE int(lua_toboolean)(lua_State* L, int idx) {
    return luaData.fflua_toboolean(L, idx);
}

FF_A_ALWAYS_INLINE lua_Integer(lua_tointegerx)(lua_State* L, int idx, int* isnum) {
    return luaData.fflua_tointegerx(L, idx, isnum);
}

FF_A_ALWAYS_INLINE lua_Number(lua_tonumberx)(lua_State* L, int idx, int* isnum) {
    return luaData.fflua_tonumberx(L, idx, isnum);
}

FF_A_ALWAYS_INLINE int(lua_type)(lua_State* L, int idx) {
    return luaData.fflua_type(L, idx);
}
    #endif

const char* ffLuaLoadState(void);

#endif
