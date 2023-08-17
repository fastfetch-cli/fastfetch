#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/networking.h"
#include "modules/publicip/publicip.h"
#include "util/stringUtils.h"

#define FF_PUBLICIP_DISPLAY_NAME "Public IP"
#define FF_PUBLICIP_NUM_FORMAT_ARGS 1

static FFNetworkingState state;
static int status = -1;

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

void ffPreparePublicIp(FFPublicIpOptions* options)
{
    if (options->url.length == 0)
        status = ffNetworkingSendHttpRequest(&state, "ipinfo.io", "/json", NULL);
    else
    {
        FF_STRBUF_AUTO_DESTROY host = ffStrbufCreateCopy(&options->url);
        ffStrbufSubstrAfterFirstS(&host, "://");
        uint32_t pathStartIndex = ffStrbufFirstIndexC(&host, '/');

        FF_STRBUF_AUTO_DESTROY path = ffStrbufCreate();
        if(pathStartIndex != host.length)
        {
            ffStrbufAppendNS(&path, pathStartIndex, host.chars + (host.length - pathStartIndex));
            host.length = pathStartIndex;
            host.chars[pathStartIndex] = '\0';
        }

        status = ffNetworkingSendHttpRequest(&state, host.chars, path.length == 0 ? "/" : path.chars, NULL);
    }
}

void ffPrintPublicIp(FFPublicIpOptions* options)
{
    if (status == -1)
        ffPreparePublicIp(options);

    if (status == 0)
    {
        ffPrintError(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, "Failed to connect to an IP detection server");
        return;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(4096);
    bool success = ffNetworkingRecvHttpResponse(&state, &result, options->timeout);
    if (success) ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if (!success || result.length == 0)
    {
        ffPrintError(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, "Failed to receive the server response");
        return;
    }

    if (options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);

        if (options->url.length == 0)
        {
            yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(result.chars, result.length, 0, NULL, NULL);
            if (doc)
            {
                yyjson_val* root = yyjson_doc_get_root(doc);
                printf("%s (%s, %s)\n",
                    yyjson_get_str(yyjson_obj_get(root, "ip")),
                    yyjson_get_str(yyjson_obj_get(root, "city")),
                    yyjson_get_str(yyjson_obj_get(root, "country"))
                );
                return;
            }
        }

        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormat(FF_PUBLICIP_DISPLAY_NAME, 0, &options->moduleArgs, FF_PUBLICIP_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result}
        });
    }
}

void ffInitPublicIpOptions(FFPublicIpOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_PUBLICIP_MODULE_NAME, ffParsePublicIpCommandOptions, ffParsePublicIpJsonObject, ffPrintPublicIp);
    ffOptionInitModuleArg(&options->moduleArgs);

    ffStrbufInit(&options->url);
    options->timeout = 0;
}

bool ffParsePublicIpCommandOptions(FFPublicIpOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_PUBLICIP_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffStrEqualsIgnCase(subKey, "url"))
    {
        ffOptionParseString(key, value, &options->url);
        return true;
    }

    if (ffStrEqualsIgnCase(subKey, "timeout"))
    {
        options->timeout = ffOptionParseUInt32(key, value);
        return true;
    }

    return false;
}

void ffDestroyPublicIpOptions(FFPublicIpOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);

    ffStrbufDestroy(&options->url);
}

void ffParsePublicIpJsonObject(FFPublicIpOptions* options, yyjson_val* module)
{
    if (module)
    {
        yyjson_val *key_, *val;
        size_t idx, max;
        yyjson_obj_foreach(module, idx, max, key_, val)
        {
            const char* key = yyjson_get_str(key_);
            if(ffStrEqualsIgnCase(key, "type"))
                continue;

            if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
                continue;

            if (ffStrEqualsIgnCase(key, "url"))
            {
                ffStrbufSetS(&options->url, yyjson_get_str(val));
                continue;
            }

            if (ffStrEqualsIgnCase(key, "timeout"))
            {
                options->timeout = (uint32_t) yyjson_get_uint(val);
                continue;
            }

            ffPrintError(FF_PUBLICIP_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
        }
    }
}
