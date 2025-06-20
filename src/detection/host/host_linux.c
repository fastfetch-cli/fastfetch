#include "host.h"
#include "common/io/io.h"
#include "common/processing.h"
#include "util/smbiosHelper.h"

#include <stdlib.h>

static void getHostProductName(FFstrbuf* name)
{
    if (ffReadFileBuffer("/sys/firmware/devicetree/base/model", name))
    {
        ffStrbufTrimRightSpace(name);
        ffStrbufTrimRight(name, '\0');
        if(ffIsSmbiosValueSet(name))
            return;
    }

    if (ffReadFileBuffer("/sys/firmware/devicetree/base/banner-name", name))
    {
        ffStrbufTrimRightSpace(name);
        ffStrbufTrimRight(name, '\0');
        if(ffIsSmbiosValueSet(name))
            return;
    }

    if (ffReadFileBuffer("/tmp/sysinfo/model", name))
    {
        ffStrbufTrimRightSpace(name);
        ffStrbufTrimRight(name, '\0');
        if(ffIsSmbiosValueSet(name))
            return;
    }

    ffStrbufClear(name);
}

static void getHostSerialNumber(FFstrbuf* serial)
{
    if (ffReadFileBuffer("/sys/firmware/devicetree/base/serial-number", serial))
    {
        ffStrbufTrimRightSpace(serial);
        ffStrbufTrimRight(serial, '\0');
        if(ffIsSmbiosValueSet(serial))
            return;
    }

    ffStrbufClear(serial);
}

const char* ffDetectHost(FFHostResult* host)
{
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_family", "/sys/class/dmi/id/product_family", &host->family);
    if (!ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_name", "/sys/class/dmi/id/product_name", &host->name))
        getHostProductName(&host->name);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_version", "/sys/class/dmi/id/product_version", &host->version);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_sku", "/sys/class/dmi/id/product_sku", &host->sku);
    if (!ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_serial", "/sys/class/dmi/id/product_serial", &host->serial))
        getHostSerialNumber(&host->serial);
    ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_uuid", "/sys/class/dmi/id/product_uuid", &host->uuid);
    if (!ffGetSmbiosValue("/sys/devices/virtual/dmi/id/sys_vendor", "/sys/class/dmi/id/sys_vendor", &host->vendor))
    {
        if (ffStrbufStartsWithS(&host->name, "Apple "))
            ffStrbufSetStatic(&host->vendor, "Apple Inc.");
    }

    #ifdef __x86_64__
    ffHostDetectMac(host);
    #endif

    //KVM/Qemu virtual machine
    if(ffStrbufStartsWithS(&host->name, "Standard PC"))
        ffStrbufPrependS(&host->name, "KVM/QEMU ");

    if(host->family.length == 0 && host->name.length == 0)
    {
        const char* wslDistroName = getenv("WSL_DISTRO_NAME");
        //On WSL, the real host can't be detected. Instead use WSL as host.
        if(wslDistroName != NULL || getenv("WSL_DISTRO") != NULL || getenv("WSL_INTEROP") != NULL)
        {
            ffStrbufSetStatic(&host->name, "Windows Subsystem for Linux");
            if (wslDistroName)
                ffStrbufAppendF(&host->name, " - %s", wslDistroName);
            ffStrbufSetStatic(&host->family, "WSL");
            ffStrbufSetStatic(&host->vendor, "Microsoft Corporation");

            if (instance.config.general.detectVersion)
            {
                ffProcessAppendStdOut(&host->version, (char* const[]){
                    "wslinfo",
                    "--wsl-version",
                    "-n",
                    NULL,
                }); // supported in 2.2.3 and later
            }
        }
        else if (ffStrbufStartsWithS(&instance.state.platform.sysinfo.version, "FreeBSD "))
        {
            ffStrbufSetStatic(&host->name, "Linux Binary Compatibility on FreeBSD");
            ffStrbufSetStatic(&host->family, "FreeBSD");
            ffStrbufSetStatic(&host->vendor, "FreeBSD Foundation");
            if (instance.config.general.detectVersion)
            {
                ffStrbufSetS(&host->version, instance.state.platform.sysinfo.version.chars + strlen("FreeBSD "));
                ffStrbufSubstrBeforeFirstC(&host->version, ' ');
            }
        }
    }

    return NULL;
}
