#pragma once

#ifndef FASTFETCH_INCLUDED_REGISTER_H
#define FASTFETCH_INCLUDED_REGISTER_H

#include "fastfetch.h"

#include <winreg.h>

static inline void wrapRegCloseKey(HKEY* phKey)
{
    if(*phKey)
        RegCloseKey(*phKey);
}

#define FF_HKEY_AUTO_DESTROY HKEY __attribute__((__cleanup__(wrapRegCloseKey)))

bool ffRegOpenKeyForRead(HKEY hKey, const wchar_t* subKeyW, HKEY* result, FFstrbuf* error);
bool ffRegReadStrbuf(HKEY hKey, const wchar_t* valueNameW, FFstrbuf* result, FFstrbuf* error);
bool ffRegReadUint(HKEY hKey, const wchar_t* valueNameW, uint32_t* result, FFstrbuf* error);

#endif
