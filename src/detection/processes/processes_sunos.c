#include "processes.h"

#include "common/io.h"
#include "common/strutil.h"

#include <procfs.h>

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
        if (ffCharIsDigit(entry->d_name[0])) {
            ++processes;

            char psinfoPath[32];
            strcpy(ffStrCopy(psinfoPath, entry->d_name, sizeof(psinfoPath)), "/psinfo");

            psinfo_t info;
            if (ffReadFileDataRelative(procfd, psinfoPath, sizeof(info), &info) == (ssize_t) sizeof(info)) {
                threads += info.pr_nlwp;
            }
        }
    }

    result->processes = processes;
    result->threads = threads;

    return nullptr;
}
