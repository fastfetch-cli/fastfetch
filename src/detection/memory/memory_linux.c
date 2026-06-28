#include "memory.h"
#include "common/io.h"

const char* ffDetectMemory(FFMemoryResult* ram) {
    char buf[PROC_FILE_BUFFSIZ];
    ssize_t nRead = ffReadFileData("/proc/meminfo", ARRAY_SIZE(buf) - 1, buf);
    if (nRead < 0) {
        return "ffReadFileData(\"/proc/meminfo\", ARRAY_SIZE(buf)-1, buf)";
    }
    buf[nRead] = '\0';

    uint64_t memTotal = 0,
             memAvailable = 0,
             shmem = 0,
             memFree = 0,
             buffers = 0,
             cached = 0,
             sReclaimable = 0;

    char* token = NULL;
    if ((token = memmem(buf, (size_t) nRead, "MemTotal:", strlen("MemTotal:"))) != NULL) {
        memTotal = (uint64_t) strtoull(token + strlen("MemTotal:"), NULL, 10);
    } else {
        return "MemTotal not found in /proc/meminfo";
    }

    if ((token = memmem(buf, (size_t) nRead, "MemAvailable:", strlen("MemAvailable:"))) != NULL) {
        memAvailable = (uint64_t) strtoull(token + strlen("MemAvailable:"), NULL, 10);
    }

    if (memAvailable == 0 || memAvailable >= memTotal) { // MemAvailable can be unreasonable. #1988
        if ((token = memmem(buf, (size_t) nRead, "MemFree:", strlen("MemFree:"))) != NULL) {
            memFree = (uint64_t) strtoull(token + strlen("MemFree:"), NULL, 10);
        }

        if ((token = memmem(buf, (size_t) nRead, "Buffers:", strlen("Buffers:"))) != NULL) {
            buffers = (uint64_t) strtoull(token + strlen("Buffers:"), NULL, 10);
        }

        if ((token = memmem(buf, (size_t) nRead, "Cached:", strlen("Cached:"))) != NULL) {
            cached = (uint64_t) strtoull(token + strlen("Cached:"), NULL, 10);
        }

        if ((token = memmem(buf, (size_t) nRead, "Shmem:", strlen("Shmem:"))) != NULL) {
            shmem = (uint64_t) strtoull(token + strlen("Shmem:"), NULL, 10);
        }

        if ((token = memmem(buf, (size_t) nRead, "SReclaimable:", strlen("SReclaimable:"))) != NULL) {
            sReclaimable = (uint64_t) strtoull(token + strlen("SReclaimable:"), NULL, 10);
        }

        memAvailable = memFree + buffers + cached + sReclaimable - shmem;
    }

    ram->bytesTotal = memTotal * 1024lu;
    ram->bytesUsed = (memTotal - memAvailable) * 1024lu;

    uint64_t arcSize = 0;
    nRead = ffReadFileData("/proc/spl/kstat/zfs/arcstats", ARRAY_SIZE(buf) - 1, buf);
    if (nRead > 0) {
        buf[nRead] = '\0';
        // 24 1 0x01 148 40256 2053603602 851614020716
        // name                            type data
        // hits                            4    752

        // Skip first line
        const char* p = memchr(buf, '\n', (size_t) nRead);
        if (__builtin_expect(p != NULL, true)) {
            ++p;
            nRead -= (p - buf);
            assert(nRead > 0);

            // Find line offset of data
            const char* pData = memmem(p, (size_t) nRead, "data\n", strlen("data\n"));
            if (__builtin_expect(pData != NULL, true)) {
                uint32_t dataOffset = (uint32_t) (pData - p);
                p += dataOffset + strlen("data\n");
                nRead -= (ssize_t) (dataOffset + strlen("data\n"));
                assert(nRead > 0);

                if ((token = memmem(p, (size_t) nRead, "\nsize ", strlen("\nsize "))) != NULL) {
                    arcSize = (uint64_t) strtoull(token + 1 + dataOffset, NULL, 10);
                    if (arcSize > 0) {
                        uint64_t arcMin = 0;
                        if ((token = memmem(p, (size_t) nRead, "\nc_min ", strlen("\nc_min "))) != NULL) {
                            arcMin = (uint64_t) strtoull(token + 1 + dataOffset, NULL, 10);
                            if (arcSize > arcMin) {
                                arcSize -= arcMin;
                                if (ram->bytesUsed > arcSize) {
                                    ram->bytesUsed -= arcSize;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}
