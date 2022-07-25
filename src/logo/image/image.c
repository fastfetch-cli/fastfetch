#include "image.h"
#include "common/io.h"
#include "common/printing.h"

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
#include <sys/ioctl.h>

#ifdef FF_HAVE_ZLIB
#include "common/library.h"
#include <stdlib.h>
#include <zlib.h>

static bool compressBlob(const FFinstance* instance, void** blob, size_t* length)
{
    FF_LIBRARY_LOAD(zlib, instance->config.libZ, false, "libz.so", 2)
    FF_LIBRARY_LOAD_SYMBOL(zlib, compressBound, false)
    FF_LIBRARY_LOAD_SYMBOL(zlib, compress2, false)

    uLong compressedLength = ffcompressBound(*length);
    void* compressed = malloc(compressedLength);
    if(compressed == NULL)
    {
        dlclose(zlib);
        return false;
    }

    int compressResult = ffcompress2(compressed, &compressedLength, *blob, *length, Z_BEST_COMPRESSION);
    dlclose(zlib);
    if(compressResult != Z_OK)
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

static inline uint32_t simpleCeil(double value)
{
    uint32_t result = (uint32_t) value;
    return value == (double) result ? result : result + 1;
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

static void printImagePixels(FFinstance* instance, FFLogoRequestData* requestData, const FFstrbuf* result, const char* cacheFileName)
{
    //Calculate character dimensions
    instance->state.logoWidth = requestData->logoCharacterWidth + instance->config.logo.paddingLeft + instance->config.logo.paddingRight;

    instance->state.logoHeight = requestData->logoCharacterHeight;
    if(requestData->type == FF_LOGO_TYPE_KITTY)
        instance->state.logoHeight -= 1;

    //Write cache files
    writeCacheStrbuf(requestData, result, cacheFileName);

    if(instance->config.logo.width == 0)
        writeCacheUint32(requestData, instance->state.logoWidth, FF_CACHE_FILE_WIDTH);

    if(instance->config.logo.height == 0)
        writeCacheUint32(requestData, instance->state.logoHeight, FF_CACHE_FILE_HEIGHT);

    //Write result to stdout
    ffPrintCharTimes(' ', instance->config.logo.paddingLeft);
    fflush(stdout);
    ffWriteFDBuffer(STDOUT_FILENO, result);

    //Go to upper left corner
    fputs("\033[9999999D", stdout);
    printf("\033[%uA", instance->state.logoHeight);
}

static bool printImageSixel(FFinstance* instance, FFLogoRequestData* requestData, const ImageData* imageData)
{
    imageData->ffCopyMagickString(imageData->imageInfo->magick, "SIXEL", 6);

    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    FFstrbuf result;
    result.chars = (char*) blob;
    result.length = (uint32_t) length;

    printImagePixels(instance, requestData, &result, FF_CACHE_FILE_SIXEL);

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

static bool printImageKitty(FFinstance* instance, FFLogoRequestData* requestData, const ImageData* imageData)
{
    imageData->ffCopyMagickString(imageData->imageInfo->magick, "RGBA", 5);

    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    #ifdef FF_HAVE_ZLIB
        bool isCompressed = compressBlob(instance, &blob, &length);
    #else
        bool isCompressed = false;
    #endif

    char* chars = imageData->ffBase64Encode(blob, length, &length);
    free(blob);
    if(!checkAllocationResult(chars, length))
        return false;

    FFstrbuf result;
    ffStrbufInitA(&result, (uint32_t) (length + 1024));

    const char* currentPos = chars;
    size_t remainingLength = length;

    ffStrbufAppendF(&result, "\033_Ga=T,f=32,s=%u,v=%u", requestData->logoPixelWidth, requestData->logoPixelHeight);
    if(isCompressed)
        ffStrbufAppendS(&result, ",o=z");
    appendKittyChunk(&result, &currentPos, &remainingLength, false);
    while(remainingLength > 0)
        appendKittyChunk(&result, &currentPos, &remainingLength, true);

    printImagePixels(instance, requestData, &result, isCompressed ? FF_CACHE_FILE_KITTY_COMPRESSED : FF_CACHE_FILE_KITTY_UNCOMPRESSED);

    ffStrbufDestroy(&result);
    free(chars);
    return true;
}

#ifdef FF_HAVE_CHAFA
#include <chafa/chafa.h>
static bool printImageChafa(FFinstance* instance, FFLogoRequestData* requestData, const ImageData* imageData)
{
    FF_LIBRARY_LOAD(chafa, instance->config.libChafa, false, "libchafa.so", 1)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_symbol_map_new, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_symbol_map_add_by_tags, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_new, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_set_geometry, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_set_symbol_map, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_new, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_draw_all_pixels, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_print, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_unref, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_canvas_config_unref, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, chafa_symbol_map_unref, false)
    FF_LIBRARY_LOAD_SYMBOL(chafa, g_string_free, false)

    imageData->ffCopyMagickString(imageData->imageInfo->magick, "RGBA", 5);
    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    ChafaSymbolMap* symbolMap = ffchafa_symbol_map_new();
    ffchafa_symbol_map_add_by_tags(symbolMap, CHAFA_SYMBOL_TAG_ALL);

    ChafaCanvasConfig* canvasConfig = ffchafa_canvas_config_new();
    ffchafa_canvas_config_set_geometry(canvasConfig, (gint) requestData->logoCharacterWidth, (gint) requestData->logoCharacterHeight);
    ffchafa_canvas_config_set_symbol_map(canvasConfig, symbolMap);

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

    ffLogoPrintChars(instance, result.chars, false);
    writeCacheStrbuf(requestData, &result, FF_CACHE_FILE_CHAFA);

    ffg_string_free(str, TRUE);
    ffchafa_canvas_unref(canvas);
    ffchafa_canvas_config_unref(canvasConfig);
    ffchafa_symbol_map_unref(symbolMap);

    return true;
}
#endif

FFLogoImageResult ffLogoPrintImageImpl(FFinstance* instance, FFLogoRequestData* requestData, const FFIMData* imData)
{
    FF_LIBRARY_LOAD_SYMBOL(imData->library, AcquireExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, DestroyExceptionInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, AcquireImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, DestroyImageInfo, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, ReadImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL(imData->library, DestroyImage, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    ImageData imageData;

    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imData->library, imageData.ffCopyMagickString, CopyMagickString, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imData->library, imageData.ffImageToBlob, ImageToBlob, FF_LOGO_IMAGE_RESULT_INIT_ERROR)
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(imData->library, imageData.ffBase64Encode, Base64Encode, FF_LOGO_IMAGE_RESULT_INIT_ERROR)

    imageData.exceptionInfo = ffAcquireExceptionInfo();
    if(imageData.exceptionInfo == NULL)
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;

    ImageInfo* imageInfoIn = ffAcquireImageInfo();
    if(imageInfoIn == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    //+1, because we need to copy the null byte too
    imageData.ffCopyMagickString(imageInfoIn->filename, instance->config.logo.source.chars, instance->config.logo.source.length + 1);

    imageData.image = ffReadImage(imageInfoIn, imageData.exceptionInfo);
    ffDestroyImageInfo(imageInfoIn);
    if(imageData.image == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
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

    requestData->logoCharacterWidth = simpleCeil((double) requestData->logoPixelWidth / requestData->characterPixelWidth);
    requestData->logoCharacterHeight = simpleCeil((double) requestData->logoPixelHeight / requestData->characterPixelHeight);

    if(requestData->logoPixelWidth == 0 || requestData->logoPixelHeight == 0 || requestData->logoCharacterWidth == 0 || requestData->logoCharacterHeight == 0)
    {
        ffDestroyImage(imageData.image);
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    Image* resized = imData->resizeFunc(imageData.image, requestData->logoPixelWidth, requestData->logoPixelHeight, imageData.exceptionInfo);
    ffDestroyImage(imageData.image);
    if(resized == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }
    imageData.image = resized;

    imageData.imageInfo = ffAcquireImageInfo();
    if(imageData.imageInfo == NULL)
    {
        ffDestroyImage(imageData.image);
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    bool printSuccessful = false;
    if(requestData->type == FF_LOGO_TYPE_CHAFA)
    {
        #ifdef FF_HAVE_CHAFA
            printSuccessful = printImageChafa(instance, requestData, &imageData);
        #endif
    }
    else if(requestData->type == FF_LOGO_TYPE_KITTY)
        printSuccessful = printImageKitty(instance, requestData, &imageData);
    else if(requestData->type == FF_LOGO_TYPE_SIXEL)
        printSuccessful = printImageSixel(instance, requestData, &imageData);

    ffDestroyImageInfo(imageData.imageInfo);
    ffDestroyImage(imageData.image);
    ffDestroyExceptionInfo(imageData.exceptionInfo);

    return printSuccessful ? FF_LOGO_IMAGE_RESULT_SUCCESS : FF_LOGO_IMAGE_RESULT_RUN_ERROR;
}

static int getCacheFD(FFLogoRequestData* requestData, const char* fileName)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, fileName);
    int fd = open(requestData->cacheDir.chars, O_RDONLY);
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
    FFstrbuf content;
    ffStrbufInit(&content);
    readCachedStrbuf(requestData, &content, cacheFileName);

    uint32_t result = 0;

    if(content.length != sizeof(result))
    {
        ffStrbufDestroy(&content);
        return 0;
    }

    memcpy(&result, content.chars, sizeof(result));

    ffStrbufDestroy(&content);

    return result;
}

static bool printCachedChars(FFinstance* instance, FFLogoRequestData* requestData)
{
    FFstrbuf content;
    ffStrbufInitA(&content, 32768);

    if(requestData->type == FF_LOGO_TYPE_CHAFA)
        readCachedStrbuf(requestData, &content, FF_CACHE_FILE_CHAFA);

    if(content.length == 0)
    {
        ffStrbufDestroy(&content);
        return false;
    }

    ffLogoPrintChars(instance, content.chars, false);
    ffStrbufDestroy(&content);
    return true;
}

static bool printCachedPixel(FFinstance* instance, FFLogoRequestData* requestData)
{
    requestData->logoCharacterWidth = instance->config.logo.width;
    if(requestData->logoCharacterWidth == 0)
    {
        requestData->logoCharacterWidth = readCachedUint32(requestData, FF_CACHE_FILE_WIDTH);
        if(requestData->logoCharacterWidth == 0)
            return false;
    }

    requestData->logoCharacterHeight = instance->config.logo.height;
    if(requestData->logoCharacterHeight == 0)
    {
        requestData->logoCharacterHeight = readCachedUint32(requestData, FF_CACHE_FILE_HEIGHT);
        if(requestData->logoCharacterHeight == 0)
            return false;
    }

    int fd = -1;
    if(requestData->type == FF_LOGO_TYPE_KITTY)
    {
        fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_COMPRESSED);
        if(fd == -1)
            fd = getCacheFD(requestData, FF_CACHE_FILE_KITTY_UNCOMPRESSED);
    }
    else if(requestData->type == FF_LOGO_TYPE_SIXEL)
        fd = getCacheFD(requestData, FF_CACHE_FILE_SIXEL);

    if(fd == -1)
        return false;

    ffPrintCharTimes(' ', instance->config.logo.paddingLeft);
    fflush(stdout);

    char buffer[32768];
    ssize_t readBytes;
    while((readBytes = read(fd, buffer, sizeof(buffer))) > 0)
        FF_UNUSED(write(STDOUT_FILENO, buffer, (size_t) readBytes));

    close(fd);

    instance->state.logoWidth = requestData->logoCharacterWidth + instance->config.logo.paddingLeft + instance->config.logo.paddingRight;
    instance->state.logoHeight = requestData->logoCharacterHeight;

    //Go to upper left corner
    fputs("\033[9999999D", stdout);
    printf("\033[%uA", instance->state.logoHeight);
    return true;
}

static bool printCached(FFinstance* instance, FFLogoRequestData* requestData)
{
    if(requestData->type == FF_LOGO_TYPE_CHAFA)
        return printCachedChars(instance, requestData);
    else
        return printCachedPixel(instance, requestData);
}

static bool getCharacterPixelDimensions(FFLogoRequestData* requestData)
{
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

    return requestData->characterPixelWidth > 1.0 && requestData->characterPixelHeight > 1.0;
}

bool ffLogoPrintImageIfExists(FFinstance* instance, FFLogoType type)
{
    //Performance optimisation
    #ifndef FF_HAVE_CHAFA
        if(type == FF_LOGO_TYPE_CHAFA)
            return false;
    #endif

    FFLogoRequestData requestData;
    requestData.type = type;
    requestData.characterPixelWidth = 1;
    requestData.characterPixelHeight = 1;

    if(
        (type != FF_LOGO_TYPE_CHAFA || instance->config.logo.width == 0 || instance->config.logo.height == 0) &&
        !getCharacterPixelDimensions(&requestData)
    )
        return false;

    requestData.logoPixelWidth = simpleCeil((double) instance->config.logo.width * requestData.characterPixelWidth);
    requestData.logoPixelHeight = simpleCeil((double) instance->config.logo.height * requestData.characterPixelHeight);

    ffStrbufInitA(&requestData.cacheDir, PATH_MAX * 2);
    ffStrbufAppend(&requestData.cacheDir, &instance->state.cacheDir);
    ffStrbufAppendS(&requestData.cacheDir, "images");

    ffStrbufEnsureFree(&requestData.cacheDir, PATH_MAX);
    if(realpath(instance->config.logo.source.chars, requestData.cacheDir.chars + requestData.cacheDir.length) == NULL)
    {
        //We can safely return here, because if realpath failed, we surely won't be able to read the file
        ffStrbufDestroy(&requestData.cacheDir);
        return false;
    }
    ffStrbufRecalculateLength(&requestData.cacheDir);
    ffStrbufEnsureEndsWithC(&requestData.cacheDir, '/');

    ffStrbufAppendF(&requestData.cacheDir, "%u", requestData.logoPixelWidth);
    ffStrbufAppendC(&requestData.cacheDir, '*');
    ffStrbufAppendF(&requestData.cacheDir, "%u", requestData.logoPixelHeight);
    ffStrbufAppendC(&requestData.cacheDir, '/');

    if(!instance->config.recache && printCached(instance, &requestData))
    {
        ffStrbufDestroy(&requestData.cacheDir);
        return true;
    }

    FFLogoImageResult result = FF_LOGO_IMAGE_RESULT_INIT_ERROR;

    #ifdef FF_HAVE_IMAGEMAGICK7
        result = ffLogoPrintImageIM7(instance, &requestData);
    #endif

    #ifdef FF_HAVE_IMAGEMAGICK6
        if(result == FF_LOGO_IMAGE_RESULT_INIT_ERROR)
            result = ffLogoPrintImageIM6(instance, &requestData);
    #endif

    ffStrbufDestroy(&requestData.cacheDir);
    return result == FF_LOGO_IMAGE_RESULT_SUCCESS;
}

#else //FF_HAVE_IMAGEMAGICK{6, 7}
bool ffLogoPrintImageIfExists(FFinstance* instance, FFLogoType type)
{
    FF_UNUSED(instance, type);
    return false;
}
#endif //FF_HAVE_IMAGEMAGICK{6, 7}
