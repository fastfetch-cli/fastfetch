#include "bootmgr.h"
#include "common/io/io.h"

const char* ffDetectBootmgr(FFBootmgrResult* result)
{
    if (ffPathExists("/System/Library/CoreServices/boot.efi", FF_PATHTYPE_FILE))
        ffStrbufSetStatic(&result->firmware, "/System/Library/CoreServices/boot.efi");

    ffStrbufSetStatic(&result->name, "iBoot");

    return NULL;
}
