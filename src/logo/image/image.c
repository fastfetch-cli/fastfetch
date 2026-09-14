#include "image.h"
#include "common/io.h"
#include "common/mallocHelper.h"
#include "common/printing.h"
#include "common/processing.h"
#include "common/strutil.h"
#include "common/base64.h"
#include "detection/terminalsize/terminalsize.h"

#include <limits.h>
#include <math.h>

#ifdef __APPLE__
    #include <sys/syslimits.h>
#elif _WIN32
    #include <windows.h>
#elif __linux__
    #include <sys/sendfile.h>
#elif __sun
    #include <sys/termios.h>
#endif

static bool printImageIterm(bool printError) {
    const FFOptionsLogo* options = &instance.config.logo;
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (!ffAppendFileBuffer(options->source.chars, &buf)) {
        if (printError) {
            fputs("Logo (iterm): Failed to load image file\n", stderr);
        }
        return false;
    }

    fflush(stdout);

    bool inTmux = false;
    {
        const char* term = getenv("TERM");
        inTmux = term && (ffStrStartsWith(term, "screen") || ffStrStartsWith(term, "tmux"));
    }

    FF_STRBUF_AUTO_DESTROY base64 = ffBase64EncodeStrbuf(&buf);
    ffStrbufClear(&buf);

    if (!options->width || !options->height) {
        if (options->position == FF_LOGO_POSITION_LEFT) {
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH", (unsigned) options->paddingTop + 1, (unsigned) options->paddingLeft + 1);
        } else if (options->position == FF_LOGO_POSITION_TOP) {
            ffStrbufAppendNC(&buf, options->paddingTop, '\n');
            ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        } else if (options->position == FF_LOGO_POSITION_RIGHT) {
            if (!options->width) {
                if (printError) {
                    fputs("Logo (iterm): Must set logo width when using position right\n", stderr);
                }
                return false;
            }
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;9999999H\e[%uD", (unsigned) options->paddingTop + 1, (unsigned) options->paddingRight + options->width);
        }
        if (inTmux) {
            ffStrbufAppendS(&buf, "\ePtmux;\e");
        }

        if (options->width) {
            ffStrbufAppendF(&buf, "\e]1337;File=inline=1;width=%u:%s\a", (unsigned) options->width, base64.chars);
        } else {
            ffStrbufAppendF(&buf, "\e]1337;File=inline=1:%s\a", base64.chars);
        }
        if (inTmux) {
            ffStrbufAppendS(&buf, "\e\\");
        }
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

        if (options->position == FF_LOGO_POSITION_LEFT || options->position == FF_LOGO_POSITION_RIGHT) {
            uint16_t X = 0, Y = 0;
            const char* error = ffGetTerminalResponse("\e[6n", 2, "%*[^0-9]%hu;%huR", &Y, &X);
            if (error) {
                fprintf(stderr, "\nLogo (iterm): fail to query cursor position: %s\n", error);
                return true; // We already printed image logo, don't print ascii logo then
            }
            if (X < options->paddingLeft + options->width) {
                X = (uint16_t) (options->paddingLeft + options->width);
            }
            if (options->position == FF_LOGO_POSITION_LEFT) {
                instance.state.logoWidth = X + options->paddingRight - 1;
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
        if (inTmux) {
            ffStrbufAppendS(&buf, "\ePtmux;\e");
        }
        ffStrbufAppendF(&buf, "\e]1337;File=inline=1;width=%u;height=%u;preserveAspectRatio=%u:%s\a", (unsigned) options->width, (unsigned) options->height, (unsigned) options->preserveAspectRatio, base64.chars);
        if (inTmux) {
            ffStrbufAppendS(&buf, "\e\\");
        }
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
            ffStrbufAppendF(&buf, "\e[1G\e[%uA", (unsigned) options->height);
        }
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
    }

    return true;
}

static bool printImageKittyIcat(bool printError) {
    const FFOptionsLogo* options = &instance.config.logo;

    if (!ffPathExists(options->source.chars, FF_PATHTYPE_FILE)) {
        if (printError) {
            fputs("Logo (kitty-icat): Failed to load image file\n", stderr);
        }
        return false;
    }

    fflush(stdout);

    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();

    if (options->position == FF_LOGO_POSITION_LEFT) {
        ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH", (unsigned) options->paddingTop + 1, (unsigned) options->paddingLeft + 1);
    } else if (options->position == FF_LOGO_POSITION_TOP) {
        if (!options->width) {
            ffStrbufAppendNC(&buf, options->paddingTop, '\n');
            ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        } else {
            if (printError) {
                fputs("Logo (kitty-icat): position top is not supported when logo width is set\n", stderr);
            }
            return false;
        }
    } else if (options->position == FF_LOGO_POSITION_RIGHT) {
        if (printError) {
            fputs("Logo (kitty-icat): position right is not supported\n", stderr);
        }
        return false;
    }

    uint32_t prevLength = buf.length;

    const char* error = nullptr;

    if (options->width) {
        char place[64];
        snprintf(place,
            ARRAY_SIZE(place),
            "--place=%ux%u@%ux%u",
            options->width,
            options->height == 0 ? 9999 : options->height,
            options->paddingLeft + 1,
            options->paddingTop + 1);

        error = ffProcessAppendStdOut(&buf, (char*[]) {
                                                "kitten",
                                                "icat",
                                                "-n",
                                                "--align=center",
                                                place,
                                                "--scale-up",
                                                options->source.chars,
                                                nullptr,
                                            });
    } else {
        error = ffProcessAppendStdOut(&buf, (char*[]) {
                                                "kitten",
                                                "icat",
                                                "-n",
                                                "--align=left",
                                                options->source.chars,
                                                nullptr,
                                            });
    }
    if (error) {
        if (printError) {
            fprintf(stderr, "Logo (kitty-icat): running `kitten icat` failed %s\n", error);
        }
        return false;
    }

    if (buf.length == prevLength) {
        if (printError) {
            fputs("Logo (kitty-icat): `kitten icat` returned empty output\n", stderr);
        }
        return false;
    }

    ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

    if (options->position == FF_LOGO_POSITION_LEFT || options->position == FF_LOGO_POSITION_RIGHT) {
        uint16_t X = 0, Y = 0;
        const char* error = ffGetTerminalResponse("\e[6n", 2, "%*[^0-9]%hu;%huR", &Y, &X);
        if (error) {
            fprintf(stderr, "\nLogo (kitty-icat): fail to query cursor position: %s\n", error);
            return true; // We already printed image logo, don't print ascii logo then
        }
        if (X < options->paddingLeft + options->width) {
            X = (uint16_t) (options->paddingLeft + options->width);
        }
        if (options->position == FF_LOGO_POSITION_LEFT) {
            instance.state.logoWidth = X + options->paddingRight - 1;
        }
        instance.state.logoHeight = Y;
        fputs("\e[H", stdout);
    } else if (options->position == FF_LOGO_POSITION_TOP) {
        instance.state.logoWidth = instance.state.logoHeight = 0;
        ffPrintCharTimes('\n', options->paddingRight);
    }

    return true;
}

static bool printImageKittyDirect(bool printError) {
    const FFOptionsLogo* options = &instance.config.logo;

    if (!ffPathExists(options->source.chars, FF_PATHTYPE_FILE)) {
        if (printError) {
            fputs("Logo (kitty-direct): Failed to load image file\n", stderr);
        }
        return false;
    }

    fflush(stdout);

    bool inTmux = false;
    {
        const char* term = getenv("TERM");
        inTmux = term && (ffStrStartsWith(term, "screen") || ffStrStartsWith(term, "tmux"));
    }

    FF_STRBUF_AUTO_DESTROY base64 = ffBase64EncodeStrbuf(&options->source);
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();

    if (!options->width || !options->height) {
        if (options->position == FF_LOGO_POSITION_LEFT) {
            // We must clear the entre screen to make sure that terminal buffer won't scroll up
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH", (unsigned) options->paddingTop + 1, (unsigned) options->paddingLeft + 1);
        } else if (options->position == FF_LOGO_POSITION_TOP) {
            ffStrbufAppendNC(&buf, options->paddingTop, '\n');
            ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        } else if (options->position == FF_LOGO_POSITION_RIGHT) {
            if (!options->width) {
                if (printError) {
                    fputs("Logo (iterm): Must set logo width when using position right\n", stderr);
                }
                return false;
            }
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;9999999H\e[%uD", (unsigned) options->paddingTop + 1, (unsigned) options->paddingRight + options->width);
        }

        if (inTmux) {
            ffStrbufAppendS(&buf, "\ePtmux;\e");
        }
        if (options->width) {
            ffStrbufAppendF(&buf, "\e_Ga=T,f=100,t=f,c=%u;%s", (unsigned) options->width, base64.chars);
        } else {
            ffStrbufAppendF(&buf, "\e_Ga=T,f=100,t=f;%s", base64.chars);
        }
        if (inTmux) {
            ffStrbufAppendC(&buf, '\e');
        }
        ffStrbufAppendS(&buf, "\e\\");
        if (inTmux) {
            ffStrbufAppendS(&buf, "\e\\");
        }

        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

        if (options->position == FF_LOGO_POSITION_LEFT || options->position == FF_LOGO_POSITION_RIGHT) {
            uint16_t X = 0, Y = 0;
            const char* error = ffGetTerminalResponse("\e[6n", 2, "%*[^0-9]%hu;%huR", &Y, &X);
            if (error) {
                if (printError) {
                    fprintf(stderr, "\nLogo (kitty-direct): fail to query cursor position: %s\n", error);
                }
                return true; // We already printed image logo, don't print ascii logo then
            }
            if (X < options->paddingLeft + options->width) {
                X = (uint16_t) (options->paddingLeft + options->width);
            }
            if (options->position == FF_LOGO_POSITION_LEFT) {
                instance.state.logoWidth = X + options->paddingRight - 1;
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

        if (inTmux) {
            ffStrbufAppendS(&buf, "\ePtmux;\e");
        }

        ffStrbufAppendF(&buf, "\e_Ga=T,f=100,t=f,c=%u,r=%u;%s\e\\", (unsigned) options->width, (unsigned) options->height, base64.chars);
        if (inTmux) {
            ffStrbufAppendS(&buf, "\e\\");
        }
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
            ffStrbufAppendF(&buf, "\e[1G\e[%uA", (unsigned) options->height);
        }

        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
    }

    return true;
}

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6) || defined(_WIN32) || defined(__APPLE__) || defined(FF_HAVE_SIXEL)

    #define FF_KITTY_MAX_CHUNK_SIZE 4096

    #define FF_CACHE_FILE_HEIGHT "height"
    #define FF_CACHE_FILE_WIDTH "width"
    #define FF_CACHE_FILE_SIXEL "sixel"
    #define FF_CACHE_FILE_KITTY_COMPRESSED "kittyc"
    #define FF_CACHE_FILE_KITTY_UNCOMPRESSED "kittyu"
    #define FF_CACHE_FILE_CHAFA "chafa"
    // Modification time of the image source the entry was produced from. Written last, so an
    // entry that was interrupted mid-write is never mistaken for a complete one.
    #define FF_CACHE_FILE_MTIME "mtime"

    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>

    #ifndef _WIN32
        #include <sys/ioctl.h>
    #else
        #include <wincon.h>
        #include "common/path.h"
    #endif

    #ifdef FF_HAVE_ZLIB
        #include "common/library.h"
        #include <zlib.h>

static bool compressBlob(void** blob, size_t* length) {
    FF_LIBRARY_LOAD(zlib, false, "libz" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL(zlib, compressBound, false)
    FF_LIBRARY_LOAD_SYMBOL(zlib, compress2, false)

        #if _WIN32
    // zlib's uLong is 32-bit on Windows (LLP64), so a >4 GiB source can't be
    // compressed through this API; reject it instead of silently truncating
    if (*length > (size_t) ULONG_MAX) {
        return false;
    }
        #endif

    uLong compressedLength = ffcompressBound((uLong) *length);
    void* compressed = malloc(compressedLength);
    if (compressed == nullptr) {
        return false;
    }

    if (ffcompress2(compressed, &compressedLength, *blob, (uLong) *length, Z_BEST_COMPRESSION) != Z_OK) {
        free(compressed);
        return false;
    }

    free(*blob);

    *length = (size_t) compressedLength;
    *blob = compressed;
    return true;
}

    #endif // FF_HAVE_ZLIB

static void writeCacheData(FFLogoRequestData* requestData, const void* value, size_t len, const char* cacheFileName) {
    // Every payload file goes through here, including the ones written from the printing helpers,
    // so this is the single place that keeps `--logo-cache false` from writing anything at all.
    if (instance.config.logo.cache == FF_LOGO_CACHE_OFF) {
        return;
    }

    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, cacheFileName);
    ffWriteFileData(requestData->cacheDir.chars, len, value);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
}

static void printImagePixels(FFLogoRequestData* requestData, const FFstrbuf* result, const char* cacheFileName) {
    const FFOptionsLogo* options = &instance.config.logo;
    // Calculate character dimensions
    instance.state.logoWidth = requestData->logoCharacterWidth + options->paddingLeft + options->paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + options->paddingTop - 1;

    // Write cache files
    writeCacheData(requestData, result->chars, result->length, cacheFileName);

    if (options->width == 0) {
        writeCacheData(requestData, &requestData->logoCharacterWidth, sizeof(requestData->logoCharacterWidth), FF_CACHE_FILE_WIDTH);
    }

    if (options->height == 0) {
        writeCacheData(requestData, &requestData->logoCharacterHeight, sizeof(requestData->logoCharacterHeight), FF_CACHE_FILE_HEIGHT);
    }

    // Write result to stdout
    ffPrintCharTimes('\n', options->paddingTop);
    if (options->position == FF_LOGO_POSITION_RIGHT) {
        printf("\e[9999999C\e[%uD", (unsigned) options->paddingRight + requestData->logoCharacterWidth);
    } else if (options->paddingLeft) {
        printf("\e[%uC", (unsigned) options->paddingLeft);
    }
    fflush(stdout);
    ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), result);

    if (options->position != FF_LOGO_POSITION_TOP) {
        // Go to upper left corner
        printf("\e[1G\e[%uA", instance.state.logoHeight);
    }

    if (options->position != FF_LOGO_POSITION_LEFT) {
        instance.state.logoWidth = instance.state.logoHeight = 0;
    }
}

// The backends report the real pixel dimensions; the character dimensions are derived here
static void fillCharacterDimensions(FFLogoRequestData* requestData) {
    requestData->logoCharacterWidth = (uint32_t) ceil((double) requestData->logoPixelWidth / requestData->characterPixelWidth);
    requestData->logoCharacterHeight = (uint32_t) ceil((double) requestData->logoPixelHeight / requestData->characterPixelHeight);
}

static bool printImageSixel(FFLogoRequestData* requestData, const FFstrbuf* result) {
    if (result->length == 0) {
        return false;
    }

    printImagePixels(requestData, result, FF_CACHE_FILE_SIXEL);
    return true;
}

static void appendKittyChunk(FFstrbuf* result, const char** blob, size_t* length, bool printEscapeCode) {
    uint32_t chunkSize = *length > FF_KITTY_MAX_CHUNK_SIZE ? FF_KITTY_MAX_CHUNK_SIZE : (uint32_t) *length;

    if (printEscapeCode) {
        ffStrbufAppendS(result, "\033_G");
    } else {
        ffStrbufAppendC(result, ',');
    }

    ffStrbufAppendS(result, chunkSize != *length ? "m=1" : "m=0");
    ffStrbufAppendC(result, ';');
    ffStrbufAppendNS(result, chunkSize, *blob);
    ffStrbufAppendS(result, "\033\\");
    *length -= chunkSize;
    *blob += chunkSize;
}

static bool printImageKitty(FFLogoRequestData* requestData, const FFImageBuffer* buffer) {
    size_t length = (size_t) buffer->width * buffer->height * 4;
    FF_AUTO_FREE void* blob = malloc(length);
    if (blob == nullptr) {
        return false;
    }
    memcpy(blob, buffer->data, length);

    #ifdef FF_HAVE_ZLIB
    bool isCompressed = compressBlob(&blob, &length);
    #else
    bool isCompressed = false;
    #endif

    // base64 output is 4 * ceil(length / 3) bytes, plus the terminating null byte
    FF_STRBUF_AUTO_DESTROY base64 = ffStrbufCreateA((uint32_t) (10 + length * 4 / 3));
    ffBase64EncodeRaw((uint32_t) length, (const char*) blob, &base64.length, base64.chars);

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(base64.length + 1024);

    const char* currentPos = base64.chars;
    size_t remainingLength = base64.length;

    ffStrbufAppendF(&result, "\033_Ga=T,f=32,s=%u,v=%u", buffer->width, buffer->height);
    if (isCompressed) {
        ffStrbufAppendS(&result, ",o=z");
    }
    appendKittyChunk(&result, &currentPos, &remainingLength, false);
    while (remainingLength > 0) {
        appendKittyChunk(&result, &currentPos, &remainingLength, true);
    }

    printImagePixels(requestData, &result, isCompressed ? FF_CACHE_FILE_KITTY_COMPRESSED : FF_CACHE_FILE_KITTY_UNCOMPRESSED);
    return true;
}

    #ifdef FF_HAVE_CHAFA
        #include <chafa.h>
static bool printImageChafa(FFLogoRequestData* requestData, const FFImageBuffer* buffer) {
        #if _WIN32
    FF_LIBRARY_LOAD(chafa, false, "libchafa-0" FF_LIBRARY_EXTENSION, 0)
        #else
    FF_LIBRARY_LOAD(chafa, false, "libchafa" FF_LIBRARY_EXTENSION, 1)
        #endif
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_symbol_map_new, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_symbol_map_apply_selectors, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_new, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_set_geometry, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_set_symbol_map, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_new, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_draw_all_pixels, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_print, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_unref, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_unref, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_symbol_map_unref, false)

    ChafaSymbolMap* symbolMap = ffchafa_symbol_map_new();
    GError* error = nullptr;
    if (!ffchafa_symbol_map_apply_selectors(symbolMap, instance.config.logo.chafaSymbols.chars, &error)) {
        fputs(error->message, stderr);
    }

    ChafaCanvasConfig* canvasConfig = ffchafa_canvas_config_new();
    ffchafa_canvas_config_set_geometry(canvasConfig, (gint) requestData->logoCharacterWidth, (gint) requestData->logoCharacterHeight);
    ffchafa_canvas_config_set_symbol_map(canvasConfig, symbolMap);

    if (instance.config.logo.chafaFgOnly) {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_fg_only_enabled);
        if (ffchafa_canvas_config_set_fg_only_enabled) {
            ffchafa_canvas_config_set_fg_only_enabled(canvasConfig, true);
        }
    }
    if (instance.config.logo.chafaCanvasMode < CHAFA_CANVAS_MODE_MAX) {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_canvas_mode);
        if (ffchafa_canvas_config_set_canvas_mode) {
            ffchafa_canvas_config_set_canvas_mode(canvasConfig, (ChafaCanvasMode) instance.config.logo.chafaCanvasMode);
        }
    }
    if (instance.config.logo.chafaColorSpace < CHAFA_COLOR_SPACE_MAX) {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_color_space)
        if (ffchafa_canvas_config_set_color_space) {
            ffchafa_canvas_config_set_color_space(canvasConfig, (ChafaColorSpace) instance.config.logo.chafaColorSpace);
        }
    }
    if (instance.config.logo.chafaDitherMode < CHAFA_DITHER_MODE_MAX) {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_dither_mode)
        if (ffchafa_canvas_config_set_dither_mode) {
            ffchafa_canvas_config_set_dither_mode(canvasConfig, (ChafaDitherMode) instance.config.logo.chafaDitherMode);
        }
    }

    ChafaCanvas* canvas = ffchafa_canvas_new(canvasConfig);
    ffchafa_canvas_draw_all_pixels(
        canvas,
        CHAFA_PIXEL_RGBA8_UNASSOCIATED,
        buffer->data,
        (gint) buffer->width,
        (gint) buffer->height,
        (gint) buffer->width * 4);

    GString* str = ffchafa_canvas_print(canvas, nullptr);
    FFstrbuf result;
    result.allocated = (uint32_t) str->allocated_len;
    result.length = (uint32_t) str->len;
    result.chars = str->str;

    ffLogoPrintChars(result.chars, false);
    writeCacheData(requestData, &result.chars, result.length, FF_CACHE_FILE_CHAFA);

    // FIXME: These functions must be imported from `libglib` dlls on Windows
    FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, g_string_free);
    if (ffg_string_free) {
        ffg_string_free(str, TRUE);
    }
    if (error) {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, g_error_free)
        if (ffg_error_free) {
            ffg_error_free(error);
        }
    }

    ffchafa_canvas_unref(canvas);
    ffchafa_canvas_config_unref(canvasConfig);
    ffchafa_symbol_map_unref(symbolMap);

    return true;
}
    #endif

bool ffImageCreate(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error) {
    #ifdef _WIN32
    return ffImageCreateWIC(requestData, out, error);
    #elif defined(__APPLE__)
    return ffImageCreateImageIO(requestData, out, error);
    #else
        #ifdef FF_HAVE_IMAGEMAGICK7
    if (ffImageCreateIM7(requestData, out, error)) {
        return true;
    }
        #endif
        #ifdef FF_HAVE_IMAGEMAGICK6
    if (ffImageCreateIM6(requestData, out, error)) {
        return true;
    }
        #endif
    return false;
    #endif
}

void ffImageDestroy(FFImageBuffer* buffer) {
    free(buffer->data);
    buffer->data = nullptr;
    buffer->width = 0;
    buffer->height = 0;
}

bool ffImageSixelEncode(FFLogoRequestData* requestData, FFstrbuf* out, const char** error) {
    // Windows (WIC) and macOS (ImageIO) decode and resize to RGBA first, then the embedded
    // libsixel encoder takes over. Other platforms let ImageMagick encode straight from the
    // decoded image without an RGBA round trip.
    #if defined(_WIN32) || defined(__APPLE__)
        #ifdef FF_HAVE_SIXEL
    FFImageBuffer buffer = {};
    if (!ffImageCreate(requestData, &buffer, error)) {
        return false;
    }
    bool ok = ffSixelEncode(&buffer, out, error);
    ffImageDestroy(&buffer);
    return ok;
        #else
    FF_UNUSED(requestData, out);
    if (error) {
        *error = "sixel support is not compiled in";
    }
    return false;
        #endif
    #else
        #ifdef FF_HAVE_IMAGEMAGICK7
    if (ffImageSixelEncodeIM7(requestData, out, error)) {
        return true;
    }
        #endif
        #ifdef FF_HAVE_IMAGEMAGICK6
    if (ffImageSixelEncodeIM6(requestData, out, error)) {
        return true;
    }
        #endif
    return false;
    #endif
}

static FFNativeFD getCacheFD(FFLogoRequestData* requestData, const char* fileName) {
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, fileName);
    #ifndef _WIN32
    int fd = open(requestData->cacheDir.chars, O_RDONLY
        #ifdef O_CLOEXEC
            | O_CLOEXEC
        #endif
    );
    #else
    HANDLE fd = CreateFileA(requestData->cacheDir.chars, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    #endif
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    return fd;
}

static bool readCachedStrbuf(FFLogoRequestData* requestData, FFstrbuf* result, const char* cacheFileName) {
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, cacheFileName);
    bool res = ffAppendFileBuffer(requestData->cacheDir.chars, result);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    return res;
}

static bool readCachedData(FFLogoRequestData* requestData, void* buffer, size_t bufferSize, const char* cacheFileName) {
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, cacheFileName);
    bool res = ffReadFileData(requestData->cacheDir.chars, bufferSize, buffer) == (ssize_t) bufferSize;
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    return res;
}

static uint32_t readCachedUint32(FFLogoRequestData* requestData, const char* cacheFileName) {
    uint32_t result = 0;
    if (!readCachedData(requestData, &result, sizeof(result), cacheFileName)) {
        return 0;
    }

    return result;
}

static uint64_t readCachedUint64(FFLogoRequestData* requestData, const char* cacheFileName) {
    uint64_t result = 0;
    if (!readCachedData(requestData, &result, sizeof(result), cacheFileName)) {
        return 0;
    }

    return result;
}

// Drops everything a previous version of the source left in the entry directory.
// The directory is keyed on the source path and the pixel size only, so it is reused across
// edits; without this, a payload written for another logo type would be read back.
static void removeCachedFiles(FFLogoRequestData* requestData) {
    static const char* const files[] = {
        FF_CACHE_FILE_MTIME,
        FF_CACHE_FILE_WIDTH,
        FF_CACHE_FILE_HEIGHT,
        FF_CACHE_FILE_SIXEL,
        FF_CACHE_FILE_KITTY_COMPRESSED,
        FF_CACHE_FILE_KITTY_UNCOMPRESSED,
        FF_CACHE_FILE_CHAFA,
    };

    uint32_t cacheDirLength = requestData->cacheDir.length;
    for (uint32_t i = 0; i < ARRAY_SIZE(files); ++i) {
        ffStrbufAppendS(&requestData->cacheDir, files[i]);
        ffRemoveFile(requestData->cacheDir.chars);
        ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    }
}

static bool printCachedChars(FFLogoRequestData* requestData, const char* cacheFileName) {
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    readCachedStrbuf(requestData, &content, cacheFileName);

    if (content.length == 0) {
        return false;
    }

    ffLogoPrintChars(content.chars, false);
    return true;
}

static bool printCachedPixel(FFLogoRequestData* requestData) {
    FFOptionsLogo* options = &instance.config.logo;

    requestData->logoCharacterWidth = options->width;
    if (requestData->logoCharacterWidth == 0) {
        requestData->logoCharacterWidth = readCachedUint32(requestData, FF_CACHE_FILE_WIDTH);
        if (requestData->logoCharacterWidth == 0) {
            return false;
        }
    }

    requestData->logoCharacterHeight = options->height;
    if (requestData->logoCharacterHeight == 0) {
        requestData->logoCharacterHeight = readCachedUint32(requestData, FF_CACHE_FILE_HEIGHT);
        if (requestData->logoCharacterHeight == 0) {
            return false;
        }
    }

    FF_AUTO_CLOSE_FD FFNativeFD fd = FF_INVALID_FD;
    if (requestData->type == FF_LOGO_TYPE_IMAGE_KITTY) {
        fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_COMPRESSED);
        if (!ffIsValidNativeFD(fd)) {
            fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_UNCOMPRESSED);
        }
    } else if (requestData->type == FF_LOGO_TYPE_IMAGE_SIXEL) {
        fd = getCacheFD(requestData, FF_CACHE_FILE_SIXEL);
    }

    if (!ffIsValidNativeFD(fd)) {
        return false;
    }

    ffPrintCharTimes('\n', options->paddingTop);
    if (options->position == FF_LOGO_POSITION_RIGHT) {
        printf("\e[9999999C\e[%uD", (unsigned) options->paddingRight + requestData->logoCharacterWidth);
    } else if (options->paddingLeft) {
        printf("\e[%uC", (unsigned) options->paddingLeft);
    }
    fflush(stdout);

    bool sent = false;
    #ifdef __linux__
    struct stat st;
    if (fstat(fd, &st) >= 0) {
        while (st.st_size > 0) {
            ssize_t bytes = sendfile(STDOUT_FILENO, fd, nullptr, (size_t) st.st_size);
            if (bytes > 0) {
                sent = true;
                st.st_size -= bytes;
            } else {
                break;
            }
        }
    }
    #endif

    if (!sent) {
        char buffer[32768];
        ssize_t readBytes;
        while ((readBytes = ffReadFDData(fd, sizeof(buffer), buffer)) > 0) {
            ffWriteFDData(FFUnixFD2NativeFD(STDOUT_FILENO), (size_t) readBytes, buffer);
        }
    }

    instance.state.logoWidth = requestData->logoCharacterWidth + options->paddingLeft + options->paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + options->paddingTop;

    if (options->position != FF_LOGO_POSITION_TOP) {
        // Go to upper left corner
        printf("\e[1G\e[%uA", instance.state.logoHeight);
    }

    if (options->position != FF_LOGO_POSITION_LEFT) {
        instance.state.logoWidth = instance.state.logoHeight = 0;
    }
    return true;
}

static bool getCharacterPixelDimensions(FFLogoRequestData* requestData) {
    #ifdef _WIN32

    CONSOLE_FONT_INFOEX cfi = { .cbSize = sizeof(cfi) };
    if (GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi)) // Only works for ConHost
    {
        requestData->characterPixelWidth = cfi.dwFontSize.X;
        requestData->characterPixelHeight = cfi.dwFontSize.Y;
    }
    if (requestData->characterPixelWidth > 1.0 && requestData->characterPixelHeight > 1.0) {
        return true;
    }
    #endif

    FFTerminalSizeResult termSize = {};
    if (ffDetectTerminalSize(&termSize)) {
        requestData->characterPixelWidth = termSize.width / (double) termSize.columns;
        requestData->characterPixelHeight = termSize.height / (double) termSize.rows;
    }

    return requestData->characterPixelWidth > 1.0 && requestData->characterPixelHeight > 1.0;
}

static bool printImageIfExistsSlowPath(FFLogoType type, bool printError) {
    FFLogoRequestData requestData;
    requestData.type = type;
    requestData.characterPixelWidth = 1;
    requestData.characterPixelHeight = 1;

    if (!getCharacterPixelDimensions(&requestData)) {
        if (printError) {
            fputs("Logo: getCharacterPixelDimensions() failed\n", stderr);
        }
        return false;
    }

    requestData.logoPixelWidth = (uint32_t) ceil((double) instance.config.logo.width * requestData.characterPixelWidth);
    requestData.logoPixelHeight = (uint32_t) ceil((double) instance.config.logo.height * requestData.characterPixelHeight);

    ffStrbufInit(&requestData.cacheDir);
    ffStrbufAppend(&requestData.cacheDir, &instance.state.platform.cacheDir);
    ffStrbufAppendS(&requestData.cacheDir, "fastfetch/images");

    ffStrbufEnsureFree(&requestData.cacheDir, PATH_MAX);
    char* filePath = requestData.cacheDir.chars + requestData.cacheDir.length;
    if (realpath(instance.config.logo.source.chars, filePath) == nullptr) {
        // We can safely return here, because if realpath failed, we surely won't be able to read the file
        ffStrbufDestroy(&requestData.cacheDir);
        if (printError) {
            fputs("Logo: Querying realpath of the image source failed\n", stderr);
        }
        return false;
    }

    #ifdef _WIN32
    filePath[1] = filePath[0]; // Drive Name
    filePath[0] = '/';
    #endif

    ffStrbufRecalculateLength(&requestData.cacheDir);
    ffStrbufEnsureEndsWithC(&requestData.cacheDir, '/');

    ffStrbufAppendF(&requestData.cacheDir, "%ux%u/", requestData.logoPixelWidth, requestData.logoPixelHeight);

    // The cached payload is a rendering, not a bit-exact artefact: every backend produces a
    // valid one for the same source and pixel size, so the backend is deliberately not part of
    // the key. What the key does have to capture is the content of the source, which the path
    // can not: the same file can be replaced in place. Hence the recorded mtime.
    // 0 means the mtime could not be read, in which case the entry is never trusted.
    const uint64_t sourceMtime = ffPathGetMtime(instance.config.logo.source.chars);

    if (instance.config.logo.cache == FF_LOGO_CACHE_ON &&
        sourceMtime != 0 &&
        readCachedUint64(&requestData, FF_CACHE_FILE_MTIME) == sourceMtime) {
        bool cacheValid = requestData.type == FF_LOGO_TYPE_IMAGE_CHAFA
            ? printCachedChars(&requestData, FF_CACHE_FILE_CHAFA)
            : printCachedPixel(&requestData);
        if (cacheValid) {
            ffStrbufDestroy(&requestData.cacheDir);
            return true;
        }
    }

    // Cache miss. The entry directory is keyed on the source path and the pixel size only, so
    // it is reused when the source is edited; drop what the previous version left behind.
    // With the cache turned off the directory is left alone entirely.
    if (instance.config.logo.cache != FF_LOGO_CACHE_OFF) {
        removeCachedFiles(&requestData);
    }

    const char* error = nullptr;
    bool printSuccessful = false;

    if (requestData.type == FF_LOGO_TYPE_IMAGE_SIXEL) {
        // The sixel encoder belongs to the backend, so it is not fed through ffImageCreate
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
        if (ffImageSixelEncode(&requestData, &result, &error)) {
            fillCharacterDimensions(&requestData);
            printSuccessful = printImageSixel(&requestData, &result);
        }
    } else {
        FFImageBuffer buffer = {};
        if (ffImageCreate(&requestData, &buffer, &error)) {
            fillCharacterDimensions(&requestData);
            if (requestData.type == FF_LOGO_TYPE_IMAGE_KITTY) {
                printSuccessful = printImageKitty(&requestData, &buffer);
            }
    #if FF_HAVE_CHAFA
            else if (requestData.type == FF_LOGO_TYPE_IMAGE_CHAFA) {
                printSuccessful = printImageChafa(&requestData, &buffer);
            }
    #endif
            ffImageDestroy(&buffer);
        }
    }

    if (printSuccessful) {
        // Written last: an entry only becomes usable once its payload is complete
        writeCacheData(&requestData, &sourceMtime, sizeof(sourceMtime), FF_CACHE_FILE_MTIME);
    }

    ffStrbufDestroy(&requestData.cacheDir);

    if (printSuccessful) {
        return true;
    }

    if (printError) {
        fprintf(stderr, "Logo: %s\n", error ? error : "Failed to load / convert the image source");
    }

    return false;
}

#endif // FF_HAVE_IMAGEMAGICK{6, 7}

bool ffLogoPrintImageIfExists(FFLogoType type, bool printError) {
    if (instance.config.display.pipe) {
        if (printError) {
            fputs("Logo: Image logo is not supported in pipe mode\n", stderr);
        }
        return false;
    }

    if (!ffPathExists(instance.config.logo.source.chars, FF_PATHTYPE_FILE)) {
        if (printError) {
            fprintf(stderr, "Logo: Image source \"%s\" does not exist\n", instance.config.logo.source.chars);
        }
        return false;
    }

    const char* term = getenv("TERM");
    if (term && ffStrEquals(term, "screen")) {
        if (printError) {
            fputs("Logo: Image logo is not supported in terminal multiplexers\n", stderr);
        }
        return false;
    }

    if (type == FF_LOGO_TYPE_IMAGE_ITERM) {
        return printImageIterm(printError);
    }

    if (type == FF_LOGO_TYPE_IMAGE_KITTY_DIRECT) {
        return printImageKittyDirect(printError);
    }

    if (type == FF_LOGO_TYPE_IMAGE_KITTY_ICAT) {
        return printImageKittyIcat(printError);
    }

#if !FF_HAVE_CHAFA
    if (type == FF_LOGO_TYPE_IMAGE_CHAFA) {
        if (printError) {
            fputs("Logo: Chafa support is not compiled in\n", stderr);
        }
        return false;
    }
#endif

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(FF_HAVE_IMAGEMAGICK7) && !defined(FF_HAVE_IMAGEMAGICK6)
    if (printError) {
        fputs("Logo: Image Magick support is not compiled in\n", stderr);
    }
    return false;
#else
    return printImageIfExistsSlowPath(type, printError);
#endif
}
