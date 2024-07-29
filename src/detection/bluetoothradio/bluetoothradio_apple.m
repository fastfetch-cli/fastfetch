#include "bluetoothradio.h"
#include "common/processing.h"

#import <IOBluetooth/IOBluetooth.h>

// For some reason the official declaration of IOBluetoothHostController don't include property `controllers`
@interface IOBluetoothHostController()
+ (id)controllers;
@end

const char* ffDetectBluetoothRadio(FFlist* devices /* FFBluetoothResult */)
{
    NSArray<IOBluetoothHostController*>* ctrls = IOBluetoothHostController.controllers;
    if(!ctrls)
        return "IOBluetoothHostController.controllers returns nil";

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buffer, (char* const[]) {
        "system_profiler",
        "SPBluetoothDataType",
        "-xml",
        "-detailLevel",
        "basic",
        NULL
    }) != NULL)
        return "Starting `system_profiler SPBluetoothDataType -xml -detailLevel basic` failed";

    NSArray* arr = [NSPropertyListSerialization propertyListWithData:[NSData dataWithBytes:buffer.chars length:buffer.length]
                    options:NSPropertyListImmutable
                    format:nil
                    error:nil];
    if (!arr || !arr.count)
        return "system_profiler SPBluetoothDataType returned an empty array";

    for (IOBluetoothHostController* ctrl in ctrls)
    {
        FFBluetoothRadioResult* device = ffListAdd(devices);
        ffStrbufInitS(&device->name, ctrl.nameAsString.UTF8String);
        ffStrbufInitS(&device->address, ctrl.addressAsString.UTF8String);
        ffStrbufInitStatic(&device->vendor, "Apple");
        device->lmpVersion = INT_MIN;
        device->lmpSubversion = INT_MIN;
        device->enabled = ctrl.powerState == kBluetoothHCIPowerStateON;
        device->discoverable = false;
        device->connectable = true;

        for (NSDictionary* itemDict in arr[0][@"_items"])
        {
            NSDictionary* props = itemDict[@"controller_properties"];
            if (!props) continue;

            if (![ctrl.addressAsString isEqualToString:props[@"controller_address"]]) continue;

            NSString* services = props[@"controller_supportedServices"];
            if ([services containsString:@" LEA "])
                device->lmpVersion = -11;
            else if ([services containsString:@" GATT "])
                device->lmpVersion = -6;

            device->discoverable = ![props[@"controller_discoverable"] isEqualToString:@"attrib_off"];
            ffStrbufSetS(&device->vendor, ((NSString*) props[@"controller_vendorID"]).UTF8String);
            ffStrbufSubstrAfterFirstC(&device->vendor, '(');
            ffStrbufTrimRight(&device->vendor, ')');
            break;
        }
    }

    return NULL;
}
