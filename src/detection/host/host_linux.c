#include "host.h"
#include "common/io.h"
#include "common/processing.h"
#include "common/smbios.h"

#include <stdlib.h>

static bool getHostProductName(FFstrbuf* name) {
    if (ffReadFileBuffer("/sys/firmware/devicetree/base/model", name) ||
        ffReadFileBuffer("/sys/firmware/devicetree/base/banner-name", name)) {
        ffStrbufTrimRight(name, '\0');
        return true;
    }

    if (ffReadFileBuffer("/tmp/sysinfo/model", name)) {
        ffStrbufTrimRightSpace(name);
        ffStrbufTrimRight(name, '\0');
        if (ffIsSmbiosValueSet(name)) {
            return true;
        }
    }

    return false;
}

static bool getHostSerialNumber(FFstrbuf* serial) {
    if (ffReadFileBuffer("/sys/firmware/devicetree/base/smbios/smbios/system/serial", serial) ||
        ffReadFileBuffer("/sys/firmware/devicetree/base/serial-number", serial)) {
        ffStrbufTrimRight(serial, '\0');
        return true;
    }
    return false;
}

static bool getHostProductFamily(FFstrbuf* family) {
    if (ffReadFileBuffer("/sys/firmware/devicetree/base/smbios/smbios/system/family", family) ||
        ffReadFileBuffer("/sys/firmware/devicetree/base/smbios/smbios/system/product", family)) {
        ffStrbufTrimRight(family, '\0');
        return true;
    }
    return false;
}

static bool getHostVendor(FFstrbuf* vendor) {
    if (ffReadFileBuffer("/sys/firmware/devicetree/base/smbios/smbios/system/manufacturer", vendor)) {
        ffStrbufTrimRight(vendor, '\0');
        return true;
    }
    return false;
}

const char* ffDetectHost(FFHostResult* host) {
    // This is a hack for Asahi Linux, whose product_family is empty
    bool productName = ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_name", "/sys/class/dmi/id/product_name", &host->name);
    bool productFamily = ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_family", "/sys/class/dmi/id/product_family", &host->family);
    if (productName || productFamily) {
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_version", "/sys/class/dmi/id/product_version", &host->version);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_sku", "/sys/class/dmi/id/product_sku", &host->sku);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/product_serial", "/sys/class/dmi/id/product_serial", &host->serial);
        ffGetSmbiosValue("/sys/devices/virtual/dmi/id/sys_vendor", "/sys/class/dmi/id/sys_vendor", &host->vendor);

#if __x86_64__
        ffHostDetectMac(host);
#endif

        // KVM/Qemu virtual machine
        if (ffStrbufStartsWithS(&host->name, "Standard PC")) {
            ffStrbufPrependS(&host->name, "KVM/QEMU ");
        }

#if __aarch64__
        else if (host->family.length == 0 && ffStrbufEqualS(&host->vendor, "Apple Inc.") && ffStrbufStartsWithS(&host->name, "Mac")) {
            // Hack for Asahi Linux
            ffStrbufDestroy(&host->family);
            ffStrbufInitMove(&host->family, &host->name);
            getHostProductName(&host->name);
            getHostSerialNumber(&host->serial);
        }
#endif
    } else {
        getHostProductFamily(&host->family);
        getHostProductName(&host->name);
        getHostSerialNumber(&host->serial);
        getHostVendor(&host->vendor);
    }

    if (host->family.length == 0 && host->name.length == 0) {
        const char* wslDistroName = getenv("WSL_DISTRO_NAME");
        // On WSL, the real host can't be detected. Instead use WSL as host.
        if (wslDistroName != NULL || getenv("WSL_DISTRO") != NULL || getenv("WSL_INTEROP") != NULL) {
            ffStrbufSetStatic(&host->name, "Windows Subsystem for Linux");
            if (wslDistroName) {
                ffStrbufAppendF(&host->name, " - %s", wslDistroName);
            }
            ffStrbufSetStatic(&host->family, "WSL");
            ffStrbufSetStatic(&host->vendor, "Microsoft Corporation");

            if (instance.config.general.detectVersion) {
                ffProcessAppendStdOut(&host->version, (char* const[]) {
                                                          "wslinfo",
                                                          "--wsl-version",
                                                          "-n",
                                                          NULL,
                                                      }); // supported in 2.2.3 and later
            }
        } else if (ffStrbufStartsWithS(&instance.state.platform.sysinfo.version, "FreeBSD ")) {
            ffStrbufSetStatic(&host->name, "Linux Binary Compatibility on FreeBSD");
            ffStrbufSetStatic(&host->family, "FreeBSD");
            ffStrbufSetStatic(&host->vendor, "FreeBSD Foundation");
            if (instance.config.general.detectVersion) {
                ffStrbufSetS(&host->version, instance.state.platform.sysinfo.version.chars + strlen("FreeBSD "));
                ffStrbufSubstrBeforeFirstC(&host->version, ' ');
            }
        }
    }

    return NULL;
}
