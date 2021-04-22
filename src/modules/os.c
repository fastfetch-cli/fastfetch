#include "fastfetch.h"

#define FF_OS_MODULE_NAME "OS"
#define FF_OS_NUM_FORMAT_ARGS 12

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_OS_MODULE_NAME, &instance->config.osKey, &instance->config.osFormat, FF_OS_NUM_FORMAT_ARGS))
        return;

    const FFOSResult* result = ffCalculateOS(instance);

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
