#include "fastfetch.h"

#define FF_OS_MODULE_NAME "OS"
#define FF_OS_NUM_FORMAT_ARGS 12

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_OS_MODULE_NAME, &instance->config.osKey, &instance->config.osFormat, FF_OS_NUM_FORMAT_ARGS))
        return;

    const FFOSResult* result = ffDetectOS(instance);

    if(result->name.length == 0 && result->prettyName.length == 0)
    {
        ffPrintError(instance, FF_OS_MODULE_NAME, 0, &instance->config.osKey, &instance->config.osFormat, FF_OS_NUM_FORMAT_ARGS, "Could not detect OS");
        return;
    }

    FFstrbuf os;
    ffStrbufInit(&os);

    //Create the basic output

    if(result->name.length > 0)
        ffStrbufAppend(&os, &result->name);
    else if(result->prettyName.length > 0)
        ffStrbufAppend(&os, &result->name);
    else if(result->id.length > 0)
        ffStrbufAppend(&os, &result->id);
    else if(result->systemName.length > 0)
        ffStrbufAppend(&os, &result->systemName);
    else
        ffStrbufAppendS(&os, "Linux");

    //Append version if it is missing

    if(result->versionID.length > 0 && ffStrbufFirstIndex(&os, &result->versionID) == os.length)
    {
        ffStrbufAppendC(&os, ' ');
        ffStrbufAppend(&os, &result->versionID);
    }
    else if(result->versionID.length == 0 && result->version.length > 0 && ffStrbufFirstIndex(&os, &result->version) == os.length)
    {
        ffStrbufAppendC(&os, ' ');
        ffStrbufAppend(&os, &result->version);
    }

    //Append variant if it is missing

    if(result->variant.length > 0 && ffStrbufFirstIndex(&os, &result->variant) == os.length)
    {
        ffStrbufAppendS(&os, " (");
        ffStrbufAppend(&os, &result->variant);
        ffStrbufAppendC(&os, ')');
    }
    else if(result->variant.length == 0 && result->variantID.length > 0 && ffStrbufFirstIndex(&os, &result->variantID) == os.length)
    {
        ffStrbufAppendS(&os, " (");
        ffStrbufAppend(&os, &result->variantID);
        ffStrbufAppendC(&os, ')');
    }

    //Append architecture if it is missing

    if(ffStrbufFirstIndex(&os, &result->architecture) == os.length)
    {
        ffStrbufAppendS(&os, " [");
        ffStrbufAppend(&os, &result->architecture);
        ffStrbufAppendC(&os, ']');
    }

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
