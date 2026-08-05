#include "processes.h"

#include "common/io.h"
#include "common/strutil.h"

const char* ffDetectProcesses(uint32_t* result) {
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        return "opendir(\"/proc\") failed";
    }

    uint32_t num = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (
#ifdef _DIRENT_HAVE_D_TYPE
            (entry->d_type == DT_DIR || entry->d_type == DT_UNKNOWN) &&
#endif
            ffCharIsDigit(entry->d_name[0]))
            ++num;
    }

    *result = num;

    return nullptr;
}
