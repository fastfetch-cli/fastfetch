#if FF_HAVE_LUA

    #include "common/lua.h"
    #include "common/mallocHelper.h"

struct FFLuaData luaData;

static yyjson_mut_val* lua2yyjson(lua_State* L, int idx, yyjson_mut_doc* doc) {
    if (idx < 0) {
        idx = lua_gettop(L) + idx + 1;
    }
    int type = lua_type(L, idx);

    switch (type) {
        case LUA_TNIL:
            return yyjson_mut_null(doc);

        case LUA_TBOOLEAN:
            return yyjson_mut_bool(doc, lua_toboolean(L, idx));

        case LUA_TNUMBER: {
            if (lua_isinteger(L, idx)) {
                lua_Integer i = lua_tointeger(L, idx);
                return yyjson_mut_sint(doc, (int64_t) i);
            } else {
                return yyjson_mut_real(doc, lua_tonumber(L, idx));
            }
        }

        case LUA_TSTRING: {
            size_t len;
            const char* str = lua_tolstring(L, idx, &len);
            return yyjson_mut_strncpy(doc, str, len);
        }

        case LUA_TTABLE: {
            lua_Unsigned len = lua_rawlen(L, idx);
            int is_array = 1;

            if (len == 0) {
                lua_pushnil(L);
                is_array = 0;
                if (lua_next(L, idx) != 0) {
                    lua_pop(L, 2);
                }
            } else {
                lua_pushnil(L);
                while (lua_next(L, idx) != 0) {
                    if (lua_type(L, -2) != LUA_TNUMBER) {
                        is_array = 0;
                        lua_pop(L, 2);
                        break;
                    }
                    int isnum = false;
                    lua_Integer k = lua_tointegerx(L, -2, &isnum);
                    if (!isnum || (lua_Unsigned) k > len) {
                        is_array = 0;
                        lua_pop(L, 2);
                        break;
                    }
                    lua_pop(L, 1);
                }
            }

            if (is_array) {
                yyjson_mut_val* arr = yyjson_mut_arr(doc);
                for (lua_Unsigned i = 1; i <= len; i++) {
                    lua_rawgeti(L, idx, (lua_Integer) i);
                    yyjson_mut_val* val = lua2yyjson(L, -1, doc);
                    yyjson_mut_arr_append(arr, val);
                    lua_pop(L, 1);
                }
                return arr;
            } else {
                yyjson_mut_val* obj = yyjson_mut_obj(doc);
                lua_pushnil(L);
                while (lua_next(L, idx) != 0) {
                    size_t klen;
                    const char* key_str = luaL_tolstring(L, -2, &klen);
                    yyjson_mut_val* key = yyjson_mut_strncpy(doc, key_str, klen);
                    lua_pop(L, 1);

                    yyjson_mut_val* val = lua2yyjson(L, -1, doc);
                    yyjson_mut_obj_add(obj, key, val);
                    lua_pop(L, 1);
                }
                return obj;
            }
        }

        default:
            return yyjson_mut_null(doc);
    }
}

static int yyjsonEncode(lua_State* L) {
    luaL_checkany(L, 1);

    bool pretty = false;
    if (lua_isboolean(L, 2)) {
        pretty = (int) lua_toboolean(L, 2);
    }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    if (!doc) {
        return luaL_error(L, "failed to create yyjson document");
    }

    yyjson_mut_val* root = lua2yyjson(L, 1, doc);
    yyjson_mut_doc_set_root(doc, root);

    size_t jsonLen;
    yyjson_write_err err = {};
    FF_AUTO_FREE const char* jsonStr = yyjson_mut_write_opts(doc, YYJSON_WRITE_ALLOW_INF_AND_NAN | (pretty ? YYJSON_WRITE_PRETTY_TWO_SPACES : 0), NULL, &jsonLen, &err);

    if (jsonStr) {
        lua_pushlstring(L, jsonStr, jsonLen);
        yyjson_mut_doc_free(doc);
        return 1;
    } else {
        yyjson_mut_doc_free(doc);
        return luaL_error(L, "failed to encode JSON: %s", err.msg);
    }
}

const char* ffLuaLoadState() {
    if (luaData.inited) {
        if (luaData.L == NULL) {
            return "Lua library is not available";
        }
        return NULL;
    }

    luaData.inited = true;
    // clang-format off
    #ifdef _WIN32
        #define FF_LOAD_LIBLUA(version) FF_LIBRARY_LOAD_MESSAGE(liblua, \
            "lua5" #version FF_LIBRARY_EXTENSION, 0)
    #else
        #define FF_LOAD_LIBLUA(version) FF_LIBRARY_LOAD_MESSAGE(liblua, \
            "liblua5." #version FF_LIBRARY_EXTENSION, 0, \
            "liblua-5." #version FF_LIBRARY_EXTENSION, 0, \
            "liblua5." #version FF_LIBRARY_EXTENSION ".5." #version, 0)
    #endif
    // clang-format on
    #if LUA_VERSION_NUM == 505
    FF_LOAD_LIBLUA(5)
    #elif LUA_VERSION_NUM == 504
    FF_LOAD_LIBLUA(4)
    #elif LUA_VERSION_NUM == 503
    FF_LOAD_LIBLUA(3)
    #else
        #error "Unsupported Lua version"
    #endif
    #undef FF_LOAD_LIBLUA
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaL_newstate)
    #if LUA_VERSION_NUM >= 505
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaL_openselectedlibs)
    #else
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaL_requiref)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaopen_base)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaopen_math)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaopen_string)
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(liblua, luaopen_table)
    #endif
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_settop)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, luaL_loadbufferx)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_tolstring)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_createtable)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushinteger)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushnumber)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushboolean)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushstring)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushlstring)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushvalue)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_seti)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushnil)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_setfield)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pcallk)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_gettop)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, luaL_tolstring)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, luaL_error)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushcclosure)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, luaL_checkany)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_callk)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_isinteger)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_next)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_rawgeti)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_rawlen)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_setglobal)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_toboolean)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_tointegerx)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_tonumberx)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_type)

    lua_State* L = ffluaL_newstate();
    if (L == NULL) {
        return "luaL_newstate() failed";
    }
    #if LUA_VERSION_NUM >= 505
    ffluaL_openselectedlibs(L, LUA_GLIBK | LUA_MATHLIBK | LUA_STRLIBK | LUA_TABLIBK, 0);
    #else
    ffluaL_requiref(L, LUA_GNAME, ffluaopen_base, 1);
    ffluaL_requiref(L, LUA_MATHLIBNAME, ffluaopen_math, 1);
    ffluaL_requiref(L, LUA_STRLIBNAME, ffluaopen_string, 1);
    ffluaL_requiref(L, LUA_TABLIBNAME, ffluaopen_table, 1);
    lua_settop(L, 0);
    #endif
    lua_pushcfunction(L, yyjsonEncode);
    lua_setglobal(L, "json_encode");
    luaData.L = L;
    liblua = NULL; // don't close lua
    return NULL;
}

#endif
