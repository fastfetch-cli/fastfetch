#include "top.h"

#include "common/io.h"
#include "common/strutil.h"

#include <stdio.h>
#include <procfs.h>
#include <sys/param.h> // DEV_BSIZE

const char* ffTopGetProcessSnapshot(FFlist* snapshots, FFTopTypes showTypes) {
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/proc");
    if (!dirp) {
        return "opendir(\"/proc\") failed";
    }

    int procfd = dirfd(dirp);

    struct dirent* dp;
    while ((dp = readdir(dirp))) {
        if (!ffCharIsDigit(dp->d_name[0])) { // Skip "." and ".."
            continue;
        }

        FF_AUTO_CLOSE_FD int subfd = openat(procfd, dp->d_name, O_RDONLY | O_DIRECTORY);
        if (subfd < 0) {
            continue;
        }

        pstatus_t status;
        if (ffReadFileDataRelative(subfd, "status", sizeof(status), &status) != (ssize_t) sizeof(status) ||
            status.pr_flags & PR_ISSYS) {
            continue; // The process may have exited, no permission or is a kernel process
        }

        // The data model of the returned structure depends on the data model of
        // the calling process, not of the observed process.
        psinfo_t psinfo;
        if (ffReadFileDataRelative(subfd, "psinfo", sizeof(psinfo), &psinfo) != (ssize_t) sizeof(psinfo)) {
            continue;
        }

        // zombies
        if (psinfo.pr_lwp.pr_sname == 'Z') {
            continue;
        }

        FFTopProcessSnapshot* item = FF_LIST_ADD(FFTopProcessSnapshot, *snapshots);

        ffStrbufInitS(&item->name, psinfo.pr_fname);

        item->pid = (uint32_t) psinfo.pr_pid;

        item->cpuTime = (uint64_t) psinfo.pr_time.tv_sec * 1000 +
            (uint64_t) psinfo.pr_time.tv_nsec / 1000000;

        item->memBytes = (uint64_t) psinfo.pr_rssize * 1024;

        item->startTime = (uint64_t) psinfo.pr_start.tv_sec * 1000 +
            (uint64_t) psinfo.pr_start.tv_nsec / 1000000;

        item->bytesRead = 0;
        item->bytesWritten = 0;
        if (showTypes & FF_TOP_TYPE_DISK) {
            prusage_t usage;
            if (ffReadFileDataRelative(subfd, "usage", sizeof(usage), &usage) == (ssize_t) sizeof(usage)) {
                item->bytesRead = (uint64_t) usage.pr_inblk * DEV_BSIZE;
                item->bytesWritten = (uint64_t) usage.pr_oublk * DEV_BSIZE;
            }
        }
    }

    return nullptr;
}
