#include "fastfetch.h"
#include "common/printing.h"
#include "common/networking.h"

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
        ffPrintError(instance, FF_PUBLICIP_MODULE_NAME, 0, &instance->config.publicIP, "Failed to connect to an IP detection server");
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
}
