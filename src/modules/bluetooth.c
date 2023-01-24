#include "common/printing.h"
#include "detection/bluetooth/bluetooth.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"
#define FF_BLUETOOTH_NUM_FORMAT_ARGS 4

void ffPrintBluetooth(FFinstance* instance)
{
    const FFBluetoothResult* bluetooth = ffDetectBluetooth(instance);

    if(bluetooth->error.length > 0)
    {
        ffPrintError(instance, FF_BLUETOOTH_MODULE_NAME, 0, &instance->config.bluetooth, "%s", bluetooth->error.chars);
        return;
    }

    if(instance->config.bluetooth.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BLUETOOTH_MODULE_NAME, 0, &instance->config.bluetooth.key);
        ffStrbufWriteTo(&bluetooth->name, stdout);

        if(bluetooth->battery > 0)
            printf(" (%d%%)", bluetooth->battery);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_BLUETOOTH_MODULE_NAME, 0, &instance->config.bluetooth, FF_BLUETOOTH_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &bluetooth->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bluetooth->address},
            {FF_FORMAT_ARG_TYPE_STRBUF, &bluetooth->type},
            {FF_FORMAT_ARG_TYPE_UINT8, &bluetooth->battery}
        });
    }
}
