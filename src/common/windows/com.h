#pragma once

#include <assert.h>
#include <unknwn.h>

// Initialize COM & WinRT
const char* ffInitCom(void);

static inline void ffReleaseComObject(void* ppUnknown) {
    assert(ppUnknown);
    IUnknown* pUnknown = *(IUnknown**) ppUnknown;
    if (pUnknown) {
#ifdef __cplusplus
        pUnknown->Release();
#else
        pUnknown->lpVtbl->Release(pUnknown);
#endif
        *(IUnknown**) ppUnknown = nullptr;
    }
}

#define FF_AUTO_RELEASE_COM_OBJECT [[gnu::cleanup(ffReleaseComObject)]]
