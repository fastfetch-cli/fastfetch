#pragma once

#ifndef FASTFETCH_INCLUDED_REGISTER_H
#define FASTFETCH_INCLUDED_REGISTER_H

#include "fastfetch.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static inline void wrapRegCloseKey(HKEY* phKey)
{
    if(*phKey)
        RegCloseKey(*phKey);
}

#define FF_HKEY_AUTO_DESTROY HKEY __attribute__((__cleanup__(wrapRegCloseKey)))

bool ffRegOpenKeyForRead(HKEY hKey, const char* lpSubKey, HKEY* result, FFstrbuf* error);
bool ffRegReadStrbuf(HKEY hKey, const char* valueName, FFstrbuf* result, FFstrbuf* error);
bool ffRegReadUint(HKEY hKey, const char* valueName, uint32_t* result, FFstrbuf* error);

#endif
