#include "common/windows/glob.h"

#include "common/FFlist.h"
#include "common/mallocHelper.h"
#include "common/time.h"
#include "common/windows/nt.h"
#include "common/windows/unicode.h"

#include <wchar.h>
#include <windows.h>

// Windows has no glob(3). FindFirstFileW() accepts wildcards only in the last component, and its
// wildcard matching follows the old DOS rules (a `?` can match a zero-length suffix, and `*.ext`
// matches files whose 8.3 short name has that extension). Enumerating a directory with `*` and
// applying our own matcher gives the same semantics as glob(3) on Unix, including character classes
// and wildcards in intermediate components.
//
// Everything below works on UTF-16 so that names outside the ANSI code page survive, and so that
// `?` matches one Unicode character instead of one UTF-8 byte.

static bool isEnvNameStart(wchar_t value) {
    return (value >= L'A' && value <= L'Z') || (value >= L'a' && value <= L'z') || value == L'_';
}

static bool isEnvNameChar(wchar_t value) {
    return isEnvNameStart(value) || (value >= L'0' && value <= L'9');
}

// Appends the value of the environment variable whose name is `name[0 .. nameLength)`.
// Returns false when the variable is not defined, so the caller can leave the original
// `%NAME%` / `$NAME` text in place.
static bool appendEnvValue(const wchar_t* name, uint32_t nameLength, FFWStrbuf* out) {
    if (nameLength == 0 || nameLength > 0x7FFF) {
        return false;
    }

    wchar_t stackName[128];
    wchar_t* nameBuffer = nameLength < ARRAY_SIZE(stackName)
        ? stackName
        : (wchar_t*) malloc(((size_t) nameLength + 1) * sizeof(wchar_t));
    if (nameBuffer == nullptr) {
        return false;
    }

    memcpy(nameBuffer, name, (size_t) nameLength * sizeof(wchar_t));
    nameBuffer[nameLength] = L'\0';

    // An empty variable is reported as a required length of zero; the last error tells it apart
    // from a variable that does not exist at all.
    SetLastError(ERROR_SUCCESS);
    DWORD needed = GetEnvironmentVariableW(nameBuffer, nullptr, 0);
    bool found = false;
    if (needed == 0) {
        found = GetLastError() == ERROR_SUCCESS;
    } else {
        FF_AUTO_FREE wchar_t* value = (wchar_t*) malloc((size_t) needed * sizeof(wchar_t));
        if (value != nullptr) {
            DWORD written = GetEnvironmentVariableW(nameBuffer, value, needed);
            if (written > 0) {
                ffWStrbufAppendNS(out, (uint32_t) written, value);
                found = true;
            }
        }
    }

    if (nameBuffer != stackName) {
        free(nameBuffer);
    }
    return found;
}

// Expands `%NAME%`, `$NAME` and `${NAME}` in one pass. Values inserted by the expansion are not
// expanded again.
static void expandEnv(const wchar_t* in, FFWStrbuf* out) {
    for (const wchar_t* p = in; *p != L'\0';) {
        if (*p == L'%') {
            const wchar_t* end = wcschr(p + 1, L'%');
            if (end != nullptr && end != p + 1 && appendEnvValue(p + 1, (uint32_t) (end - p - 1), out)) {
                p = end + 1;
                continue;
            }
            ffWStrbufAppendC(out, *p++);
        } else if (*p == L'$') {
            const wchar_t* name = p + 1;
            const wchar_t* after = nullptr;

            if (*name == L'{') {
                const wchar_t* end = wcschr(name + 1, L'}');
                if (end != nullptr && end != name + 1) {
                    ++name;
                    after = end + 1;
                    if (appendEnvValue(name, (uint32_t) (end - name), out)) {
                        p = after;
                        continue;
                    }
                }
            } else if (isEnvNameStart(*name)) {
                const wchar_t* end = name + 1;
                while (isEnvNameChar(*end)) {
                    ++end;
                }
                after = end;
                if (appendEnvValue(name, (uint32_t) (end - name), out)) {
                    p = after;
                    continue;
                }
            }

            ffWStrbufAppendC(out, *p++);
        } else {
            ffWStrbufAppendC(out, *p++);
        }
    }
}

static bool isSeparator(wchar_t value) {
    return value == L'\\' || value == L'/';
}

static const wchar_t* findSeparator(const wchar_t* value) {
    while (*value != L'\0' && !isSeparator(*value)) {
        ++value;
    }
    return *value == L'\0' ? nullptr : value;
}

static void appendPathComponent(FFWStrbuf* path, const wchar_t* component, uint32_t length) {
    // `C:` is drive-relative: `C:foo`, not `C:\foo`.
    if (path->length > 0 && !isSeparator(path->chars[path->length - 1]) &&
        !(path->length == 2 && path->chars[1] == L':')) {
        ffWStrbufAppendC(path, L'\\');
    }
    ffWStrbufAppendNS(path, length, component);
}

static bool componentHasWildcard(const wchar_t* value, uint32_t length) {
    for (uint32_t i = 0; i < length; ++i) {
        if (value[i] == L'*' || value[i] == L'?' || value[i] == L'[') {
            return true;
        }
    }
    return false;
}

// Returns -1 if the pattern is not a valid bracket expression, 0 if it is valid but does not
// match `value`, and 1 if it matches.
static int matchBracket(wchar_t value, const wchar_t* pattern, uint32_t length, uint32_t* consumed) {
    assert(length > 0 && pattern[0] == L'[');

    uint32_t i = 1;
    bool negate = false;
    if (i < length && (pattern[i] == L'!' || pattern[i] == L'^')) {
        negate = true;
        ++i;
    }

    bool matched = false;
    bool first = true;
    uint32_t closing = UINT32_MAX;
    wchar_t upperValue = (wchar_t) RtlUpcaseUnicodeChar(value);

    while (i < length) {
        if (pattern[i] == L']' && !first) {
            closing = i;
            break;
        }
        first = false;

        wchar_t start = pattern[i];
        wchar_t end = start;
        uint32_t next = i + 1;
        if (next + 1 < length && pattern[next] == L'-' && pattern[next + 1] != L']') {
            end = pattern[next + 1];
            next += 2;
        }

        wchar_t upperStart = (wchar_t) RtlUpcaseUnicodeChar(start);
        wchar_t upperEnd = (wchar_t) RtlUpcaseUnicodeChar(end);
        if (upperValue >= upperStart && upperValue <= upperEnd) {
            matched = true;
        }

        i = next;
    }

    if (closing == UINT32_MAX) {
        return -1;
    }

    *consumed = closing + 1;
    return negate ? !matched : matched;
}

// Matches one character, and reports how many pattern characters were consumed.
static bool matchOne(wchar_t value, const wchar_t* pattern, uint32_t length, uint32_t* consumed) {
    if (pattern[0] == L'?') {
        *consumed = 1;
        return true;
    }

    if (pattern[0] == L'[') {
        int result = matchBracket(value, pattern, length, consumed);
        if (result >= 0) {
            return result != 0;
        }
        // Not a valid bracket expression: treat '[' as a literal character.
    }

    *consumed = 1;
    return RtlUpcaseUnicodeChar(value) == RtlUpcaseUnicodeChar(pattern[0]);
}

static bool wildcardMatch(const wchar_t* name, uint32_t nameLength, const wchar_t* pattern, uint32_t patternLength) {
    uint32_t nameIndex = 0;
    uint32_t patternIndex = 0;
    uint32_t starPattern = UINT32_MAX;
    uint32_t starName = 0;

    while (nameIndex < nameLength) {
        if (patternIndex < patternLength && pattern[patternIndex] == L'*') {
            while (patternIndex < patternLength && pattern[patternIndex] == L'*') {
                ++patternIndex;
            }
            if (patternIndex == patternLength) {
                return true;
            }
            starPattern = patternIndex;
            starName = nameIndex;
            continue;
        }

        uint32_t consumed = 0;
        if (patternIndex < patternLength &&
            matchOne(name[nameIndex], pattern + patternIndex, patternLength - patternIndex, &consumed)) {
            ++nameIndex;
            patternIndex += consumed;
            continue;
        }

        if (starPattern == UINT32_MAX) {
            return false;
        }

        patternIndex = starPattern;
        nameIndex = ++starName;
    }

    while (patternIndex < patternLength && pattern[patternIndex] == L'*') {
        ++patternIndex;
    }
    return patternIndex == patternLength;
}

// Splits the volume / root prefix from `pattern` and appends it to `path`.
static const wchar_t* parseRoot(FFWStrbuf* path, const wchar_t* pattern) {
    if (isSeparator(pattern[0]) && isSeparator(pattern[1])) {
        // UNC path: \\server\share\...
        const wchar_t* server = pattern + 2;
        const wchar_t* serverEnd = findSeparator(server);
        if (serverEnd == nullptr) {
            uint32_t length = (uint32_t) wcslen(pattern);
            ffWStrbufAppendNS(path, length, pattern);
            return pattern + length;
        }

        const wchar_t* shareEnd = findSeparator(serverEnd + 1);
        if (shareEnd == nullptr) {
            uint32_t length = (uint32_t) wcslen(pattern);
            ffWStrbufAppendNS(path, length, pattern);
            return pattern + length;
        }

        uint32_t prefixLength = (uint32_t) (shareEnd + 1 - pattern);
        ffWStrbufAppendNS(path, prefixLength, pattern);
        return shareEnd + 1;
    }

    if (((pattern[0] >= L'A' && pattern[0] <= L'Z') || (pattern[0] >= L'a' && pattern[0] <= L'z')) &&
        pattern[1] == L':') {
        ffWStrbufAppendNS(path, 2, pattern);
        if (isSeparator(pattern[2])) {
            ffWStrbufAppendC(path, L'\\');
            return pattern + 3;
        }
        return pattern + 2;
    }

    if (isSeparator(pattern[0])) {
        ffWStrbufAppendC(path, L'\\');
        return pattern + 1;
    }

    return pattern;
}

static void globRecurse(FFWStrbuf* path, const wchar_t* pattern, FFlist* results) {
    while (isSeparator(*pattern)) {
        ++pattern;
    }

    if (*pattern == L'\0') {
        if (GetFileAttributesW(path->chars) == INVALID_FILE_ATTRIBUTES) {
            return;
        }

        FFstrbuf* result = FF_LIST_ADD(FFstrbuf, *results);
        ffStrbufInitNWS(result, path->length, path->chars);
        return;
    }

    const wchar_t* end = pattern;
    while (*end != L'\0' && !isSeparator(*end)) {
        ++end;
    }
    uint32_t componentLength = (uint32_t) (end - pattern);

    uint32_t savedLength = path->length;
    if (!componentHasWildcard(pattern, componentLength)) {
        appendPathComponent(path, pattern, componentLength);
        globRecurse(path, end, results);
        path->length = savedLength;
        path->chars[savedLength] = L'\0';
        return;
    }

    appendPathComponent(path, L"*", 1);
    WIN32_FIND_DATAW entry;
    HANDLE handle = FindFirstFileW(path->chars, &entry);
    path->length = savedLength;
    path->chars[savedLength] = L'\0';
    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        // Like glob(3), a wildcard does not match entries whose name starts with a dot unless the
        // pattern itself starts with one. `.` and `..` are never returned as glob results.
        if (entry.cFileName[0] == L'.' && pattern[0] != L'.') {
            continue;
        }
        if (entry.cFileName[0] == L'.' &&
            (entry.cFileName[1] == L'\0' || (entry.cFileName[1] == L'.' && entry.cFileName[2] == L'\0'))) {
            continue;
        }

        uint32_t nameLength = (uint32_t) wcslen(entry.cFileName);
        if (!wildcardMatch(entry.cFileName, nameLength, pattern, componentLength)) {
            continue;
        }

        uint32_t lengthBeforeEntry = path->length;
        appendPathComponent(path, entry.cFileName, nameLength);
        globRecurse(path, end, results);
        path->length = lengthBeforeEntry;
        path->chars[lengthBeforeEntry] = L'\0';
    } while (FindNextFileW(handle, &entry));

    FindClose(handle);
}

bool ffGlobWindows(const char* in, FFstrbuf* out) {
    FF_WSTRBUF_AUTO_DESTROY wide = ffWStrbufCreateUtf8(in);
    if (wide.chars == nullptr) {
        return false;
    }

    FF_WSTRBUF_AUTO_DESTROY expanded = ffWStrbufCreate();
    expandEnv(wide.chars, &expanded);

    if (wcspbrk(expanded.chars, L"*?[") == nullptr) {
        ffStrbufSetNWS(out, expanded.length, expanded.chars);
        return true;
    }

    FF_WSTRBUF_AUTO_DESTROY path = ffWStrbufCreate();
    const wchar_t* components = parseRoot(&path, expanded.chars);

    FF_LIST_AUTO_DESTROY results = ffListCreate();
    globRecurse(&path, components, &results);

    bool found = results.length > 0;
    if (found) {
        uint32_t index = results.length > 1 ? (uint32_t) (ffTimeGetNow() % results.length) : 0;
        ffStrbufSet(out, FF_LIST_GET(FFstrbuf, results, index));
    }

    if (results.length > 0) {
        FF_LIST_FOR_EACH (FFstrbuf, result, results) {
            ffStrbufDestroy(result);
        }
    }
    return found;
}
