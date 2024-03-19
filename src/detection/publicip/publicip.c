#include "publicip.h"
#include "common/networking.h"

static FFNetworkingState state;
static int status = -1;

void ffPreparePublicIp(FFPublicIpOptions* options)
{
    if (status != -1)
    {
        fputs("Error: this module can only be used once due to internal limitations\n", stderr);
        exit(1);
    }

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

static inline void wrapYyjsonFree(yyjson_doc** doc)
{
    assert(doc);
    if (*doc)
        yyjson_doc_free(*doc);
}

const char* ffDetectPublicIp(FFPublicIpOptions* options, FFPublicIpResult* result)
{
    if (status == -1)
        ffPreparePublicIp(options);

    if (status == 0)
        return "Failed to connect to an IP detection server";

    FF_STRBUF_AUTO_DESTROY response = ffStrbufCreateA(4096);
    bool success = ffNetworkingRecvHttpResponse(&state, &response, options->timeout);
    if (success) ffStrbufSubstrAfterFirstS(&response, "\r\n\r\n");

    if (!success || response.length == 0)
        return "Failed to receive the server response";

    if (options->url.length == 0)
    {
        yyjson_doc* __attribute__((__cleanup__(wrapYyjsonFree))) doc = yyjson_read_opts(response.chars, response.length, 0, NULL, NULL);
        if (doc)
        {
            yyjson_val* root = yyjson_doc_get_root(doc);
            ffStrbufAppendS(&result->ip, yyjson_get_str(yyjson_obj_get(root, "ip")));
            ffStrbufDestroy(&result->location);
            ffStrbufInitF(&result->location, "%s, %s", yyjson_get_str(yyjson_obj_get(root, "city")), yyjson_get_str(yyjson_obj_get(root, "country")));
            return NULL;
        }
    }

    ffStrbufDestroy(&result->ip);
    ffStrbufInitMove(&result->ip, &response);
    ffStrbufTrimRightSpace(&result->ip);
    return NULL;
}
