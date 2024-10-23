#include "poweradapter.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

static void parsePowerAdapter(int dfd, FF_MAYBE_UNUSED const char* id, FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY tmpBuffer = ffStrbufCreate();

    //type must exist and be "Mains"
    if (ffReadFileBufferRelative(dfd, "type", &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);

    if(!ffStrbufIgnCaseEqualS(&tmpBuffer, "Mains"))
        return;

    //scope may not exist or must not be "Device" (?)
    if (ffReadFileBufferRelative(dfd, "scope", &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);

    if(ffStrbufIgnCaseEqualS(&tmpBuffer, "Device"))
        return;

    char online = '\0';
    ffReadFileDataRelative(dfd, "online", sizeof(online), &online);

    if (online != '1')
        return;

    //input_power_limit must exist and be not empty
    if (!ffReadFileBufferRelative(dfd, "input_power_limit", &tmpBuffer))
        return;

    FFPowerAdapterResult* result = ffListAdd(results);
    ffStrbufInit(&result->name);
    ffStrbufInit(&result->description);
    result->watts = (int) (ffStrbufToDouble(&tmpBuffer) / 1e6 + 0.5);
    ffStrbufInit(&result->manufacturer);
    ffStrbufInit(&result->modelName);
    ffStrbufInit(&result->serial);

    if (ffReadFileBufferRelative(dfd, "manufacturer", &result->manufacturer))
        ffStrbufTrimRightSpace(&result->manufacturer);
    else if (ffStrEquals(id, "macsmc-ac")) // asahi
        ffStrbufSetStatic(&result->manufacturer, "Apple Inc.");

    if (ffReadFileBufferRelative(dfd, "model_name", &result->modelName))
        ffStrbufTrimRightSpace(&result->modelName);

    if (ffReadFileBufferRelative(dfd, "serial_number", &result->serial))
        ffStrbufTrimRightSpace(&result->serial);
}

const char* ffDetectPowerAdapter(FFlist* results)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/power_supply/");
    if(dirp == NULL)
        return "opendir(\"/sys/class/power_supply/\") == NULL";

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(entry->d_name[0] == '.')
            continue;

        FF_AUTO_CLOSE_FD int dfd = openat(dirfd(dirp), entry->d_name, O_RDONLY | O_CLOEXEC);
        if (dfd > 0) parsePowerAdapter(dfd, entry->d_name, results);
    }

    return NULL;
}
