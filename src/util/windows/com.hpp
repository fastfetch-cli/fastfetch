#pragma once

#ifndef FF_INCLUDED_util_windows_com
#define FF_INCLUDED_util_windows_com

#ifdef __cplusplus

#include <unknwn.h>

const char* ffInitCom(void);

static inline void ffReleaseComObject(void* ppUnknown)
{
    IUnknown* pUnknown = *(IUnknown**) ppUnknown;
    if (pUnknown) pUnknown->Release();
}

#define FF_AUTO_RELEASE_COM_OBJECT __attribute__((__cleanup__(ffReleaseComObject)))

#else
    // Win32 COM headers requires C++ compiler
    #error Must be included in C++ source file
#endif

#endif
