#include "gpu.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <fcntl.h>

static double parseTZDir(int dfd, FFstrbuf* buffer)
{
    if (!ffReadFileBufferRelative(dfd, "type", buffer) || !ffStrbufStartsWithS(buffer, "gpu"))
        return FF_GPU_TEMP_UNSET;

    if (!ffReadFileBufferRelative(dfd, "temp", buffer))
        return FF_GPU_TEMP_UNSET;

    double value = ffStrbufToDouble(buffer, FF_GPU_TEMP_UNSET);// millidegree Celsius
    if (value == FF_GPU_TEMP_UNSET)
        return FF_GPU_TEMP_UNSET;

    return value / 1000.;
}

double ffGPUDetectTempFromTZ(void)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/thermal/");
    if(dirp)
    {
        FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
        int dfd = dirfd(dirp);
        struct dirent* entry;
        while((entry = readdir(dirp)) != NULL)
        {
            if(entry->d_name[0] == '.')
                continue;
            if(!ffStrStartsWith(entry->d_name, "thermal_zone"))
                continue;

            FF_AUTO_CLOSE_FD int subfd = openat(dfd, entry->d_name, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
            if(subfd < 0)
                continue;

            double result = parseTZDir(subfd, &buffer);
            if (result != FF_GPU_TEMP_UNSET)
                return result;
        }
    }
    return FF_GPU_TEMP_UNSET;
}

const char* ffDetectGPUImpl(const FFGPUOptions* options, FFlist* gpus)
{
    FF_UNUSED(options, gpus);
    return "No permission. Fallbacks to Vulkan, OpenCL or OpenGL instead";
}
