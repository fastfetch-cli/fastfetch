#pragma once

#include "fastfetch.h"
#include "common/argType.h"
#include "common/io.h"

#ifndef HKEY_CURRENT_USER
    #define HKEY_CLASSES_ROOT ((HKEY) (ULONG_PTR) ((LONG) 0x80000000))
    #define HKEY_CURRENT_USER ((HKEY) (ULONG_PTR) ((LONG) 0x80000001))
    #define HKEY_LOCAL_MACHINE ((HKEY) (ULONG_PTR) ((LONG) 0x80000002))
    #define HKEY_USERS ((HKEY) (ULONG_PTR) ((LONG) 0x80000003))
    #define HKEY_PERFORMANCE_DATA ((HKEY) (ULONG_PTR) ((LONG) 0x80000004))
    #define HKEY_CURRENT_CONFIG ((HKEY) (ULONG_PTR) ((LONG) 0x80000005))
    #define HKEY_DYN_DATA ((HKEY) (ULONG_PTR) ((LONG) 0x80000006))
    #define HKEY_CURRENT_USER_LOCAL_SETTINGS ((HKEY) (ULONG_PTR) ((LONG) 0x80000007))
#endif

typedef struct FFRegValueArg {
    FFArgType type;
    const void* value;
    const wchar_t* name;
} FFRegValueArg;

HANDLE ffRegGetRootKeyHandle(HKEY hKey);
bool ffRegOpenSubkeyForRead(HANDLE hKey, const wchar_t* subKeyW, HANDLE* result, FFstrbuf* error);
bool ffRegReadValue(HANDLE hKey, const FFRegValueArg* arg, FFstrbuf* error);
bool ffRegReadValues(HANDLE hKey, uint32_t argc, const FFRegValueArg argv[], FFstrbuf* error);
bool ffRegGetSubKey(HANDLE hKey, uint32_t index, FFstrbuf* result, FFstrbuf* error);
bool ffRegGetNSubKeys(HANDLE hKey, uint32_t* result, FFstrbuf* error);

static inline bool ffRegOpenKeyForRead(HKEY hRootKey, const wchar_t* subKeyW, HANDLE* result, FFstrbuf* error) {
    return ffRegOpenSubkeyForRead(ffRegGetRootKeyHandle(hRootKey), subKeyW, result, error);
}

static inline bool ffRegReadStrbuf(HANDLE hKey, const wchar_t* valueNameW, FFstrbuf* result, FFstrbuf* error) {
    return ffRegReadValue(hKey, &(FFRegValueArg) {
                                    .type = FF_ARG_TYPE_STRBUF,
                                    .value = result,
                                    .name = valueNameW,
                                },
        error);
}
static inline bool ffRegReadUint(HANDLE hKey, const wchar_t* valueNameW, uint32_t* result, FFstrbuf* error) {
    return ffRegReadValue(hKey, &(FFRegValueArg) {
                                    .type = FF_ARG_TYPE_UINT,
                                    .value = result,
                                    .name = valueNameW,
                                },
        error);
}
static inline bool ffRegReadUint64(HANDLE hKey, const wchar_t* valueNameW, uint64_t* result, FFstrbuf* error) {
    return ffRegReadValue(hKey, &(FFRegValueArg) {
                                    .type = FF_ARG_TYPE_UINT64,
                                    .value = result,
                                    .name = valueNameW,
                                },
        error);
}
static inline bool ffRegReadData(HANDLE hKey, const wchar_t* valueNameW, FFArgBuffer* buffer, FFstrbuf* error) {
    return ffRegReadValue(hKey, &(FFRegValueArg) {
                                    .type = FF_ARG_TYPE_BUFFER,
                                    .value = buffer,
                                    .name = valueNameW,
                                },
        error);
}
