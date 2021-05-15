#include "fastfetch.h"

#include <pthread.h>

#define FF_OS_MODULE_NAME "OS"
#define FF_OS_NUM_FORMAT_ARGS 12

const FFOSResult* ffDetectOS(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFOSResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    FILE* osRelease = fopen("/etc/os-release", "r");

    if(osRelease == NULL)
        osRelease = fopen("/usr/lib/os-release", "r");

    ffStrbufInitA(&result.error, 64);
    if(osRelease == NULL)
    {
        ffStrbufAppendS(&result.error, "couldn't read /etc/os-release nor /usr/lib/os-release");
        return &result;
    }

    ffStrbufInit(&result.systemName);
    ffStrbufInit(&result.name);
    ffStrbufInit(&result.prettyName);
    ffStrbufInit(&result.id);
    ffStrbufInit(&result.idLike);
    ffStrbufInit(&result.variant);
    ffStrbufInit(&result.variantID);
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.versionID);
    ffStrbufInit(&result.codename);
    ffStrbufInit(&result.buildID);
    ffStrbufInit(&result.architecture);
    ffStrbufInit(&result.error);

    ffStrbufSetS(&result.systemName, instance->state.utsname.sysname);
    ffStrbufSetS(&result.architecture, instance->state.utsname.machine);

    char* line = NULL;
    size_t len = 0;

    // Documentation of the fields:
    // https://www.freedesktop.org/software/systemd/man/os-release.html
    while (getline(&line, &len, osRelease) != -1)
    {
        ffGetPropValue(line, "NAME =", &result.name);
        ffGetPropValue(line, "PRETTY_NAME =", &result.prettyName);
        ffGetPropValue(line, "ID =", &result.id);
        ffGetPropValue(line, "ID_LIKE =", &result.idLike);
        ffGetPropValue(line, "VARIANT =", &result.variant);
        ffGetPropValue(line, "VARIANT_ID =", &result.variantID);
        ffGetPropValue(line, "VERSION =", &result.version);
        ffGetPropValue(line, "VERSION_ID =", &result.versionID);
        ffGetPropValue(line, "VERSION_CODENAME =", &result.codename);
        ffGetPropValue(line, "BUILD_ID =", &result.buildID);
    }

    if(line != NULL)
        free(line);

    fclose(osRelease);

    pthread_mutex_unlock(&mutex);

    return &result;
}

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_OS_MODULE_NAME, &instance->config.osKey, &instance->config.osFormat, FF_OS_NUM_FORMAT_ARGS))
        return;

    const FFOSResult* result = ffDetectOS(instance);

    if(result->error.length > 0)
    {
        ffPrintError(instance, FF_OS_MODULE_NAME, 0, &instance->config.osKey, &instance->config.osFormat, FF_OS_NUM_FORMAT_ARGS, result->error.chars);
        return;
    }

    FF_STRBUF_CREATE(os);

    //Name
    if(result->name.length > 0)
        ffStrbufAppend(&os, &result->name);
    else if(result->id.length > 0)
        ffStrbufAppend(&os, &result->id);
    else if(result->prettyName.length > 0)
        ffStrbufAppend(&os, &result->prettyName);
    else
        ffStrbufAppend(&os, &result->systemName);

    //Version
    if(result->versionID.length > 0)
    {
        ffStrbufAppendC(&os, ' ');
        ffStrbufAppend(&os, &result->versionID);
    }
    else if(result->version.length > 0)
    {
        ffStrbufAppendC(&os, ' ');
        ffStrbufAppend(&os, &result->version);
    }

    //Codename
    if(result->codename.length > 0)
    {
        ffStrbufAppendC(&os, ' ');
        ffStrbufAppend(&os, &result->codename);
    }

    //Variant
    if(result->variant.length > 0)
    {
        ffStrbufAppendS(&os, " [");
        ffStrbufAppend(&os, &result->variant);
        ffStrbufAppendC(&os, ']');
    }
    else if(result->variantID.length > 0)
    {
        ffStrbufAppendS(&os, " [");
        ffStrbufAppend(&os, &result->variantID);
        ffStrbufAppendC(&os, ']');
    }

    ffStrbufAppendS(&os, " (");
    ffStrbufAppend(&os, &result->architecture);
    ffStrbufAppendC(&os, ')');

    ffPrintAndSaveToCache(instance, FF_OS_MODULE_NAME, &instance->config.osKey, &os, &instance->config.osFormat, FF_OS_NUM_FORMAT_ARGS, (FFformatarg[]){
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->systemName},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->prettyName},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->id},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->idLike},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->variant},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->variantID},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->version},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->versionID},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->codename},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->buildID},
        {FF_FORMAT_ARG_TYPE_STRBUF, &result->architecture}
    });

    ffStrbufDestroy(&os);
}
