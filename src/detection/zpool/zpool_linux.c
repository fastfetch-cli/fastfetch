#include "zpool.h"
#include "common/library.h"
#include "libzfs_simplified.h"

typedef struct FFZfsData
{
    FF_LIBRARY_SYMBOL(libzfs_fini)
    FF_LIBRARY_SYMBOL(zpool_iter)
    FF_LIBRARY_SYMBOL(zpool_get_prop_int)
    FF_LIBRARY_SYMBOL(zpool_get_name)
    FF_LIBRARY_SYMBOL(zpool_get_state_str)
    FF_LIBRARY_SYMBOL(zpool_get_status)

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
    ffStrbufInitS(&item->name, data->ffzpool_get_name(zpool));
    ffStrbufInitS(&item->state, data->ffzpool_get_state_str(zpool));
    const char* msgid = NULL;
    zpool_status_t status = data->ffzpool_get_status(zpool, &msgid, NULL);
    if (msgid && *msgid)
        ffStrbufInitS(&item->status, msgid);
    else
        ffStrbufInitStatic(&item->status, status == ZPOOL_STATUS_OK ? "OK" : "NOT OK");

    item->version = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_VERSION, &source);
    item->total = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_SIZE, &source);
    item->used = item->total - data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_FREE, &source);
    item->fragmentation = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_FRAGMENTATION, &source);
    return 0;
}

const char* ffDetectZpool(FFlist* result /* list of FFZpoolResult */)
{
    FF_LIBRARY_LOAD(libzfs, NULL, "dlopen libzfs" FF_LIBRARY_EXTENSION " failed", "libzfs" FF_LIBRARY_EXTENSION, 4);
    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libzfs, libzfs_init);

    libzfs_handle_t* handle = fflibzfs_init();
    if (!handle) return "libzfs_init() failed";

    __attribute__((__cleanup__(cleanLibzfs))) FFZfsData data = {
        .handle = handle,
        .result = result,
    };

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libzfs, zpool_iter);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, libzfs_fini);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_prop_int);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_name);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_state_str);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_status);

    if (ffzpool_iter(handle, enumZpoolCallback, &data) < 0)
        return "zpool_iter() failed";

    return NULL;
}
