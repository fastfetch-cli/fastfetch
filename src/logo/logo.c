#include "logo/logo.h"
#include "common/io.h"
#include "common/printing.h"
#include "common/processing.h"
#include "common/textModifier.h"
#include "common/strutil.h"
#include "detection/media/media.h"
#include "detection/os/os.h"
#include "detection/terminalshell/terminalshell.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct FFLogoCachedLine {
    FFstrbuf chars;
    uint32_t width;
} FFLogoCachedLine;

static void logoLineCacheClear(FFLogoLineCacheState* cache) {
    FF_LIST_FOR_EACH (FFLogoCachedLine, line, cache->lines) {
        ffStrbufDestroy(&line->chars);
    }
    ffListDestroy(&cache->lines);
    cache->nextLine = 0;
    cache->rightOffset = 0;
}

static void logoLineCachePush(const FFstrbuf* chars, uint32_t width, FFLogoLineCacheState* cache) {
    FFLogoCachedLine* line = FF_LIST_ADD(FFLogoCachedLine, cache->lines);
    if (width > 0) {
        ffStrbufInitCopy(&line->chars, chars);
        if (!instance.config.display.pipe) {
            ffStrbufAppendS(&line->chars, FASTFETCH_TEXT_MODIFIER_RESET);
        }
    } else {
        ffStrbufInit(&line->chars);
    }
    line->width = width;
}

static void logoLineCacheBuild(FFLogoLineCacheState* cache, const char* data, bool doColorReplacement) {
    FFOptionsLogo* options = &instance.config.logo;
    bool keepCarryColor = options->type != FF_LOGO_TYPE_IMAGE_CHAFA;

    logoLineCacheClear(cache);

    // Overrides the auto detected max-width with the configured width, if set.
    // In case we fail to get the actual width of the logo, as we don't use `wcwidth`
    uint32_t maxLineWidth = options->width;
    uint32_t parsedHeight = 0;

    FF_STRBUF_AUTO_DESTROY carryColor = ffStrbufCreate();
    if (keepCarryColor && doColorReplacement && !instance.config.display.pipe) {
        ffStrbufSetF(&carryColor, "\e[%sm", options->colors[0].chars);
    }

    for (uint32_t i = 0; i < options->paddingTop; ++i) {
        logoLineCachePush(NULL, 0, cache);
    }

    if (*data != '\0') {
        while (true) {
            FF_STRBUF_AUTO_DESTROY line = ffStrbufCreateA(256);
            uint32_t lineWidth = 0;

            if (!instance.config.display.pipe && instance.config.display.brightColor) {
                ffStrbufAppendS(&line, FASTFETCH_TEXT_MODIFIER_BOLT);
            }

            if (keepCarryColor && carryColor.length > 0) {
                ffStrbufAppend(&line, &carryColor);
            }

            if ((options->position != FF_LOGO_POSITION_RIGHT) && options->paddingLeft > 0) {
                ffStrbufAppendNC(&line, options->paddingLeft, ' ');
                lineWidth += options->paddingLeft;
            }

            while (*data != '\0' && *data != '\n' && !(*data == '\r' && *(data + 1) == '\n')) {
                if (*data == '\t') {
                    ffStrbufAppendNC(&line, 4, ' ');
                    lineWidth += 4;
                    ++data;
                    continue;
                }

                if (*data == '\e' && *(data + 1) == '[') {
                    const char* start = data;
                    data += 2;

                    while (ffCharIsDigit(*data) || *data == ';') {
                        ++data;
                    }

                    if (isascii(*data)) {
                        ++data;

                        uint32_t escLen = (uint32_t) (data - start);
                        ffStrbufAppendNS(&line, escLen, start);

                        if (keepCarryColor && start[escLen - 1] == 'm') {
                            ffStrbufSetNS(&carryColor, escLen, start);
                        }
                        continue;
                    }

                    lineWidth += (uint32_t) (data - start - 1);
                }

                if (doColorReplacement && *data == '$') {
                    ++data;

                    if (*data == '$' || *data == '\0') {
                        ffStrbufAppendC(&line, '$');
                        ++lineWidth;
                        ++data;
                        continue;
                    }

                    if (!instance.config.display.pipe) {
                        int index = *data - '1';
                        if (index >= 0 && index < FASTFETCH_LOGO_MAX_COLORS) {
                            if (keepCarryColor) {
                                ffStrbufSetF(&carryColor, "\e[%sm", options->colors[index].chars);
                                ffStrbufAppend(&line, &carryColor);
                            } else {
                                ffStrbufAppendF(&line, "\e[%sm", options->colors[index].chars);
                            }
                            ++data;
                            continue;
                        }

                        ffStrbufAppendC(&line, '$');
                        ++lineWidth;
                    } else {
                        ++data;
                        continue;
                    }
                }

                uint8_t charWidth;
                uint8_t bytes = ffUtf8CharLenWidth(data, UINT32_MAX, &charWidth);
                lineWidth += charWidth;

                for (uint8_t i = 0; i < bytes; ++i) {
                    if (*data == '\0') {
                        break;
                    }

                    ffStrbufAppendC(&line, *data);
                    ++data;
                }
            }

            logoLineCachePush(&line, lineWidth, cache);
            if (lineWidth > maxLineWidth) {
                maxLineWidth = lineWidth;
            }

            if (*data == '\n' || (*data == '\r' && *(data + 1) == '\n')) {
                if (*data == '\r') {
                    ++data;
                }
                ++data;
                ++parsedHeight;
                continue;
            }

            break;
        }
    }

    if (options->type != FF_LOGO_TYPE_IMAGE_CHAFA && options->height > parsedHeight) {
        parsedHeight = options->height;
    }

    instance.state.logoHeight = options->paddingTop + parsedHeight;
    if (options->position == FF_LOGO_POSITION_LEFT) {
        instance.state.logoWidth = maxLineWidth + options->paddingRight;
    } else {
        instance.state.logoWidth = 0;
    }

    uint32_t totalLines = instance.state.logoHeight + 1;
    while (cache->lines.length < totalLines) {
        logoLineCachePush(NULL, 0, cache);
    }

    cache->nextLine = 0;
    cache->rightOffset = maxLineWidth + options->paddingRight - 1;
}

static bool ffLogoPrintCharsRaw(const char* data, size_t length, bool printError) {
    FFOptionsLogo* options = &instance.config.logo;
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();

    if (!options->width || !options->height) {
        if (options->position == FF_LOGO_POSITION_LEFT) {
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH", (unsigned) options->paddingTop + 1, (unsigned) options->paddingLeft + 1);
        } else if (options->position == FF_LOGO_POSITION_TOP) {
            ffStrbufAppendNC(&buf, options->paddingTop, '\n');
            ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        } else if (options->position == FF_LOGO_POSITION_RIGHT) {
            if (!options->width) {
                if (printError) {
                    fputs("Logo (image-raw): Must set logo width when using position right\n", stderr);
                }
                return false;
            }
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;9999999H\e[%uD", (unsigned) options->paddingTop + 1, (unsigned) options->paddingRight + options->width);
        }
        ffStrbufAppendNS(&buf, (uint32_t) length, data);
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

        if (options->position == FF_LOGO_POSITION_LEFT || options->position == FF_LOGO_POSITION_RIGHT) {
            uint16_t X = 0, Y = 0;
            // Windows Terminal doesn't report `\e` for some reason
            const char* error = ffGetTerminalResponse("\e[6n", 2, "%*[^0-9]%hu;%huR", &Y, &X); // %*[^0-9]: ignore optional \e[
            if (error) {
                if (printError) {
                    fprintf(stderr, "\nLogo (image-raw): fail to query cursor position: %s\n", error);
                }
                return true;
            }
            if (options->position == FF_LOGO_POSITION_LEFT) {
                if (options->width + options->paddingLeft > X) {
                    X = (uint16_t) (options->width + options->paddingLeft);
                }
                instance.state.logoWidth = X + instance.config.logo.paddingRight - 1;
            }
            instance.state.logoHeight = Y;
            fputs("\e[H", stdout);
        } else if (options->position == FF_LOGO_POSITION_TOP) {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffPrintCharTimes('\n', options->paddingRight);
        }
    } else {
        ffStrbufAppendNC(&buf, options->paddingTop, '\n');

        if (options->position == FF_LOGO_POSITION_RIGHT) {
            ffStrbufAppendF(&buf, "\e[9999999C\e[%uD", (unsigned) options->paddingRight + options->width);
        } else if (options->paddingLeft) {
            ffStrbufAppendF(&buf, "\e[%uC", (unsigned) options->paddingLeft);
        }

        ffStrbufAppendNS(&buf, (uint32_t) length, data);
        ffStrbufAppendC(&buf, '\n');

        if (options->position == FF_LOGO_POSITION_LEFT) {
            instance.state.logoWidth = options->width + options->paddingLeft + options->paddingRight;
            instance.state.logoHeight = options->paddingTop + options->height;
            ffStrbufAppendF(&buf, "\e[%uA", (unsigned) instance.state.logoHeight);
        } else if (options->position == FF_LOGO_POSITION_TOP) {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffStrbufAppendNC(&buf, options->paddingRight, '\n');
        } else if (options->position == FF_LOGO_POSITION_RIGHT) {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffStrbufAppendF(&buf, "\e[%uA", (unsigned) options->height);
        }
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
    }

    return true;
}

void ffLogoPrintChars(const char* data, bool doColorReplacement) {
    FFOptionsLogo* options = &instance.config.logo;
    FFLogoLineCacheState* cache = &instance.state.logoLineCache;

    logoLineCacheBuild(cache, data, doColorReplacement);

    if (options->position != FF_LOGO_POSITION_TOP) {
        return;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(4096);
    FF_LIST_FOR_EACH (FFLogoCachedLine, line, cache->lines) {
        ffStrbufAppend(&result, &line->chars);
        ffStrbufAppendC(&result, '\n');
    }
    ffStrbufAppendNC(&result, options->paddingBottom, '\n');
    ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &result);
    instance.state.logoWidth = instance.state.logoHeight = 0;
    logoLineCacheClear(cache);
}

static void logoApplyColors(const FFlogo* logo, bool replacement) {
    if (instance.config.display.colorTitle.length == 0) {
        ffStrbufAppendS(&instance.config.display.colorTitle, logo->colorTitle ?: logo->colors[0]);
    }

    if (instance.config.display.colorKeys.length == 0) {
        ffStrbufAppendS(&instance.config.display.colorKeys, logo->colorKeys ?: logo->colors[1]);
    }

    if (replacement) {
        FFOptionsLogo* options = &instance.config.logo;

        const char* const* colors = logo->colors;
        for (int i = 0; *colors != NULL && i < FASTFETCH_LOGO_MAX_COLORS; i++, colors++) {
            if (options->colors[i].length == 0) {
                ffStrbufAppendS(&options->colors[i], *colors);
            }
        }
    }
}

static bool logoHasName(const FFlogo* logo, const FFstrbuf* name, bool small) {
    for (
        const char* const* logoName = logo->names;
        *logoName != NULL && logoName <= &logo->names[FASTFETCH_LOGO_MAX_NAMES];
        ++logoName) {
        if (small) {
            uint32_t logoNameLength = (uint32_t) (strlen(*logoName) - strlen("_small"));
            if (name->length == logoNameLength && strncasecmp(*logoName, name->chars, logoNameLength) == 0) {
                return true;
            }
        }
        if (ffStrbufIgnCaseEqualS(name, *logoName)) {
            return true;
        }
    }

    return false;
}

static const FFlogo* logoGetBuiltin(const FFstrbuf* name, FFLogoSize size) {
    if (name->length == 0 || !isalpha(name->chars[0])) {
        return NULL;
    }

    for (const FFlogo* logo = ffLogoBuiltins[toupper(name->chars[0]) - 'A']; *logo->names; ++logo) {
        switch (size) {
            // Never use alternate logos
            case FF_LOGO_SIZE_NORMAL:
                if (logo->type != FF_LOGO_LINE_TYPE_NORMAL) {
                    continue;
                }
                break;
            case FF_LOGO_SIZE_SMALL:
                if (logo->type != FF_LOGO_LINE_TYPE_SMALL_BIT) {
                    continue;
                }
                break;
            default:
                break;
        }

        if (logoHasName(logo, name, size == FF_LOGO_SIZE_SMALL)) {
            return logo;
        }
    }

    return NULL;
}

static const FFlogo* logoGetBuiltinDetected(FFLogoSize size) {
    const FFOSResult* os = ffDetectOS();

    const FFlogo* logo = logoGetBuiltin(&os->id, size);
    if (logo != NULL) {
        return logo;
    }

    logo = logoGetBuiltin(&os->name, size);
    if (logo != NULL) {
        return logo;
    }

    if (ffStrbufContainC(&os->idLike, ' ')) {
        FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
        for (
            uint32_t start = 0, end = ffStrbufFirstIndexC(&os->idLike, ' ');
            true;
            start = end + 1, end = ffStrbufNextIndexC(&os->idLike, start, ' ')) {
            ffStrbufSetNS(&buf, end - start, os->idLike.chars + start);
            logo = logoGetBuiltin(&buf, size);
            if (logo != NULL) {
                return logo;
            }

            if (end >= os->idLike.length) {
                break;
            }
        }
    } else {
        logo = logoGetBuiltin(&os->idLike, size);
        if (logo != NULL) {
            return logo;
        }
    }

    logo = logoGetBuiltin(&instance.state.platform.sysinfo.name, size);
    if (logo != NULL) {
        return logo;
    }

    return &ffLogoUnknown;
}

static void logoPrintStruct(const FFlogo* logo) {
    logoApplyColors(logo, true);

    ffLogoPrintChars(logo->lines, true);
}

static void logoPrintNone(void) {
    if (!instance.config.display.pipe) {
        logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), false);
    }
    instance.state.logoHeight = 0;
    instance.state.logoWidth = 0;
}

static bool logoPrintBuiltinIfExists(const FFstrbuf* name, FFLogoSize size) {
    if (name->chars[0] == '~' || name->chars[0] == '.' || name->chars[0] == '/'
#if _WIN32
        || (ffCharIsEnglishAlphabet(name->chars[0]) && name->chars[1] == ':') // Windows drive letter
#endif
    )
        return false; // Paths

    if (ffStrbufIgnCaseEqualS(name, "none")) {
        logoPrintNone();
        return true;
    }

    const FFlogo* logo = ffLogoGetBuiltinForName(name, size);
    if (logo == NULL) {
        return false;
    }

    logoPrintStruct(logo);

    return true;
}

void ffLogoPrintDetected(FFLogoSize size) {
    logoPrintStruct(logoGetBuiltinDetected(size));
}

static bool logoPrintData(bool doColorReplacement, FFstrbuf* source) {
    if (source->length == 0) {
        return false;
    }

    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), doColorReplacement);
    ffLogoPrintChars(source->chars, doColorReplacement);
    return true;
}

static bool updateLogoPath(void) {
    FFOptionsLogo* options = &instance.config.logo;

    if (ffPathExists(options->source.chars, FF_PATHTYPE_FILE)) {
        return true;
    }

    if (ffStrbufEqualS(&options->source, "-")) { // stdin
        return true;
    }

    #if !FF_MODULE_DISABLE_MEDIA
    if (ffStrbufIgnCaseEqualS(&options->source, "media-cover")) {
        const FFMediaResult* media = ffDetectMedia(true);
        if (media->cover.length == 0) {
            return false;
        }
        ffStrbufSet(&options->source, &media->cover);
        return true;
    }
    #endif

    FF_STRBUF_AUTO_DESTROY fullPath = ffStrbufCreateA(128);
    if (ffPathExpandEnv(options->source.chars, &fullPath) && ffPathExists(fullPath.chars, FF_PATHTYPE_FILE)) {
        ffStrbufDestroy(&options->source);
        ffStrbufInitMove(&options->source, &fullPath);
        return true;
    }

    FF_LIST_FOR_EACH (FFstrbuf, dataDir, instance.state.platform.dataDirs) {
        // We need to copy it, because multiple threads might be using dataDirs at the same time
        ffStrbufSet(&fullPath, dataDir);
        ffStrbufAppendS(&fullPath, "fastfetch/logos/");
        ffStrbufAppend(&fullPath, &options->source);

        if (ffPathExists(fullPath.chars, FF_PATHTYPE_FILE)) {
            ffStrbufDestroy(&options->source);
            ffStrbufInitMove(&options->source, &fullPath);
            return true;
        }
    }

    return false;
}

static bool logoPrintFileIfExists(bool doColorReplacement, bool raw) {
    FFOptionsLogo* options = &instance.config.logo;

    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();

    if (ffStrbufEqualS(&options->source, "-")
            ? !ffAppendFDBuffer(FFUnixFD2NativeFD(STDIN_FILENO), &content)
            : !ffAppendFileBuffer(options->source.chars, &content)) {
        if (instance.config.display.showErrors) {
            fprintf(stderr, "Logo: Failed to load file content from logo source: %s\n", options->source.chars);
        }
        return false;
    }

    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), doColorReplacement);
    if (raw) {
        return ffLogoPrintCharsRaw(content.chars, content.length, instance.config.display.showErrors);
    }

    ffLogoPrintChars(content.chars, doColorReplacement);
    return true;
}

static bool logoPrintImageIfExists(FFLogoType logo, bool printError) {
    if (!ffLogoPrintImageIfExists(logo, printError)) {
        return false;
    }

    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), false);
    return true;
}

static bool logoTryKnownType(void) {
    FFOptionsLogo* options = &instance.config.logo;

    if (options->type == FF_LOGO_TYPE_NONE) {
        logoPrintNone();
        return true;
    }

    if (options->type == FF_LOGO_TYPE_BUILTIN) {
        return logoPrintBuiltinIfExists(&options->source, FF_LOGO_SIZE_UNKNOWN);
    }

    if (options->type == FF_LOGO_TYPE_SMALL) {
        return logoPrintBuiltinIfExists(&options->source, FF_LOGO_SIZE_SMALL);
    }

    if (options->type == FF_LOGO_TYPE_DATA) {
        return logoPrintData(true, &options->source);
    }

    if (options->type == FF_LOGO_TYPE_DATA_RAW) {
        return logoPrintData(false, &options->source);
    }

    if (options->type == FF_LOGO_TYPE_COMMAND_RAW) {
        FF_STRBUF_AUTO_DESTROY source = ffStrbufCreate();

        const char* error = ffProcessAppendStdOut(&source, (char* const[]){
#ifdef _WIN32
                                                               "cmd.exe", "/c",
#else
                                                               "/bin/sh", "-c",
#endif
                                                               options->source.chars,
                                                               NULL });

        if (error) {
            if (instance.config.display.showErrors) {
                fprintf(stderr, "Logo: failed to execute command `%s`: %s\n", options->source.chars, error);
            }
            return false;
        }

        return logoPrintData(false, &source);
    }

    // We sure have a file, resolve relative paths
    if (!updateLogoPath()) {
        if (instance.config.display.showErrors) {
            fprintf(stderr, "Logo: Failed to resolve logo source: %s\n", options->source.chars);
        }
        return false;
    }

    if (options->type == FF_LOGO_TYPE_FILE) {
        return logoPrintFileIfExists(true, false);
    }

    if (options->type == FF_LOGO_TYPE_FILE_RAW) {
        return logoPrintFileIfExists(false, false);
    }

    if (options->type == FF_LOGO_TYPE_IMAGE_RAW) {
        return logoPrintFileIfExists(false, true);
    }

    return logoPrintImageIfExists(options->type, instance.config.display.showErrors);
}

void ffLogoPrint(void) {
    const FFOptionsLogo* options = &instance.config.logo;

    if (options->type == FF_LOGO_TYPE_NONE) {
        logoPrintNone();
        return;
    }

    // If the source is not set, we can directly print the detected logo.
    if (options->source.length == 0) {
        ffLogoPrintDetected(options->type == FF_LOGO_TYPE_SMALL ? FF_LOGO_SIZE_SMALL : FF_LOGO_SIZE_NORMAL);
        return;
    }

    // If the source and source type is set to something else than auto, always print with the set type.
    if (options->source.length > 0 && options->type != FF_LOGO_TYPE_AUTO) {
        if (!logoTryKnownType()) {
            if (instance.config.display.showErrors) {
                // Image logo should have been handled
                if (options->type == FF_LOGO_TYPE_BUILTIN || options->type == FF_LOGO_TYPE_SMALL) {
                    fprintf(stderr, "Logo: Failed to load %s logo: %s\n", options->type == FF_LOGO_TYPE_BUILTIN ? "builtin" : "builtin small", options->source.chars);
                }
            }

            ffLogoPrintDetected(FF_LOGO_SIZE_UNKNOWN);
        }
        return;
    }

    // If source matches the name of a builtin logo, print it and return.
    if (logoPrintBuiltinIfExists(&options->source, FF_LOGO_SIZE_UNKNOWN)) {
        return;
    }

    // Make sure the logo path is set correctly.
    if (updateLogoPath()) {
        if (ffStrbufEndsWithIgnCaseS(&options->source, ".raw")) {
            if (logoPrintFileIfExists(false, true)) {
                return;
            }
        }

        if (!ffStrbufEndsWithIgnCaseS(&options->source, ".txt")) {
            #if !FF_MODULE_DISABLE_TERMINAL
            const FFTerminalResult* terminal = ffDetectTerminal();

            bool supportsIterm2 = ffStrbufEqualS(&terminal->prettyName, "iTerm");

            if (supportsIterm2 && logoPrintImageIfExists(FF_LOGO_TYPE_IMAGE_ITERM, false)) {
                return;
            }

            // Terminal emulators that support kitty graphics protocol.
            bool supportsKitty =
                ffStrbufIgnCaseEqualS(&terminal->processName, "kitty") ||
                ffStrbufIgnCaseEqualS(&terminal->processName, "konsole") ||
                ffStrbufIgnCaseEqualS(&terminal->processName, "wezterm") ||
                ffStrbufIgnCaseEqualS(&terminal->processName, "wayst") ||
                ffStrbufIgnCaseEqualS(&terminal->processName, "ghostty") ||
#ifdef __APPLE__
                ffStrbufIgnCaseEqualS(&terminal->processName, "WarpTerminal") ||
#else
                ffStrbufIgnCaseEqualS(&terminal->processName, "warp") ||
#endif
                false;
            #else
            bool supportsKitty = false;
            #endif

            // Try to load the logo as an image. If it succeeds, print it and return.
            if (logoPrintImageIfExists(supportsKitty ? FF_LOGO_TYPE_IMAGE_KITTY : FF_LOGO_TYPE_IMAGE_CHAFA, false)) {
                return;
            }
        }

        // Try to load the logo as a file. If it succeeds, print it and return.
        if (logoPrintFileIfExists(true, false)) {
            return;
        }
    } else {
        if (instance.config.display.showErrors) {
            fprintf(stderr, "Logo: Failed to resolve logo source: %s\n", options->source.chars);
        }
    }

    ffLogoPrintDetected(FF_LOGO_SIZE_UNKNOWN);
}

void ffLogoPrintLine(void) {
    uint32_t printedLineWidth = 0;
    FFLogoLineCacheState* cache = &instance.state.logoLineCache;
    FFOptionsLogo* logo = &instance.config.logo;

    if (cache->lines.length > 0 && (logo->position == FF_LOGO_POSITION_LEFT || logo->position == FF_LOGO_POSITION_RIGHT) && cache->nextLine < cache->lines.length) {
        FFLogoCachedLine* line = FF_LIST_GET(FFLogoCachedLine, cache->lines, cache->nextLine);

        if (logo->position == FF_LOGO_POSITION_RIGHT && line->chars.length > 0) {
            printf("\033[9999999C\033[%uD", cache->rightOffset);
            ffStrbufWriteTo(&line->chars, stdout);
            fputs("\033[G", stdout);
        } else {
            ffStrbufWriteTo(&line->chars, stdout);
            printedLineWidth = line->width;
        }

        ++cache->nextLine;
    }

    if (instance.state.logoWidth > 0) {
        if (instance.config.logo.position == FF_LOGO_POSITION_LEFT) {
            uint32_t remaining = instance.state.logoWidth;
            remaining = printedLineWidth < remaining ? remaining - printedLineWidth : 0;
            ffPrintCharTimes(' ', remaining);
        } else {
            printf("\033[%uC", instance.state.logoWidth);
        }
    }

    if (instance.state.dynamicInterval > 0) {
        fputs("\033[K", stdout); // Clear to the end of the line
    }

    ++instance.state.keysHeight;
}

void ffLogoPrintRemaining(void) {
    FFLogoLineCacheState* cache = &instance.state.logoLineCache;
    FFOptionsLogo* logo = &instance.config.logo;

    if (cache->lines.length > 0 && (logo->position == FF_LOGO_POSITION_LEFT || logo->position == FF_LOGO_POSITION_RIGHT)) {
        while (cache->nextLine < cache->lines.length) {
            FFLogoCachedLine* line = FF_LIST_GET(FFLogoCachedLine, cache->lines, cache->nextLine);

            if (logo->position == FF_LOGO_POSITION_RIGHT) {
                printf("\033[9999999C\033[%uD", cache->rightOffset);
            }
            ffStrbufPutTo(&line->chars, stdout);

            ++cache->nextLine;
        }

        if (!instance.config.display.pipe) {
            fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
        }

        instance.state.keysHeight = instance.state.logoHeight + 1;
        logoLineCacheClear(cache);
        return;
    }

    if (instance.state.keysHeight <= instance.state.logoHeight) {
        ffPrintCharTimes('\n', instance.state.logoHeight - instance.state.keysHeight + 1);
    }
    instance.state.keysHeight = instance.state.logoHeight + 1;
}

void ffLogoBuiltinPrint(void) {
    FFOptionsLogo* options = &instance.config.logo;
    options->position = FF_LOGO_POSITION_TOP;
    options->paddingRight = 2; // empty line after logo printing
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();

    for (uint8_t ch = 0; ch < 26; ++ch) {
        for (const FFlogo* logo = ffLogoBuiltins[ch]; *logo->names; ++logo) {
            if (instance.config.display.pipe) {
                ffStrbufSetF(&buf, "%s:\n", logo->names[0]);
            } else {
                ffStrbufSetF(&buf, "\e[%sm%s:\e[0m\n", logo->colors[0], logo->names[0]);
            }
            ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
            logoPrintStruct(logo);

            for (uint8_t i = 0; i < FASTFETCH_LOGO_MAX_COLORS; i++) {
                ffStrbufClear(&options->colors[i]);
            }
        }
    }
}

void ffLogoBuiltinList(void) {
    uint32_t counter = 0;
    for (uint8_t ch = 0; ch < 26; ++ch) {
        for (const FFlogo* logo = ffLogoBuiltins[ch]; *logo->names; ++logo) {
            ++counter;
            printf("%u)%s ", counter, counter < 10 ? " " : "");

            for (
                const char* const* names = logo->names;
                *names != NULL && names <= &logo->names[FASTFETCH_LOGO_MAX_NAMES];
                ++names) {
                printf("\"%s\" ", *names);
            }

            putchar('\n');
        }
    }
}

void ffLogoBuiltinListAutocompletion(void) {
    for (uint8_t ch = 0; ch < 26; ++ch) {
        for (const FFlogo* logo = ffLogoBuiltins[ch]; *logo->names; ++logo) {
            printf("%s\n", logo->names[0]);
        }
    }
}

const FFlogo* ffLogoGetBuiltinForName(const FFstrbuf* name, FFLogoSize size) {
    return ffStrbufEqualS(name, "?") ? &ffLogoUnknown : logoGetBuiltin(name, size);
}

const FFlogo* ffLogoGetBuiltinDetected(FFLogoSize size) {
    return logoGetBuiltinDetected(size);
}
