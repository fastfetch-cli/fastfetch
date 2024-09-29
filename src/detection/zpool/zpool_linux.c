#include "zpool.h"

#ifdef FF_HAVE_LIBZFS
#include "util/kmod.h"

#ifdef __sun
#define FF_DISABLE_DLOPEN
#include <libzfs.h>

const char* zpool_get_state_str(zpool_handle_t* zpool)
{
    if (zpool_get_state(zpool) == POOL_STATE_UNAVAIL)
        return "FAULTED";
    else
    {
        const char *str;
        zpool_errata_t errata;
        zpool_status_t status = zpool_get_status(zpool, (char**) &str, &errata);
        if (status == ZPOOL_STATUS_IO_FAILURE_WAIT ||
            status == ZPOOL_STATUS_IO_FAILURE_CONTINUE ||
            status == ZPOOL_STATUS_IO_FAILURE_MMP)
            return "SUSPENDED";
        else
        {
            nvlist_t *nvroot = fnvlist_lookup_nvlist(zpool_get_config(zpool, NULL), ZPOOL_CONFIG_VDEV_TREE);
            uint_t vsc;
            vdev_stat_t *vs;
            #ifdef __x86_64__
            if (nvlist_lookup_uint64_array(nvroot, ZPOOL_CONFIG_VDEV_STATS, (uint64_t**) &vs, &vsc) != 0)
            #else
            if (nvlist_lookup_uint32_array(nvroot, ZPOOL_CONFIG_VDEV_STATS, (uint32_t**) &vs, &vsc) != 0)
            #endif
                return "UNKNOWN";
            else
                return zpool_state_to_name(vs->vs_state, vs->vs_aux);
        }
    }
}
#else
#include "libzfs_simplified.h"
#endif

#include "common/library.h"

typedef struct FFZfsData
{
    FF_LIBRARY_SYMBOL(libzfs_fini)
    FF_LIBRARY_SYMBOL(zpool_iter)
    FF_LIBRARY_SYMBOL(zpool_get_prop_int)
    FF_LIBRARY_SYMBOL(zpool_get_name)
    FF_LIBRARY_SYMBOL(zpool_get_state_str)

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
    item->version = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_VERSION, &source);
    item->total = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_SIZE, &source);
    item->used = item->total - data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_FREE, &source);
    item->fragmentation = data->ffzpool_get_prop_int(zpool, ZPOOL_PROP_FRAGMENTATION, &source);
    return 0;
}

const char* ffDetectZpool(FFlist* result /* list of FFZpoolResult */)
{
    FF_LIBRARY_LOAD(libzfs, "dlopen libzfs" FF_LIBRARY_EXTENSION " failed", "libzfs" FF_LIBRARY_EXTENSION, 4);
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
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_name);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_state_str);

    if (ffzpool_iter(handle, enumZpoolCallback, &data) < 0)
        return "zpool_iter() failed";

    return NULL;
}

#else

const char* ffDetectZpool(FF_MAYBE_UNUSED FFlist* result)
{
    return "Fastfetch was compiled without libzfs support";
}

#endif
