#include "fastfetch.h"

#include <dlfcn.h>
#include <pthread.h>
#include <dconf/client/dconf-client.h>

typedef struct DConfData
{
    void* library;
    DConfClient*(*ffdconf_client_new)(void);
    GVariant*(*ffdconf_client_read)(DConfClient*, const gchar*);
    const gchar*(*ffg_variant_get_string)(GVariant*, gsize*);
    DConfClient* dconfClient;
} DConfData;

static inline const char* getDConfValue(DConfData* dconf,  const char* key)
{
    if(dconf->dconfClient == NULL)
        return NULL;

    GVariant* variant = dconf->ffdconf_client_read(dconf->dconfClient, key);
    if(variant != NULL)
        return dconf->ffg_variant_get_string(variant, NULL);

    return NULL;
}

const char* ffDConfGetValue(FFinstance* instance, const char* key)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static bool init = false;

    static DConfData dconf;

    pthread_mutex_lock(&mutex);

    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return getDConfValue(&dconf, key);
    }

    init = true;

    dconf.dconfClient = NULL; //error indicator

    if(instance->config.libDConf.length == 0)
        dconf.library = dlopen("libdconf.so", RTLD_LAZY);
    else
        dconf.library = dlopen(instance->config.libDConf.chars, RTLD_LAZY);

    if(dconf.library == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    dconf.ffdconf_client_new = dlsym(dconf.library, "dconf_client_new");
    if(dconf.ffdconf_client_new == NULL)
    {
        pthread_mutex_unlock(&mutex);
        dlclose(dconf.library);
        return NULL;
    }

    dconf.ffg_variant_get_string = dlsym(dconf.library, "g_variant_get_string");
    if(dconf.ffg_variant_get_string == NULL)
    {
        pthread_mutex_unlock(&mutex);
        dlclose(dconf.library);
        return NULL;
    }

    GVariant*(*ffdconf_client_read)(DConfClient*, const gchar*) = dlsym(dconf.library, "dconf_client_read");
    if(ffdconf_client_read == NULL)
    {
        pthread_mutex_unlock(&mutex);
        dlclose(dconf.library);
        return NULL;
    }

    DConfClient* dconfClient = dconf.ffdconf_client_new();
    if(dconfClient == NULL)
    {
        pthread_mutex_unlock(&mutex);
        dlclose(dconf.library);
        return NULL;
    }

    pthread_mutex_unlock(&mutex);
    return getDConfValue(&dconf, key);
}
