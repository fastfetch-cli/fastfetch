#include "common/printing.h"
#include "detection/bluetooth/bluetooth.h"

#define FF_BLUETOOTH_MODULE_NAME "Bluetooth"
#define FF_BLUETOOTH_NUM_FORMAT_ARGS 4

static void printDevice(FFinstance* instance, const FFBluetoothDevice* device, uint8_t index)
{
    if(instance->config.bluetooth.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_BLUETOOTH_MODULE_NAME, index, &instance->config.bluetooth.key);
        ffStrbufWriteTo(&device->name, stdout);

        if(device->battery > 0)
            printf(" (%d%%)", device->battery);

        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_BLUETOOTH_MODULE_NAME, index, &instance->config.bluetooth, FF_BLUETOOTH_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->address},
            {FF_FORMAT_ARG_TYPE_STRBUF, &device->type},
            {FF_FORMAT_ARG_TYPE_UINT8, &device->battery}
        });
    }
}

void ffPrintBluetooth(FFinstance* instance)
{
    const FFBluetoothResult* bluetooth = ffDetectBluetooth(instance);

    if(bluetooth->error.length > 0)
    {
        ffPrintError(instance, FF_BLUETOOTH_MODULE_NAME, 0, &instance->config.bluetooth, "%s", bluetooth->error.chars);
        return;
    }

    FFlist filtered;
    ffListInit(&filtered, sizeof(FFBluetoothDevice*));

    FF_LIST_FOR_EACH(FFBluetoothDevice, device, bluetooth->devices)
    {
        if(!device->connected && !instance->config.bluetoothShowDisconnected)
            continue;

        *(FFBluetoothDevice**)ffListAdd(&filtered) = device;
    }

    if(filtered.length == 0)
    {
        ffPrintError(instance, FF_BLUETOOTH_MODULE_NAME, 0, &instance->config.bluetooth, "No bluetooth devices found");
        return;
    }

    for(uint32_t i = 0; i < filtered.length; i++)
    {
        uint8_t index = (uint8_t) (filtered.length == 1 ? 0 : i + 1);
        printDevice(instance, *(FFBluetoothDevice**)ffListGet(&filtered, i), index);
    }

    ffListDestroy(&filtered);
}
