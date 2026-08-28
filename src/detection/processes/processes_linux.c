#include "processes.h"

#include "common/io.h"
#include "common/memrchr.h"
#include "common/strutil.h"

const char* ffDetectProcesses(FFProcessesResult* result) {
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        return "opendir(\"/proc\") failed";
    }

    uint32_t processes = 0;
    uint32_t threads = 0;

    int procfd = dirfd(dir);

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if ((entry->d_type == DT_DIR || entry->d_type == DT_UNKNOWN) && ffCharIsDigit(entry->d_name[0])) {
            ++processes;

            char statPath[32];
            strcpy(ffStrCopy(statPath, entry->d_name, sizeof(statPath)), "/stat");

            char statBuffer[256];
            ssize_t statLength = ffReadFileDataRelative(procfd, statPath, sizeof(statBuffer) - 1, statBuffer);
            if (statLength < 0) {
                continue;
            }
            statBuffer[statLength] = '\0';

            const char* cursor = (const char*) memrchr(statBuffer, ')', (size_t) statLength);
            if (!cursor) {
                continue;
            }
            ++cursor;

            for (uint32_t field = 2 /*comm*/; field < 20 /*num_threads*/; ++field) {
                while (*cursor != ' ' && __builtin_expect(*cursor != '\0', true)) {
                    ++cursor;
                }
                ++cursor;
                if (__builtin_expect(*cursor == '\0', false)) {
                    break;
                }
            }
            if (__builtin_expect(*cursor == '\0', false)) {
                continue;
            }

            threads += (uint32_t) strtoul(cursor, nullptr, 10);
        }
    }

    result->processes = processes;
    result->threads = threads;

    return nullptr;
}
