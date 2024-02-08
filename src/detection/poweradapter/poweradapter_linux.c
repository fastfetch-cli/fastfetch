#include "poweradapter.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

#include <dirent.h>

static void parsePowerAdapter(FFstrbuf* dir, FF_MAYBE_UNUSED const char* id, FFlist* results)
{
    uint32_t dirLength = dir->length;

    FF_STRBUF_AUTO_DESTROY tmpBuffer = ffStrbufCreate();

    //type must exist and be "Mains"
    ffStrbufAppendS(dir, "/type");
    if (ffReadFileBuffer(dir->chars, &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(!ffStrbufIgnCaseEqualS(&tmpBuffer, "Mains"))
        return;

    //scope may not exist or must not be "Device" (?)
    ffStrbufAppendS(dir, "/scope");
    if (ffReadFileBuffer(dir->chars, &tmpBuffer))
        ffStrbufTrimRightSpace(&tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if(ffStrbufIgnCaseEqualS(&tmpBuffer, "Device"))
        return;

    ffStrbufAppendS(dir, "/online");
    char online = '1';
    ffReadFileData(dir->chars, sizeof(online), &online);
    ffStrbufSubstrBefore(dir, dirLength);

    if (online == '0')
        return;

    //input_power_limit must exist and be not empty
    ffStrbufAppendS(dir, "/input_power_limit");
    bool available = ffReadFileBuffer(dir->chars, &tmpBuffer);
    ffStrbufSubstrBefore(dir, dirLength);

    if (!available)
        return;
    FFPowerAdapterResult* result = ffListAdd(results);
    ffStrbufInit(&result->name);
    ffStrbufInit(&result->description);
    result->watts = (int) (ffStrbufToDouble(&tmpBuffer) / 1e6 + 0.5);
    ffStrbufInit(&result->manufacturer);
    ffStrbufInit(&result->modelName);
    ffStrbufInit(&result->serial);

    ffStrbufAppendS(dir, "/manufacturer");
    if (ffReadFileBuffer(dir->chars, &result->manufacturer))
        ffStrbufTrimRightSpace(&result->manufacturer);
    else if (ffStrEquals(id, "macsmc-ac")) // asahi
        ffStrbufSetStatic(&result->manufacturer, "Apple Inc.");
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "/model_name");
    if (ffReadFileBuffer(dir->chars, &result->modelName))
        ffStrbufTrimRightSpace(&result->modelName);
    ffStrbufSubstrBefore(dir, dirLength);

    ffStrbufAppendS(dir, "/serial_number");
    if (ffReadFileBuffer(dir->chars, &result->serial))
        ffStrbufTrimRightSpace(&result->serial);
    ffStrbufSubstrBefore(dir, dirLength);
}

const char* ffDetectPowerAdapter(FFlist* results)
{
    FF_STRBUF_AUTO_DESTROY baseDir = ffStrbufCreateA(64);
    ffStrbufAppendS(&baseDir, "/sys/class/power_supply/");

    uint32_t baseDirLength = baseDir.length;

    FF_AUTO_CLOSE_DIR DIR* dirp = opendir(baseDir.chars);
    if(dirp == NULL)
        return "opendir(\"/sys/class/power_supply/\") == NULL";

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(ffStrEquals(entry->d_name, ".") || ffStrEquals(entry->d_name, ".."))
            continue;

        ffStrbufAppendS(&baseDir, entry->d_name);
        parsePowerAdapter(&baseDir, entry->d_name, results);
        ffStrbufSubstrBefore(&baseDir, baseDirLength);
    }

    return NULL;
}
