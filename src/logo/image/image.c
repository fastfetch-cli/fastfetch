#include "image.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6)

#define FF_KITTY_MAX_CHUNK_SIZE 4096

#define FF_CACHE_FILE_HEIGHT "height"
#define FF_CACHE_FILE_SIXEL "sixel"
#define FF_CACHE_FILE_KITTY_COMPRESSED "kittyc"
#define FF_CACHE_FILE_KITTY_UNCOMPRESSED "kittyu"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef FF_HAVE_ZLIB
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

static void finishPrinting(FFinstance* instance, const FFLogoRequestData* requestData)
{
    instance->state.logoWidth = (uint32_t) (requestData->imagePixelWidth / requestData->characterPixelWidth) + instance->config.logoPaddingLeft + instance->config.logoPaddingRight;
    instance->state.logoHeight = (uint32_t) (requestData->imagePixelHeight / requestData->characterPixelHeight);

    //Seems to be required on all consoles that support sixel. TBH i don't know why.
    if(requestData->type == FF_LOGO_TYPE_SIXEL)
        instance->state.logoHeight += 1;

    fputs("\033[9999999D", stdout);
    printf("\033[%uA", instance->state.logoHeight);
}

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

static void writeCacheFile(FFLogoRequestData* requestData, const char* filename, char* blob, size_t length)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, filename);

    FFstrbuf cachedContent;
    cachedContent.chars = blob;
    cachedContent.length = (uint32_t) length;
    ffWriteFileContent(requestData->cacheDir.chars, &cachedContent);

    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
}


static void printImageKittyBlobChunk(const char** blob, size_t* length)
{
    size_t toWrite = FF_KITTY_MAX_CHUNK_SIZE < *length ? FF_KITTY_MAX_CHUNK_SIZE : *length;
    fputs("\033_Gm=1;", stdout);
    fwrite(*blob, sizeof(**blob), toWrite, stdout);
    fputs("\033\\", stdout);
    *blob += toWrite;
    *length -= toWrite;
}

static bool printImageKitty(const FFinstance* instance, FFLogoRequestData* requestData, const ImageData* imageData)
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

    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);

    const char* currentPos = chars;
    size_t remainingLength = length;

    printf("\033_Ga=T,f=32,s=%u,v=%u%s,m=1;\033\\", requestData->imagePixelWidth, requestData->imagePixelHeight, isCompressed ? ",o=z" : "");
    while(remainingLength > 0)
        printImageKittyBlobChunk(&currentPos, &remainingLength);
    fputs("\033_Gm=0;\033\\", stdout);

    if(isCompressed)
        writeCacheFile(requestData, FF_CACHE_FILE_KITTY_COMPRESSED, chars, length);
    else
        writeCacheFile(requestData, FF_CACHE_FILE_KITTY_UNCOMPRESSED, chars, length);

    free(chars);
    return true;
}

static bool printImageSixel(const FFinstance* instance, FFLogoRequestData* requestData, const ImageData* imageData)
{
    imageData->ffCopyMagickString(imageData->imageInfo->magick, "SIXEL", 6);

    size_t length;
    void* blob = imageData->ffImageToBlob(imageData->imageInfo, imageData->image, &length, imageData->exceptionInfo);
    if(!checkAllocationResult(blob, length))
        return false;

    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);
    fwrite(blob, sizeof(*blob), length, stdout);

    writeCacheFile(requestData, FF_CACHE_FILE_SIXEL, blob, length);

    free(blob);
    return true;
}

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
    imageData.ffCopyMagickString(imageInfoIn->filename, instance->config.logoName.chars, instance->config.logoName.length + 1);

    Image* originalImage = ffReadImage(imageInfoIn, imageData.exceptionInfo);
    ffDestroyImageInfo(imageInfoIn);
    if(originalImage == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    //imagePixelWidth is set in ffLogoPrintImageIfExists
    requestData->imagePixelHeight = (uint32_t) ((requestData->imagePixelWidth / (double) originalImage->columns) * (double) originalImage->rows);
    if(requestData->imagePixelHeight < 1)
    {
        ffDestroyImage(originalImage);
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    imageData.image = (Image*) imData->resizeFunc(originalImage, requestData->imagePixelWidth, requestData->imagePixelHeight, imageData.exceptionInfo);
    ffDestroyImage(originalImage);
    if(imageData.image == NULL)
    {
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    imageData.imageInfo = ffAcquireImageInfo();
    if(imageData.imageInfo == NULL)
    {
        ffDestroyImage(imageData.image);
        ffDestroyExceptionInfo(imageData.exceptionInfo);
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;
    }

    bool printSuccessful;
    if(requestData->type == FF_LOGO_TYPE_KITTY)
        printSuccessful = printImageKitty(instance, requestData, &imageData);
    else
        printSuccessful = printImageSixel(instance, requestData, &imageData);

    ffDestroyImageInfo(imageData.imageInfo);
    ffDestroyImage(imageData.image);
    ffDestroyExceptionInfo(imageData.exceptionInfo);

    if(!printSuccessful)
        return FF_LOGO_IMAGE_RESULT_RUN_ERROR;

    //Write height file
    uint32_t folderPathLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, FF_CACHE_FILE_HEIGHT);

    FFstrbuf content;
    content.chars = (char*) &requestData->imagePixelHeight;
    content.length = sizeof(requestData->imagePixelHeight);
    ffWriteFileContent(requestData->cacheDir.chars, &content);

    ffStrbufSubstrBefore(&requestData->cacheDir, folderPathLength);

    finishPrinting(instance, requestData);
    return FF_LOGO_IMAGE_RESULT_SUCCESS;
}

#endif //FF_HAVE_IMAGEMAGICK{6, 7}

static bool printCachedSixelFile(const FFinstance* instance, const FFLogoRequestData* requestData)
{
    int fd = open(requestData->cacheDir.chars, O_RDONLY);
    if(fd == -1)
        return false;

    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);
    fflush(stdout);

    char buffer[4096];
    ssize_t readBytes;

    while((readBytes = read(fd, buffer, sizeof(buffer))) > 0)
        write(STDOUT_FILENO, buffer, (size_t) readBytes);

    close(fd);
    return true;
}

static bool printCachedSixel(const FFinstance* instance, FFLogoRequestData* requestData)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    bool result;

    ffStrbufAppendS(&requestData->cacheDir, FF_CACHE_FILE_SIXEL);
    result = printCachedSixelFile(instance, requestData);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);

    return result;
}

static bool printCachedKittyFile(const FFinstance* instance, const FFLogoRequestData* requestData, bool isCompressed)
{
    int fd = open(requestData->cacheDir.chars, O_RDONLY);
    if(fd == -1)
        return false;

    char buffer[FF_KITTY_MAX_CHUNK_SIZE];

    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);
    printf("\033_Ga=T,f=32,s=%u,v=%u%s,m=1;\033\\", requestData->imagePixelWidth, requestData->imagePixelHeight, isCompressed ? ",o=z" : "");

    ssize_t readBytes;
    while((readBytes = read(fd, buffer, sizeof(buffer))) > 0)
    {
        fputs("\033_Gm=1;",stdout);
        fwrite(buffer, 1, (size_t) readBytes, stdout);
        fputs("\033\\", stdout);
    }

    fputs("\033_Gm=0;\033\\", stdout);

    close(fd);
    return true;
}

static bool printCachedKitty(const FFinstance* instance, FFLogoRequestData* requestData)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    bool result;

    ffStrbufAppendS(&requestData->cacheDir, FF_CACHE_FILE_KITTY_COMPRESSED);
    result = printCachedKittyFile(instance, requestData, true);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);
    if(result)
        return true;

    ffStrbufAppendS(&requestData->cacheDir, FF_CACHE_FILE_KITTY_UNCOMPRESSED);
    result = printCachedKittyFile(instance, requestData, false);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);

    return result;
}

static bool printCached(const FFinstance* instance, FFLogoRequestData* requestData)
{
    uint32_t cacheDirLength = requestData->cacheDir.length;
    ffStrbufAppendS(&requestData->cacheDir, FF_CACHE_FILE_HEIGHT);

    FFstrbuf content;
    ffStrbufInitA(&content, sizeof(requestData->imagePixelHeight) + 1); // +1 because strbuf is null terminated
    memset(content.chars, 0, sizeof(requestData->imagePixelHeight));

    ffAppendFileContent(requestData->cacheDir.chars, &content);
    memcpy(&requestData->imagePixelHeight, content.chars, sizeof(requestData->imagePixelHeight));

    ffStrbufDestroy(&content);
    ffStrbufSubstrBefore(&requestData->cacheDir, cacheDirLength);

    if(requestData->imagePixelHeight < 1)
        return false;

    if(requestData->type == FF_LOGO_TYPE_SIXEL)
        return printCachedSixel(instance, requestData);
    else
        return printCachedKitty(instance, requestData);
}

static bool getTermInfo(struct winsize* winsize)
{
    //Initialize every member to 0, because it isn't guaranteed that every terminal sets them all
    memset(winsize, 0, sizeof(struct winsize));

    ioctl(STDOUT_FILENO, TIOCGWINSZ, winsize);

    if(winsize->ws_row == 0 || winsize->ws_col == 0)
        ffGetTerminalResponse("\033[18t", 't', "\033[8;%hu;%hut", &winsize->ws_row, &winsize->ws_col);

    if(winsize->ws_ypixel == 0 || winsize->ws_xpixel == 0)
        ffGetTerminalResponse("\033[14t", 't', "\033[4;%hu;%hut", &winsize->ws_ypixel, &winsize->ws_xpixel);

    return
        winsize->ws_row > 0 &&
        winsize->ws_col > 0 &&
        winsize->ws_ypixel > 0 &&
        winsize->ws_xpixel > 0;
}

bool ffLogoPrintImageIfExists(FFinstance* instance, FFLogoType type)
{
    FFLogoRequestData requestData;

    if(!getTermInfo(&requestData.winsize))
        return false;

    requestData.type = type;
    requestData.characterPixelWidth = requestData.winsize.ws_xpixel / (double) requestData.winsize.ws_col;
    requestData.characterPixelHeight = requestData.winsize.ws_ypixel / (double) requestData.winsize.ws_row;
    requestData.imagePixelWidth = (uint32_t) (instance->config.logoWidth * requestData.characterPixelWidth);
    if(requestData.imagePixelWidth < 1)
        return false;

    ffStrbufInitA(&requestData.cacheDir, PATH_MAX * 2);
    ffStrbufAppend(&requestData.cacheDir, &instance->state.cacheDir);
    ffStrbufAppendS(&requestData.cacheDir, "images");

    ffStrbufEnsureFree(&requestData.cacheDir, PATH_MAX);
    if(realpath(instance->config.logoName.chars, requestData.cacheDir.chars + requestData.cacheDir.length) == NULL)
    {
        //We can safely return here, because if realpath failed, we surely won't be able to read the file
        ffStrbufDestroy(&requestData.cacheDir);
        return false;
    }
    ffStrbufRecalculateLength(&requestData.cacheDir);
    if(!ffStrbufEndsWithC(&requestData.cacheDir, '/'))
        ffStrbufAppendC(&requestData.cacheDir, '/');

    ffStrbufAppendF(&requestData.cacheDir, "%u", requestData.imagePixelWidth);
    ffStrbufAppendC(&requestData.cacheDir, '/');

    if(!instance->config.recache && printCached(instance, &requestData))
    {
        finishPrinting(instance, &requestData);
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
