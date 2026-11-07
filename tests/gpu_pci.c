// Tests for ffGPUParsePciIdsFile(), the pci.ids lookup that resolves a PCI GPU name on Linux and
// the BSDs.
//
// The parser takes the file content as a buffer instead of opening the file itself, which is
// exactly what makes it testable: a small hand written pci.ids excerpt is enough, and the
// expectations stay readable.
//
// The function is only compiled where libfastfetch ships the pci.ids based detection, so on other
// platforms this test has nothing to link against and only reports success.

#include "common/textModifier.h"
#include "detection/gpu/gpu.h"

#include <stdio.h>
#include <stdlib.h>

#if FF_GPU_HAS_PCI_IDS

static void verifyPciEx(
    const char* content, uint8_t subclass, uint16_t vendor, uint16_t device,
    const char* presetVendor, const char* presetName,
    const char* expectedVendor, const char* expectedName,
    int lineNo
) {
    FFGPUResult gpu = {};
    ffStrbufInit(&gpu.vendor);
    ffStrbufInit(&gpu.name);

    // A caller may already know the vendor (for example from the PCI subsystem), in which case the
    // parser must not clobber it.
    if (presetVendor) {
        ffStrbufSetS(&gpu.vendor, presetVendor);
    }
    if (presetName) {
        ffStrbufSetS(&gpu.name, presetName);
    }

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateS(content);
    ffGPUParsePciIdsFile(&buffer, subclass, vendor, device, &gpu);

    if (!ffStrbufEqualS(&gpu.vendor, expectedVendor) || !ffStrbufEqualS(&gpu.name, expectedName)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] ffGPUParsePciIdsFile(vendor=0x%04x, device=0x%04x, subclass=%u): expected vendor=\"%s\" name=\"%s\", got vendor=\"%s\" name=\"%s\"\n" FASTFETCH_TEXT_MODIFIER_RESET,
            lineNo, vendor, device, (unsigned) subclass, expectedVendor, expectedName, gpu.vendor.chars, gpu.name.chars);
        exit(1);
    }

    ffStrbufDestroy(&gpu.vendor);
    ffStrbufDestroy(&gpu.name);
}

#define VERIFY_PCI(content, subclass, vendor, device, expectedVendor, expectedName) \
    verifyPciEx((content), (subclass), (vendor), (device), nullptr, nullptr, (expectedVendor), (expectedName), __LINE__)

#define VERIFY_PCI_EX(content, subclass, vendor, device, presetVendor, presetName, expectedVendor, expectedName) \
    verifyPciEx((content), (subclass), (vendor), (device), (presetVendor), (presetName), (expectedVendor), (expectedName), __LINE__)

// A miniature pci.ids. Vendors are ordered, device ids grow within a block, and the block is closed
// by a blank line -- the same shape the real file has.
static const char PCI_IDS[] =
    "# pci.ids excerpt used by the tests\n"
    "#\n"
    "1a2b  Vendor Alpha\n"
    "\t0001  Alpha One\n"
    "\t0005  Alpha Five\n"
    "\t\t0005 0abc  Subsystem that must be ignored\n"
    "\t00ff  Alpha FF [Alias]\n"
    "\t1000  Alpha Thousand\n"
    "\n"
    "1a2c  Vendor Beta\n"
    "\t0001  Beta One\n"
    "\t0002  Beta Two\n"
    "\t1234  Beta TwelveThirtyFour\n"
    "\n"
    "abcd  Vendor Delta\n"
    "\t0e1f  Delta Device\n"
    "\n"
    "ffff  Vendor NoDevices\n"
    "\n"
    "C 00  Unclassified device\n";

// The last line has no terminating newline, so the parser has to fall back to the end of the
// buffer.
static const char PCI_IDS_NO_TRAILING_NL[] =
    "#\n"
    "1a2b  Vendor Alpha\n"
    "\t0001  Alpha One";

// A vendor with no device lines at all, again without a terminating newline.
static const char PCI_IDS_VENDOR_ONLY[] =
    "#\n"
    "1a2b  Vendor Alpha";

int main(void) {
    // An empty buffer is not an error: the name is synthesized from the vendor and device id.
    {
        VERIFY_PCI("", 0, 0x1a2b, 0x0001, "", "Unknown Device 0001 (VGA compatible)");
        VERIFY_PCI("", 2, 0x1a2b, 0x0001, "", "Unknown Device 0001 (3D)");
    }

    // The happy path: vendor and device are both present.
    {
        VERIFY_PCI(PCI_IDS, 2, 0x1a2b, 0x0001, "Vendor Alpha", "Alpha One");
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x0005, "Vendor Alpha", "Alpha Five");
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x1000, "Vendor Alpha", "Alpha Thousand");
    }

    // Hex letters in the ids work on both sides.
    {
        VERIFY_PCI(PCI_IDS, 0, 0xabcd, 0x0e1f, "Vendor Delta", "Delta Device");
    }

    // A name of the form `... [Alias]` is reduced to the alias. Only the trailing bracket pair is
    // considered, and a `[` without a matching `]` at the end is kept verbatim.
    {
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x00ff, "Vendor Alpha", "Alias");
    }

    // The lookup must stay inside the vendor block. `Beta` also has a 0x1234, but a lookup under
    // `Alpha` must not find it -- `Alpha` has no 0x1234 of its own.
    {
        VERIFY_PCI(PCI_IDS, 0, 0x1a2c, 0x1234, "Vendor Beta", "Beta TwelveThirtyFour");
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x1234, "Vendor Alpha", "Vendor Alpha Device 1234 (VGA compatible)");
    }

    // An unknown device under a known vendor synthesizes a name from the vendor.
    {
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x0002, "Vendor Alpha", "Vendor Alpha Device 0002 (VGA compatible)");
        VERIFY_PCI(PCI_IDS, 1, 0x1a2b, 0x0002, "Vendor Alpha", "Vendor Alpha Device 0002 (XGA compatible)");
        VERIFY_PCI(PCI_IDS, 2, 0x1a2b, 0x0002, "Vendor Alpha", "Vendor Alpha Device 0002 (3D)");
        VERIFY_PCI(PCI_IDS, 5, 0x1a2b, 0x0002, "Vendor Alpha", "Vendor Alpha Device 0002");
    }

    // The synthesized name uses the upper case form of the device id, while the lookup needle is
    // lower case.
    {
        VERIFY_PCI(PCI_IDS, 0, 0xabcd, 0xbeef, "Vendor Delta", "Vendor Delta Device BEEF (VGA compatible)");
    }

    // A device id below the first entry, between two entries and above the last one all miss. The
    // first case is also what the "stop once the ids grow past the target" shortcut relies on.
    {
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x0000, "Vendor Alpha", "Vendor Alpha Device 0000 (VGA compatible)");
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x0020, "Vendor Alpha", "Vendor Alpha Device 0020 (VGA compatible)");
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x9999, "Vendor Alpha", "Vendor Alpha Device 9999 (VGA compatible)");
    }

    // A subsystem id lives at twice the indentation and must never be mistaken for a device id,
    // even when it is the value being searched for.
    {
        VERIFY_PCI(PCI_IDS, 0, 0x1a2b, 0x0abc, "Vendor Alpha", "Vendor Alpha Device 0ABC (VGA compatible)");
    }

    // A vendor with no device lines falls back to the synthesized name, and the vendor is still
    // reported.
    {
        VERIFY_PCI(PCI_IDS, 0, 0xffff, 0x0001, "Vendor NoDevices", "Vendor NoDevices Device 0001 (VGA compatible)");
    }

    // An unknown vendor leaves the vendor buffer empty, so the synthesized name says "Unknown".
    {
        VERIFY_PCI(PCI_IDS, 0, 0x1234, 0x5678, "", "Unknown Device 5678 (VGA compatible)");
    }

    // Lines beyond the vendor section (the class list) are not reachable from a vendor block.
    {
        VERIFY_PCI(PCI_IDS, 0, 0x000c, 0x0000, "", "Unknown Device 0000 (VGA compatible)");
    }

    // A device line as the very last line of the buffer, with no trailing newline.
    {
        VERIFY_PCI(PCI_IDS_NO_TRAILING_NL, 0, 0x1a2b, 0x0001, "Vendor Alpha", "Alpha One");
        VERIFY_PCI(PCI_IDS_NO_TRAILING_NL, 0, 0x1a2b, 0x0002, "Vendor Alpha", "Vendor Alpha Device 0002 (VGA compatible)");
    }

    // A vendor line as the very last line of the buffer, with no device lines after it.
    {
        VERIFY_PCI(PCI_IDS_VENDOR_ONLY, 0, 0x1a2b, 0x0001, "Vendor Alpha", "Vendor Alpha Device 0001 (VGA compatible)");
    }

    // A known vendor is not overwritten, but resolving the device name still happens.
    {
        VERIFY_PCI_EX(PCI_IDS, 0, 0x1a2b, 0x0001, "Custom Vendor", nullptr, "Custom Vendor", "Alpha One");
    }

    // Likewise a name the caller already resolved is kept when the entry is a plain one.
    {
        VERIFY_PCI_EX(PCI_IDS, 0, 0x1a2b, 0x0001, nullptr, "Custom Name", "Vendor Alpha", "Custom Name");
    }

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}

#else // FF_TEST_HAS_PCI_IDS

int main(void) {
    puts("\033[32mTests skipped. (pci.ids parser not available on this platform)" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}

#endif
