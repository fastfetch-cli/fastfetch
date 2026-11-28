#include "common/windows/glob.h"
#include "common/windows/unicode.h"
#include "common/textModifier.h"

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <wchar.h>

noreturn static void testFailed(const char* expression, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expression);
    exit(1);
}

#define VERIFY(expression) \
    if (!(expression)) testFailed(#expression, __LINE__)

static void joinPath(wchar_t* result, size_t capacity, const wchar_t* base, const wchar_t* name) {
    if (swprintf(result, capacity, L"%ls\\%ls", base, name) < 0) {
        testFailed("swprintf", __LINE__);
    }
}

static void writeFile(const wchar_t* path) {
    HANDLE handle = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        testFailed("CreateFileW", __LINE__);
    }
    CloseHandle(handle);
}

static void createDirectory(const wchar_t* path) {
    if (!CreateDirectoryW(path, nullptr)) {
        testFailed("CreateDirectoryW", __LINE__);
    }
}

static void createFileIn(const wchar_t* directory, const wchar_t* name) {
    wchar_t path[MAX_PATH];
    joinPath(path, MAX_PATH, directory, name);
    writeFile(path);
}

static void expectGlob(const wchar_t* pattern, const wchar_t* expected) {
    FF_STRBUF_AUTO_DESTROY patternUtf8 = ffStrbufCreateWS(pattern);
    FF_STRBUF_AUTO_DESTROY expectedUtf8 = ffStrbufCreateWS(expected);
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    if (!ffGlobWindows(patternUtf8.chars, &result) || !ffStrbufEqual(&result, &expectedUtf8)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "glob mismatch for %ls: expected %ls, got %s\n" FASTFETCH_TEXT_MODIFIER_RESET, pattern, expected, result.chars ? result.chars : "(null)");
        exit(1);
    }
}

static void expectNoGlob(const wchar_t* pattern) {
    FF_STRBUF_AUTO_DESTROY patternUtf8 = ffStrbufCreateWS(pattern);
    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
    if (ffGlobWindows(patternUtf8.chars, &result)) {
        fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "glob unexpectedly matched %ls: %s\n" FASTFETCH_TEXT_MODIFIER_RESET, pattern, result.chars);
        exit(1);
    }
}

int main(void) {
    wchar_t tempPath[MAX_PATH];
    DWORD tempLength = GetTempPathW(MAX_PATH, tempPath);
    VERIFY(tempLength > 0 && tempLength < MAX_PATH);

    wchar_t root[MAX_PATH];
    VERIFY(swprintf(root, MAX_PATH, L"%lsfastfetch-glob-%lu-%lu", tempPath, GetCurrentProcessId(), GetTickCount()) > 0);
    createDirectory(root);

    wchar_t sub[MAX_PATH], dot[MAX_PATH], weirdDollar[MAX_PATH], weirdPercent[MAX_PATH];
    joinPath(sub, MAX_PATH, root, L"sub");
    joinPath(dot, MAX_PATH, root, L"dot");
    joinPath(weirdDollar, MAX_PATH, root, L"$weird");
    joinPath(weirdPercent, MAX_PATH, root, L"%weird%");
    createDirectory(sub);
    createDirectory(dot);
    createDirectory(weirdDollar);
    createDirectory(weirdPercent);

    createFileIn(root, L"a.txt");
    createFileIn(root, L"b.txt");
    createFileIn(root, L"c.log");
    createFileIn(root, L"d.pngx");
    createFileIn(root, L".hidden.txt");
    createFileIn(sub, L"x.txt");
    createFileIn(dot, L"only.txt");
    createFileIn(dot, L".hidden.txt");
    createFileIn(weirdDollar, L"z.txt");
    createFileIn(weirdPercent, L"w.txt");

    wchar_t pattern[MAX_PATH], expected[MAX_PATH];

    // Plain paths are returned as they are.
    joinPath(pattern, MAX_PATH, root, L"a.txt");
    expectGlob(pattern, pattern);

    // `?`, character classes and negated classes.
    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\?.log", root) > 0);
    joinPath(expected, MAX_PATH, root, L"c.log");
    expectGlob(pattern, expected);

    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\[a].txt", root) > 0);
    joinPath(expected, MAX_PATH, root, L"a.txt");
    expectGlob(pattern, expected);

    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\[!a].txt", root) > 0);
    joinPath(expected, MAX_PATH, root, L"b.txt");
    expectGlob(pattern, expected);

    // Wildcards work in intermediate components too.
    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\*\\x.txt", root) > 0);
    joinPath(expected, MAX_PATH, sub, L"x.txt");
    expectGlob(pattern, expected);

    // Dot files are excluded unless the pattern starts with a dot.
    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\dot\\*.txt", root) > 0);
    joinPath(expected, MAX_PATH, dot, L"only.txt");
    expectGlob(pattern, expected);

    // `%NAME%`, `$NAME` and `${NAME}` are all accepted.
    joinPath(expected, MAX_PATH, sub, L"x.txt");
    VERIFY(SetEnvironmentVariableW(L"FASTFETCH_TEST_GLOB_DIR", sub));
    VERIFY(swprintf(pattern, MAX_PATH, L"%%FASTFETCH_TEST_GLOB_DIR%%\\x.txt") > 0);
    expectGlob(pattern, expected);
    VERIFY(swprintf(pattern, MAX_PATH, L"$FASTFETCH_TEST_GLOB_DIR\\x.txt") > 0);
    expectGlob(pattern, expected);
    VERIFY(swprintf(pattern, MAX_PATH, L"${FASTFETCH_TEST_GLOB_DIR}\\x.txt") > 0);
    expectGlob(pattern, expected);
    VERIFY(SetEnvironmentVariableW(L"FASTFETCH_TEST_GLOB_DIR", nullptr));

    // Undefined variables are literal, so paths containing `$` or `%` still work.
    VERIFY(SetEnvironmentVariableW(L"weird", nullptr));
    joinPath(pattern, MAX_PATH, weirdDollar, L"z.txt");
    expectGlob(pattern, pattern);
    joinPath(pattern, MAX_PATH, weirdPercent, L"w.txt");
    expectGlob(pattern, pattern);

    // No match is reported as such. `*.png` must not fall back to the 8.3 short name of `d.pngx`,
    // and `?` must not match an empty suffix.
    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\nope*.txt", root) > 0);
    expectNoGlob(pattern);
    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\*.png", root) > 0);
    expectNoGlob(pattern);
    VERIFY(swprintf(pattern, MAX_PATH, L"%ls\\c?.log", root) > 0);
    expectNoGlob(pattern);

    wchar_t path[MAX_PATH];
    const wchar_t* files[] = { L"a.txt", L"b.txt", L"c.log", L"d.pngx", L".hidden.txt" };
    for (uint32_t i = 0; i < ARRAY_SIZE(files); ++i) {
        joinPath(path, MAX_PATH, root, files[i]);
        DeleteFileW(path);
    }
    const wchar_t* subFiles[] = { L"x.txt" };
    for (uint32_t i = 0; i < ARRAY_SIZE(subFiles); ++i) {
        joinPath(path, MAX_PATH, sub, subFiles[i]);
        DeleteFileW(path);
    }
    const wchar_t* dotFiles[] = { L"only.txt", L".hidden.txt" };
    for (uint32_t i = 0; i < ARRAY_SIZE(dotFiles); ++i) {
        joinPath(path, MAX_PATH, dot, dotFiles[i]);
        DeleteFileW(path);
    }
    joinPath(path, MAX_PATH, weirdDollar, L"z.txt");
    DeleteFileW(path);
    joinPath(path, MAX_PATH, weirdPercent, L"w.txt");
    DeleteFileW(path);

    RemoveDirectoryW(sub);
    RemoveDirectoryW(dot);
    RemoveDirectoryW(weirdDollar);
    RemoveDirectoryW(weirdPercent);
    RemoveDirectoryW(root);

    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
    return 0;
}
