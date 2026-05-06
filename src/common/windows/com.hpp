#pragma once

#ifdef __cplusplus

    #include "common/attributes.h"
    #include <unknwn.h>

// Initialize COM & WinRT
const char* ffInitCom(void);

static inline void ffReleaseComObject(void* ppUnknown) {
    IUnknown* pUnknown = *(IUnknown**) ppUnknown;
    if (pUnknown) {
        pUnknown->Release();
    }
}

    #define FF_AUTO_RELEASE_COM_OBJECT FF_A_CLEANUP(ffReleaseComObject)

#else
    // Win32 COM headers requires C++ compiler
    #error Must be included in C++ source file
#endif
