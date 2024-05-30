#include "processes.h"

#include "common/io/io.h"
#include "util/stringUtils.h"

const char* ffDetectProcesses(uint32_t* result)
{
    FF_AUTO_CLOSE_DIR DIR* dir = opendir("/proc");
    if(dir == NULL)
        return "opendir(\"/proc\") failed";

    uint32_t num = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && ffCharIsDigit(entry->d_name[0]))
            ++num;
    }

    *result = num;

    return NULL;
}
