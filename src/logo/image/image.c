#include "image.h"
#include "common/io/io.h"
#include "common/printing.h"
#include "util/stringUtils.h"

#include <limits.h>
#include <math.h>

#ifdef __APPLE__
    #include <sys/syslimits.h>
#elif _WIN32
    #include <windows.h>
#endif

// https://github.com/kostya/benchmarks/blob/master/base64/test-nolib.c#L145
static void base64EncodeRaw(uint32_t size, const char *str, uint32_t *out_size, char *output)
{
    static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *out = output;
    const char *ends = str + (size - size % 3);
    while (str != ends) {
        uint32_t n = __builtin_bswap32(*(uint32_t*) str);
        *out++ = chars[(n >> 26) & 63];
        *out++ = chars[(n >> 20) & 63];
        *out++ = chars[(n >> 14) & 63];
        *out++ = chars[(n >> 8) & 63];
        str += 3;
    }

    if (size % 3 == 1) {
        uint64_t n = (uint64_t)*str << 16;
        *out++ = chars[(n >> 18) & 63];
        *out++ = chars[(n >> 12) & 63];
        *out++ = '=';
        *out++ = '=';
    } else if (size % 3 == 2) {
        uint64_t n = (uint64_t)*str++ << 16;
        n |= (uint64_t)*str << 8;
        *out++ = chars[(n >> 18) & 63];
        *out++ = chars[(n >> 12) & 63];
        *out++ = chars[(n >> 6) & 63];
        *out++ = '=';
    }
    *out = '\0';
    *out_size = (uint32_t) (out - output);
}

static FFstrbuf base64Encode(const FFstrbuf* in)
{
    FFstrbuf out = ffStrbufCreateA(10 + in->length * 4 / 3);
    base64EncodeRaw(in->length, in->chars, &out.length, out.chars);
    assert(out.length < out.allocated);

    return out;
}

static bool printImageIterm(bool printError)
{
    const FFOptionsLogo* options = &instance.config.logo;
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if(!ffAppendFileBuffer(options->source.chars, &buf))
    {
        if (printError)
            fputs("Logo (iterm): Failed to load image file\n", stderr);
        return false;
    }

    fflush(stdout);

    FF_STRBUF_AUTO_DESTROY base64 = base64Encode(&buf);
    ffStrbufClear(&buf);

    if (!options->width || !options->height)
    {
        if (!options->separate)
        {
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH",
                (unsigned) options->paddingTop,
                (unsigned) options->paddingLeft
            );
        }
        else
        {
            ffStrbufAppendNC(&buf, options->paddingTop, '\n');
            ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        }
        ffStrbufAppendF(&buf, "\e]1337;File=inline=1:%s\a",
            base64.chars
        );
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

        if (!options->separate)
        {
            uint16_t X = 0, Y = 0;
            const char* error = ffGetTerminalResponse("\e[6n", "\e[%hu;%huR", &Y, &X);
            if (error)
            {
                fprintf(stderr, "\nLogo (iterm): fail to query cursor position: %s\n", error);
                return true; // We already printed image logo, don't print ascii logo then
            }
            instance.state.logoWidth = X + options->paddingRight;
            instance.state.logoHeight = Y;
            fputs("\e[H", stdout);
        }
        else
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffPrintCharTimes('\n', options->paddingRight);
        }
    }
    else
    {
        ffStrbufAppendNC(&buf, options->paddingTop, '\n');
        ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        ffStrbufAppendF(&buf, "\e]1337;File=inline=1;width=%u;height=%u;preserveAspectRatio=%u:%s\a\n",
            (unsigned) options->width,
            (unsigned) options->height,
            (unsigned) options->preserveAspectRatio,
            base64.chars
        );

        if (!options->separate)
        {
            instance.state.logoWidth = options->width + options->paddingLeft + options->paddingRight;
            instance.state.logoHeight = options->paddingTop + options->height;
            ffStrbufAppendF(&buf, "\e[%uA", (unsigned) instance.state.logoHeight);
        }
        else
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffStrbufAppendNC(&buf, options->paddingRight, '\n');
        }
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
    }

    return true;
}

static bool printImageKittyDirect(bool printError)
{
    const FFOptionsLogo* options = &instance.config.logo;

    if (!ffPathExists(options->source.chars, FF_PATHTYPE_FILE))
    {
        if (printError)
            fputs("Logo (kitty-direct): Failed to load image file\n", stderr);
        return false;
    }

    FF_STRBUF_AUTO_DESTROY base64 = base64Encode(&options->source);

    if (!options->width || !options->height)
    {
        if (!options->separate)
        {
            // We must clear the entre screen to make sure that terminal buffer won't scroll up
            printf("\e[2J\e[3J\e[%u;%uH",
                (unsigned) options->paddingTop,
                (unsigned) options->paddingLeft
            );
        }
        else
        {
            ffPrintCharTimes('\n', options->paddingTop);
            ffPrintCharTimes(' ', options->paddingLeft);
        }
        printf("\e_Ga=T,f=100,t=f;%s\e\\", base64.chars);
        fflush(stdout);
        if (!options->separate)
        {
            uint16_t X = 0, Y = 0;
            const char* error = ffGetTerminalResponse("\e[6n", "\e[%hu;%huR", &Y, &X);
            if (error)
            {
                fprintf(stderr, "\nLogo (kitty-direct): fail to query cursor position: %s\n", error);
                return true; // We already printed image logo, don't print ascii logo then
            }
            instance.state.logoWidth = X + options->paddingRight;
            instance.state.logoHeight = Y;
            fputs("\e[H", stdout);
        }
        else
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffPrintCharTimes('\n', options->paddingRight);
        }
    }
    else
    {
        ffPrintCharTimes('\n', options->paddingTop);
        ffPrintCharTimes(' ', options->paddingLeft);

        printf("\e_Ga=T,f=100,t=f,c=%u,r=%u;%s\e\\\n",
            (unsigned) options->width,
            (unsigned) options->height,
            base64.chars
        );
        if (!options->separate)
        {
            instance.state.logoWidth = options->width + options->paddingLeft + options->paddingRight;
            instance.state.logoHeight = options->paddingTop + options->height;
            printf("\e[%uA", (unsigned) instance.state.logoHeight);
        }
        else
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffPrintCharTimes('\n', options->paddingRight);
        }
    }

    return true;
}

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

#define FF_KITTY_MAX_CHUNK_SIZE 4096

#define FF_CACHE_FILE_HEIGHT "height"
#define FF_CACHE_FILE_WIDTH "width"
#define FF_CACHE_FILE_SIXEL "sixel"
#define FF_CACHE_FILE_KITTY_COMPRESSED "kittyc"
#define FF_CACHE_FILE_KITTY_UNCOMPRESSED "kittyu"
#define FF_CACHE_FILE_CHAFA "chafa"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#else
#include <wincon.h>

static inline char* realpath(const char* restrict file_name, char* restrict resolved_name)
{
    char* result = _fullpath(resolved_name, file_name, _MAX_PATH);
    if(result)
    {
        resolved_name[1] = resolved_name[0]; // Drive Name
        resolved_name[0] = '/';
    }
    return result;
}
#endif

#ifdef FF_HAVE_ZLIB
#include "common/library.h"
#include <stdlib.h>
#include <zlib.h>

static bool compressBlob(void** blob, size_t* length)
{
    FF_LIBRARY_LOAD(zlib, &instance.config.library.libZ, false, "libz" FF_LIBRARY_EXTENSION, 2)
    FF_LIBRARY_LOAD_SYMBOL(zlib, compressBound, false)
    FF_LIBRARY_LOAD_SYMBOL(zlib, compress2, false)

    uLong compressedLength = ffcompressBound(*length);
    void* compressed = malloc(compressedLength);
    if(compressed == NULL)
        return false;

    if(ffcompress2(compressed, &compressedLength, *blob, *length, Z_BEST_COMPRESSION) != Z_OK)
    {
        free(compressed);
        return false;
    }

    free(*blob);

    *length = (size_t) compressedLength;
    *blob = compressed;
    return true;
}

#endif // FF_HAVE_ZLIB

//We use only the defines from here, that are exactly the same in both versions
#ifdef FF_HAVE_IMAGEMAGICK7
    #include <MagickCore/MagickCore.h>
#else
    #include <magick/MagickCore.h>
#endif

typedef struct ImageData
{
    FF_LIBRARY_SYMBOL(CopyMagickString)
    FF_LIBRARY_SYMBOL(ImageToBlob)
    FF_LIBRARY_SYMBOL(Base64Encode)

    ImageInfo* imageInfo;
    Image* image;
    ExceptionInfo* exceptionInfo;
} ImageData;

static inline bool checkAllocationResult(void* data, size_t length)
{
    if(data == NULL)
        return false;

    if(length == 0)
    {
        free(data);
        return false;
    }

    return true;
}

static void writeCacheStrbuf(FFLogoRequestData* requestData, const FFstrbuf* value, const char* cacheFileName)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, cacheFileName);
    ffWriteFileBuffer(requestData->cacheDir.chars, value);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
}

static void writeCacheUint32(FFLogoRequestData* requestData, uint32_t value, const char* cacheFileName)
{
    FFstrbuf content;
    content.chars = (char*) &value;
    content.length = sizeof(value);
    writeCacheStrbuf(requestData, &content, cacheFileName);
}

static void printImagePixels(FFLogoRequestData* requestData, const FFstrbuf* result, const char* cacheFileName)
{
    const FFOptionsLogo* options = &instance.config.logo;
    //Calculate character dimensions
    instance.state.logoWidth = requestData->logoCharacterWidth + options->paddingLeft + options->paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + options->paddingTop - 1;

    //Write cache files
    writeCacheStrbuf(requestData, result, cacheFileName);

    if(options->width == 0)
        writeCacheUint32(requestData, requestData->logoCharacterWidth, FF_CACHE_FILE_WIDTH);

    if(options->height == 0)
        writeCacheUint32(requestData, requestData->logoCharacterHeight, FF_CACHE_FILE_HEIGHT);

    //Write result to stdout
    ffPrintCharTimes('\n', options->paddingTop);
    ffPrintCharTimes(' ', options->paddingLeft);
    fflush(stdout);
    ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), result);

    //Go to upper left corner
    printf("\e[1G\e[%uA", instance.state.logoHeight);
}

static bool printImageSixel(FFLogoRequestData* requestData, const ImageData* imageData)
{
    imageData->ffCopyMagickString(imageData->imageInfo->magick, "SIXEL", 6);

    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    FFstrbuf result;
    result.chars = (char*) blob;
    result.length = (uint32_t) length;

    printImagePixels(requestData, &result, FF_CACHE_FILE_SIXEL);

    free(blob);
    return true;
}

static void appendKittyChunk(FFstrbuf* result, const char** blob, size_t* length, bool printEscapeCode)
{
    uint32_t chunkSize = *length > FF_KITTY_MAX_CHUNK_SIZE ? FF_KITTY_MAX_CHUNK_SIZE : (uint32_t) *length;

    if(printEscapeCode)
        ffStrbufAppendS(result, "\033_G");
    else
        ffStrbufAppendC(result, ',');

    ffStrbufAppendS(result, chunkSize != *length ? "m=1" : "m=0");
    ffStrbufAppendC(result, ';');
    ffStrbufAppendNS(result, chunkSize, *blob);
    ffStrbufAppendS(result, "\033\\");
    *length -= chunkSize;
    *blob += chunkSize;
}

static bool printImageKitty(FFLogoRequestData* requestData, const ImageData* imageData)
{
    imageData->ffCopyMagickString(imageData->imageInfo->magick, "RGBA", 5);

    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    #ifdef FF_HAVE_ZLIB
        bool isCompressed = compressBlob(&blob, &length);
    #else
        bool isCompressed = false;
    #endif

    char* chars = imageData->ffBase64Encode(blob, length, &length);
    free(blob);
    if(!checkAllocationResult(chars, length))
        return false;

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA((uint32_t) (length + 1024));

    const char* currentPos = chars;
    size_t remainingLength = length;

    ffStrbufAppendF(&result, "\033_Ga=T,f=32,s=%u,v=%u", requestData->logoPixelWidth, requestData->logoPixelHeight);
    if(isCompressed)
        ffStrbufAppendS(&result, ",o=z");
    appendKittyChunk(&result, &currentPos, &remainingLength, false);
    while(remainingLength > 0)
        appendKittyChunk(&result, &currentPos, &remainingLength, true);

    printImagePixels(requestData, &result, isCompressed ? FF_CACHE_FILE_KITTY_COMPRESSED : FF_CACHE_FILE_KITTY_UNCOMPRESSED);

    free(chars);
    return true;
}

#ifdef FF_HAVE_CHAFA
#include <chafa.h>
static bool printImageChafa(FFLogoRequestData* requestData, const ImageData* imageData)
{
    FF_LIBRARY_LOAD(chafa, &instance.config.library.libChafa, false,
        "libchafa" FF_LIBRARY_EXTENSION, 1,
        "libchafa-0" FF_LIBRARY_EXTENSION, -1 // Required for Windows
    )
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

    imageData->ffCopyMagickString(imageData->imageInfo->magick, "RGBA", 5);
    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    ChafaSymbolMap* symbolMap = ffchafa_symbol_map_new();
    GError* error = NULL;
    if(!ffchafa_symbol_map_apply_selectors(symbolMap, instance.config.logo.chafaSymbols.chars, &error))
        fputs(error->message, stderr);

    ChafaCanvasConfig* canvasConfig = ffchafa_canvas_config_new();
    ffchafa_canvas_config_set_geometry(canvasConfig, (gint) requestData->logoCharacterWidth, (gint) requestData->logoCharacterHeight);
    ffchafa_canvas_config_set_symbol_map(canvasConfig, symbolMap);

    if(instance.config.logo.chafaFgOnly)
    {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_fg_only_enabled);
        if(ffchafa_canvas_config_set_fg_only_enabled)
            ffchafa_canvas_config_set_fg_only_enabled(canvasConfig, true);
    }
    if(instance.config.logo.chafaCanvasMode < CHAFA_CANVAS_MODE_MAX)
    {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_canvas_mode);
        if(ffchafa_canvas_config_set_canvas_mode)
            ffchafa_canvas_config_set_canvas_mode(canvasConfig, (ChafaCanvasMode) instance.config.logo.chafaCanvasMode);
    }
    if(instance.config.logo.chafaColorSpace < CHAFA_COLOR_SPACE_MAX)
    {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_color_space)
        if(ffchafa_canvas_config_set_color_space)
            ffchafa_canvas_config_set_color_space(canvasConfig, (ChafaColorSpace) instance.config.logo.chafaColorSpace);
    }
    if(instance.config.logo.chafaDitherMode < CHAFA_DITHER_MODE_MAX)
    {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, chafa_canvas_config_set_dither_mode)
        if(ffchafa_canvas_config_set_dither_mode)
            ffchafa_canvas_config_set_dither_mode(canvasConfig, (ChafaDitherMode) instance.config.logo.chafaDitherMode);
    }

    ChafaCanvas* canvas = ffchafa_canvas_new(canvasConfig);
    ffchafa_canvas_draw_all_pixels(
        canvas,
        CHAFA_PIXEL_RGBA8_UNASSOCIATED,
        blob,
        (gint) imageData->image->columns,
        (gint) imageData->image->rows,
        (gint) imageData->image->columns * 4
    );

    GString* str = ffchafa_canvas_print(canvas, NULL);
    FFstrbuf result;
    result.allocated = (uint32_t) str->allocated_len;
    result.length = (uint32_t) str->len;
    result.chars = str->str;

    ffLogoPrintChars(result.chars, false);
    writeCacheStrbuf(requestData, &result, FF_CACHE_FILE_CHAFA);

    // FIXME: These functions must be imported from `libglib` dlls on Windows
    FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, g_string_free);
    if(ffg_string_free)
        ffg_string_free(str, TRUE);
    if(error)
    {
        FF_LIBRARY_LOAD_SYMBOL_LAZY(chafa, g_error_free)
        if(ffg_error_free)
            ffg_error_free(error);
    }

    ffchafa_canvas_unref(canvas);
    ffchafa_canvas_config_unref(canvasConfig);
    ffchafa_symbol_map_unref(symbolMap);

    return true;
}
#endif

FFLogoImageResult ffLogoPrintImageImpl(FFLogoRequestData* requestData, const FFIMData* imData)
{
    FF_LIBRARY_LOAD_SYMBOL(imData->library, MagickCoreGenesis, FF_LOGO_IMAGE_RESULT_INIT_ERROR);
    FF_LIBRARY_LOAD_SYMBOL(imData->library, MagickCoreTerminus, FF_LOGO_IMAGE_RESULT_INIT_ERROR);
    FF_LIBRARY_LOAD_SYMBOL(imData->library, AcquireExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, DestroyExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, AcquireImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, DestroyImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, ReadImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, DestroyImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    ImageData imageData;

    FF_LIBRARY_LOAD_SYMBOL_VAR(imData->library, imageData, CopyMagickString, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL_VAR(imData->library, imageData, ImageToBlob, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL_VAR(imData->library, imageData, Base64Encode, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    ffMagickCoreGenesis(NULL, MagickFalse);

    imageData.exceptionInfo = ffAcquireExceptionInfo();
    if(imageData.exceptionInfo == NULL)
    {
        ffMagickCoreTerminus();
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    ImageInfo* imageInfoIn = ffAcquireImageInfo();
    if(imageInfoIn == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        ffMagickCoreTerminus();
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    //+1, because we need to copy the null byte too
    imageData.ffCopyMagickString(imageInfoIn->filename, instance.config.logo.source.chars, instance.config.logo.source.length + 1);

    imageData.image = ffReadImage(imageInfoIn, imageData.exceptionInfo);
    ffDestroyImageInfo(imageInfoIn);
    if(imageData.image == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        ffMagickCoreTerminus();
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    if(requestData->logoPixelWidth == 0 && requestData->logoPixelHeight == 0)
    {
        requestData->logoPixelWidth = (uint32_t) imageData.image->columns;
        requestData->logoPixelHeight = (uint32_t) imageData.image->rows;
    }
    else if(requestData->logoPixelWidth == 0)
        requestData->logoPixelWidth = (uint32_t) ((double) imageData.image->columns / (double) imageData.image->rows * requestData->logoPixelHeight);
    else if(requestData->logoPixelHeight == 0)
        requestData->logoPixelHeight = (uint32_t) ((double) imageData.image->rows / (double) imageData.image->columns * requestData->logoPixelWidth);

    requestData->logoCharacterWidth = (uint32_t) ceil((double) requestData->logoPixelWidth / requestData->characterPixelWidth);
    requestData->logoCharacterHeight = (uint32_t) ceil((double) requestData->logoPixelHeight / requestData->characterPixelHeight);

    if(requestData->logoPixelWidth == 0 || requestData->logoPixelHeight == 0 || requestData->logoCharacterWidth == 0 || requestData->logoCharacterHeight == 0)
    {
        ffDestroyImage(imageData.image);
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        ffMagickCoreTerminus();
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    Image* resized = imData->resizeFunc(imageData.image, requestData->logoPixelWidth, requestData->logoPixelHeight, imageData.exceptionInfo);
    ffDestroyImage(imageData.image);
    if(resized == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        ffMagickCoreTerminus();
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }
    imageData.image = resized;

    imageData.imageInfo = ffAcquireImageInfo();
    if(imageData.imageInfo == NULL)
    {
        ffDestroyImage(imageData.image);
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        ffMagickCoreTerminus();
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    bool printSuccessful = false;
    if(requestData->type == FF_LOGO_TYPE_IMAGE_CHAFA)
    {
        #ifdef FF_HAVE_CHAFA
            printSuccessful = printImageChafa(requestData, &imageData);
        #endif
    }
    else if(requestData->type == FF_LOGO_TYPE_IMAGE_KITTY)
        printSuccessful = printImageKitty(requestData, &imageData);
    else if(requestData->type == FF_LOGO_TYPE_IMAGE_SIXEL)
        printSuccessful = printImageSixel(requestData, &imageData);

    ffDestroyImageInfo(imageData.imageInfo);
    ffDestroyImage(imageData.image);
    ffDestroyExceptionInfo(imageData.exceptionInfo);
    ffMagickCoreTerminus();

    return printSuccessful ? FF_LOGO_IMAGE_RESULT_SUCCESS : FF_LOGO_IMAGE_RESULT_RUN_ERROR;
}

static int getCacheFD(FFLogoRequestData* requestData, const char* fileName)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, fileName);
    int fd = open(requestData->cacheDir.chars, O_RDONLY
        #ifdef O_CLOEXEC
            | O_CLOEXEC
        #endif
    );
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    return fd;
}

static void readCachedStrbuf(FFLogoRequestData* requestData, FFstrbuf* result, const char* cacheFileName)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, cacheFileName);
    ffAppendFileBuffer(requestData->cacheDir.chars, result);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
}

static uint32_t readCachedUint32(FFLogoRequestData* requestData, const char* cacheFileName)
{
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();
    readCachedStrbuf(requestData, &content, cacheFileName);

    uint32_t result = 0;

    if(content.length != sizeof(result))
        return 0;

    memcpy(&result, content.chars, sizeof(result));

    return result;
}

static bool printCachedChars(FFLogoRequestData* requestData)
{
    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreateA(32768);

    if(requestData->type == FF_LOGO_TYPE_IMAGE_CHAFA)
        readCachedStrbuf(requestData, &content, FF_CACHE_FILE_CHAFA);

    if(content.length == 0)
        return false;

    ffLogoPrintChars(content.chars, false);
    return true;
}

static bool printCachedPixel(FFLogoRequestData* requestData)
{
    requestData->logoCharacterWidth = instance.config.logo.width;
    if(requestData->logoCharacterWidth == 0)
    {
        requestData->logoCharacterWidth = readCachedUint32(requestData, FF_CACHE_FILE_WIDTH);
        if(requestData->logoCharacterWidth == 0)
            return false;
    }

    requestData->logoCharacterHeight = instance.config.logo.height;
    if(requestData->logoCharacterHeight == 0)
    {
        requestData->logoCharacterHeight = readCachedUint32(requestData, FF_CACHE_FILE_HEIGHT);
        if(requestData->logoCharacterHeight == 0)
            return false;
    }

    int fd = -1;
    if(requestData->type == FF_LOGO_TYPE_IMAGE_KITTY)
    {
        fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_COMPRESSED);
        if(fd == -1)
            fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_UNCOMPRESSED);
    }
    else if(requestData->type == FF_LOGO_TYPE_IMAGE_SIXEL)
        fd = getCacheFD(requestData, FF_CACHE_FILE_SIXEL);

    if(fd == -1)
        return false;

    ffPrintCharTimes('\n', instance.config.logo.paddingTop);
    ffPrintCharTimes(' ', instance.config.logo.paddingLeft);
    fflush(stdout);

    char buffer[32768];
    ssize_t readBytes;
    while((readBytes = ffReadFDData(FFUnixFD2NativeFD(fd), sizeof(buffer), buffer)) > 0)
        ffWriteFDData(FFUnixFD2NativeFD(STDOUT_FILENO), (size_t) readBytes, buffer);

    close(fd);

    instance.state.logoWidth = requestData->logoCharacterWidth + instance.config.logo.paddingLeft + instance.config.logo.paddingRight;
    instance.state.logoHeight = requestData->logoCharacterHeight + instance.config.logo.paddingTop;

    //Go to upper left corner
    printf("\e[1G\e[%uA", instance.state.logoHeight);
    return true;
}

static bool printCached(FFLogoRequestData* requestData)
{
    if(requestData->type == FF_LOGO_TYPE_IMAGE_CHAFA)
        return printCachedChars(requestData);
    else
        return printCachedPixel(requestData);
}

static bool getCharacterPixelDimensions(FFLogoRequestData* requestData)
{
    #ifndef _WIN32

    struct winsize winsize;

    //Initialize every member to 0, because it isn't guaranteed that every terminal sets them all
    memset(&winsize, 0, sizeof(struct winsize));

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

    if(winsize.ws_row == 0 || winsize.ws_col == 0)
        ffGetTerminalResponse("\033[18t", "\033[8;%hu;%hut", &winsize.ws_row, &winsize.ws_col);

    if(winsize.ws_row == 0 || winsize.ws_col == 0)
        return false;

    if(winsize.ws_ypixel == 0 || winsize.ws_xpixel == 0)
        ffGetTerminalResponse("\033[14t", "\033[4;%hu;%hut", &winsize.ws_ypixel, &winsize.ws_xpixel);

    requestData->characterPixelWidth = winsize.ws_xpixel / (double) winsize.ws_col;
    requestData->characterPixelHeight = winsize.ws_ypixel / (double) winsize.ws_row;

    #else

    CONSOLE_FONT_INFO cfi;
    if(GetCurrentConsoleFont(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi) == FALSE) // Only works for ConHost
        return false;

    requestData->characterPixelWidth = cfi.dwFontSize.X;
    requestData->characterPixelHeight = cfi.dwFontSize.Y;

    #endif

    return requestData->characterPixelWidth > 1.0 && requestData->characterPixelHeight > 1.0;
}

static bool printImageIfExistsSlowPath(FFLogoType type, bool printError)
{
    FFLogoRequestData requestData;
    requestData.type = type;
    requestData.characterPixelWidth = 1;
    requestData.characterPixelHeight = 1;

    if(!getCharacterPixelDimensions(&requestData))
    {
        if(printError)
            fputs("Logo: getCharacterPixelDimensions() failed\n", stderr);
        return false;
    }

    requestData.logoPixelWidth = (uint32_t) ceil((double) instance.config.logo.width * requestData.characterPixelWidth);
    requestData.logoPixelHeight = (uint32_t) ceil((double) instance.config.logo.height * requestData.characterPixelHeight);

    ffStrbufInit(&requestData.cacheDir);
    ffStrbufAppend(&requestData.cacheDir, &instance.state.platform.cacheDir);
    ffStrbufAppendS(&requestData.cacheDir, "fastfetch/images");

    ffStrbufEnsureFree(&requestData.cacheDir, PATH_MAX);
    if(realpath(instance.config.logo.source.chars, requestData.cacheDir.chars + requestData.cacheDir.length) == NULL)
    {
        //We can safely return here, because if realpath failed, we surely won't be able to read the file
        ffStrbufDestroy(&requestData.cacheDir);
        if(printError)
            fputs("Logo: Querying realpath of the image source failed\n", stderr);
        return false;
    }
    ffStrbufRecalculateLength(&requestData.cacheDir);
    ffStrbufEnsureEndsWithC(&requestData.cacheDir, '/');
    ffStrbufAppendF(&requestData.cacheDir, "%u*%u/", requestData.logoPixelWidth, requestData.logoPixelHeight);

    if(!instance.config.logo.recache && printCached(&requestData))
    {
        ffStrbufDestroy(&requestData.cacheDir);
        return true;
    }

    FFLogoImageResult result = FF_LOGO_IMAGE_RESULT_INIT_ERROR;

    #ifdef FF_HAVE_IMAGEMAGICK7
        result = ffLogoPrintImageIM7(&requestData);
    #endif

    #ifdef FF_HAVE_IMAGEMAGICK6
        if(result == FF_LOGO_IMAGE_RESULT_INIT_ERROR)
            result = ffLogoPrintImageIM6(&requestData);
    #endif

    ffStrbufDestroy(&requestData.cacheDir);

    if(result == FF_LOGO_IMAGE_RESULT_SUCCESS)
        return true;

    if(printError)
    {
        if(result == FF_LOGO_IMAGE_RESULT_INIT_ERROR)
            fputs("Logo: Image Magick library not found\n", stderr);
        else
            fputs("Logo: Failed to load / convert the image source\n", stderr);
    }

    return false;
}

#endif //FF_HAVE_IMAGEMAGICK{6, 7}

bool ffLogoPrintImageIfExists(FFLogoType type, bool printError)
{
    if(instance.config.display.pipe)
    {
        if(printError)
            fputs("Logo: Image logo is not supported in pipe mode\n", stderr);
        return false;
    }

    if(!ffPathExists(instance.config.logo.source.chars, FF_PATHTYPE_FILE))
    {
        if(printError)
            fprintf(stderr, "Logo: Image source \"%s\" does not exist\n", instance.config.logo.source.chars);
        return false;
    }

    const char* term = getenv("TERM");
    if((term && ffStrEquals(term, "screen")) || getenv("ZELLIJ"))
    {
        if(printError)
            fputs("Logo: Image logo is not supported in terminal multiplexers\n", stderr);
        return false;
    }

    if(type == FF_LOGO_TYPE_IMAGE_ITERM)
        return printImageIterm(printError);

    if(type == FF_LOGO_TYPE_IMAGE_KITTY_DIRECT)
        return printImageKittyDirect(printError);

    #if !defined(FF_HAVE_CHAFA)
        if(type == FF_LOGO_TYPE_IMAGE_CHAFA)
        {
            if(printError)
                fputs("Logo: Chafa support is not compiled in\n", stderr);
            return false;
        }
    #endif

    #if !defined(FF_HAVE_IMAGEMAGICK7) && !defined(FF_HAVE_IMAGEMAGICK6)
        if(printError)
            fputs("Logo: Image Magick support is not compiled in\n", stderr);
        return false;
    #else
        return printImageIfExistsSlowPath(type, printError);
    #endif
}
