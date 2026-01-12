#pragma once

#include "fastfetch.h"

#include <windows.h>

static inline void wrapRegCloseKey(HKEY* phKey)
{
    if(*phKey)
        RegCloseKey(*phKey);
}

#define FF_HKEY_AUTO_DESTROY HKEY __attribute__((__cleanup__(wrapRegCloseKey)))

bool ffRegOpenKeyForRead(HKEY hKey, const wchar_t* subKeyW, HKEY* result, FFstrbuf* error);
bool ffRegReadStrbuf(HKEY hKey, const wchar_t* valueNameW, FFstrbuf* result, FFstrbuf* error);
bool ffRegReadData(HKEY hKey, const wchar_t* valueNameW, uint8_t** result, uint32_t* length, FFstrbuf* error);
bool ffRegReadUint(HKEY hKey, const wchar_t* valueNameW, uint32_t* result, FFstrbuf* error);
bool ffRegReadUint64(HKEY hKey, const wchar_t* valueNameW, uint64_t* result, FFstrbuf* error);
bool ffRegGetSubKey(HKEY hKey, uint32_t index, FFstrbuf* result, FFstrbuf* error);
bool ffRegGetNSubKeys(HKEY hKey, uint32_t* result, FFstrbuf* error);
