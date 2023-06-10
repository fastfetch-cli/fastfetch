#include "common/json.h"

#ifdef FF_HAVE_JSONC

#include "common/thread.h"

static const FFJsonLibrary* loadLibSymbols(const FFinstance* instance)
{
    assert(instance);

    static FFJsonLibrary lib;
    FF_LIBRARY_LOAD(libjsonc, &instance->config.libJSONC, NULL,
        #ifdef _WIN32
            "libjson-c-5" FF_LIBRARY_EXTENSION, -1
        #else
            "libjson-c" FF_LIBRARY_EXTENSION, 5
        #endif
    )
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_tokener_parse, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_is_type, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_array, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_boolean, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_double, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_int, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_string_len, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_string, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_get_object, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_object_del, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_object_get, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_object_length, NULL)
    FF_LIBRARY_LOAD_SYMBOL_VAR(libjsonc, lib, json_object_put, NULL)
    libjsonc = NULL; // don't dlclose automatically
    return &lib;
}

const FFJsonLibrary* ffJsonLib = NULL;
bool ffJsonLoadLibrary(const FFinstance* instance)
{
    static bool loaded = false;
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;

    if (!loaded)
    {
        ffThreadMutexLock(&mutex);

        if(!loaded)
        {
            loaded = true;
            ffJsonLib = loadLibSymbols(instance);
        }

        ffThreadMutexUnlock(&mutex);
    }
    return !!ffJsonLib;
}

#endif
