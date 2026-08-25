#include "top.h"

#include "common/debug.h"
#include "common/io.h"
#include "common/memrchr.h"
#include "common/strutil.h"

#include <inttypes.h>

#define PF_KTHREAD 0x00200000 // defined in kernel include/linux/sched.h

static bool parseStat(const char* buffer, size_t length, FFTopProcessSnapshot* result) {
    const char* open = strchr(buffer, '(');
    const char* close = (const char*) memrchr(buffer, ')', length);
    if (!open || !close || close <= open || close + 2 >= buffer + length) {
        FF_DEBUG("parseStat: malformed stat content");
        return false;
    }

    ffStrbufSetNS(&result->name, (uint32_t) (close - open - 1), open + 1);
    const char* cursor = close + 2; // state, then fields 4..
    if (*cursor == '\0') {
        FF_DEBUG("parseStat(%s): truncated after state field", result->name.chars);
        return false;
    }
    ++cursor;

    uint64_t values[19];
    for (uint32_t field = 4; field <= 22; ++field) {
        while (*cursor == ' ') {
            ++cursor;
        }
        if (*cursor == '\0') {
            FF_DEBUG("parseStat(%s): truncated at field %u", result->name.chars, field);
            return false;
        }
        char* end;
        errno = 0;
        unsigned long long value = strtoull(cursor, &end, 10);
        if (end == cursor || errno == ERANGE) {
            FF_DEBUG("parseStat(%s): invalid number at field %u", result->name.chars, field);
            return false;
        }
        values[field - 4] = (uint64_t) value;
        cursor = end;
    }

    if (values[5] & PF_KTHREAD) { // values[5] is field 9 (flags)
        FF_DEBUG("Skip kernel thread: %s", result->name.chars);
        return false;
    }

    result->cpuTime = values[11] + values[12]; // user + system
    result->startTime = values[18];
    return true;
}

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
    const long ticks = sysconf(_SC_CLK_TCK);
    const long pageSize = instance.state.platform.sysinfo.pageSize;
    FF_DEBUG("Scanning /proc: clk_tck=%ld, pageSize=%ld", ticks, pageSize);
    if (ticks <= 0 || pageSize <= 0) {
        return "sysconf(_SC_CLK_TCK or _SC_PAGESIZE) failed";
    }

    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if (!dir) {
        FF_DEBUG("opendir(\"/proc\") failed: %s", strerror(errno));
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
            FF_DEBUG("Skip invalid pid directory: %s", entry->d_name);
            continue;
        }

        FF_AUTO_CLOSE_FD int subfd = openat(procfd, entry->d_name, O_RDONLY | O_DIRECTORY);
        if (subfd < 0) {
            FF_DEBUG("openat(/proc/%s) failed: %s", entry->d_name, strerror(errno));
            continue;
        }

        ssize_t statLength;
        if ((statLength = ffReadFileDataRelative(subfd, "stat", sizeof(statBuffer) - 1, statBuffer)) < 0) {
            FF_DEBUG("Failed to read /proc/%s/stat: %s", entry->d_name, strerror(errno));
            continue;
        }
        statBuffer[statLength] = '\0';

        ssize_t statmLength = 0;
        if (showTypes & FF_TOP_TYPE_MEMORY) {
            if ((statmLength = ffReadFileDataRelative(subfd, "statm", sizeof(statmBuffer) - 1, statmBuffer)) < 0) {
                FF_DEBUG("Failed to read /proc/%s/statm: %s", entry->d_name, strerror(errno));
                statmLength = 0;
            }
            statmBuffer[statmLength] = '\0';
        }

        char ioBuffer[512];
        ssize_t ioLength = 0;
        if (showTypes & FF_TOP_TYPE_DISK) {
            if ((ioLength = ffReadFileDataRelative(subfd, "io", sizeof(ioBuffer) - 1, ioBuffer)) < 0) {
                FF_DEBUG("Failed to read /proc/%s/io: %s", entry->d_name, strerror(errno));
                ioLength = 0;
            }
            ioBuffer[ioLength] = '\0';
        }

        auto snapshot = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);
        ffStrbufInit(&snapshot->name);
        if (!parseStat(statBuffer, (size_t) statLength, snapshot)) {
            ffStrbufDestroy(&snapshot->name);
            --snapshots->length;
            continue;
        }

        uint64_t rssPages = 0;
        if (__builtin_expect((showTypes & FF_TOP_TYPE_MEMORY) && sscanf(statmBuffer, "%*u %" SCNu64, &rssPages) != 1, false)) {
            FF_DEBUG("Failed to parse statm of /proc/%lu", pid);
            ffStrbufDestroy(&snapshot->name);
            --snapshots->length;
            continue;
        }

        snapshot->pid = (uint32_t) pid;
        snapshot->memBytes = rssPages * (uint64_t) pageSize;
        snapshot->bytesRead = 0;
        snapshot->bytesWritten = 0;
        if (ioLength > 0) {
            sscanf(ioBuffer, "rchar: %*u\nwchar: %*u\nsyscr: %*u\nsyscw: %*u\nread_bytes: %" SCNu64 "\nwrite_bytes: %" SCNu64, &snapshot->bytesRead, &snapshot->bytesWritten);
        }
        snapshot->cpuTime = snapshot->cpuTime * 1000u / (uint64_t) ticks;
        FF_DEBUG(
            "Captured process %u (%s): cpuTime=%" PRIu64 "ms, mem=%" PRIu64 "B, diskRead=%" PRIu64 "B, diskWrite=%" PRIu64 "B",
            snapshot->pid, snapshot->name.chars, snapshot->cpuTime, snapshot->memBytes, snapshot->bytesRead, snapshot->bytesWritten);
    }
    FF_DEBUG("Captured %u process snapshots in total", snapshots->length);
    return nullptr;
}
