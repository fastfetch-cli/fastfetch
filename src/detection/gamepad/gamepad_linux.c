#include "gamepad.h"
#include "common/io/io.h"

#include <dirent.h>
#include <ctype.h>

const char* ffDetectGamepad(FF_MAYBE_UNUSED const FFinstance* instance, FFlist* devices /* List of FFGamepadDevice */)
{
    DIR* dirp = opendir("/sys/class/input/");
    if(dirp == NULL)
        return "opendir(\"/sys/class/input/\") == NULL";

    FF_STRBUF_AUTO_DESTROY path;
    ffStrbufInitS(&path, "/sys/class/input/");
    uint32_t baseLen = path.length;

    struct dirent* entry;
    while((entry = readdir(dirp)) != NULL)
    {
        if(strncmp(entry->d_name, "js", 2) != 0)
            continue;
        if(!isdigit(entry->d_name[2]))
            continue;

        ffStrbufAppendS(&path, entry->d_name);
        ffStrbufAppendS(&path, "/device/name");

        if (ffPathExists(path.chars, FF_PATHTYPE_FILE))
        {
            FFGamepadDevice* device = (FFGamepadDevice*) ffListAdd(devices);
            ffStrbufInitS(&device->identifier, entry->d_name);
            ffStrbufInit(&device->name);
            ffAppendFileBuffer(path.chars, &device->name);
        }

        ffStrbufSubstrBefore(&path, baseLen);
    }

    closedir(dirp);

    return NULL;
}
