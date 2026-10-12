#include "localip.h"
#include "common/apple/cf_helpers.h"
#include "common/debug.h"

#include <IOKit/IOBSD.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IONetworkController.h>
#include <ifaddrs.h>
#include <net/if.h>

// macOS 27 hides the link-layer address from every process that is not an Apple platform
// binary, i.e. anything whose code signature has no `Platform identifier` field -- which
// includes ad-hoc and linker-signed builds. Both getifaddrs() and sysctl(NET_RT_IFLIST)
// then report the placeholder 02:00:00:00:00:00 for all interfaces. Re-signing the very
// same /sbin/ifconfig ad-hoc flips its `ether` line from the real address to the
// placeholder while every other field stays byte-identical, so the placeholder itself is
// the tell.
//
// The IORegistry is not affected. The node matching the BSD name never carries the
// property itself and the depth at which it appears depends on the driver -- en0 has it on
// the parent (IO80211Controller), en4 on the parent (AppleUSBDeviceNCM11Data), ap1 on the
// node itself -- so walk up until it shows up.
//
// This reports the *hardware* address. That differs from the address a platform binary
// sees whenever Private Wi-Fi Address is enabled (for en0 the randomized address sits one
// level further up, on IOUserNetworkWLAN), so it is only used to replace the placeholder
// and never preferred over a value read from the interface.
//
// Measured with `fastfetch -s localip --stat` on this machine: replacing the placeholders
// costs ~0.2ms for a single interface and ~1.3ms for the 21 interfaces getifaddrs()
// reports. The SystemConfiguration alternative needed a single SCNetworkInterfaceCopyAll()
// at 2.6-5.2ms (an XPC round trip to configd) and does not even list anpi0-2 or ap1, so the
// IORegistry covers strictly more interfaces for less.
#define FF_LOCALIP_REDACTED_MAC "02:00:00:00:00:00"

// The classes that expose a `BSD Name` in the IORegistry. anpi*/en* are IONetworkInterface
// subclasses, but the dext-based Wi-Fi interfaces (ap1) are IOUserNetworkWLAN and are not,
// so they need a pass of their own.
static const char* const iokitInterfaceClasses[] = {
    "IONetworkInterface",
    "IOUserNetworkWLAN",
};

// Looks for the first IOMACAddress on `entry` or one of its ancestors. Does not consume it.
static bool getMacFromRegistry(io_object_t entry, FFstrbuf* result) {
    if (!entry) {
        return false;
    }

    FF_IOOBJECT_AUTO_RELEASE io_object_t current = entry;
    IOObjectRetain(current);

    for (uint32_t depth = 0; depth < 4; ++depth) {
        FF_CFTYPE_AUTO_RELEASE CFDataRef data = (CFDataRef) IORegistryEntryCreateCFProperty(current, CFSTR(kIOMACAddress), kCFAllocatorDefault, kNilOptions);
        if (data) {
            if (CFGetTypeID(data) != CFDataGetTypeID() || CFDataGetLength(data) < 6) {
                return false;
            }
            const uint8_t* ptr = CFDataGetBytePtr(data);
            ffStrbufSetF(result, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
            return true;
        }

        io_object_t parent = IO_OBJECT_NULL;
        if (IORegistryEntryGetParentEntry(current, kIOServicePlane, &parent) != KERN_SUCCESS) {
            return false;
        }
        IOObjectRelease(current);
        current = parent;
    }

    return false;
}

static FFLocalIpResult* findRedacted(FFlist* results, const char* ifName) {
    FF_LIST_FOR_EACH (FFLocalIpResult, item, *results) {
        if (ffStrbufEqualS(&item->mac, FF_LOCALIP_REDACTED_MAC) && ffStrbufEqualS(&item->name, ifName)) {
            return item;
        }
    }
    return nullptr;
}

// Enumerating the interface classes once and matching on `BSD Name` is measurably cheaper
// than looking every interface up by name -- 1.5ms versus 3.0ms for the 21 interfaces
// getifaddrs() reports here -- which matters as soon as defaultRouteOnly is disabled and a
// dozen interfaces are listed at once. Interfaces the enumeration does not reach are looked
// up by name afterwards, so this can only ever be faster than, never worse than, the direct
// lookup.
void ffLocalIpFixRedactedMacs(FFlist* results) {
    bool hasRedacted = false;
    FF_LIST_FOR_EACH (FFLocalIpResult, item, *results) {
        if (ffStrbufEqualS(&item->mac, FF_LOCALIP_REDACTED_MAC)) {
            hasRedacted = true;
            break;
        }
    }
    if (!hasRedacted) {
        return;
    }

    for (uint32_t c = 0; c < ARRAY_SIZE(iokitInterfaceClasses); ++c) {
        FF_IOOBJECT_AUTO_RELEASE io_iterator_t iterator = IO_OBJECT_NULL;
        if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching(iokitInterfaceClasses[c]), &iterator) != KERN_SUCCESS) {
            continue;
        }

        io_object_t entry;
        while ((entry = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
            FF_IOOBJECT_AUTO_RELEASE io_object_t entryRef = entry;

            FF_CFTYPE_AUTO_RELEASE CFStringRef bsdName = (CFStringRef) IORegistryEntryCreateCFProperty(entryRef, CFSTR(kIOBSDNameKey), kCFAllocatorDefault, kNilOptions);
            if (!bsdName) {
                continue;
            }

            char ifName[IFNAMSIZ];
            if (CFGetTypeID(bsdName) != CFStringGetTypeID() || !CFStringGetCString(bsdName, ifName, sizeof(ifName), kCFStringEncodingUTF8)) {
                continue;
            }

            FFLocalIpResult* item = findRedacted(results, ifName);
            if (item && getMacFromRegistry(entryRef, &item->mac)) {
                FF_DEBUG("Replaced the redacted MAC address for interface %s", ifName);
            }
        }
    }

    FF_LIST_FOR_EACH (FFLocalIpResult, item, *results) {
        if (!ffStrbufEqualS(&item->mac, FF_LOCALIP_REDACTED_MAC)) {
            continue;
        }

        FF_IOOBJECT_AUTO_RELEASE io_object_t service = IOServiceGetMatchingService(MACH_PORT_NULL, IOBSDNameMatching(MACH_PORT_NULL, 0, item->name.chars));
        if (service && getMacFromRegistry(service, &item->mac)) {
            FF_DEBUG("Replaced the redacted MAC address for interface %s", item->name.chars);
        } else {
            FF_DEBUG("No IOMACAddress in the IORegistry for interface %s", item->name.chars);
        }
    }
}
