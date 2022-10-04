#include "host.h"
#include "common/io.h"

#include <stdlib.h>

static bool hostValueSet(FFstrbuf* value)
{
    return
        value->length > 0 &&
        ffStrbufStartsWithIgnCaseS(value, "To be filled") != true &&
        ffStrbufStartsWithIgnCaseS(value, "To be set") != true &&
        ffStrbufStartsWithIgnCaseS(value, "OEM") != true &&
        ffStrbufStartsWithIgnCaseS(value, "O.E.M.") != true &&
        ffStrbufIgnCaseCompS(value, "None") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "Default string") != 0 &&
        ffStrbufIgnCaseCompS(value, "Undefined") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Specified") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Applicable") != 0 &&
        ffStrbufIgnCaseCompS(value, "INVALID") != 0 &&
        ffStrbufIgnCaseCompS(value, "Type1ProductConfigId") != 0 &&
        ffStrbufIgnCaseCompS(value, "All Series") != 0
    ;
}

static void getHostValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffReadFileBuffer(devicesPath, buffer);
    if(hostValueSet(buffer))
        return;

    ffReadFileBuffer(classPath, buffer);
    if(hostValueSet(buffer))
        return;

    ffStrbufClear(buffer);
}

static void getHostProductName(FFstrbuf* name)
{
    getHostValue("/sys/devices/virtual/dmi/id/product_name", "/sys/class/dmi/id/product_name", name);
    if(name->length > 0)
        return;

    ffReadFileBuffer("/sys/firmware/devicetree/base/model", name);
    if(hostValueSet(name))
        return;

    //does a clear before the read
    ffReadFileBuffer("/tmp/sysinfo/model", name);
    if(hostValueSet(name))
        return;

    ffStrbufClear(name);
}

void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productFamily);
    getHostValue("/sys/devices/virtual/dmi/id/product_family", "/sys/class/dmi/id/product_family", &host->productFamily);

    ffStrbufInit(&host->productName);
    getHostProductName(&host->productName);

    ffStrbufInit(&host->productVersion);
    getHostValue("/sys/devices/virtual/dmi/id/product_version", "/sys/class/dmi/id/product_version", &host->productVersion);

    ffStrbufInit(&host->productSku);
    getHostValue("/sys/devices/virtual/dmi/id/product_sku", "/sys/class/dmi/id/product_sku", &host->productSku);

    ffStrbufInit(&host->boardName);
    getHostValue("/sys/devices/virtual/dmi/id/board_name", "/sys/class/dmi/id/board_name", &host->boardName);

    ffStrbufInit(&host->boardVendor);
    getHostValue("/sys/devices/virtual/dmi/id/board_vendor", "/sys/class/dmi/id/board_vendor", &host->boardVendor);

    ffStrbufInit(&host->boardVersion);
    getHostValue("/sys/devices/virtual/dmi/id/board_version", "/sys/class/dmi/id/board_version", &host->boardVersion);

    ffStrbufInit(&host->chassisType);
    getHostValue("/sys/devices/virtual/dmi/id/chassis_type", "/sys/class/dmi/id/chassis_type", &host->chassisType);

    ffStrbufInit(&host->chassisVendor);
    getHostValue("/sys/devices/virtual/dmi/id/chassis_vendor", "/sys/class/dmi/id/chassis_vendor", &host->chassisVendor);

    ffStrbufInit(&host->chassisVersion);
    getHostValue("/sys/devices/virtual/dmi/id/chassis_version", "/sys/class/dmi/id/chassis_version", &host->chassisVersion);

    ffStrbufInit(&host->sysVendor);
    getHostValue("/sys/devices/virtual/dmi/id/sys_vendor", "/sys/class/dmi/id/sys_vendor", &host->sysVendor);

    //KVM/Qemu virtual machine
    if(ffStrbufStartsWithS(&host->productName, "Standard PC"))
        ffStrbufPrependS(&host->productName, "KVM/QEMU ");

    if(host->productFamily.length == 0 && host->productName.length == 0)
    {
        //On WSL, the real host can't be detected. Instead use WSL as host.
        if(getenv("WSL_DISTRO") != NULL || getenv("WSL_INTEROP") != NULL)
            ffStrbufAppendS(&host->productName, FF_HOST_PRODUCT_NAME_WSL);
        else if(getenv("MSYSTEM") != NULL && strcmp(getenv("MSYSTEM"), "MSYS") == 0)
            ffStrbufAppendS(&host->productName, FF_HOST_PRODUCT_NAME_MSYS);
    }
}
