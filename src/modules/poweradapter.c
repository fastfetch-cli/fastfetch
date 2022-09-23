#include "fastfetch.h"
#include "common/printing.h"
#include "detection/poweradapter/poweradapter.h"

#define FF_POWER_ADAPTER_MODULE_NAME "Power Adapter"
#define FF_POWER_ADAPTER_MODULE_ARGS 5

static void printPowerAdapter(FFinstance* instance, const PowerAdapterResult* result, uint8_t index)
{
    if(result->watts != FF_POWER_ADAPTER_UNSET)
    {
        if(instance->config.powerAdapter.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(instance, FF_POWER_ADAPTER_MODULE_NAME, index, &instance->config.powerAdapter.key);

            if(result->name.length > 0)
                puts(result->name.chars);
            else if(result->watts == FF_POWER_ADAPTER_NOT_CONNECTED)
                puts("not connected");
            else
                printf("%dW\n", result->watts);
        }
        else
        {
            ffPrintFormat(instance, FF_POWER_ADAPTER_MODULE_NAME, index, &instance->config.powerAdapter, FF_POWER_ADAPTER_MODULE_ARGS, (FFformatarg[]){
                {FF_FORMAT_ARG_TYPE_INT, &result->watts},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->name},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->manufacturer},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->modelName},
                {FF_FORMAT_ARG_TYPE_STRBUF, &result->description},
            });
        }
    }
}

void ffPrintPowerAdapter(FFinstance* instance)
{
    FFlist results;
    ffListInitA(&results, sizeof(PowerAdapterResult), 0);

    const char* error = ffDetectPowerAdapterImpl(instance, &results);

    if (error)
    {
        ffPrintError(instance, FF_POWER_ADAPTER_MODULE_NAME, 0, &instance->config.powerAdapter, "%s", error);
    }
    else if(results.length == 0)
    {
        ffPrintError(instance, FF_POWER_ADAPTER_MODULE_NAME, 0, &instance->config.powerAdapter, "No power adapters found");
    }
    else
    {
        for(uint8_t i = 0; i < (uint8_t) results.length; i++)
        {
            PowerAdapterResult* result = ffListGet(&results, i);
            printPowerAdapter(instance, result, i);

            ffStrbufDestroy(&result->manufacturer);
            ffStrbufDestroy(&result->description);
            ffStrbufDestroy(&result->modelName);
            ffStrbufDestroy(&result->name);
        }
    }

    ffListDestroy(&results);
}
