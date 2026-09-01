#include "processes.h"

#include "common/io.h"
#include "common/memrchr.h"
#include "common/strutil.h"

#define PF_KTHREAD 0x00200000 // defined in kernel include/linux/sched.h

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        return "opendir(\"/proc\") failed";
    }

    int procfd = dirfd(dir);

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if ((entry->d_type == DT_DIR || entry->d_type == DT_UNKNOWN) && ffCharIsDigit(entry->d_name[0])) {
            char statPath[32];
            strcpy(ffStrCopy(statPath, entry->d_name, sizeof(statPath)), "/stat");

            char statBuffer[512];
            ssize_t statLength = ffReadFileDataRelative(procfd, statPath, sizeof(statBuffer) - 1, statBuffer);
            if (statLength < 0) {
                continue;
            }
            statBuffer[statLength] = '\0';

            const char* cursor = (const char*) memrchr(statBuffer, ')', (size_t) statLength);
            if (!cursor) {
                continue;
            }
            cursor += 2; // skip ") "
            if (__builtin_expect(cursor >= statBuffer + statLength, false)) {
                goto skip_process;
            }

            char* p;
            for (uint32_t field = 3; field <= 20 /*num_threads*/; ++field) {
                switch (field) {
                    case 9: // flags
                        if ((strtoul(cursor, &p, 10) & PF_KTHREAD) && !options->countKprocs) {
                            goto skip_process;
                        }
                        cursor = p;
                        break;

                    case 20: // num_threads
                        result->threads += (uint32_t) strtoul(cursor, &p, 10);
                        cursor = p;
                        break;

                    default:
                        while (*cursor != ' ' && __builtin_expect(*cursor != '\0', true)) {
                            ++cursor;
                        }
                        break;
                }

                ++cursor;
                if (__builtin_expect(cursor >= statBuffer + statLength, false)) {
                    goto skip_process;
                }
            }

            ++result->processes;
        }
        skip_process:;
    }

    return nullptr;
}
