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

    ffStrbufInitA(&result.systemName, 256);
    ffStrbufInitA(&result.name, 256);
    ffStrbufInitA(&result.prettyName, 256);
    ffStrbufInitA(&result.id, 256);
    ffStrbufInitA(&result.idLike, 256);
    ffStrbufInitA(&result.variant, 256);
    ffStrbufInitA(&result.variantID, 256);
    ffStrbufInitA(&result.version, 256);
    ffStrbufInitA(&result.versionID, 256);
    ffStrbufInitA(&result.codename, 256);
    ffStrbufInitA(&result.buildID, 256);
    ffStrbufInitA(&result.architecture, 256);
    ffStrbufInitA(&result.error, 256);

    ffStrbufSetS(&result.systemName, instance->state.utsname.sysname);
    ffStrbufSetS(&result.architecture, instance->state.utsname.machine);

    char* line = NULL;
    size_t len = 0;

    // Documentation of the fields:
    // https://www.freedesktop.org/software/systemd/man/os-release.html
    while (getline(&line, &len, osRelease) != -1)
    {
        sscanf(line, "NAME=%[^\n]", result.name.chars);
        sscanf(line, "NAME=\"%[^\"]+", result.name.chars);
        sscanf(line, "PRETTY_NAME=%[^\n]", result.prettyName.chars);
        sscanf(line, "PRETTY_NAME=\"%[^\"]+", result.prettyName.chars);
        sscanf(line, "ID=%[^\n]", result.id.chars);
        sscanf(line, "ID=\"%[^\"]+", result.id.chars);
        sscanf(line, "ID_LIKE=%[^\n]", result.idLike.chars);
        sscanf(line, "ID_LIKE=\"%[^\"]+", result.idLike.chars);
        sscanf(line, "VARIANT=%[^\n]", result.variant.chars);
        sscanf(line, "VARIANT=\"%[^\"]+", result.variant.chars);
        sscanf(line, "VARIANT_ID=%[^\n]", result.variantID.chars);
        sscanf(line, "VARIANT_ID=\"%[^\"]+", result.variantID.chars);
        sscanf(line, "VERSION=%[^\n]", result.version.chars);
        sscanf(line, "VERSION=\"%[^\"]+", result.version.chars);
        sscanf(line, "VERSION_ID=%[^\n]", result.versionID.chars);
        sscanf(line, "VERSION_ID=\"%[^\"]+", result.versionID.chars);
        sscanf(line, "VERSION_CODENAME=%[^\n]", result.codename.chars);
        sscanf(line, "VERSION_CODENAME=\"%[^\"]+", result.codename.chars);
        sscanf(line, "BUILD_ID=%[^\n]", result.buildID.chars);
        sscanf(line, "BUILD_ID=\"%[^\"]+", result.buildID.chars);
    }

    ffStrbufRecalculateLength(&result.systemName);
    ffStrbufRecalculateLength(&result.name);
    ffStrbufRecalculateLength(&result.prettyName);
    ffStrbufRecalculateLength(&result.id);
    ffStrbufRecalculateLength(&result.idLike);
    ffStrbufRecalculateLength(&result.variant);
    ffStrbufRecalculateLength(&result.variantID);
    ffStrbufRecalculateLength(&result.version);
    ffStrbufRecalculateLength(&result.versionID);
    ffStrbufRecalculateLength(&result.codename);
    ffStrbufRecalculateLength(&result.buildID);
    ffStrbufRecalculateLength(&result.architecture);
    ffStrbufRecalculateLength(&result.error);

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

    if(result->prettyName.length > 0)
    {
        ffStrbufAppend(&os, &result->prettyName);
    }
    else if(result->name.length == 0 && result->id.length == 0)
    {
        ffStrbufAppend(&os, &result->systemName);
    }
    else
    {
        if(result->name.length > 0)
            ffStrbufAppend(&os, &result->name);
        else
            ffStrbufAppend(&os, &result->id);

        if(result->version.length > 0)
        {
            ffStrbufAppendC(&os, ' ');
            ffStrbufAppend(&os, &result->version);
        }
        else
        {
            if(result->versionID.length > 0)
            {
                ffStrbufAppendC(&os, ' ');
                ffStrbufAppend(&os, &result->versionID);
            }

            if(result->variant.length > 0)
            {
                ffStrbufAppendS(&os, " (");
                ffStrbufAppend(&os, &result->variant);
                ffStrbufAppendC(&os, ')');
            }
            else if(result->variantID.length > 0)
            {
                ffStrbufAppendS(&os, " (");
                ffStrbufAppend(&os, &result->variantID);
                ffStrbufAppendC(&os, ')');
            }
        }
    }

    ffStrbufAppendC(&os, ' ');
    ffStrbufAppendS(&os, instance->state.utsname.machine);

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
