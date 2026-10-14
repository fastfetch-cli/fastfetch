#pragma once

#include "fastfetch.h"
#include "common/io.h"

#include <wchar.h> // ffGetProfilePath() takes a SID as a wide string

// The known folders fastfetch needs. They are spelled out instead of taking a REFKNOWNFOLDERID so
// that this header does not have to drag in <shlobj.h> -- the call sites include it for reasons of
// their own, and a header that only works when <windows.h> came first is a trap.
typedef enum FFKnownFolder : uint8_t {
    FF_KNOWN_FOLDER_LOCAL_APP_DATA,
    FF_KNOWN_FOLDER_ROAMING_APP_DATA,
    FF_KNOWN_FOLDER_PROGRAM_DATA,
    FF_KNOWN_FOLDER_PROGRAM_FILES,
    FF_KNOWN_FOLDER_COUNT,
} FFKnownFolder;

// Resolves a known folder without initializing the shell.
//
// The first SHGetKnownFolderPath() call in a process costs ~2.4 ms (measured on Windows 10 22H2):
// the shell's known-folder machinery is built on demand, and whichever folder is asked for first
// pays for it. Every later call is free, so a process that needs a handful of folders pays that
// price once and can never amortize it. Each folder below is instead one registry value or one
// environment variable away -- ~0.03 ms for all of them together -- so the shell is never touched.
//
// The answer is cached: these five folders are asked for about a dozen times per process, and each
// answer is a pure function of the registry and this process' environment. That is safe without a
// lock, because nothing that resolves a known folder ever runs on the one thread fastfetch creates.
//
// Returns false, leaving `result` untouched, when the value is missing or unusable, so that the
// caller keeps whatever fallback it had for the shell API failing. The path uses backslashes,
// exactly like SHGetKnownFolderPath().
bool ffGetKnownFolderPath(FFKnownFolder folder, FFstrbuf* result);

// Resolves the profile directory recorded for `sidW`, a SID string as produced by
// RtlConvertSidToUnicodeString().
//
// There is no FF_KNOWN_FOLDER_PROFILE because the caller already has the SID:
// FFPlatform_windows.c resolves it once for `platform->sid`, and deriving it a second time here
// would mean a second NtQueryInformationToken(). The answer is deliberately not cached -- this is
// called once per process, and from one place.
//
// Returns false, leaving `result` untouched, when the value is missing or unusable, so that the
// caller keeps whatever fallback it had for the shell API failing. The path uses backslashes,
// exactly like SHGetKnownFolderPath().
bool ffGetProfilePath(const wchar_t* sidW, FFstrbuf* result);
