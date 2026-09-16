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

// `kitten icat` parses a cursor positioning sequence it may have written: `\e[<n>C` (relative) or
// `\e[<row>;<col>H` (absolute). Returns 1 for the first form, 2 for the second one, 0 for anything
// else, in which case nothing is written back.
static int parseIcatCsi(const char* data, uint32_t length, uint32_t pos, uint32_t* end, uint32_t* first, uint32_t* second) {
    if (pos + 2 >= length || data[pos] != '\e' || data[pos + 1] != '[') {
        return 0;
    }

    uint32_t values[2] = { 0, 0 };
    uint32_t count = 0;
    uint32_t i = pos + 2;
    while (i < length) {
        char c = data[i];
        if (c >= '0' && c <= '9') {
            if (count == 0) {
                count = 1;
            }
            values[count - 1] = values[count - 1] * 10 + (uint32_t) (c - '0');
            ++i;
        } else if (c == ';' && count == 1) {
            count = 2;
            ++i;
        } else {
            break;
        }
    }

    if (i >= length) {
        return 0;
    }

    if (data[i] == 'C' && count == 1) {
        *end = i + 1;
        *first = values[0];
        *second = 0;
        return 1;
    }

    if (data[i] == 'H' && count == 2) {
        *end = i + 1;
        *first = values[0];
        *second = values[1];
        return 2;
    }

    return 0;
}

// `kitten icat` positions the image itself: it always writes a carriage return, followed by at most
// one positioning sequence, and then the graphics escape. The carriage return alone would pull the
// cursor back to column 1 and undo the padding, and the absolute cursor move `kitten` writes when
// `--place` is used points at screen coordinates it picked itself, which are not necessarily the ones
// the padding asks for. Both are dropped here and the position is left to the caller, which has
// already moved the cursor to where the padding asks for. The horizontal centering `kitten` applied
// inside the requested rectangle is kept, but re-applied relatively, so it still follows the caller's
// padding.
static void appendIcatOutput(FFstrbuf* buf, const FFstrbuf* output, const FFOptionsLogo* options) {
    const char* data = output->chars;
    uint32_t pos = 0;    // Start of the graphics escape; 0 means "layout not recognised"
    uint32_t offset = 0; // Column offset `kitten` applied inside the requested rectangle

    if (output->length > 0 && data[0] == '\r') {
        uint32_t cursor = 1;
        uint32_t end = 0, first = 0, second = 0;
        int kind = parseIcatCsi(data, output->length, cursor, &end, &first, &second);
        if (kind == 1) { // Relative move, written instead of the absolute one when `--place` is not used
            offset = first;
            cursor = end;
        } else if (kind == 2 && second >= options->paddingLeft + 1) { // Absolute move, produced by `--place`
            // The row is dropped along with the move, so it does not matter which one `kitten` picked.
            // The column is where the padding put it, plus the centering, which is all that is kept.
            offset = second - options->paddingLeft - 1;
            cursor = end;
        }

        // The graphics escape has to follow once the positioning is gone
        if (cursor + 1 < output->length && data[cursor] == '\e' && data[cursor + 1] == '_') {
            pos = cursor;
        }
    }

    if (pos == 0) {
        // Unrecognised layout: keep it as is, positioning included
        ffStrbufAppend(buf, output);
        return;
    }

    if (offset > 0) {
        // The cursor is already at the position the padding asks for
        ffStrbufAppendF(buf, "\e[%uC", (unsigned) offset);
    }
    ffStrbufAppendNS(buf, output->length - pos, data + pos);
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

    // `kitten icat` writes its own positioning, so its output is collected separately and only
    // appended once that positioning has been dropped (see appendIcatOutput)
    FF_STRBUF_AUTO_DESTROY icatOutput = ffStrbufCreate();

    const char* error = nullptr;

    if (options->width) {
        // `--place` measures `left` and `top` from the top left corner of the screen, so the padding is
        // passed through unchanged. The move `kitten` writes for it is dropped again in appendIcatOutput,
        // which leaves the position to the caller, so this is not what puts the image at the padding in
        // the normal case. It is what keeps the fallback there working: when the output layout is not
        // recognised - as under tmux, where `kitten` switches to unicode placeholders - its own
        // positioning is kept, and this is what then places the image at the padding.
        char place[64];
        snprintf(place,
            ARRAY_SIZE(place),
            "--place=%ux%u@%ux%u",
            options->width,
            options->height == 0 ? 9999 : options->height,
            options->paddingLeft,
            options->paddingTop);

        error = ffProcessAppendStdOut(&icatOutput, (char*[]) {
                                                "kitten",
                                                "icat",
                                                "-n",
                                                "--stdin=no",
                                                "--align=center",
                                                place,
                                                "--scale-up",
                                                options->source.chars,
                                                nullptr,
                                            });
    } else {
        error = ffProcessAppendStdOut(&icatOutput, (char*[]) {
                                                "kitten",
                                                "icat",
                                                "-n",
                                                "--stdin=no",
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

    if (icatOutput.length == 0) {
        if (printError) {
            fputs("Logo (kitty-icat): `kitten icat` returned empty output\n", stderr);
        }
        return false;
    }

    appendIcatOutput(&buf, &icatOutput, options);

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
    // Animation payload. The envelope can not be cached because it carries the image id, and that
    // id has to differ on every run; this entry holds only what is id-independent (see §7.2).
    #define FF_CACHE_FILE_KITTY_ANIMATION "kittyanim"
    #define FF_CACHE_FILE_CHAFA "chafa"
    // The frame the payloads in the same directory were rendered from. A selected frame is stored
    // exactly like the first frame is -- same entries, same payload format -- so this sidecar is
    // what tells the two apart, and what makes a change of --logo-animation-frame refresh the entry
    // instead of being served the frame that was asked for last time. It can not be part of the
    // entry name: the frame number ranges over the whole int32, and removeCachedFiles works from a
    // fixed list that could never enumerate it.
    // The entry sits next to every payload of the directory, so it describes whichever of them was
    // written last. Switching --logo-protocol between two protocols that both have a payload here,
    // and asking for different frames, can therefore pair one protocol's payload with the other's
    // frame number; the cache has to be refreshed after such a switch (`--logo-cache regen`).
    #define FF_CACHE_FILE_FRAME "frame"
    // Modification time of the image source the entry was produced from. Written last, so an
    // entry that was interrupted mid-write is never mistaken for a complete one.
    #define FF_CACHE_FILE_MTIME "mtime"

    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <fcntl.h>

    #include "common/time.h"

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
    FF_LIBRARY_LOAD(zlib, false,
        #ifdef _WIN32
        "zlib1"
        #else
        "libz"
        #endif
        FF_LIBRARY_EXTENSION,
        2)
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

    // Level 6, not 9. Measured on a 44 frame 400x400 GIF (2026-09-16): the level 9 encode is 573 ms
    // of the 720 ms the whole kitty animation path takes, i.e. it dwarfs decoding and the per frame
    // export combined. Level 6 does the same job in a fifth of the time for 4.9% more bytes, which is
    // also the level kitten icat uses. Level 1 would halve it again but doubles the payload, and past
    // that point the terminal is the slow one, not us.
    // The gap is far wider on Windows, where the same frames cost 37.7 ms each at level 9 against
    // 6.4 at level 6: raising this back to 9 now that the DLL name above actually loads is a 4.2x
    // slowdown there (398 ms -> 1692 ms). Re-measure both platforms before touching it.
    if (ffcompress2(compressed, &compressedLength, *blob, (uLong) *length, Z_DEFAULT_COMPRESSION) != Z_OK) {
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

// The character dimensions describe the source, not one particular rendering of it, so they live
// in their own entries. The animation path reuses them without writing a payload of its own.
static void writeImageSizeCache(FFLogoRequestData* requestData) {
    const FFOptionsLogo* options = &instance.config.logo;
    if (options->width == 0) {
        writeCacheData(requestData, &requestData->logoCharacterWidth, sizeof(requestData->logoCharacterWidth), FF_CACHE_FILE_WIDTH);
    }

    if (options->height == 0) {
        writeCacheData(requestData, &requestData->logoCharacterHeight, sizeof(requestData->logoCharacterHeight), FF_CACHE_FILE_HEIGHT);
    }
}

// Records which frame the payload written alongside it came from. Every payload write is followed
// by this one, so asking for another frame refreshes the entry instead of being served the frame
// that was asked for last time. What goes in is the *requested* selector, not the index it resolves
// to: on a single-frame source every selector resolves to frame 0, and storing that would make
// every selector but one miss on every run.
static void writeFrameCache(FFLogoRequestData* requestData) {
    const int32_t frame = instance.config.logo.animationFrame;
    writeCacheData(requestData, &frame, sizeof(frame), FF_CACHE_FILE_FRAME);
}

static void printImageResult(FFLogoRequestData* requestData, const FFstrbuf* result) {
    const FFOptionsLogo* options = &instance.config.logo;
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

static void printImagePixels(FFLogoRequestData* requestData, const FFstrbuf* result, const char* cacheFileName) {
    const FFOptionsLogo* options = &instance.config.logo;
    // Calculate character dimensions
    instance.state.logoWidth = requestData->logoCharacterWidth + options->paddingLeft + options->paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + options->paddingTop - 1;

    // Write cache files
    writeCacheData(requestData, result->chars, result->length, cacheFileName);
    writeImageSizeCache(requestData);

    printImageResult(requestData, result);
}

// Same as printImagePixels, but the payload is not written to the cache: the bytes handed to the
// terminal are not the cached ones. Used by the animation path, whose envelope carries the image
// id and therefore differs on every run, and which stores its frames in the `kittyanim` entry
// instead.
static void printImagePixelsNoCache(FFLogoRequestData* requestData, const FFstrbuf* result) {
    const FFOptionsLogo* options = &instance.config.logo;
    // Calculate character dimensions
    instance.state.logoWidth = requestData->logoCharacterWidth + options->paddingLeft + options->paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + options->paddingTop - 1;

    writeImageSizeCache(requestData);

    printImageResult(requestData, result);
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
    writeFrameCache(requestData);
    return true;
}

// Appends one chunk of a chunked graphics command. `printEscapeCode` is false for the first chunk
// only, because the caller has already written the escape introducer and the control data.
// `controlPrefix` adds keys in front of `m`; animation frame data needs it on continuation chunks
// ("When sending animation frame data, subsequent chunks must also specify the a=f key"), the
// static path passes nullptr and is therefore unchanged.
static void appendKittyChunk(FFstrbuf* result, const char** blob, size_t* length, bool printEscapeCode, const char* controlPrefix) {
    uint32_t chunkSize = *length > FF_KITTY_MAX_CHUNK_SIZE ? FF_KITTY_MAX_CHUNK_SIZE : (uint32_t) *length;

    if (printEscapeCode) {
        ffStrbufAppendS(result, "\033_G");
    } else {
        ffStrbufAppendC(result, ',');
    }

    if (controlPrefix) {
        ffStrbufAppendS(result, controlPrefix);
        ffStrbufAppendC(result, ',');
    }

    ffStrbufAppendS(result, chunkSize != *length ? "m=1" : "m=0");
    ffStrbufAppendC(result, ';');
    ffStrbufAppendNS(result, chunkSize, *blob);
    ffStrbufAppendS(result, "\033\\");
    *length -= chunkSize;
    *blob += chunkSize;
}

// The compressed and the uncompressed entry hold the same rendering and share one frame sidecar.
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
    appendKittyChunk(&result, &currentPos, &remainingLength, false, nullptr);
    while (remainingLength > 0) {
        appendKittyChunk(&result, &currentPos, &remainingLength, true, nullptr);
    }

    printImagePixels(requestData, &result, isCompressed ? FF_CACHE_FILE_KITTY_COMPRESSED : FF_CACHE_FILE_KITTY_UNCOMPRESSED);
    writeFrameCache(requestData);
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
    writeCacheData(requestData, result.chars, result.length, FF_CACHE_FILE_CHAFA);
    writeFrameCache(requestData);

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

void ffImageFrameDestroy(FFImageFrame* frame) {
    free(frame->data);
    frame->data = nullptr;
    frame->delayMs = 0;
}

// The session itself is platform independent; a backend only has to fill it in through
// ffImageAnimationCreate and implement the two callbacks.
struct FFImageAnimation {
    uint32_t frameCount;
    int32_t loopCount;
    void* impl;
    bool (*getFrame)(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error);
    void (*destroy)(FFImageAnimation* animation);
};

FFImageAnimation* ffImageAnimationCreate(uint32_t frameCount, int32_t loopCount, void* impl,
    bool (*getFrame)(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error),
    void (*destroy)(FFImageAnimation* animation)) {
    FFImageAnimation* animation = malloc(sizeof(*animation));
    if (animation == nullptr) {
        return nullptr;
    }

    *animation = (FFImageAnimation) {
        .frameCount = frameCount,
        .loopCount = loopCount,
        .impl = impl,
        .getFrame = getFrame,
        .destroy = destroy,
    };
    return animation;
}

void* ffImageAnimationGetImpl(const FFImageAnimation* animation) {
    return animation->impl;
}

bool ffImageAnimationOpen(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error) {
    // The sessions belong to the backends: ImageIO composes frames internally, WIC does not and
    // has to keep a canvas, ImageMagick needs CoalesceImages. Windows, macOS and ImageMagick 7 are
    // wired up; ImageMagick 6 is deliberately left out (see image.h), and a build with none of them
    // keeps failing loudly rather than falling back to a still image.
    //
    // The order mirrors ffImageCreate: on macOS ImageIO is the decoder even when ImageMagick is
    // available too, so the animation has to come from the same place the stills do.
    #ifdef _WIN32
    return ffImageAnimationOpenWIC(requestData, out, error);
    #elif defined(__APPLE__)
    return ffImageAnimationOpenImageIO(requestData, out, error);
    #elif defined(FF_HAVE_IMAGEMAGICK7)
    return ffImageAnimationOpenIM7(requestData, out, error);
    #else
    FF_UNUSED(requestData, out);
    *error = "the image source can not be animated by this build";
    return false;
    #endif
}

uint32_t ffImageAnimationFrameCount(const FFImageAnimation* animation) {
    return animation->frameCount;
}

int32_t ffImageAnimationLoopCount(const FFImageAnimation* animation) {
    return animation->loopCount;
}

bool ffImageAnimationGetFrame(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error) {
    if (index >= animation->frameCount) {
        *error = "the requested frame is out of range";
        return false;
    }

    return animation->getFrame(animation, index, out, error);
}

void ffImageAnimationClose(FFImageAnimation* animation) {
    if (animation == nullptr) {
        return;
    }

    animation->destroy(animation);
    free(animation);
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

bool ffImageSixelEncodeBuffer(const FFImageBuffer* buffer, FFstrbuf* out, const char** error) {
    // Windows and macOS hand the pixels to the embedded encoder. ImageMagick has no way to encode
    // pixels it was not given an Image for, so its SIXEL coder is reached through a ConstituteImage
    // round trip instead.
    #if defined(_WIN32) || defined(__APPLE__)
        #ifdef FF_HAVE_SIXEL
    return ffSixelEncode(buffer, out, error);
        #else
    FF_UNUSED(buffer, out);
    if (error) {
        *error = "sixel support is not compiled in";
    }
    return false;
        #endif
    #else
        #ifdef FF_HAVE_IMAGEMAGICK7
    return ffImageSixelEncodeBufferIM7(buffer, out, error);
        #else
    // Only the animation path calls this, and only ImageMagick 7 has an animation backend, so a
    // build without it never gets here with something to encode.
    FF_UNUSED(buffer, out);
    if (error) {
        *error = "sixel support is not compiled in";
    }
    return false;
        #endif
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

// The frame the payload in the same directory was rendered from. A payload with no sidecar predates
// the sidecar and holds the first frame, which is exactly what the default selector asks for, so
// treating it as such keeps the caches of older builds usable.
static int32_t readCachedFrame(FFLogoRequestData* requestData, const char* frameFileName) {
    int32_t frame = FF_LOGO_ANIMATION_FRAME_FIRST;
    readCachedData(requestData, &frame, sizeof(frame), frameFileName);
    return frame;
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
        FF_CACHE_FILE_KITTY_ANIMATION,
        FF_CACHE_FILE_CHAFA,
        FF_CACHE_FILE_FRAME,
    };

    // Every entry that is ever written is listed here; an entry left out would survive a source
    // change and be read back for the new source.
    uint32_t cacheDirLength = requestData->cacheDir.length;
    for (uint32_t i = 0; i < ARRAY_SIZE(files); ++i) {
        ffStrbufAppendS(&requestData->cacheDir, files[i]);
        ffRemoveFile(requestData->cacheDir.chars);
        ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    }
}

static bool printCachedChars(FFLogoRequestData* requestData, const char* cacheFileName) {
    if (instance.config.logo.animationFrame != readCachedFrame(requestData, FF_CACHE_FILE_FRAME)) {
        // The entry holds the rendering of another frame, or -- for the animation selector -- of
        // no single frame at all. Reporting a miss here is what sends the slow path off to render
        // the one that was asked for.
        return false;
    }

    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    readCachedStrbuf(requestData, &content, cacheFileName);

    if (content.length == 0) {
        return false;
    }

    ffLogoPrintChars(content.chars, false);
    return true;
}

// ---------------------------------------------------------------------------------------------
// kitty animations
//
// A `kittyanim` entry holds only what is independent of the image id: the pixel size, the frame
// count, each frame's gap, the loop count, and the frames' base64 payloads. The envelope is
// rebuilt on every run, because it carries the id and that id has to differ between runs.
// ---------------------------------------------------------------------------------------------

static const char FF_KITTY_ANIMATION_MAGIC[4] = { 'F', 'K', 'A', '1' };

typedef struct FFKittyAnimationFrame {
    int32_t delayMs;
    uint32_t payloadLength; // base64 bytes this frame occupies inside the payload block
    bool compressed;        // the frame's base64 decodes to a zlib stream, so it needs `o=z`
} FFKittyAnimationFrame;

typedef struct FFKittyAnimation {
    uint32_t width;
    uint32_t height;
    uint32_t frameCount;
    int32_t loopCount; // 0 = infinite; -1 = the source declares none
    FFKittyAnimationFrame* frames;
    char* payload; // base64 of every frame, concatenated in frame order
    uint32_t payloadLength;
} FFKittyAnimation;

static void destroyKittyAnimation(FFKittyAnimation* animation) {
    free(animation->frames);
    free(animation->payload);
    animation->frames = nullptr;
    animation->payload = nullptr;
}

// The entry is written and read by the same machine, so the host's own representation is used.
static void appendRaw(FFstrbuf* result, const void* value, uint32_t size) {
    ffStrbufAppendNS(result, size, (const char*) value);
}

static void appendUint32(FFstrbuf* result, uint32_t value) {
    appendRaw(result, &value, sizeof(value));
}

static void appendInt32(FFstrbuf* result, int32_t value) {
    appendRaw(result, &value, sizeof(value));
}

typedef struct FFKittyAnimationReader {
    const char* data;
    uint32_t length;
    uint32_t offset;
} FFKittyAnimationReader;

static bool readRaw(FFKittyAnimationReader* reader, void* out, uint32_t size) {
    if (size > reader->length - reader->offset) {
        return false;
    }

    memcpy(out, reader->data + reader->offset, size);
    reader->offset += size;
    return true;
}

static bool readUint32(FFKittyAnimationReader* reader, uint32_t* out) {
    return readRaw(reader, out, sizeof(*out));
}

static bool readInt32(FFKittyAnimationReader* reader, int32_t* out) {
    return readRaw(reader, out, sizeof(*out));
}

static void serializeKittyAnimation(FFstrbuf* result, const FFKittyAnimation* animation) {
    appendRaw(result, FF_KITTY_ANIMATION_MAGIC, sizeof(FF_KITTY_ANIMATION_MAGIC));
    appendUint32(result, animation->width);
    appendUint32(result, animation->height);
    appendUint32(result, animation->frameCount);
    appendInt32(result, animation->loopCount);

    for (uint32_t i = 0; i < animation->frameCount; ++i) {
        const FFKittyAnimationFrame* frame = &animation->frames[i];
        uint8_t flags = frame->compressed ? 1 : 0;
        appendInt32(result, frame->delayMs);
        appendUint32(result, frame->payloadLength);
        appendRaw(result, &flags, sizeof(flags));
    }

    appendRaw(result, animation->payload, animation->payloadLength);
}

static bool parseKittyAnimation(const char* data, uint32_t length, FFKittyAnimation* out, const char** error) {
    FFKittyAnimationReader reader = { .data = data, .length = length, .offset = 0 };

    char magic[sizeof(FF_KITTY_ANIMATION_MAGIC)];
    if (!readRaw(&reader, magic, sizeof(magic)) || memcmp(magic, FF_KITTY_ANIMATION_MAGIC, sizeof(magic)) != 0) {
        *error = "the cached animation has an unknown format";
        return false;
    }

    if (!readUint32(&reader, &out->width) || !readUint32(&reader, &out->height) ||
        !readUint32(&reader, &out->frameCount) || !readInt32(&reader, &out->loopCount)) {
        *error = "the cached animation is truncated";
        return false;
    }

    if (out->frameCount == 0) {
        *error = "the cached animation has no frames";
        return false;
    }

    out->frames = calloc(out->frameCount, sizeof(*out->frames));
    if (out->frames == nullptr) {
        *error = "out of memory";
        return false;
    }

    uint32_t totalPayloadLength = 0;
    for (uint32_t i = 0; i < out->frameCount; ++i) {
        FFKittyAnimationFrame* frame = &out->frames[i];
        uint8_t flags = 0;
        if (!readInt32(&reader, &frame->delayMs) || !readUint32(&reader, &frame->payloadLength) ||
            !readRaw(&reader, &flags, sizeof(flags))) {
            *error = "the cached animation is truncated";
            return false;
        }

        frame->compressed = flags != 0;

        if (totalPayloadLength > UINT32_MAX - frame->payloadLength) {
            *error = "the cached animation is too large";
            return false;
        }
        totalPayloadLength += frame->payloadLength;
    }

    if (totalPayloadLength > reader.length - reader.offset) {
        *error = "the cached animation is truncated";
        return false;
    }

    out->payloadLength = totalPayloadLength;
    out->payload = malloc((size_t) totalPayloadLength + 1);
    if (out->payload == nullptr) {
        *error = "out of memory";
        return false;
    }

    memcpy(out->payload, reader.data + reader.offset, totalPayloadLength);
    out->payload[totalPayloadLength] = '\0';
    return true;
}

// The id has to differ on every run: reusing one makes the terminal append the new frames to the
// image the previous run left in its storage, which doubles the frame count. The low 24 bits take
// part in the colour / decoration encoding, so they are kept non-zero (kitten icat does the same
// in next_random()). The entropy comes from the clock, the pid and a stack address, which is
// plenty for a process that runs once.
static uint32_t getKittyImageId(void) {
    uint32_t id = (uint32_t) ffTimeGetNow();
    id ^= (uint32_t) getpid() * 2654435761u;
    id ^= (uint32_t) ((uintptr_t) &id >> 4);
    id &= 0xFFFFFF;
    return id != 0 ? id : 1;
}

// The envelope, per the kitty protocol: the root frame is transmitted with `a=T` and has no gap
// of its own, so its gap is set with a separate `a=a` command; every further frame is an `a=f`
// command that replaces the pixels (`X=1`) because the backend already composed a full canvas.
// Every command carries the image id and `q=2`, so nothing is ever written back to the tty.
static void emitKittyAnimation(FFstrbuf* result, const FFKittyAnimation* animation, uint32_t imageId) {
    const char* currentPos = animation->payload;

    for (uint32_t i = 0; i < animation->frameCount; ++i) {
        const FFKittyAnimationFrame* frame = &animation->frames[i];
        size_t frameLength = frame->payloadLength;

        if (i == 0) {
            // Here s / v are the source rectangle, not the animation state
            ffStrbufAppendF(result, "\033_Ga=T,f=32,s=%u,v=%u,i=%u,q=2", animation->width, animation->height, imageId);
        } else {
            // A frame that covers the whole image is transmitted exactly like image data, plus the
            // frame keys. s / v are not optional here even though they always repeat the image size:
            // the terminal sizes the frame's canvas from them and rejects the command outright when
            // they are missing.
            ffStrbufAppendF(result, "\033_Ga=f,f=32,s=%u,v=%u,i=%u,z=%d,X=1,q=2",
                animation->width, animation->height, imageId, (int) frame->delayMs);
        }

        if (frame->compressed) {
            ffStrbufAppendS(result, ",o=z");
        }

        appendKittyChunk(result, &currentPos, &frameLength, false, nullptr);
        while (frameLength > 0) {
            // "When sending animation frame data, subsequent chunks must also specify the a=f key"
            appendKittyChunk(result, &currentPos, &frameLength, true, i == 0 ? "q=2" : "a=f,q=2");
        }

        if (i == 0) {
            // "the first frame or root frame is created with the base image data and has no gap,
            // so its gap must be set using this control code"
            ffStrbufAppendF(result, "\033_Ga=a,i=%u,r=1,z=%d,q=2\033\\", imageId, (int) frame->delayMs);
        }
    }

    // v is the loop count here: 1 loops forever, N loops N-1 times
    int32_t loops = 1;
    if (animation->loopCount > 0) {
        loops = animation->loopCount < INT32_MAX ? animation->loopCount + 1 : INT32_MAX;
    }
    ffStrbufAppendF(result, "\033_Ga=a,i=%u,v=%d,q=2\033\\", imageId, (int) loops);
    ffStrbufAppendF(result, "\033_Ga=a,i=%u,s=3,q=2\033\\", imageId);
}

static bool printCachedKittyAnimation(FFLogoRequestData* requestData) {
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    if (!readCachedStrbuf(requestData, &content, FF_CACHE_FILE_KITTY_ANIMATION) || content.length == 0) {
        return false;
    }

    FFKittyAnimation animation = {};
    const char* error = "the cached animation is corrupt";
    if (!parseKittyAnimation(content.chars, content.length, &animation, &error)) {
        return false;
    }

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(animation.payloadLength + animation.frameCount * 128 + 512);
    emitKittyAnimation(&result, &animation, getKittyImageId());

    const FFOptionsLogo* options = &instance.config.logo;
    instance.state.logoWidth = requestData->logoCharacterWidth + options->paddingLeft + options->paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + options->paddingTop;
    printImageResult(requestData, &result);

    destroyKittyAnimation(&animation);
    return true;
}

// Decodes the source frame by frame, encodes every frame, and stores the result as a `kittyanim`
// entry. Frames are taken, encoded and released one at a time: keeping them all would cost
// frames * W * H * 4 bytes, which a tool that prints a logo once can not afford.
static bool encodeKittyAnimation(FFLogoRequestData* requestData, const char** error) {
    FFImageAnimation* animation = nullptr;
    if (!ffImageAnimationOpen(requestData, &animation, error)) {
        return false;
    }

    fillCharacterDimensions(requestData);

    const uint32_t frameCount = ffImageAnimationFrameCount(animation);
    const int32_t loopCount = ffImageAnimationLoopCount(animation);

    if (frameCount == 1) {
        // Nothing to animate. Rendering it through the static path keeps the cache entry, and
        // therefore the bytes written to the terminal, identical to the default rendering.
        FFImageFrame frame = {};
        bool ok = ffImageAnimationGetFrame(animation, 0, &frame, error);
        ffImageAnimationClose(animation);
        if (!ok) {
            return false;
        }

        FFImageBuffer buffer = {
            .data = frame.data,
            .width = requestData->logoPixelWidth,
            .height = requestData->logoPixelHeight,
        };
        ok = printImageKitty(requestData, &buffer);
        ffImageDestroy(&buffer);
        return ok;
    }

    FFKittyAnimationFrame* frames = calloc(frameCount, sizeof(*frames));
    if (frames == nullptr) {
        ffImageAnimationClose(animation);
        *error = "out of memory";
        return false;
    }

    const size_t frameSize = (size_t) requestData->logoPixelWidth * requestData->logoPixelHeight * 4;
    FF_STRBUF_AUTO_DESTROY payload = ffStrbufCreate();
    bool ok = true;

    for (uint32_t i = 0; i < frameCount && ok; ++i) {
        FFImageFrame frame = {};
        if (!ffImageAnimationGetFrame(animation, i, &frame, error)) {
            ok = false;
            break;
        }

        frames[i].delayMs = frame.delayMs;

        FF_AUTO_FREE void* blob = malloc(frameSize);
        if (blob == nullptr) {
            ffImageFrameDestroy(&frame);
            *error = "out of memory";
            ok = false;
            break;
        }
        memcpy(blob, frame.data, frameSize);
        ffImageFrameDestroy(&frame);

        size_t blobLength = frameSize;
        #ifdef FF_HAVE_ZLIB
        frames[i].compressed = compressBlob(&blob, &blobLength);
        #else
        frames[i].compressed = false;
        #endif

        FF_STRBUF_AUTO_DESTROY base64 = ffStrbufCreateA((uint32_t) (10 + blobLength * 4 / 3));
        ffBase64EncodeRaw((uint32_t) blobLength, (const char*) blob, &base64.length, base64.chars);
        frames[i].payloadLength = base64.length;
        ffStrbufAppend(&payload, &base64);
    }

    if (ok) {
        FFKittyAnimation built = {
            .width = requestData->logoPixelWidth,
            .height = requestData->logoPixelHeight,
            .frameCount = frameCount,
            .loopCount = loopCount,
            .frames = frames,
            .payload = payload.chars,
            .payloadLength = payload.length,
        };

        FF_STRBUF_AUTO_DESTROY serialized = ffStrbufCreate();
        serializeKittyAnimation(&serialized, &built);
        writeCacheData(requestData, serialized.chars, serialized.length, FF_CACHE_FILE_KITTY_ANIMATION);

        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(serialized.length + frameCount * 128 + 512);
        emitKittyAnimation(&result, &built, getKittyImageId());
        printImagePixelsNoCache(requestData, &result);
    }

    free(frames);
    ffImageAnimationClose(animation);
    return ok;
}

// Maps --logo-animation-frame to a 0-based frame index. Negative values count back from the end,
// so -1 is the last frame. Anything out of range clamps instead of failing: the frame count of the
// source is only known once it has been opened, and a typo in a logo option is not worth aborting
// the run over (see §10.2).
static uint32_t getAnimationFrameIndex(int32_t requested, uint32_t frameCount) {
    if (frameCount == 0) {
        return 0;
    }

    if (requested < 0) {
        uint32_t back = (uint32_t) (-(int64_t) requested);
        return back <= frameCount ? frameCount - back : 0;
    }

    uint32_t index = (uint32_t) (requested - 1);
    return index < frameCount ? index : frameCount - 1;
}

// Renders one frame of an animated source as a still image. `N` is 1-based and negative values
// count from the end, matching --logo-animation-frame. Every protocol that goes through this slow
// path is served: the decoded frame is handed to the same renderers the static path uses, and the
// result is cached exactly like the first frame is (see FF_CACHE_FILE_FRAME).
static bool printAnimationFrame(FFLogoRequestData* requestData, const char** error) {
    FFImageAnimation* animation = nullptr;
    if (!ffImageAnimationOpen(requestData, &animation, error)) {
        return false;
    }

    fillCharacterDimensions(requestData);

    const uint32_t frameCount = ffImageAnimationFrameCount(animation);
    FFImageFrame frame = {};
    bool ok = ffImageAnimationGetFrame(animation, getAnimationFrameIndex(instance.config.logo.animationFrame, frameCount), &frame, error);
    ffImageAnimationClose(animation);
    if (!ok) {
        return false;
    }

    FFImageBuffer buffer = {
        .data = frame.data,
        .width = requestData->logoPixelWidth,
        .height = requestData->logoPixelHeight,
    };

    if (requestData->type == FF_LOGO_TYPE_IMAGE_SIXEL) {
        // The sixel encoder belongs to the backend, so it does not go through ffImageCreate -- but
        // it does have to be handed the frame that was just composed. ffImageSixelEncode would
        // re-read the source and encode the first frame instead of the selected one.
        FF_STRBUF_AUTO_DESTROY result = ffStrbufCreate();
        ok = ffImageSixelEncodeBuffer(&buffer, &result, error) && printImageSixel(requestData, &result);
    } else if (requestData->type == FF_LOGO_TYPE_IMAGE_KITTY) {
        ok = printImageKitty(requestData, &buffer);
    }
#if FF_HAVE_CHAFA
    else if (requestData->type == FF_LOGO_TYPE_IMAGE_CHAFA) {
        ok = printImageChafa(requestData, &buffer);
    }
#endif
    else {
        *error = "this image protocol can not render a selected frame";
        ok = false;
    }

    ffImageDestroy(&buffer);
    return ok;
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

    if (requestData->type == FF_LOGO_TYPE_IMAGE_KITTY &&
        options->animationFrame == FF_LOGO_ANIMATION_FRAME_ANIMATE) {
        // An animation entry holds payloads, not an envelope, so it can not be streamed out like
        // the static ones. It is also never allowed to fall back to a static entry: doing so would
        // silently show a still logo to a user who asked for an animation.
        return printCachedKittyAnimation(requestData);
    }

    // A selected frame shares the entry with the first frame; the sidecar is what tells the two
    // apart. This also covers the animation selector: no still rendering may answer it.
    if (options->animationFrame != readCachedFrame(requestData, FF_CACHE_FILE_FRAME)) {
        return false;
    }

    FF_AUTO_CLOSE_FD FFNativeFD fd = FF_INVALID_FD;
    if (requestData->type == FF_LOGO_TYPE_IMAGE_KITTY) {
        fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_COMPRESSED);
        if (!ffIsValidNativeFD(fd)) {
            // The pre-existing pair. Falling back between them is fine, they hold the same
            // rendering; falling back to anything else is not (see §7.1).
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

    bool sourceUnchanged = false;
    if (instance.config.logo.cache == FF_LOGO_CACHE_ON && sourceMtime != 0) {
        sourceUnchanged = readCachedUint64(&requestData, FF_CACHE_FILE_MTIME) == sourceMtime;
        if (sourceUnchanged) {
            bool cacheValid = requestData.type == FF_LOGO_TYPE_IMAGE_CHAFA
                ? printCachedChars(&requestData, FF_CACHE_FILE_CHAFA)
                : printCachedPixel(&requestData);
            if (cacheValid) {
                ffStrbufDestroy(&requestData.cacheDir);
                return true;
            }
        }
    }

    // Cache miss. The entry directory is keyed on the source path and the pixel size only, so it
    // is reused when the source is edited; drop what the previous version left behind. This only
    // happens when the source actually changed: a miss on one entry of an unchanged source just
    // means that rendering was never asked for before, and clearing the directory would throw
    // away the sibling entries every time the requested rendering changes.
    // With the cache turned off the directory is left alone entirely.
    if (instance.config.logo.cache != FF_LOGO_CACHE_OFF && !sourceUnchanged) {
        removeCachedFiles(&requestData);
    }

    const char* error = nullptr;
    bool printSuccessful = false;

    if (instance.config.logo.animationFrame == FF_LOGO_ANIMATION_FRAME_ANIMATE &&
        requestData.type != FF_LOGO_TYPE_IMAGE_KITTY) {
        // Only the kitty graphics protocol can play an animation. Falling through to the static
        // path would silently show a still logo to a user who asked for an animation.
        error = "the kitty graphics protocol is the only one that can play an animation";
    } else if (instance.config.logo.animationFrame == FF_LOGO_ANIMATION_FRAME_ANIMATE) {
        // Decided before anything is decoded: the static path never enters a loop over frames,
        // and this path never goes through ffImageCreate.
        printSuccessful = encodeKittyAnimation(&requestData, &error);
    } else if (instance.config.logo.animationFrame != FF_LOGO_ANIMATION_FRAME_FIRST) {
        // A frame other than the first. This must not fall through to the static path: that one
        // only ever produces the first frame, so it would silently hand back the wrong image.
        printSuccessful = printAnimationFrame(&requestData, &error);
    } else if (requestData.type == FF_LOGO_TYPE_IMAGE_SIXEL) {
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
