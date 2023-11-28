#include "board.h"

#include "common/sysctl.h"

const char* ffDetectBoard(FFBoardResult* result)
{
    const char* error = ffSysctlGetString("hw.model", &result->name);
    if (error)
        return error;

    ffStrbufSetStatic(&result->vendor, "Apple");
    return NULL;
}
