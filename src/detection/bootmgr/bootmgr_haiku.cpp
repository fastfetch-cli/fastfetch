extern "C" {
    #include "bootmgr.h"
    #include "common/io.h"
}

const char* ffDetectBootmgr(FFBootmgrResult* result) {
    // TODO: glob haiku_loader.* + check EFI partition
    if (ffPathExists("/system/haiku_loader.bios_ia32", FF_PATHTYPE_FILE)) {
        ffStrbufSetStatic(&result->firmware, "/system/haiku_loader.bios_ia32");
    }

    ffStrbufSetStatic(&result->name, "haiku_loader");

    // TODO: detectSecureBoot(&result->secureBoot);

    return NULL;
}
