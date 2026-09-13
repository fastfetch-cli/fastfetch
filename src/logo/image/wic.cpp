extern "C" {
#include "image.h"
#include "common/mallocHelper.h"
#include "common/windows/com.h"
#include "common/windows/nt.h"
}

#include <initguid.h>
#include <windows.h>
#include <wincodec.h>

bool ffImageCreateWIC(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error) {
    const char* comError = ffInitCom();
    if (comError) {
        if (error) *error = comError;
        return false;
    }

    FF_AUTO_RELEASE_COM_OBJECT IWICImagingFactory* factory = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapDecoder* decoder = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapFrameDecode* frame = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapScaler* scaler = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICFormatConverter* premultiplyConverter = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICFormatConverter* converter = nullptr;

    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory, (void**) &factory))) {
        if (error) *error = "WIC imaging factory is unavailable";
        return false;
    }

    // The source path is UTF-8, WIC only accepts UTF-16
    wchar_t widePath[MAX_PATH + 1];
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(widePath, (ULONG) sizeof(widePath), nullptr,
            instance.config.logo.source.chars, (ULONG) instance.config.logo.source.length + 1))) {
        if (error) *error = "failed to convert the image path to UTF-16";
        return false;
    }

    if (FAILED(factory->CreateDecoderFromFilename(widePath, nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnDemand, &decoder))) {
        if (error) *error = "unsupported or unreadable image format";
        return false;
    }

    // Only the first frame, matching ImageMagick's ReadImage (neither handles GIF animation)
    if (FAILED(decoder->GetFrame(0, &frame))) {
        if (error) *error = "failed to get the first frame";
        return false;
    }

    UINT sourceWidth = 0, sourceHeight = 0;
    frame->GetSize(&sourceWidth, &sourceHeight);
    if (sourceWidth == 0 || sourceHeight == 0) {
        if (error) *error = "invalid image dimensions";
        return false;
    }

    // Fill in the missing dimension, keeping the source aspect ratio (same as the IM path)
    uint32_t width = requestData->logoPixelWidth;
    uint32_t height = requestData->logoPixelHeight;
    if (width == 0 && height == 0) {
        width = sourceWidth;
        height = sourceHeight;
    } else if (width == 0) {
        width = (uint32_t) ((double) sourceWidth / (double) sourceHeight * height);
    } else if (height == 0) {
        height = (uint32_t) ((double) sourceHeight / (double) sourceWidth * width);
    }

    if (width == 0 || height == 0) {
        if (error) *error = "invalid target dimensions";
        return false;
    }

    requestData->logoPixelWidth = width;
    requestData->logoPixelHeight = height;

    IWICBitmapSource* source = nullptr;
    if (width == sourceWidth && height == sourceHeight) {
        // Same size: don't resample. ImageMagick clones the image in this case too,
        // and resampling would only blur it
        source = frame;
    } else {
        // Scale premultiplied alpha to avoid transparent RGB values bleeding into
        // the visible edge pixels. The final output is converted back to straight
        // alpha below, matching FFImageBuffer's RGBA8 contract.
        if (FAILED(factory->CreateFormatConverter(&premultiplyConverter)) ||
            FAILED(premultiplyConverter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)) ||
            FAILED(factory->CreateBitmapScaler(&scaler)) ||
            FAILED(scaler->Initialize(premultiplyConverter, width, height,
                (WICBitmapInterpolationMode) 0x4 /* WICBitmapInterpolationModeHighQualityCubic */))) {
            if (error) *error = "image scaling failed";
            return false;
        }
        source = scaler;
    }

    // Normalize to straight-alpha RGBA8: kitty (f=32) and chafa
    // (CHAFA_PIXEL_RGBA8_UNASSOCIATED) both consume exactly this
    if (FAILED(factory->CreateFormatConverter(&converter)) ||
        FAILED(converter->Initialize(source, GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
        if (error) *error = "pixel format conversion failed";
        return false;
    }

    UINT stride = width * 4;
    FF_AUTO_FREE uint8_t* pixels = (uint8_t*) malloc((size_t) stride * height);
    if (pixels == nullptr) {
        if (error) *error = "out of memory";
        return false;
    }

    // prc == nullptr means the whole image; WIC fills the buffer using the stride we pass in
    if (FAILED(converter->CopyPixels(nullptr, stride, stride * height, pixels))) {
        if (error) *error = "pixel copy failed";
        return false;
    }

    out->data = pixels;
    out->width = width;
    out->height = height;
    pixels = nullptr; // Ownership is transferred to `out`
    return true;
}
