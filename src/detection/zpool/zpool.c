#include "zpool.h"

#if FF_HAVE_LIBZFS

#include "common/kmod.h"

#ifdef __sun
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

    // The fields in this struct store property IDs returned by `zpool_name_to_prop`,
    // not the property values themselves.
    struct {
        int name;
        int health;
        int guid;
        int size;
        int free;
        int allocated;
        int fragmentation;
        int readonly;
    } props;

    libzfs_handle_t* handle;
    FFlist* result;
} FFZfsData;

static inline int ff_zpool_get_prop_wrapper(
    FFZfsData* data,
    zpool_handle_t *zhp,
    zpool_prop_t prop,
    char *buf,
    size_t len,
    zprop_source_t *srctype,
    boolean_t literal __attribute__((unused))
)
{
    /* 
    Solaris header: 
        extern int zpool_get_prop(zpool_handle_t *, zpool_prop_t, char *, size_t __proplen, zprop_source_t *);
    Illumos header:
        extern int zpool_get_prop(zpool_handle_t *, zpool_prop_t, char *, size_t proplen, zprop_source_t *, boolean_t);
    */
#if defined(__sun) && ! defined(__illumos__)
    return data->ffzpool_get_prop(zhp, prop, buf, len, srctype);
#else
    return data->ffzpool_get_prop(zhp, prop, buf, len, srctype, literal);
#endif

}

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
    if (ff_zpool_get_prop_wrapper(data, zpool, data->props.name, buf, ARRAY_SIZE(buf), &source, false) == 0)
        ffStrbufInitS(&item->name, buf);
    else
        ffStrbufInitStatic(&item->name, "unknown");
    if (ff_zpool_get_prop_wrapper(data, zpool, data->props.health, buf, ARRAY_SIZE(buf), &source, false) == 0)
        ffStrbufInitS(&item->state, buf);
    else
        ffStrbufInitStatic(&item->state, "unknown");
    item->guid = data->ffzpool_get_prop_int(zpool, data->props.guid, &source);
    item->total = data->ffzpool_get_prop_int(zpool, data->props.size, &source);
    item->used = item->total - data->ffzpool_get_prop_int(zpool, data->props.free, &source);
    item->allocated = data->ffzpool_get_prop_int(zpool, data->props.allocated, &source);
    uint64_t fragmentation = data->ffzpool_get_prop_int(zpool, data->props.fragmentation, &source);
    item->fragmentation = fragmentation == UINT64_MAX ? -DBL_MAX : (double) fragmentation;
    item->readOnly = (bool) data->ffzpool_get_prop_int(zpool, data->props.readonly, &source);
    data->ffzpool_close(zpool);
    return 0;
}

const char* ffDetectZpool(FFlist* result /* list of FFZpoolResult */)
{
    FF_LIBRARY_LOAD_MESSAGE(libzfs, "libzfs" FF_LIBRARY_EXTENSION, 6);
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

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libzfs, zpool_name_to_prop);

    #define FF_QUERY_ZPOOL_PROP_FROM_NAME(prop_name) do { \
        data.props.prop_name = ffzpool_name_to_prop(#prop_name); \
        if (data.props.prop_name < 0) \
            return "Failed to query prop: " #prop_name; \
    } while (false)
    FF_QUERY_ZPOOL_PROP_FROM_NAME(name);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(health);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(guid);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(size);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(free);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(allocated);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(fragmentation);
    FF_QUERY_ZPOOL_PROP_FROM_NAME(readonly);
    #undef FF_QUERY_ZPOOL_PROP_FROM_NAME

    FF_LIBRARY_LOAD_SYMBOL_MESSAGE(libzfs, zpool_iter);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, libzfs_fini);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_prop_int);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_get_prop);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(libzfs, data, zpool_close);

    if (ffzpool_iter(handle, enumZpoolCallback, &data) < 0)
        return "zpool_iter() failed";

    return NULL;
}

#else

const char* ffDetectZpool(FF_MAYBE_UNUSED FFlist* result)
{
    return "fastfetch was compiled without libzfs support";
}

#endif
