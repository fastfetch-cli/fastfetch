#include "host.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

static void getHostProductName(FFstrbuf* name)
{
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_name", "/sys/class/dmi/id/product_name", name);
    if(name->length > 0)
        return;

    ffReadFileBuffer("/sys/firmware/devicetree/base/model", name);
    if(ffIsSmbiosValueSet(name))
        return;

    ffReadFileBuffer("/sys/firmware/devicetree/base/banner-name", name);
    if(ffIsSmbiosValueSet(name))
        return;

    //does a clear before the read
    ffReadFileBuffer("/tmp/sysinfo/model", name);
    if(ffIsSmbiosValueSet(name))
        return;

    ffStrbufClear(name);
}

const char* ffDetectHost(FFHostResult* host)
{
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_family", "/sys/class/dmi/id/product_family", &host->productFamily);
    getHostProductName(&host->productName);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_version", "/sys/class/dmi/id/product_version", &host->productVersion);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_sku", "/sys/class/dmi/id/product_sku", &host->productSku);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_serial", "/sys/class/dmi/id/product_serial", &host->productSerial);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_uuid", "/sys/class/dmi/id/product_uuid", &host->productUuid);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/sys_vendor", "/sys/class/dmi/id/sys_vendor", &host->sysVendor);

    //KVM/Qemu virtual machine
    if(ffStrbufStartsWithS(&host->productName, "Standard PC"))
        ffStrbufPrependS(&host->productName, "KVM/QEMU ");

    if(host->productFamily.length == 0 && host->productName.length == 0)
    {
        const char* wslDistroName = getenv("WSL_DISTRO_NAME");
        //On WSL, the real host can't be detected. Instead use WSL as host.
        if(wslDistroName != NULL || getenv("WSL_DISTRO") != NULL || getenv("WSL_INTEROP") != NULL)
        {
            ffStrbufAppendS(&host->productName, "Windows Subsystem for Linux");
            if (wslDistroName)
                ffStrbufAppendF(&host->productName, " - %s", wslDistroName);
            ffStrbufAppendS(&host->productFamily, "WSL");

            FF_STRBUF_AUTO_DESTROY wslVer = ffStrbufCreate(); //Wide characters
            if(!ffProcessAppendStdOut(&wslVer, (char* const[]){
                "wsl.exe",
                "--version",
                NULL
            }) && wslVer.length > 0)
            {
                ffStrbufSubstrBeforeFirstC(&wslVer, '\r'); //CRLF
                ffStrbufSubstrAfterLastC(&wslVer, ' ');
                for(uint32_t i = 0; i < wslVer.length; ++i) {
                    if(wslVer.chars[i]) //don't append \0
                        ffStrbufAppendC(&host->productVersion, wslVer.chars[i]);
                }
            }
        }
    }

    return NULL;
}
