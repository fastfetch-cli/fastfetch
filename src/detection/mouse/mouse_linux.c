#include "mouse.h"
#include "common/io/io.h"
#include "util/stringUtils.h"

const char* ffDetectMouse(FFlist* devices /* List of FFMouseDevice */)
{
    FF_AUTO_CLOSE_DIR DIR* dirp = opendir("/sys/class/input/");
    if (dirp == NULL)
        return "opendir(\"/sys/class/input/\") == NULL";

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateS("/sys/class/input/");
    uint32_t baseLen = path.length;

    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL)
    {
        if (!ffStrStartsWith(entry->d_name, "mouse"))
            continue;
        if (!ffCharIsDigit(entry->d_name[strlen("mouse")]))
            continue;

        ffStrbufAppendS(&path, entry->d_name);
        ffStrbufAppendS(&path, "/device/name");

        FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
        if (ffAppendFileBuffer(path.chars, &name))
        {
            ffStrbufTrimRightSpace(&name);
            ffStrbufSubstrBefore(&path, path.length - 4);

            FFMouseDevice* device = (FFMouseDevice*) ffListAdd(devices);
            ffStrbufInitMove(&device->name, &name);
            ffStrbufInit(&device->serial);

            ffStrbufAppendS(&path, "uniq");
            if (ffAppendFileBuffer(path.chars, &device->serial))
                ffStrbufTrimRightSpace(&device->serial);
        }

        ffStrbufSubstrBefore(&path, baseLen);
    }

    return NULL;
}
