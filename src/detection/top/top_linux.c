#include "top.h"

#include "common/io.h"
#include "common/memrchr.h"
#include "common/strutil.h"

#include <inttypes.h>

#define PF_KTHREAD 0x00200000 // defined in kernel include/linux/sched.h

static bool parseStat(const char* buffer, size_t length, FFTopProcessSnapshot* result) {
    const char* open = strchr(buffer, '(');
    const char* close = (const char*) memrchr(buffer, ')', length);
    if (!open || !close || close <= open || close + 2 >= buffer + length) {
        return false;
    }

    ffStrbufSetNS(&result->name, (uint32_t) (close - open - 1), open + 1);
    const char* cursor = close + 2; // state, then fields 4..
    if (*cursor == '\0') {
        return false;
    }
    ++cursor;

    uint64_t values[19];
    for (uint32_t field = 4; field <= 22; ++field) {
        while (*cursor == ' ') {
            ++cursor;
        }
        if (*cursor == '\0') {
            return false;
        }
        char* end;
        errno = 0;
        unsigned long long value = strtoull(cursor, &end, 10);
        if (end == cursor || errno == ERANGE) {
            return false;
        }
        values[field - 4] = (uint64_t) value;
        cursor = end;
    }

    if (values[5] & PF_KTHREAD) { // values[5] is field 9 (flags)
        return false;
    }

    result->cpuTime = values[11] + values[12]; // user + system
    result->startTime = values[18];
    return true;
}

const char* ffTopGetProcessSnapshot(FFlist* snapshots) {
    const long ticks = sysconf(_SC_CLK_TCK);
    const long pageSize = instance.state.platform.sysinfo.pageSize;
    if (ticks <= 0 || pageSize <= 0) {
        return "sysconf(_SC_CLK_TCK or _SC_PAGESIZE) failed";
    }

    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if (!dir) {
        return "opendir(\"/proc\") failed";
    }

    int procfd = dirfd(dir);

    struct dirent* entry;
    char statBuffer[4096];
    char statmBuffer[256];
    while ((entry = readdir(dir))) {
        if (!entry->d_name[0] || !ffCharIsDigit(entry->d_name[0])) {
            continue;
        }

        unsigned long pid = strtoul(entry->d_name, nullptr, 10);
        if (pid == 0 || pid > UINT32_MAX) {
            continue;
        }

        FF_AUTO_CLOSE_FD int subfd = openat(procfd, entry->d_name, O_RDONLY | O_DIRECTORY);
        if (subfd < 0) {
            continue;
        }

        ssize_t statLength;
        if ((statLength = ffReadFileDataRelative(subfd, "stat", sizeof(statBuffer) - 1, statBuffer)) < 0) {
            continue;
        }
        statBuffer[statLength] = '\0';

        ssize_t statmLength;
        if ((statmLength = ffReadFileDataRelative(subfd, "statm", sizeof(statmBuffer) - 1, statmBuffer)) < 0) {
            continue;
        }
        statmBuffer[statmLength] = '\0';

        char ioBuffer[512];
        ssize_t ioLength;
        if ((ioLength = ffReadFileDataRelative(subfd, "io", sizeof(ioBuffer) - 1, ioBuffer)) < 0) {
            continue;
        }
        ioBuffer[ioLength] = '\0';

        auto snapshot = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);
        ffStrbufInit(&snapshot->name);
        if (!parseStat(statBuffer, (size_t) statLength, snapshot)) {
            ffStrbufDestroy(&snapshot->name);
            --snapshots->length;
            continue;
        }

        unsigned long long rssPages;
        if (__builtin_expect(sscanf(statmBuffer, "%*llu %llu", &rssPages) != 1, false)) {
            ffStrbufDestroy(&snapshot->name);
            --snapshots->length;
            continue;
        }

        snapshot->pid = (uint32_t) pid;
        snapshot->memBytes = rssPages * (uint64_t) pageSize;
        snapshot->bytesRead = 0;
        snapshot->bytesWritten = 0;
        sscanf(ioBuffer, "rchar: %*" SCNu64 "\nwchar: %*" SCNu64 "\nsyscr: %*" SCNu64 "\nsyscw: %*" SCNu64 "\nread_bytes: %" SCNu64 "\nwrite_bytes: %" SCNu64, &snapshot->bytesRead, &snapshot->bytesWritten);
        snapshot->cpuTime = snapshot->cpuTime * 1000u / (uint64_t) ticks;
    }
    return nullptr;
}
