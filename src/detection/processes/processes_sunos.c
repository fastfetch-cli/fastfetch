#include "processes.h"

#include "common/io.h"
#include "common/strutil.h"

#include <procfs.h>

const char* ffDetectProcesses(const FFProcessesOptions* options, FFProcessesResult* result) {
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        return "opendir(\"/proc\") failed";
    }

    int procfd = dirfd(dir);

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (ffCharIsDigit(entry->d_name[0])) {
            char path[32];
            char* pidEnd = ffStrCopy(path, entry->d_name, sizeof(path));
            if (!options->countKprocs) {
                pstatus_t status;
                strcpy(pidEnd, "/status");
                if (ffReadFileDataRelative(procfd, path, sizeof(status), &status) != (ssize_t) sizeof(status) ||
                    status.pr_flags & PR_ISSYS) {
                    continue; // The process may have exited, no permission or is a kernel process
                }
            }

            strcpy(pidEnd, "/psinfo");
            psinfo_t info;
            if (ffReadFileDataRelative(procfd, path, sizeof(info), &info) == (ssize_t) sizeof(info)) {
                ++result->processes;
                result->threads += (uint32_t) info.pr_nlwp;
            }
        }
    }

    return nullptr;
}
