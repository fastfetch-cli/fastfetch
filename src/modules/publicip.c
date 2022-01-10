#include "fastfetch.h"

#define FF_PUBLICIP_MODULE_NAME "Public IP"
#define FF_PUBLICIP_NUM_FORMAT_ARGS 1

void ffPrintPublicIp(FFinstance* instance)
{
    FFstrbuf result;
    ffStrbufInitA(&result, 4096);
    ffNetworkingGetHttp("ipinfo.io", "/ip", instance->config.publicIpTimeout, &result);
    ffStrbufSubstrAfterFirstS(&result, "\r\n\r\n");

    if(result.length == 0)
    {
        ffPrintError(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIpKey, &instance->config.publicIpFormat, FF_PUBLICIP_NUM_FORMAT_ARGS, "Failed to connect to an IP detection server");
        return;
    }

    if(instance->config.publicIpFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIpKey);
        ffStrbufPutTo(&result, stdout);
    }
    else
    {
        ffPrintFormatString(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIpKey, &instance->config.publicIpFormat, NULL, FF_PUBLICIP_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result}
        });
    }
}
