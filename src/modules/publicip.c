#include "fastfetch.h"
#include "common/printing.h"
#include "common/networking.h"

#define FF_PUBLICIP_MODULE_NAME "Public IP"
#define FF_PUBLICIP_NUM_FORMAT_ARGS 1

static int sockfd;

void ffPreparePublicIp(FFinstance* instance)
{
    if(instance->config.publicIpUrl.length == 0)
        sockfd = ffNetworkingSendHttpRequest("ipinfo.io", "/ip", instance->config.publicIpTimeout);
    else
    {
        FFstrbuf host;
        ffStrbufInitCopy(&host, &instance->config.publicIpUrl);
        ffStrbufSubstrAfterFirstS(&host, "://");
        uint32_t pathStartIndex = ffStrbufFirstIndexC(&host, '/');

        FFstrbuf path;
        ffStrbufInit(&path);
        if(pathStartIndex != host.length)
        {
            ffStrbufAppendNS(&path, pathStartIndex, host.chars + (host.length - pathStartIndex));
            host.length = pathStartIndex;
            host.chars[pathStartIndex] = '\0';
        }

        sockfd = ffNetworkingSendHttpRequest(host.chars, path.length == 0 ? "/" : path.chars, instance->config.publicIpTimeout);

        ffStrbufDestroy(&path);
        ffStrbufDestroy(&host);
    }
}

void ffPrintPublicIp(FFinstance* instance)
{
    if(sockfd == 0)
        ffPreparePublicIp(instance);

    if(sockfd < 0)
    {
        ffPrintError(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIP, "Failed to connect to an IP detection server");
        return;
    }

    FFstrbuf result;
    ffStrbufInitA(&result, 4096);
    ffNetworkingRecvHttpResponse(sockfd, &result);
    ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if(result.length == 0)
    {
        ffPrintError(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIP, "Failed to receive the server response");
        ffStrbufDestroy(&result);
        return;
    }

    if(instance->config.publicIP.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIP.key);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIP, FF_PUBLICIP_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result}
        });
    }

    ffStrbufDestroy(&result);
}
