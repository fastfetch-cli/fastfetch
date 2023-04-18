#include "fastfetch.h"
#include "common/printing.h"
#include "detection/network/network.h"

#define FF_NETWORK_MODULE_NAME "Network"
#define FF_NETWORK_NUM_FORMAT_ARGS 5

void ffPrintNetwork(FFinstance* instance)
{
    FF_LIST_AUTO_DESTROY networks;
    ffListInit(&networks, sizeof(FFNetworkResult));
    const char* error = ffDetectNetwork(instance, &networks);

    if(error != NULL)
    {
        ffPrintError(instance, FF_NETWORK_MODULE_NAME, 0, &instance->config.network, "%s", error);
        return;
    }

    FF_STRBUF_AUTO_DESTROY key;
    ffStrbufInit(&key);

    FF_LIST_FOR_EACH(FFNetworkResult, network, networks)
    {
        if(instance->config.network.key.length == 0)
        {
            ffStrbufSetF(&key, "%s (%s)", FF_NETWORK_MODULE_NAME, network->type.chars);
        }
        else
        {
            ffParseFormatString(&key, &instance->config.network.key, 1, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRBUF, &network->type},
                {FF_FORMAT_ARG_TYPE_STRBUF, &network->name},
            });
        }

        if (instance->config.network.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, key.chars, 0, NULL);
            printf("%s - %s\n", network->name.chars, network->address.chars);
        }
        else
        {
            ffPrintFormatString(instance, key.chars, 0, NULL, &instance->config.network.outputFormat, FF_NETWORK_NUM_FORMAT_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_STRING, network->on ? "ON" : "OFF"},
                {FF_FORMAT_ARG_TYPE_STRBUF, &network->type},
                {FF_FORMAT_ARG_TYPE_STRBUF, &network->name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &network->address},
                {FF_FORMAT_ARG_TYPE_INT, &network->mtu},
            });
        }
    }
}
