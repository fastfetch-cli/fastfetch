#include "zpool.h"

#include "util/kmod.h"

#ifdef __sun
#define FF_DISABLE_DLOPEN
#include <libzfs.h>
#else
#include "libzfs_simplified.h"
#endif

#include "common/library.h"

typedef struct FFZfsData
{
    FF_LIBRARY_SYMBOL(libzfs_fini)
    FF_LIBRARY_SYMBOL(zpool_get_prop_int)
    FF_LIBRARY_SYMBOL(zpool_get_prop)
    FF_LIBRARY_SYMBOL(zpool_close)

    libzfs_handle_t* handle;
    FFlist* result;
} FFZfsData;

static inline void cleanLibzfs(FFZfsData* data)
{
    if (data->fflibzfs_fini && data->handle)
    {
        data->fflibzfs_fini(data->handle);
        data->handle = NULL;
    }
}

static int enumZpoolCallback(zpool_handle_t* zpool, void* param)
{
    FFZfsData* data = (FFZfsData*) param;
    zprop_source_t source;
    FFZpoolResult* item = ffListAdd(data->result);
    char buf[1024];
    if (data->ffzpool_get_prop(zpool, ZPOOL_PROP_NAME, buf, ARRAY_SIZE(buf), &source, false) == 0)
        ffStrbufInitS(&item->name, buf);
    else
        ffStrbufInitStatic(&item->name, "unknown");
    if (data->ffzpool_get_prop(zpool, ZPOOL_PROP_HEALTH, buf, ARRAY_SIZE(buf), &source, false) == 0)
        ffStrbufInitS(&item->state, buf);
    else
        ffStrbufInitStatic(&item->state, "unknown");
    item->guid = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_GUID, &source);
    item->total = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_SIZE, &source);
    item->used = item->total - data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_FREE, &source);
    item->allocated = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_ALLOCATED, &source);
    uint64_t fragmentation = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_FRAGMENTATION, &source);
    item->fragmentation = fragmentation == UINT64_MAX ? -DBL_MAX : (double) fragmentation;
    item->readOnly = (bool) data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_READONLY, &source);
    data->ffzpool_close(zpool);
    return 0;
}

const char* ffDetectZpool(FFlist* result /* list of FFZpoolResult */)
{
    FF_LIBRARY_LOAD(libzfs, "dlopen libzfs" FF_LIBRARY_EXTENSION " failed", "libzfs" FF_LIBRARY_EXTENSION, 6);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libzfs, libzfs_init);

    libzfs_handle_t* handle = fflibzfs_init();
    if (!handle)
    {
        if (!ffKmodLoaded("zfs")) return "`zfs` kernel module is not loaded";
        return "libzfs_init() failed";
    }

    __attribute__((__cleanup__(cleanLibzfs))) FFZfsData data = {
        .handle = handle,
        .result = result,
    };

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libzfs, zpool_iter);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, libzfs_fini);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_prop_int);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_prop);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_close);

    if (ffzpool_iter(handle, enumZpoolCallback, &data) < 0)
        return "zpool_iter() failed";

    return NULL;
}
