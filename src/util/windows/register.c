#include "register.h"

static const char* hKey2Str(HKEY hKey)
{
    #define HKEY_CASE(compareKey) if(hKey == compareKey) return #compareKey;
    HKEY_CASE(HKEY_CLASSES_ROOT)
    HKEY_CASE(HKEY_CURRENT_USER)
    HKEY_CASE(HKEY_LOCAL_MACHINE)
    HKEY_CASE(HKEY_USERS)
    HKEY_CASE(HKEY_PERFORMANCE_DATA)
    HKEY_CASE(HKEY_PERFORMANCE_TEXT)
    HKEY_CASE(HKEY_PERFORMANCE_NLSTEXT)
    HKEY_CASE(HKEY_CURRENT_CONFIG)
    HKEY_CASE(HKEY_DYN_DATA)
    HKEY_CASE(HKEY_CURRENT_USER_LOCAL_SETTINGS)
    #undef HKEY_CASE

    return "UNKNOWN";
}

bool ffRegOpenKeyForRead(HKEY hKey, const char* lpSubKey, HKEY* result, FFstrbuf* error)
{
    if(RegOpenKeyExA(hKey, lpSubKey, 0, KEY_READ, result) != ERROR_SUCCESS)
    {
        if(error)
            ffStrbufAppendF(error, "RegOpenKeyExW(%s\\%s) failed", hKey2Str(hKey), lpSubKey);
        return false;
    }
    return true;
}

bool ffRegReadStrbuf(HKEY hKey, const char* valueName, FFstrbuf* result, FFstrbuf* error)
{
    DWORD bufSize; //with tailing '\0'
    if(RegGetValueA(hKey, NULL, valueName, RRF_RT_REG_SZ, NULL, NULL, &bufSize) != ERROR_SUCCESS)
    {
        if(error) ffStrbufAppendF(error, "RegGetValueA(%s, NULL, RRF_RT_REG_SZ) failed", valueName ? valueName : "(default)");
        return false;
    }
    ffStrbufEnsureFree(result, bufSize - 1);
    if(RegGetValueA(hKey, NULL, valueName, RRF_RT_REG_SZ, NULL, result->chars, &bufSize) != ERROR_SUCCESS)
    {
        if(error) ffStrbufAppendF(error, "RegGetValueA(%s, result, RRF_RT_REG_SZ) failed", valueName ? valueName : "(default)");
        return false;
    }
    result->length = bufSize - 1;
    return true;
}

bool ffRegReadUint(HKEY hKey, const char* valueName, uint32_t* result, FFstrbuf* error)
{
    DWORD bufSize = sizeof(*result);
    if(RegGetValueA(hKey, NULL, valueName, RRF_RT_DWORD, NULL, result, &bufSize) != ERROR_SUCCESS)
    {
        if(error) ffStrbufAppendF(error, "RegGetValueA(%s, result, RRF_RT_DWORD) failed", valueName ? valueName : "(default)");
        return false;
    }
    return true;
}
