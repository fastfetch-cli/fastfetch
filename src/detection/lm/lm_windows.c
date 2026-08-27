#include "lm.h"

#include "common/windows/version.h"
#include "common/windows/nt.h"

#include <wchar.h>

const char* ffDetectLM(FFLMResult* result) {
    ffStrbufSetStatic(&result->service, "LogonUI");
    ffStrbufSetStatic(&result->prettyName, "Logon User Interface");

    if (instance.config.general.detectVersion) {
        wchar_t exePath[MAX_PATH];
        _snwprintf(exePath, ARRAY_SIZE(exePath), L"%ls\\system32\\%ls", (const wchar_t*) SharedUserData->NtSystemRoot, L"LogonUI.exe");
        ffGetFileVersion(exePath, nullptr, &result->version);
    }

    return nullptr;
}
