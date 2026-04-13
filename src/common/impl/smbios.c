#include "common/smbios.h"
#include "common/io.h"
#include "common/unused.h"
#include "common/mallocHelper.h"
#include "common/debug.h"

bool ffIsSmbiosValueSet(FFstrbuf* value) {
    ffStrbufTrimRightSpace(value);
    return value->length > 0 &&
        !ffStrbufStartsWithIgnCaseS(value, "To be filled") &&
        !ffStrbufStartsWithIgnCaseS(value, "To be set") &&
        !ffStrbufStartsWithIgnCaseS(value, "OEM") &&
        !ffStrbufStartsWithIgnCaseS(value, "O.E.M.") &&
        !ffStrbufStartsWithIgnCaseS(value, "System Product") &&
        !ffStrbufStartsWithIgnCaseS(value, "Unknown Product") &&
        !ffStrbufIgnCaseEqualS(value, "None") &&
        !ffStrbufIgnCaseEqualS(value, "System Name") &&
        !ffStrbufIgnCaseEqualS(value, "System Version") &&
        !ffStrbufIgnCaseEqualS(value, "System SKU#") &&
        !ffStrbufIgnCaseEqualS(value, "Default string") &&
        !ffStrbufIgnCaseEqualS(value, "Undefined") &&
        !ffStrbufIgnCaseEqualS(value, "Not Specified") &&
        !ffStrbufIgnCaseEqualS(value, "Not Applicable") &&
        !ffStrbufIgnCaseEqualS(value, "Not Defined") &&
        !ffStrbufIgnCaseEqualS(value, "Not Available") &&
        !ffStrbufIgnCaseEqualS(value, "INVALID") &&
        !ffStrbufIgnCaseEqualS(value, "Type1ProductConfigId") &&
        !ffStrbufIgnCaseEqualS(value, "TBD by OEM") &&
        !ffStrbufIgnCaseEqualS(value, "No Enclosure") &&
        !ffStrbufIgnCaseEqualS(value, "Chassis Version") &&
        !ffStrbufIgnCaseEqualS(value, "All Series") &&
        !ffStrbufIgnCaseEqualS(value, "N/A") &&
        !ffStrbufIgnCaseEqualS(value, "Unknown") &&
        !ffStrbufIgnCaseEqualS(value, "Standard") && ({
            // Some SMBIOS implementations use "0x0000" to indicate an unset value, even for strings.
            bool zero = ffStrbufStartsWithS(value, "0x0");
            if (zero) {
                for (size_t i = 2; i < value->length; i++) {
                    char c = value->chars[i];
                    if (c != '0') {
                        zero = false;
                        break;
                    }
                }
            }
            !zero;
        });
}

static bool smbiosTableInitialized = false;
static FFSmbiosHeaderTable smbiosTable;

const FFSmbiosHeader* ffSmbiosNextEntry(const FFSmbiosHeader* header) {
    const char* p = ((const char*) header) + header->Length;
    if (*p) {
        do {
            p += strlen(p) + 1;
        } while (*p);
    } else { // The terminator is always double 0 even if there is no string
        p++;
    }

    return (const FFSmbiosHeader*) (p + 1);
}

static bool parseSmbiosTable(const uint8_t* data, uint32_t length) {
    const FFSmbiosHeader* endOfTable = NULL;

    FF_DEBUG("Parsing SMBIOS table structures with length %u bytes", length);
    FF_A_UNUSED int structureCount = 0, totalCount = 0;
    for (
        const FFSmbiosHeader* header = (const FFSmbiosHeader*) data;
        (const uint8_t*) header + sizeof(FFSmbiosHeader) < (const uint8_t*) data + length;
        header = ffSmbiosNextEntry(header)) {
        ++totalCount;
        endOfTable = header;

        // This doesn't verify the entire structure (e.g. string section can still be truncated),
        // but at least ensures the formatted section is valid and prevents infinite loops
        // when the table is severely malformed.
        if (__builtin_expect((const uint8_t*) header + header->Length > (const uint8_t*) data + length, false)) {
            FF_DEBUG("Truncated SMBIOS structure at offset 0x%lx: length %u is too small",
                (unsigned long) ((const uint8_t*) header - data),
                header->Length);
            break;
        }

        if (header->Type < FF_SMBIOS_TYPE_END_OF_TABLE) {
            if (!smbiosTable[header->Type]) {
                smbiosTable[header->Type] = header;
                FF_DEBUG("Found SMBIOS structure type %u, handle 0x%04X, length %u",
                    header->Type,
                    header->Handle,
                    header->Length);
                structureCount++;
            } else {
                FF_DEBUG("Duplicate SMBIOS structure type %u, handle 0x%04X, length %u",
                    header->Type,
                    header->Handle,
                    header->Length);
            }
        } else if (header->Type == FF_SMBIOS_TYPE_END_OF_TABLE) {
            FF_DEBUG("Reached SMBIOS end of type %u, handle 0x%04X, length %u",
                header->Type,
                header->Handle,
                header->Length);
            break;
        } else {
            FF_DEBUG("Found custom SMBIOS structure type %u, handle 0x%04X, length %u; ignoring",
                header->Type,
                header->Handle,
                header->Length);
        }
    }

    if (!endOfTable) {
        FF_DEBUG("No SMBIOS structures found in table");
        return false;
    }

    FF_DEBUG("Parsed %d/%d SMBIOS structures, end-of-table (Type 127) %s",
        structureCount,
        totalCount,
        endOfTable->Type == FF_SMBIOS_TYPE_END_OF_TABLE ? "found." : "not found! SMBIOS data may be malformed.");
    smbiosTable[FF_SMBIOS_TYPE_END_OF_TABLE] = endOfTable;

    return true;
}

#if defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__sun) || defined(__HAIKU__) || defined(__OpenBSD__) || defined(__GNU__)
#    include <fcntl.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <sys/mman.h>
#    include <stddef.h>

#    ifdef __linux__
#        include "common/properties.h"
#    elif defined(__FreeBSD__)
#        include "common/settings.h"
#        define loff_t off_t // FreeBSD doesn't have loff_t
#    elif defined(__sun)
#        define loff_t off_t
#    elif defined(__NetBSD__)
#        include "common/sysctl.h"
#        define loff_t off_t
#    endif

#    ifdef __linux__
bool ffGetSmbiosValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer) {
    // /sys/class/dmi/id/* are all pseudo-files with very small content
    // so reading the whole file at once is efficient
    ffStrbufEnsureFixedLengthFree(buffer, 127);

    ssize_t len = ffReadFileData(devicesPath, buffer->allocated - 1, buffer->chars);
    if (len > 0) {
        assert(len < buffer->allocated);
        buffer->chars[len] = '\0';
        buffer->length = (uint32_t) len;
        ffStrbufTrimRightSpace(buffer);
        if (ffIsSmbiosValueSet(buffer)) {
            return true;
        }
    }

    len = ffReadFileData(classPath, buffer->allocated - 1, buffer->chars);
    if (len > 0) {
        assert(len < buffer->allocated);
        buffer->chars[len] = '\0';
        buffer->length = (uint32_t) len;
        ffStrbufTrimRightSpace(buffer);
        if (ffIsSmbiosValueSet(buffer)) {
            return true;
        }
    }

    ffStrbufClear(buffer);
    return false;
}
#    endif

typedef struct FFSmbios20EntryPoint {
    uint8_t AnchorString[4];
    uint8_t EntryPointStructureChecksum;
    uint8_t EntryPointLength;
    uint8_t SmbiosMajorVersion;
    uint8_t SmbiosMinorVersion;
    uint16_t MaximumStructureSize;
    uint8_t EntryPointRevision;
    uint8_t FormattedArea[5];
    uint8_t IntermediateAnchorString[5];
    uint8_t IntermediateChecksum;
    uint16_t StructureTableLength;
    uint32_t StructureTableAddress;
    uint16_t NumberOfSmbiosStructures;
    uint8_t SmbiosBcdRevision;
} FF_A_PACKED FFSmbios20EntryPoint;
static_assert(offsetof(FFSmbios20EntryPoint, SmbiosBcdRevision) == 0x1E,
    "FFSmbios20EntryPoint: Wrong struct alignment");

typedef struct FFSmbios30EntryPoint {
    uint8_t AnchorString[5];
    uint8_t EntryPointStructureChecksum;
    uint8_t EntryPointLength;
    uint8_t SmbiosMajorVersion;
    uint8_t SmbiosMinorVersion;
    uint8_t SmbiosDocrev;
    uint8_t EntryPointRevision;
    uint8_t Reversed;
    uint32_t StructureTableMaximumSize;
    uint64_t StructureTableAddress;
} FF_A_PACKED FFSmbios30EntryPoint;

static_assert(offsetof(FFSmbios30EntryPoint, StructureTableAddress) == 0x10,
    "FFSmbios30EntryPoint: Wrong struct alignment");

typedef union FFSmbiosEntryPoint {
    FFSmbios20EntryPoint Smbios20;
    FFSmbios30EntryPoint Smbios30;
} FFSmbiosEntryPoint;

const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable() {
    static FFstrbuf buffer;

    if (!smbiosTableInitialized) {
        smbiosTableInitialized = true;
        FF_DEBUG("Initializing SMBIOS buffer");
        ffStrbufInit(&buffer);
#    if !__HAIKU__ && !__GNU__
#        ifdef __linux__
        FF_DEBUG("Using Linux implementation - trying /sys/firmware/dmi/tables/DMI");
        if (!ffAppendFileBuffer("/sys/firmware/dmi/tables/DMI", &buffer))
#        elif defined(__OpenBSD__)
        {
                FF_DEBUG("Using OpenBSD /var/run/dmesg.boot implementation");
                char dmesg[8192];
                ssize_t size =  ffReadFileData("/var/run/dmesg.boot", sizeof(dmesg), dmesg);
                if (size <= 0) {
                    goto fallback;
                }
                char* line = memmem(dmesg, sizeof(dmesg), "\nbios0 at mainbios0: SMBIOS rev. ", strlen("\nbios0 at mainbios0: SMBIOS rev. "));
                if (!line) {
                    goto fallback;
                }
                line += strlen("\nbios0 at mainbios0: SMBIOS rev. ");

        }
#        endif
        {
#        if !defined(__sun) && !defined(__NetBSD__)
            FF_DEBUG("Using memory-mapped implementation");
            FF_STRBUF_AUTO_DESTROY strEntryAddress = ffStrbufCreate();
#            ifdef __FreeBSD__
            FF_DEBUG("Using FreeBSD kenv implementation");
            if (!ffSettingsGetFreeBSDKenv("hint.smbios.0.mem", &strEntryAddress)) {
                FF_DEBUG("Failed to get SMBIOS address from FreeBSD kenv");
                goto fallback; // For non-UEFI systems
            }
            FF_DEBUG("Got SMBIOS address from kenv: %s", strEntryAddress.chars);
#            elif defined(__linux__)
            {
                FF_DEBUG("Using Linux EFI systab implementation");
                FF_STRBUF_AUTO_DESTROY systab = ffStrbufCreate();
                if (!ffAppendFileBuffer("/sys/firmware/efi/systab", &systab)) {
                    FF_DEBUG("Failed to read /sys/firmware/efi/systab");
                    return NULL;
                }
                if (!ffParsePropLines(systab.chars, "SMBIOS3=", &strEntryAddress) &&
                    !ffParsePropLines(systab.chars, "SMBIOS=", &strEntryAddress)) {
                    FF_DEBUG("Failed to find SMBIOS entry in systab");
                    return NULL;
                }
                FF_DEBUG("Found SMBIOS entry in systab: %s", strEntryAddress.chars);
            }
#            endif

            loff_t entryAddress = (loff_t) strtol(strEntryAddress.chars, NULL, 16);
            if (entryAddress == 0) {
                FF_DEBUG("Invalid SMBIOS entry address: 0");
                return NULL;
            }
            FF_DEBUG("Parsed SMBIOS entry address: 0x%lx", (unsigned long) entryAddress);

            FF_AUTO_CLOSE_FD int fd = open("/dev/mem", O_RDONLY | O_CLOEXEC);
            if (fd < 0) {
                FF_DEBUG("Failed to open /dev/mem: %s", strerror(errno));
                return NULL;
            }
            FF_DEBUG("/dev/mem opened successfully with fd=%d", fd);

            FFSmbiosEntryPoint entryPoint;
            FF_DEBUG("Attempting to read %zu bytes from physical address 0x%lx",
                sizeof(entryPoint),
                (unsigned long) entryAddress);

            if (pread(fd, &entryPoint, sizeof(entryPoint), entryAddress) < 0x10) {
                FF_DEBUG("pread failed: %s. Trying mmap", strerror(errno));
                // `pread /dev/mem` returns EFAULT in FreeBSD
                // https://stackoverflow.com/questions/69372330/how-to-read-dev-mem-using-read
                void* p = mmap(NULL, sizeof(entryPoint), PROT_READ, MAP_SHARED, fd, entryAddress);
                if (p == MAP_FAILED) {
                    FF_DEBUG("mmap failed: %s", strerror(errno));
                    return NULL;
                }
                memcpy(&entryPoint, p, sizeof(entryPoint));
                munmap(p, sizeof(entryPoint));
                FF_DEBUG("Successfully read entry point data via mmap");
            } else {
                FF_DEBUG("Successfully read entry point data via pread");
            }
#        else
            // Sun or NetBSD
            FF_DEBUG("Using %s specific implementation",
#            ifdef __NetBSD__
                "NetBSD"
#            else
                "SunOS"
#            endif
            );

            FF_AUTO_CLOSE_FD int fd = open("/dev/smbios", O_RDONLY | O_CLOEXEC);
            if (fd < 0) {
                FF_DEBUG("Failed to open /dev/smbios: %s", strerror(errno));
                return NULL;
            }
            FF_DEBUG("/dev/smbios opened successfully with fd=%d", fd);

            FFSmbiosEntryPoint entryPoint;
#            ifdef __NetBSD__
            off_t addr = (off_t) ffSysctlGetInt64("machdep.smbios", 0);
            if (addr == 0) {
                FF_DEBUG("Failed to get SMBIOS address from sysctl");
                return NULL;
            }
            FF_DEBUG("Got SMBIOS address from sysctl: 0x%lx", (unsigned long) addr);

            if (pread(fd, &entryPoint, sizeof(entryPoint), addr) < 1) {
                FF_DEBUG("Failed to read SMBIOS entry point: %s", strerror(errno));
                return NULL;
            }
            FF_DEBUG("Successfully read SMBIOS entry point");
#            else
            FF_DEBUG("Reading SMBIOS entry point from /dev/smbios");
            if (ffReadFDData(fd, sizeof(entryPoint), &entryPoint) < 1) {
                FF_DEBUG("Failed to read SMBIOS entry point: %s", strerror(errno));
                return NULL;
            }
            FF_DEBUG("Successfully read SMBIOS entry point");
#            endif
#        endif

            uint32_t tableLength = 0;
            loff_t tableAddress = 0;
            if (memcmp(entryPoint.Smbios20.AnchorString, "_SM_", sizeof(entryPoint.Smbios20.AnchorString)) == 0) {
                FF_DEBUG("Found SMBIOS 2.0 entry point");
                if (entryPoint.Smbios20.EntryPointLength != sizeof(entryPoint.Smbios20)) {
                    FF_DEBUG("Invalid SMBIOS 2.0 entry point length: %u (expected %zu)",
                        entryPoint.Smbios20.EntryPointLength,
                        sizeof(entryPoint.Smbios20));
                    return NULL;
                }
                tableLength = entryPoint.Smbios20.StructureTableLength;
                tableAddress = (loff_t) entryPoint.Smbios20.StructureTableAddress;
                FF_DEBUG("SMBIOS 2.0: tableLength=0x%x, tableAddress=0x%lx, version=%u.%u",
                    tableLength,
                    (unsigned long) tableAddress,
                    entryPoint.Smbios20.SmbiosMajorVersion,
                    entryPoint.Smbios20.SmbiosMinorVersion);
            } else if (memcmp(entryPoint.Smbios30.AnchorString, "_SM3_", sizeof(entryPoint.Smbios30.AnchorString)) == 0) {
                FF_DEBUG("Found SMBIOS 3.0 entry point");
                if (entryPoint.Smbios30.EntryPointLength != sizeof(entryPoint.Smbios30)) {
                    FF_DEBUG("Invalid SMBIOS 3.0 entry point length: %u (expected %zu)",
                        entryPoint.Smbios30.EntryPointLength,
                        sizeof(entryPoint.Smbios30));
                    return NULL;
                }
                tableLength = entryPoint.Smbios30.StructureTableMaximumSize;
                tableAddress = (loff_t) entryPoint.Smbios30.StructureTableAddress;
                FF_DEBUG("SMBIOS 3.0: tableLength=0x%x, tableAddress=0x%lx, version=%u.%u.%u",
                    tableLength,
                    (unsigned long) tableAddress,
                    entryPoint.Smbios30.SmbiosMajorVersion,
                    entryPoint.Smbios30.SmbiosMinorVersion,
                    entryPoint.Smbios30.SmbiosDocrev);
            } else {
                FF_DEBUG("Unknown SMBIOS entry point format");
                goto fallback; // Dragonfly goes here
            }

            ffStrbufEnsureFixedLengthFree(&buffer, tableLength);
            FF_DEBUG("Attempting to read SMBIOS table data: %u bytes at 0x%lx", tableLength, (unsigned long) tableAddress);
            if (pread(fd, buffer.chars, tableLength, tableAddress) == (ssize_t) tableLength) {
                buffer.length = tableLength;
                buffer.chars[buffer.length] = '\0';
                FF_DEBUG("Successfully read SMBIOS table data: %u bytes", tableLength);
            } else {
                FF_DEBUG("pread failed, trying mmap");
                // entryPoint.StructureTableAddress must be page aligned.
                // Unaligned physical memory access results in all kinds of crashes.
                void* p = mmap(NULL, tableLength, PROT_READ, MAP_SHARED, fd, tableAddress);
                if (p == MAP_FAILED) {
                    FF_DEBUG("mmap failed: %s", strerror(errno));
                    ffStrbufDestroy(&buffer); // free buffer and reset state
                    return NULL;
                }
                ffStrbufSetNS(&buffer, tableLength, (char*) p);
                munmap(p, tableLength);
                FF_DEBUG("Successfully read SMBIOS table data via mmap: %u bytes", tableLength);
            }
        }

        return NULL;
#    endif

    fallback:
        if (buffer.length == 0) {
            const char* devMem =
#        if __HAIKU__
                "/dev/misc/mem";
#        else
                "/dev/mem"; // kern.securelevel must be -1
#        endif
            FF_DEBUG("Using physical memory searching implementation: %s", devMem);

            uint32_t tableLength = 0;
            off_t tableAddress = 0;
            FF_AUTO_CLOSE_FD int fd = open(devMem, O_RDONLY | O_CLOEXEC);
            if (fd < 0) {
                FF_DEBUG("Failed to open memory device: %s", strerror(errno));
                return NULL;
            }
            FF_DEBUG("Memory device opened successfully with fd=%d", fd);

            // Works on legacy BIOS only
            // See: https://wiki.osdev.org/System_Management_BIOS#UEFI_systems
            // On BSD systems, we can get EFI system resource table (ESRT) via EFIIOC_GET_TABLE
            // However, to acquire SMBIOS entry point, we need EFI configuration table (provided by EFI system table)
            // which is not available via EFIIOC_GET_TABLE.
            FF_AUTO_FREE uint8_t* smBiosBase = malloc(0x10000);
            if (pread(fd, smBiosBase, 0x10000, 0xF0000) != 0x10000) {
                FF_DEBUG("Failed to read SMBIOS memory region: %s", strerror(errno));
                return NULL;
            }
            FF_DEBUG("Successfully read 0x10000 bytes from physical address 0xF0000");

            for (off_t offset = 0; offset <= 0xffe0; offset += 0x10) {
                FFSmbiosEntryPoint* p = (void*) (smBiosBase + offset);
                if (memcmp(p, "_SM3_", sizeof(p->Smbios30.AnchorString)) == 0) {
                    FF_DEBUG("Found SMBIOS 3.0 entry point at phyaddr 0x%05lX", (unsigned long) (0xF0000 + offset));
                    if (p->Smbios30.EntryPointLength != sizeof(p->Smbios30)) {
                        FF_DEBUG("Invalid SMBIOS 3.0 entry point length: %u (expected %zu)",
                            p->Smbios30.EntryPointLength,
                            sizeof(p->Smbios30));
                        return NULL;
                    }
                    tableLength = p->Smbios30.StructureTableMaximumSize;
                    tableAddress = (off_t) p->Smbios30.StructureTableAddress;
                    FF_DEBUG("SMBIOS 3.0: tableLength=0x%x, tableAddress=0x%lx, version=%u.%u.%u",
                        tableLength,
                        (unsigned long) tableAddress,
                        p->Smbios30.SmbiosMajorVersion,
                        p->Smbios30.SmbiosMinorVersion,
                        p->Smbios30.SmbiosDocrev);
                    break;
                } else if (memcmp(p, "_SM_", sizeof(p->Smbios20.AnchorString)) == 0) {
                    FF_DEBUG("Found SMBIOS 2.0 entry point at phyaddr 0x%05lX", (unsigned long) (0xF0000 + offset));
                    if (p->Smbios20.EntryPointLength != sizeof(p->Smbios20)) {
                        FF_DEBUG("Invalid SMBIOS 2.0 entry point length: %u (expected %zu)",
                            p->Smbios20.EntryPointLength,
                            sizeof(p->Smbios20));
                        return NULL;
                    }
                    tableLength = p->Smbios20.StructureTableLength;
                    tableAddress = (off_t) p->Smbios20.StructureTableAddress;
                    FF_DEBUG("SMBIOS 2.0: tableLength=0x%x, tableAddress=0x%lx, version=%u.%u",
                        tableLength,
                        (unsigned long) tableAddress,
                        p->Smbios20.SmbiosMajorVersion,
                        p->Smbios20.SmbiosMinorVersion);
                    break;
                }
            }
            if (tableLength == 0) {
                FF_DEBUG("No valid SMBIOS entry point found in memory region");
                return NULL;
            }

            ffStrbufEnsureFixedLengthFree(&buffer, tableLength);
            FF_DEBUG("Attempting to read SMBIOS table data: %u bytes at 0x%lx", tableLength, (unsigned long) tableAddress);
            if (pread(fd, buffer.chars, tableLength, tableAddress) == tableLength) {
                buffer.length = tableLength;
                buffer.chars[buffer.length] = '\0';
                FF_DEBUG("Successfully read SMBIOS table data: %u bytes", tableLength);
            } else {
                FF_DEBUG("Failed to read SMBIOS table data: %s", strerror(errno));
                return NULL;
            }
        }

        if (!parseSmbiosTable((const uint8_t*) buffer.chars, buffer.length)) {
            ffStrbufClear(&buffer);
        }
    }

    if (buffer.length == 0) {
        FF_DEBUG("No valid SMBIOS data available");
        return NULL;
    }

    return &smbiosTable;
}
#elif defined(_WIN32)
#    include "common/windows/nt.h"

#    pragma GCC diagnostic ignored "-Wmultichar"

typedef struct FFRawSmbiosData {
    uint8_t Used20CallingMethod;
    uint8_t SMBIOSMajorVersion;
    uint8_t SMBIOSMinorVersion;
    uint8_t DmiRevision;
    uint32_t Length;
    uint8_t SMBIOSTableData[];
} FFRawSmbiosData;

const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable() {
    static SYSTEM_FIRMWARE_TABLE_INFORMATION* buffer;

    if (!smbiosTableInitialized) {
        smbiosTableInitialized = true;
        FF_DEBUG("Initializing Windows SMBIOS buffer");

        FF_DEBUG("Querying system firmware table size with signature 'RSMB'");
        SYSTEM_FIRMWARE_TABLE_INFORMATION sfti = {
            .ProviderSignature = 'RSMB',
            .Action = SystemFirmwareTableGet,
        };
        ULONG bufSize = 0;
        NtQuerySystemInformation(SystemFirmwareTableInformation, &sfti, sizeof(sfti), &bufSize);
        if (bufSize <= sizeof(FFRawSmbiosData) + sizeof(sfti)) {
            FF_DEBUG("Invalid firmware table size: %lu (must be > %zu)", bufSize, sizeof(FFRawSmbiosData) + sizeof(sfti));
            return NULL;
        }
        if (bufSize != sfti.TableBufferLength + (ULONG) sizeof(sfti)) {
            FF_DEBUG("Firmware table size mismatch: NtQuerySystemInformation returned %lu but expected %lu",
                bufSize,
                sfti.TableBufferLength + (ULONG) sizeof(sfti));
            return NULL;
        }
        FF_DEBUG("Firmware table size: %lu bytes", bufSize);

        buffer = malloc(bufSize);
        *buffer = sfti;
        FF_DEBUG("Allocated buffer for SMBIOS data");

        if (!NT_SUCCESS(NtQuerySystemInformation(SystemFirmwareTableInformation, buffer, bufSize, &bufSize))) {
            FF_DEBUG("NtQuerySystemInformation(SystemFirmwareTableInformation) failed");
            free(buffer);
            buffer = NULL;
            return NULL;
        }
        FFRawSmbiosData* rawData = (FFRawSmbiosData*) buffer->TableBuffer;

        FF_DEBUG("Successfully retrieved SMBIOS data: version %u.%u, length %u bytes",
            rawData->SMBIOSMajorVersion,
            rawData->SMBIOSMinorVersion,
            rawData->Length);

        if (!parseSmbiosTable(rawData->SMBIOSTableData, rawData->Length)) {
            free(buffer);
            buffer = NULL;
            return NULL;
        }
    }

    if (!buffer) {
        FF_DEBUG("No valid SMBIOS data available");
        return NULL;
    }
    return &smbiosTable;
}
#elif defined(__APPLE__)
#    include "common/apple/cf_helpers.h"

const FFSmbiosHeaderTable* ffGetSmbiosHeaderTable() {
    static CFDataRef smbiosDataBuffer;

    if (!smbiosTableInitialized) {
        smbiosTableInitialized = true;
        FF_DEBUG("Initializing SMBIOS buffer on Apple platform");

        FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t registryEntry = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceMatching("AppleSMBIOS"));

        if (!registryEntry) {
            FF_DEBUG("IOServiceGetMatchingService() failed to find AppleSMBIOS");
            return NULL;
        }

        FF_DEBUG("AppleSMBIOS service found, retrieving SMBIOS data");
        smbiosDataBuffer = IORegistryEntryCreateCFProperty(registryEntry, CFSTR("SMBIOS"), kCFAllocatorDefault, kNilOptions);
        if (!smbiosDataBuffer) {
            FF_DEBUG("IORegistryEntryCreateCFProperty() failed to get SMBIOS data");
            return NULL;
        }
        if (CFGetTypeID(smbiosDataBuffer) != CFDataGetTypeID()) {
            FF_DEBUG("Unexpected SMBIOS data type: expected CFData");
            CFRelease(smbiosDataBuffer);
            smbiosDataBuffer = NULL;
            return NULL;
        }

        FF_DEBUG("Successfully retrieved SMBIOS data: %lu bytes", CFDataGetLength(smbiosDataBuffer));
        if (!parseSmbiosTable((const uint8_t*) CFDataGetBytePtr(smbiosDataBuffer), (uint32_t) CFDataGetLength(smbiosDataBuffer))) {
            CFRelease(smbiosDataBuffer);
            smbiosDataBuffer = NULL;
            return NULL;
        }
    }

    if (!smbiosDataBuffer) {
        FF_DEBUG("No valid SMBIOS data available");
        return NULL;
    }

    return &smbiosTable;
}
#endif
