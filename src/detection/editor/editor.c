#include "editor.h"
#include "common/processing.h"
#include "common/library.h"
#include "common/strutil.h"
#include "common/path.h"
#include "common/binary.h"

#include <stdlib.h>

static bool extractNvimVersionFromBinary(const char* str, [[maybe_unused]] uint32_t len, void* userdata) {
    if (!ffStrStartsWith(str, "NVIM v")) {
        return true;
    }
    ffStrbufSetS((FFstrbuf*) userdata, str + strlen("NVIM v"));
    return false;
}

static bool extractVimVersionFromBinary(const char* str, [[maybe_unused]] uint32_t len, void* userdata) {
    if (!ffStrStartsWith(str, "VIM - Vi IMproved ")) {
        return true;
    }
    ffStrbufSetS((FFstrbuf*) userdata, str + strlen("VIM - Vi IMproved "));
    ffStrbufSubstrBeforeFirstC(userdata, ' ');
    return false;
}

static bool extractNanoVersionFromBinary(const char* str, [[maybe_unused]] uint32_t len, void* userdata) {
    if (!ffStrStartsWith(str, "GNU nano ")) {
        return true;
    }
    ffStrbufSetS((FFstrbuf*) userdata, str + strlen("GNU nano "));
    return false;
}

// The message is printed to the user verbatim, so it has to name the variable that was wrong:
// `"Executable not found"` from `ffFindExecutableInPath()` doesn't say what was searched for.
static const char* notAnExecutable(bool isVisual) {
    return isVisual ? "$VISUAL does not point to an executable" : "$EDITOR does not point to an executable";
}

const char* ffDetectEditor(FFEditorResult* result) {
    bool isVisual = true;
    ffStrbufSetS(&result->name, getenv("VISUAL"));
    if (!result->name.length) {
        isVisual = false;
        ffStrbufSetS(&result->name, getenv("EDITOR"));
    }

    if (!result->name.length) {
        return "$VISUAL or $EDITOR not set";
    }
    result->type = isVisual ? "Visual" : "Editor";

    // `$VISUAL`/`$EDITOR` hold a shell command line rather than a bare name, so they routinely
    // carry arguments: `EDITOR="code -w"`, `EDITOR="vim -f"`. Only the first word names the
    // program. The whole string used to be handed to `ffFindExecutableInPath()`, which could never
    // match it, and the module then printed the unparsed value as if it had been resolved.
    ffStrbufSubstrBefore(&result->name, (uint32_t) strcspn(result->name.chars, " \t\n\v\f\r"));

    if (result->name.length == 0) {
        return notAnExecutable(isVisual);
    }

    if (ffIsAbsolutePath(result->name.chars)) {
        ffStrbufSet(&result->path, &result->name);
    } else {
        const char* error = ffFindExecutableInPath(result->name.chars, &result->path);
        if (error) {
            // Reporting success here is what made `EDITOR="code -w"` print the raw value: the module
            // reads `nullptr` as "detection succeeded" and then falls back to printing `name`.
            return notAnExecutable(isVisual);
        }
    }

    {
        char buf[PATH_MAX + 1];
        if (!realpath(result->path.chars, buf)) {
            // An absolute `$VISUAL`/`$EDITOR` is copied into `path` before it is verified, so a path
            // that does not exist has to be reported here instead of being handed to the caller as
            // if it had been checked.
            return notAnExecutable(isVisual);
        }

        // WIN32: Should we handle scoop shim exe here?

#ifdef __linux__
        if (!ffStrEndsWith(buf, "/snap"))
#endif
            ffStrbufSetS(&result->path, buf);
    }

    {
        uint32_t index = ffStrbufLastIndexC(&result->path,
#ifndef _WIN32
            '/'
#else
            '\\'
#endif
        );
        if (index == result->path.length) {
            // `realpath()` always yields an absolute path, so neither of these two is reachable in
            // practice. They used to `return nullptr`, which the module read as a success.
            return "Failed to determine the executable name";
        }
        ffStrbufSetS(&result->exe, &result->path.chars[index + 1]);
        if (!result->exe.length) {
            return "Failed to determine the executable name";
        }

#ifdef _WIN32
        if (ffStrbufEndsWithS(&result->exe, ".exe")) {
            ffStrbufSubstrBefore(&result->exe, result->exe.length - 4);
        }
#endif
    }

    if (!instance.config.general.detectVersion) {
        // Not an error: the editor was found, version probing was just turned off.
        return nullptr;
    }

    if (ffStrbufEqualS(&result->exe, "nvim")) {
        ffBinaryExtractStrings(result->path.chars, extractNvimVersionFromBinary, &result->version, (uint32_t) strlen("NVIM v0.0.0"));
    } else if (ffStrbufEqualS(&result->exe, "vim") || ffStrbufStartsWithS(&result->exe, "vim.")) {
        ffBinaryExtractStrings(result->path.chars, extractVimVersionFromBinary, &result->version, (uint32_t) strlen("VIM - Vi IMproved 0.0"));
    } else if (ffStrbufEqualS(&result->exe, "nano")) {
        ffBinaryExtractStrings(result->path.chars, extractNanoVersionFromBinary, &result->version, (uint32_t) strlen("GNU nano 0.0"));
    }

    if (result->version.length > 0) {
        return nullptr;
    }

    const char* param = nullptr;
    if (
        ffStrbufEqualS(&result->exe, "nano") ||
        ffStrbufEqualS(&result->exe, "vim") ||
        ffStrbufStartsWithS(&result->exe, "vim.") || // vim.basic/vim.tiny
        ffStrbufEqualS(&result->exe, "nvim") ||
        ffStrbufEqualS(&result->exe, "micro") ||
        ffStrbufEqualS(&result->exe, "emacs") ||
        ffStrbufStartsWithS(&result->exe, "emacs-") || // emacs-29.3
        ffStrbufEqualS(&result->exe, "hx") ||
        ffStrbufEqualS(&result->exe, "code") ||
        ffStrbufEqualS(&result->exe, "pluma") ||
        ffStrbufEqualS(&result->exe, "sublime_text") ||
        ffStrbufEqualS(&result->exe, "zeditor")) {
        param = "--version";
    } else if (
        ffStrbufEqualS(&result->exe, "kak") ||
        ffStrbufEqualS(&result->exe, "pico")) {
        param = "-version";
    } else if (
        ffStrbufEqualS(&result->exe, "ne")) {
        param = "-h";
    } else {
        return nullptr;
    }

    ffProcessAppendStdOut(&result->version, (char* const[]) {
                                                result->path.chars,
                                                (char*) param,
                                                nullptr,
                                            });

    if (result->version.length == 0) {
        return nullptr;
    }

    ffStrbufSubstrBeforeFirstC(&result->version, '\n');
    const char* versionStart = strpbrk(result->version.chars, "0123456789");
    if (versionStart != nullptr) {
        const char* versionEnd = strpbrk(versionStart, " \t\v\f\r");
        if (versionEnd != nullptr) {
            ffStrbufSubstrBefore(&result->version, (uint32_t) (versionEnd - result->version.chars));
        }

        if (versionStart != result->version.chars) {
            ffStrbufSubstrAfter(&result->version, (uint32_t) (versionStart - result->version.chars - 1));
        }
    }

    return nullptr;
}
