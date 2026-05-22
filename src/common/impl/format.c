#include "fastfetch.h"
#include "common/format.h"
#include "common/parsing.h"
#include "common/textModifier.h"
#include "common/stringUtils.h"
#include "common/library.h"

#include <inttypes.h>

void ffFormatAppendFormatArg(FFstrbuf* buffer, const FFformatarg* formatarg) {
    switch (formatarg->type) {
        case FF_ARG_TYPE_INT:
            ffStrbufAppendSInt(buffer, *(int32_t*) formatarg->value);
            break;
        case FF_ARG_TYPE_UINT:
            ffStrbufAppendUInt(buffer, *(uint32_t*) formatarg->value);
            break;
        case FF_ARG_TYPE_UINT64:
            ffStrbufAppendUInt(buffer, *(uint64_t*) formatarg->value);
            break;
        case FF_ARG_TYPE_UINT16:
            ffStrbufAppendUInt(buffer, *(uint16_t*) formatarg->value);
            break;
        case FF_ARG_TYPE_UINT8:
            ffStrbufAppendUInt(buffer, *(uint8_t*) formatarg->value);
            break;
        case FF_ARG_TYPE_STRING:
            ffStrbufAppendS(buffer, (const char*) formatarg->value);
            break;
        case FF_ARG_TYPE_STRBUF:
            ffStrbufAppend(buffer, (const FFstrbuf*) formatarg->value);
            break;
        case FF_ARG_TYPE_FLOAT:
            ffStrbufAppendDouble(buffer, *(float*) formatarg->value, instance.config.display.fractionNdigits, instance.config.display.fractionTrailingZeros != FF_FRACTION_TRAILING_ZEROS_TYPE_NEVER);
            break;
        case FF_ARG_TYPE_DOUBLE:
            ffStrbufAppendDouble(buffer, *(double*) formatarg->value, instance.config.display.fractionNdigits, instance.config.display.fractionTrailingZeros != FF_FRACTION_TRAILING_ZEROS_TYPE_NEVER);
            break;
        case FF_ARG_TYPE_BOOL:
            ffStrbufAppendS(buffer, *(bool*) formatarg->value ? "true" : "false");
            break;
        case FF_ARG_TYPE_LIST: {
            const FFlist* list = (const FFlist*) formatarg->value;
            for (uint32_t i = 0; i < list->length; i++) {
                ffStrbufAppend(buffer, FF_LIST_GET(FFstrbuf, *list, i));
                if (i < list->length - 1) {
                    ffStrbufAppendS(buffer, ", ");
                }
            }
            break;
        }
        case FF_ARG_TYPE_BUFFER: {
            // Placeholder for binary data, just print the size for now
            const FFArgBuffer* argBuffer = (const FFArgBuffer*) formatarg->value;
            ffStrbufAppendF(buffer, "buffer(%u bytes)", argBuffer->length);
            break;
        }
        default:
            if (formatarg->type != FF_ARG_TYPE_NULL) {
                fprintf(stderr, "Error: format string \"%s\": argument is not implemented: %i\n", buffer->chars, formatarg->type);
            }
            break;
    }
}

/**
 * @brief parses a string to a uint32_t
 *
 * If the string can't be parsed, or is < 1, uint32_t max is returned.
 *
 * @param placeholderValue the string to parse
 * @return uint32_t the parsed value
 */
static uint32_t getArgumentIndex(const char* placeholderValue, uint32_t numArgs, const FFformatarg* arguments) {
    char firstChar = placeholderValue[0];
    if (firstChar == '\0') {
        return 0; // use arg counter
    }

    if (firstChar >= '0' && firstChar <= '9') {
        char* pEnd = NULL;
        uint32_t result = (uint32_t) strtoul(placeholderValue, &pEnd, 10);
        if (result > numArgs) {
            return UINT32_MAX;
        }
        if (*pEnd != '\0') {
            return UINT32_MAX;
        }
        return result;
    } else if (ffCharIsEnglishAlphabet(firstChar)) {
        for (uint32_t i = 0; i < numArgs; ++i) {
            const FFformatarg* arg = &arguments[i];
            if (arg->name && ffStrEqualsIgnCase(placeholderValue, arg->name)) {
                return i + 1;
            }
        }
    }

    return UINT32_MAX;
}

static inline void appendInvalidPlaceholder(FFstrbuf* buffer, const char* start, const FFstrbuf* placeholderValue, uint32_t index, uint32_t formatStringLength) {
    ffStrbufAppendS(buffer, start);
    ffStrbufAppend(buffer, placeholderValue);

    if (index < formatStringLength) {
        ffStrbufAppendC(buffer, '}');
    }
}

static inline bool formatArgSet(const FFformatarg* arg) {
    return arg->value != NULL && ((arg->type == FF_ARG_TYPE_DOUBLE && *(double*) arg->value > 0.0) || (arg->type == FF_ARG_TYPE_FLOAT && *(float*) arg->value > 0.0) || (arg->type == FF_ARG_TYPE_INT && *(int32_t*) arg->value > 0) || (arg->type == FF_ARG_TYPE_STRBUF && ((FFstrbuf*) arg->value)->length > 0) || (arg->type == FF_ARG_TYPE_STRING && ffStrSet((char*) arg->value)) || (arg->type == FF_ARG_TYPE_UINT8 && *(uint8_t*) arg->value > 0) || (arg->type == FF_ARG_TYPE_UINT16 && *(uint16_t*) arg->value > 0) || (arg->type == FF_ARG_TYPE_UINT && *(uint32_t*) arg->value > 0) || (arg->type == FF_ARG_TYPE_UINT64 && *(uint64_t*) arg->value > 0) || (arg->type == FF_ARG_TYPE_BOOL && *(bool*) arg->value) || (arg->type == FF_ARG_TYPE_LIST && ((FFlist*) arg->value)->length > 0));
}

FF_A_UNUSED static inline void normalizeArgName(FFstrbuf* dst, const char* src) {
    ffStrbufClear(dst);
    bool flag = false;
    for (const char* p = src; *p; ++p) {
        if (*p == '-') {
            flag = true;
        } else if (flag) {
            ffStrbufAppendC(dst, (char) toupper((unsigned char) *p));
            flag = false;
        } else {
            ffStrbufAppendC(dst, *p);
        }
    }
}

#if FF_HAVE_LUA
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>

    #ifndef LUA_GNAME
        #define LUA_GNAME "_G"
    #endif

struct FFLuaData {
    FF_LIBRARY_SYMBOL(lua_settop)
    FF_LIBRARY_SYMBOL(luaL_newstate)
    #if LUA_VERSION_NUM >= 505
    FF_LIBRARY_SYMBOL(luaL_openselectedlibs)
    #else
    FF_LIBRARY_SYMBOL(luaL_openlibs)
    #endif
    FF_LIBRARY_SYMBOL(luaL_loadbufferx)
    FF_LIBRARY_SYMBOL(lua_tolstring)
    FF_LIBRARY_SYMBOL(lua_createtable)
    FF_LIBRARY_SYMBOL(lua_pushinteger)
    FF_LIBRARY_SYMBOL(lua_pushnumber)
    FF_LIBRARY_SYMBOL(lua_pushboolean)
    FF_LIBRARY_SYMBOL(lua_pushstring)
    FF_LIBRARY_SYMBOL(lua_pushlstring)
    FF_LIBRARY_SYMBOL(lua_seti)
    FF_LIBRARY_SYMBOL(lua_pushnil)
    FF_LIBRARY_SYMBOL(lua_setfield)
    FF_LIBRARY_SYMBOL(lua_pcallk)
    FF_LIBRARY_SYMBOL(lua_gettop)
    FF_LIBRARY_SYMBOL(luaL_tolstring)

    lua_State* L;
    bool inited;
} luaData;

static const char* loadLuaState() {
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
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_seti)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pushnil)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_setfield)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_pcallk)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, lua_gettop)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(liblua, luaData, luaL_tolstring)

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
    luaData.fflua_settop(L, 0);
    #endif
    luaData.L = L;
    liblua = NULL; // don't close lua
    return NULL;
}

static void appendLuaError(FFstrbuf* buffer, const char* prefix, lua_State* L) {
    const char* err = luaData.fflua_tolstring(L, -1, NULL);
    if (err) {
        const char* tmp = strchr(err, ':');
        if (tmp) {
            err = tmp + 1;
            while (*err == ' ') {
                ++err;
            }
        }
    }
    ffStrbufAppendF(buffer, "%s: %s", prefix, err ? err : "unknown");
}

static bool parseLuaString(FFstrbuf* buffer, const char* script, uint32_t scriptLen, uint32_t numArgs, const FFformatarg* arguments) {
    const char* err = loadLuaState();
    if (err) {
        ffStrbufAppendF(buffer, "Lua init error: %s", err);
        return false;
    }

    FF_STRBUF_AUTO_DESTROY argNameBuf = ffStrbufCreate();
    bool ret = false;

    lua_State* L = luaData.L;
    // Clear stack and load chunk
    luaData.fflua_settop(L, 0);
    if (luaData.ffluaL_loadbufferx(L, script, scriptLen, "", NULL) != LUA_OK) {
        appendLuaError(buffer, "Lua load error", L);
    } else {
        // Build args table for name lookup only.
        luaData.fflua_createtable(L, 0, 0);

        for (uint32_t i = 0; i < numArgs; ++i) {
            const FFformatarg* arg = &arguments[i];
            switch (arg->type) {
                case FF_ARG_TYPE_INT:
                    luaData.fflua_pushinteger(L, (lua_Integer) * (int32_t*) arg->value);
                    break;
                case FF_ARG_TYPE_UINT:
                    luaData.fflua_pushinteger(L, (lua_Integer) * (uint32_t*) arg->value);
                    break;
                case FF_ARG_TYPE_UINT64:
                    luaData.fflua_pushinteger(L, (lua_Integer) * (uint64_t*) arg->value);
                    break;
                case FF_ARG_TYPE_UINT16:
                    luaData.fflua_pushinteger(L, (lua_Integer) * (uint16_t*) arg->value);
                    break;
                case FF_ARG_TYPE_UINT8:
                    luaData.fflua_pushinteger(L, (lua_Integer) * (uint8_t*) arg->value);
                    break;
                case FF_ARG_TYPE_FLOAT:
                    luaData.fflua_pushnumber(L, (lua_Number) * (float*) arg->value);
                    break;
                case FF_ARG_TYPE_DOUBLE:
                    luaData.fflua_pushnumber(L, (lua_Number) * (double*) arg->value);
                    break;
                case FF_ARG_TYPE_BOOL:
                    luaData.fflua_pushboolean(L, *(bool*) arg->value);
                    break;
                case FF_ARG_TYPE_STRING:
                    luaData.fflua_pushstring(L, (const char*) arg->value);
                    break;
                case FF_ARG_TYPE_STRBUF: {
                    const FFstrbuf* sb = (const FFstrbuf*) arg->value;
                    luaData.fflua_pushlstring(L, sb->chars, sb->length);
                    break;
                }
                case FF_ARG_TYPE_LIST: {
                    const FFlist* list = (const FFlist*) arg->value;
                    luaData.fflua_createtable(L, 0, 0);
                    for (uint32_t li = 0; li < list->length; ++li) {
                        const FFstrbuf* item = FF_LIST_GET(FFstrbuf, *list, li);
                        luaData.fflua_pushlstring(L, item->chars, item->length);
                        luaData.fflua_seti(L, -2, (lua_Integer) (li + 1));
                    }
                    break;
                }
                default:
                    luaData.fflua_pushnil(L);
                    break;
            }
            if (arg->name && arg->name[0]) {
                normalizeArgName(&argNameBuf, arg->name);
            } else {
                ffStrbufSetF(&argNameBuf, "arg%" PRIu32, i + 1);
            }
            luaData.fflua_setfield(L, -2, argNameBuf.chars);
        }

        if (luaData.fflua_pcallk(L, 1, LUA_MULTRET, 0, 0, NULL) != LUA_OK) {
            appendLuaError(buffer, "Lua runtime error", L);
        } else {
            int nresults = luaData.fflua_gettop(L);
            if (nresults == 0) {
                ffStrbufAppendS(buffer, "Lua result error: no result");
            } else {
                // Convert first result to string
                const char* res = luaData.fflua_tolstring(L, 1, NULL);
                if (res) {
                    ffStrbufAppendS(buffer, res);
                } else {
                    // Fallback: use luaL_tolstring to get a reasonable representation
                    luaData.ffluaL_tolstring(L, 1, NULL);
                    const char* sval = luaData.fflua_tolstring(L, -1, NULL);
                    if (sval) {
                        ffStrbufAppendS(buffer, sval);
                    }
                }
                ret = true;
            }
        }
    }
    luaData.fflua_settop(L, 0);
    return ret;
}
#endif

#if FF_HAVE_QUICKJS
    #include <quickjs.h>

struct FFQuickJSData {
    FF_LIBRARY_SYMBOL(JS_NewRuntime)
    FF_LIBRARY_SYMBOL(JS_NewContext)
    FF_LIBRARY_SYMBOL(JS_FreeRuntime)
    FF_LIBRARY_SYMBOL(JS_EvalThis)
    FF_LIBRARY_SYMBOL(JS_GetException)
    FF_LIBRARY_SYMBOL(JS_ToCStringLen2)
    FF_LIBRARY_SYMBOL(JS_FreeCString)
    FF_LIBRARY_SYMBOL(JS_NewStringLen)
    FF_LIBRARY_SYMBOL(JS_NewArray)
    FF_LIBRARY_SYMBOL(JS_NewBigUint64)
    FF_LIBRARY_SYMBOL(JS_SetPropertyUint32)
    FF_LIBRARY_SYMBOL(JS_NewObject)
    FF_LIBRARY_SYMBOL(JS_SetPropertyStr)
    FF_LIBRARY_SYMBOL(JS_FreeValue)

    JSRuntime* rt;
    JSContext* ctx;
    bool inited;
} qjsData;

static const char* loadQuickJSState(void) {
    if (qjsData.inited) {
        if (qjsData.ctx == NULL) {
            return "QuickJS is not available";
        }
        return NULL;
    }

    qjsData.inited = true;
    #ifdef _WIN32
    FF_LIBRARY_LOAD_MESSAGE(libqjs, "libqjs-0" FF_LIBRARY_EXTENSION, 0)
    #else
    FF_LIBRARY_LOAD_MESSAGE(libqjs, "libqjs" FF_LIBRARY_EXTENSION, 0)
    #endif

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_NewRuntime)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_NewContext)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_FreeRuntime)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_EvalThis)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_GetException)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_ToCStringLen2)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_FreeCString)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_NewStringLen)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_NewArray)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_SetPropertyUint32)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_NewObject)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_SetPropertyStr)
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libqjs, qjsData, JS_FreeValue)

    qjsData.rt = qjsData.ffJS_NewRuntime();
    if (qjsData.rt == NULL) {
        return "JS_NewRuntime() failed";
    }

    qjsData.ctx = qjsData.ffJS_NewContext(qjsData.rt);
    if (qjsData.ctx == NULL) {
        qjsData.ffJS_FreeRuntime(qjsData.rt);
        qjsData.rt = NULL;
        return "JS_NewContext() failed";
    }

    libqjs = NULL; // don't close quickjs

    return NULL;
}

static bool parseQuickJSString(FFstrbuf* buffer, const char* script, uint32_t scriptLen, uint32_t numArgs, const FFformatarg* arguments) {
    const char* err = loadQuickJSState();
    if (err) {
        ffStrbufAppendF(buffer, "Qjs init error: %s", err);
        return false;
    }
    JSContext* ctx = qjsData.ctx;
    JSValue argsObj = qjsData.ffJS_NewObject(ctx);
    FF_STRBUF_AUTO_DESTROY argNameBuf = ffStrbufCreate();

    for (uint32_t i = 0; i < numArgs; ++i) {
        const FFformatarg* arg = &arguments[i];

        JSValue value;
        switch (arg->type) {
            case FF_ARG_TYPE_INT:
                value = JS_NewInt32(ctx, *(int32_t*) arg->value);
                break;
            case FF_ARG_TYPE_UINT:
                value = JS_NewUint32(ctx, *(uint32_t*) arg->value);
                break;
            case FF_ARG_TYPE_UINT64: {
                uint64_t val = *(uint64_t*) arg->value;
                if (val <= INT32_MAX) {
                    value = JS_NewInt32(ctx, (int32_t) val);
                } else {
                    value = JS_NewFloat64(ctx, (double) val);
                }
                break;
            }
            case FF_ARG_TYPE_UINT16:
                value = JS_NewUint32(ctx, *(uint16_t*) arg->value);
                break;
            case FF_ARG_TYPE_UINT8:
                value = JS_NewUint32(ctx, *(uint8_t*) arg->value);
                break;
            case FF_ARG_TYPE_FLOAT:
                value = JS_NewFloat64(ctx, *(float*) arg->value);
                break;
            case FF_ARG_TYPE_DOUBLE:
                value = JS_NewFloat64(ctx, *(double*) arg->value);
                break;
            case FF_ARG_TYPE_BOOL:
                value = JS_NewBool(ctx, *(bool*) arg->value);
                break;
            case FF_ARG_TYPE_STRING:
                value = qjsData.ffJS_NewStringLen(ctx, (const char*) arg->value, strlen((const char*) arg->value));
                break;
            case FF_ARG_TYPE_STRBUF: {
                const FFstrbuf* sb = (const FFstrbuf*) arg->value;
                value = qjsData.ffJS_NewStringLen(ctx, sb->chars, sb->length);
                break;
            }
            case FF_ARG_TYPE_LIST: {
                const FFlist* list = (const FFlist*) arg->value;
                JSValue arr = qjsData.ffJS_NewArray(ctx);
                for (uint32_t li = 0; li < list->length; ++li) {
                    const FFstrbuf* item = FF_LIST_GET(FFstrbuf, *list, li);
                    JSValue itemValue = qjsData.ffJS_NewStringLen(ctx, item->chars, item->length);
                    qjsData.ffJS_SetPropertyUint32(ctx, arr, li, itemValue);
                }

                value = arr;
                break;
            }
            default:
                value = JS_UNDEFINED;
                break;
        }

        if (arg->name && arg->name[0]) {
            normalizeArgName(&argNameBuf, arg->name);
        } else {
            ffStrbufSetF(&argNameBuf, "arg%" PRIu32, i + 1);
        }
        qjsData.ffJS_SetPropertyStr(ctx, argsObj, argNameBuf.chars, value);
    }
    JSValue result = qjsData.ffJS_EvalThis(ctx, argsObj, script, scriptLen, "", JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_STRICT);

    qjsData.ffJS_FreeValue(ctx, argsObj);

    bool ret = false;
    if (JS_IsException(result)) {
        JSValue exc = qjsData.ffJS_GetException(ctx);
        const char* message = qjsData.ffJS_ToCStringLen2(ctx, NULL, exc, false);
        qjsData.ffJS_FreeValue(ctx, exc);
        ffStrbufAppendF(buffer, "Qjs runtime error: %s", message ?: "unknown");
        if (message) {
            qjsData.ffJS_FreeCString(ctx, message);
        }
    } else if (JS_IsUndefined(result)) {
        ffStrbufAppendS(buffer, "Qjs result error: undefined result");
    } else {
        size_t len;
        const char* res = qjsData.ffJS_ToCStringLen2(ctx, &len, result, false);
        if (res) {
            ffStrbufAppendNS(buffer, (uint32_t) len, res);
            qjsData.ffJS_FreeCString(ctx, res);
        }
        ret = true;
    }

    qjsData.ffJS_FreeValue(ctx, result);
    return ret;
}
#endif

bool ffParseFormatString(FFstrbuf* buffer, const FFstrbuf* formatstr, uint32_t numArgs, const FFformatarg* arguments) {
#if FF_HAVE_QUICKJS
    if (ffStrbufStartsWithS(formatstr, "qjs:")) {
        return parseQuickJSString(buffer, formatstr->chars + 4, formatstr->length - 4, numArgs, arguments);
    }
#endif

#if FF_HAVE_LUA
    if (ffStrbufStartsWithS(formatstr, "lua:")) {
        // If outputFormat starts with "lua:", treat the rest as a Lua script
        return parseLuaString(buffer, formatstr->chars + 4, formatstr->length - 4, numArgs, arguments);
    }
#endif

    uint32_t argCounter = 0;

    uint32_t numOpenIfs = 0;
    uint32_t numOpenNotIfs = 0;

    FF_STRBUF_AUTO_DESTROY placeholderValue = ffStrbufCreate();

    for (uint32_t i = 0; i < formatstr->length; ++i) {
        // if we don't have a placeholder start just copy the chars over to output buffer
        if (formatstr->chars[i] != '{') {
            ffStrbufAppendC(buffer, formatstr->chars[i]);
            continue;
        }

        // jump to next char, the start of the placeholder value
        ++i;

        // unmatched trailing '{'
        if (i >= formatstr->length) {
            ffStrbufAppendC(buffer, '{');
            break;
        }

        // double {{ elvaluates to a single { and doesn't count as start
        if (formatstr->chars[i] == '{') {
            ffStrbufAppendC(buffer, '{');
            continue;
        }

        ffStrbufClear(&placeholderValue);

        {
            uint32_t iEnd = ffStrbufNextIndexC(formatstr, i, '}');
            ffStrbufAppendNS(&placeholderValue, iEnd - i, &formatstr->chars[i]);
            i = iEnd;
        }

        char firstChar = placeholderValue.chars[0];

        if (placeholderValue.length == 1) {
            // test if for stop, if so break the loop
            if (firstChar == '-') {
                break;
            }

            // test for end of an if, if so do nothing
            if (firstChar == '?') {
                if (numOpenIfs == 0) {
                    appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                } else {
                    --numOpenIfs;
                }

                continue;
            }

            // test for end of a not if, if so do nothing
            if (firstChar == '/') {
                if (numOpenNotIfs == 0) {
                    appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                } else {
                    --numOpenNotIfs;
                }

                continue;
            }

            // test for end of a color, if so do nothing
            if (firstChar == '#') {
                if (!instance.config.display.pipe) {
                    ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
                }

                continue;
            }
        }

        // test for if, if so evaluate it
        if (firstChar == '?') {
            ffStrbufSubstrAfter(&placeholderValue, 0);

            uint32_t index = getArgumentIndex(placeholderValue.chars, numArgs, arguments);

            // testing for an invalid index
            if (index > numArgs || index < 1) {
                appendInvalidPlaceholder(buffer, "{?", &placeholderValue, i, formatstr->length);
                continue;
            }

            // continue normally if an format arg is set and the value is > 0
            if (formatArgSet(&arguments[index - 1])) {
                ++numOpenIfs;
                continue;
            }

            // fastforward to the end of the if without printing the in between
            i = ffStrbufNextIndexS(formatstr, i, "{?}") + 2; // 2 is the length of "{?}" - 1 because the loop will increment it again directly after continue
            continue;
        }

        // test for not if, if so evaluate it
        if (firstChar == '/') {
            ffStrbufSubstrAfter(&placeholderValue, 0);

            uint32_t index = getArgumentIndex(placeholderValue.chars, numArgs, arguments);

            // testing for an invalid index
            if (index > numArgs || index < 1) {
                appendInvalidPlaceholder(buffer, "{/", &placeholderValue, i, formatstr->length);
                continue;
            }

            // continue normally if an format arg is not set or the value is 0
            if (!formatArgSet(&arguments[index - 1])) {
                ++numOpenNotIfs;
                continue;
            }

            // fastforward to the end of the if without printing the in between
            i = ffStrbufNextIndexS(formatstr, i, "{/}") + 2; // 2 is the length of "{/}" - 1 because the loop will increment it again directly after continue
            continue;
        }

        // test for color, if so evaluate it
        if (firstChar == '#') {
            if (!instance.config.display.pipe) {
                ffStrbufAppendS(buffer, "\e[");
                ffOptionParseColorNoClear(placeholderValue.chars + 1, buffer);
                ffStrbufAppendC(buffer, 'm');
            }
            continue;
        }

        // test for constant or env var, if so evaluate it
        if (firstChar == '$') {
            char* pend = NULL;
            int32_t indexSigned = (int32_t) strtol(placeholderValue.chars + 1, &pend, 10);
            if (pend == placeholderValue.chars + 1) {
                // treat placeholder as an environment variable
                char* envValue = getenv(placeholderValue.chars + 1);
                if (envValue) {
                    ffStrbufAppendS(buffer, envValue);
                } else {
                    appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                }
            } else {
                // treat placeholder as a constant
                uint32_t index = (uint32_t) (indexSigned < 0 ? (int32_t) instance.config.display.constants.length + indexSigned : indexSigned - 1);

                if (*pend != '\0' || instance.config.display.constants.length <= index) {
                    appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                } else {
                    FFstrbuf* item = FF_LIST_GET(FFstrbuf, instance.config.display.constants, index);
                    ffStrbufAppend(buffer, item);
                }
            }
            continue;
        }

        char* pSep = placeholderValue.chars;
        char cSep = '\0';
        while (*pSep && *pSep != ':' && *pSep != '<' && *pSep != '>' && *pSep != '~') {
            ++pSep;
        }
        if (*pSep) {
            cSep = *pSep;
            *pSep = '\0';
        } else {
            pSep = NULL;
        }

        uint32_t index = getArgumentIndex(placeholderValue.chars, numArgs, arguments);

        // test for invalid index
        if (index == 0) {
            index = ++argCounter;
        }

        if (index > numArgs) {
            if (pSep) {
                *pSep = cSep;
            }
            appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
            continue;
        }

        if (!cSep) {
            ffFormatAppendFormatArg(buffer, &arguments[index - 1]);
        } else if (cSep == '~') {
            FF_STRBUF_AUTO_DESTROY tempString = ffStrbufCreate();
            ffFormatAppendFormatArg(&tempString, &arguments[index - 1]);

            char* pEnd = NULL;
            int32_t start = (int32_t) strtol(pSep + 1, &pEnd, 10);
            if (start < 0) {
                start = (int32_t) tempString.length + start;
            }
            if (start >= 0 && (uint32_t) start < tempString.length) {
                if (*pEnd == '\0') {
                    ffStrbufAppendNS(buffer, tempString.length - (uint32_t) start, &tempString.chars[start]);
                } else if (*pEnd == ',') {
                    int32_t end = (int32_t) strtol(pEnd + 1, &pEnd, 10);
                    if (!*pEnd) {
                        if (end < 0) {
                            end = (int32_t) tempString.length + end;
                        }
                        if ((uint32_t) end > tempString.length) {
                            end = (int32_t) tempString.length;
                        }
                        if (end > start) {
                            ffStrbufAppendNS(buffer, (uint32_t) (end - start), &tempString.chars[start]);
                        }
                    }
                }
            }

            if (*pEnd) {
                *pSep = cSep;
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                continue;
            }
        } else {
            char* pEnd = NULL;
            int32_t truncLength = (int32_t) strtol(pSep + 1, &pEnd, 10);
            if (*pEnd != '\0') {
                *pSep = cSep;
                appendInvalidPlaceholder(buffer, "{", &placeholderValue, i, formatstr->length);
                continue;
            }

            bool ellipsis = false;
            if (truncLength < 0) {
                ellipsis = true;
                truncLength = -truncLength;
            }

            FF_STRBUF_AUTO_DESTROY tempString = ffStrbufCreate();
            ffFormatAppendFormatArg(&tempString, &arguments[index - 1]);
            if (tempString.length == (uint32_t) truncLength) {
                ffStrbufAppend(buffer, &tempString);
            } else if (tempString.length > (uint32_t) truncLength) {
                if (cSep == ':') {
                    ffStrbufSubstrBefore(&tempString, (uint32_t) truncLength);
                    ffStrbufTrimRightSpace(&tempString);
                } else {
                    ffStrbufSubstrBefore(&tempString, (uint32_t) (!ellipsis ? truncLength : truncLength - 1));
                }
                ffStrbufAppend(buffer, &tempString);

                if (ellipsis) {
                    ffStrbufAppendS(buffer, "…");
                }
            } else if (cSep == ':') {
                ffStrbufAppend(buffer, &tempString);
            } else {
                if (cSep == '<') {
                    ffStrbufAppend(buffer, &tempString);
                    ffStrbufAppendNC(buffer, (uint32_t) truncLength - tempString.length, ' ');
                } else {
                    ffStrbufAppendNC(buffer, (uint32_t) truncLength - tempString.length, ' ');
                    ffStrbufAppend(buffer, &tempString);
                }
            }
        }
    }

    if (!instance.config.display.pipe) {
        ffStrbufAppendS(buffer, FASTFETCH_TEXT_MODIFIER_RESET);
    }

    return true;
}
