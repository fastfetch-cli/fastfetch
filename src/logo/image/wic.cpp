extern "C" {
#include "image.h"
#include "common/mallocHelper.h"
#include "common/windows/com.h"
#include "common/windows/nt.h"
}

#include <initguid.h>
#include <windows.h>
#include <wincodec.h>

// ---------------------------------------------------------------------------------------------
// Metadata
//
// `QueryInterface(IID_IWICMetadataQueryReader)` returns E_NOINTERFACE on this platform for every
// decoder and every frame, so the interface has to be reached through the method instead. Copying
// the QueryInterface out of Microsoft's own WicAnimatedGif.cpp sample reads no data at all here.
//
// The two readers are not interchangeable: `/logscrdesc/*` and `/appext/*` exist only on the
// decoder's reader, `/imgdesc/*` and `/grctlext/*` only on a frame's.
// ---------------------------------------------------------------------------------------------

static bool readMetadata(IWICMetadataQueryReader* reader, const wchar_t* name, PROPVARIANT* out) {
    if (reader == nullptr) {
        return false;
    }

    PropVariantInit(out);
    if (FAILED(reader->GetMetadataByName(name, out))) {
        PropVariantClear(out);
        return false;
    }

    return true;
}

// Every integer path used here is one of these; the width varies with the item.
static bool readUint(IWICMetadataQueryReader* reader, const wchar_t* name, uint32_t* out) {
    PROPVARIANT value;
    if (!readMetadata(reader, name, &value)) {
        return false;
    }

    bool ok = true;
    switch (value.vt) {
        case VT_UI1: *out = value.bVal; break;
        case VT_UI2: *out = value.uiVal; break;
        case VT_UI4: *out = value.ulVal; break;
        case VT_I2: *out = (uint32_t) value.iVal; break;
        case VT_I4: *out = (uint32_t) value.lVal; break;
        default: ok = false; break;
    }

    PropVariantClear(&value);
    return ok;
}

// The loop count lives in the NETSCAPE2.0 application extension: `[0]=size=3`, `[1]=sub-block
// id=1`, `[2..3]=LE16 count`. `/logscrdesc/LoopCount` and `/logscrdesc/Iterations` do not exist.
// A source without the extension declares no loop count at all, which is -1, not 0.
static bool readLoopCount(IWICMetadataQueryReader* reader, int32_t* out) {
    PROPVARIANT value;
    if (!readMetadata(reader, L"/appext/Data", &value)) {
        return false;
    }

    bool ok = false;
    if (value.vt == (VARTYPE) (VT_VECTOR | VT_UI1) && value.caub.cElems >= 4 &&
        value.caub.pElems[0] == 3 && value.caub.pElems[1] == 1) {
        *out = (int32_t) ((uint32_t) value.caub.pElems[2] | ((uint32_t) value.caub.pElems[3] << 8));
        ok = true;
    }

    PropVariantClear(&value);
    return ok;
}

// ---------------------------------------------------------------------------------------------
// Pixels
// ---------------------------------------------------------------------------------------------

// Scales `source` to `outWidth` x `outHeight` and converts it to straight-alpha RGBA8, which is
// what kitty (f=32) and chafa (CHAFA_PIXEL_RGBA8_UNASSOCIATED) both consume. Ownership of
// `*outPixels` passes to the caller.
static bool convertToRGBA(IWICImagingFactory* factory, IWICBitmapSource* source,
    uint32_t outWidth, uint32_t outHeight, uint8_t** outPixels, const char** error) {
    UINT sourceWidth = 0, sourceHeight = 0;
    if (FAILED(source->GetSize(&sourceWidth, &sourceHeight))) {
        if (error) *error = "failed to query the image dimensions";
        return false;
    }

    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapScaler* scaler = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICFormatConverter* premultiplyConverter = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICFormatConverter* converter = nullptr;

    IWICBitmapSource* sized = source;
    if (outWidth != sourceWidth || outHeight != sourceHeight) {
        // Same size: don't resample. ImageMagick clones the image in this case too,
        // and resampling would only blur it.
        //
        // Otherwise scale premultiplied alpha to avoid transparent RGB values bleeding into
        // the visible edge pixels. The final output is converted back to straight alpha below.
        if (FAILED(factory->CreateFormatConverter(&premultiplyConverter)) ||
            FAILED(premultiplyConverter->Initialize(source, GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom)) ||
            FAILED(factory->CreateBitmapScaler(&scaler)) ||
            FAILED(scaler->Initialize(premultiplyConverter, outWidth, outHeight,
                (WICBitmapInterpolationMode) 0x4 /* WICBitmapInterpolationModeHighQualityCubic */))) {
            if (error) *error = "image scaling failed";
            return false;
        }
        sized = scaler;
    }

    // Normalize to straight-alpha RGBA8: kitty (f=32) and chafa
    // (CHAFA_PIXEL_RGBA8_UNASSOCIATED) both consume exactly this
    if (FAILED(factory->CreateFormatConverter(&converter)) ||
        FAILED(converter->Initialize(sized, GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
        if (error) *error = "pixel format conversion failed";
        return false;
    }

    // `stride` and the length WIC is told are both UINT, while the buffer is a size_t allocation, so
    // computing either in UINT would let a large request wrap and have WIC fill a buffer it believes
    // is smaller than it is. Not reachable today -- the scaler above rejects any dimension above
    // 32767, which caps the stride at 131068 and the buffer at 4294705156, both inside UINT -- but
    // the guard costs nothing and every other backend keeps its buffer size associative. Kept in
    // size_t throughout so the check also covers the allocation overflowing size_t.
    const size_t strideSize = (size_t) outWidth * 4;
    const size_t bufferSize = strideSize * outHeight;
    if (strideSize > UINT_MAX || bufferSize > UINT_MAX) {
        if (error) *error = "the requested image dimensions are too large";
        return false;
    }
    const UINT stride = (UINT) strideSize;

    uint8_t* pixels = (uint8_t*) malloc(bufferSize);
    if (pixels == nullptr) {
        if (error) *error = "out of memory";
        return false;
    }

    // prc == nullptr means the whole image; WIC fills the buffer using the stride we pass in
    if (FAILED(converter->CopyPixels(nullptr, stride, (UINT) bufferSize, pixels))) {
        free(pixels);
        if (error) *error = "pixel copy failed";
        return false;
    }

    *outPixels = pixels;
    return true;
}

// Fills in the missing dimension, keeping the source aspect ratio (same as the IM path)
static bool resolveTargetSize(FFLogoRequestData* requestData, uint32_t sourceWidth, uint32_t sourceHeight, const char** error) {
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
    return true;
}

// The source path is UTF-8, WIC only accepts UTF-16
static bool createDecoder(IWICImagingFactory* factory, IWICBitmapDecoder** out, const char** error) {
    wchar_t widePath[MAX_PATH + 1];
    if (!NT_SUCCESS(RtlUTF8ToUnicodeN(widePath, (ULONG) sizeof(widePath), nullptr,
            instance.config.logo.source.chars, (ULONG) instance.config.logo.source.length + 1))) {
        if (error) *error = "failed to convert the image path to UTF-16";
        return false;
    }

    if (FAILED(factory->CreateDecoderFromFilename(widePath, nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnDemand, out))) {
        if (error) *error = "unsupported or unreadable image format";
        return false;
    }

    return true;
}

bool ffImageCreateWIC(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error) {
    const char* comError = ffInitCom();
    if (comError) {
        if (error) *error = comError;
        return false;
    }

    FF_AUTO_RELEASE_COM_OBJECT IWICImagingFactory* factory = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapDecoder* decoder = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapFrameDecode* frame = nullptr;

    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory, (void**) &factory))) {
        if (error) *error = "WIC imaging factory is unavailable";
        return false;
    }

    if (!createDecoder(factory, &decoder, error)) {
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

    if (!resolveTargetSize(requestData, sourceWidth, sourceHeight, error)) {
        return false;
    }

    uint8_t* pixels = nullptr;
    if (!convertToRGBA(factory, frame, requestData->logoPixelWidth, requestData->logoPixelHeight, &pixels, error)) {
        return false;
    }

    out->data = pixels;
    out->width = requestData->logoPixelWidth;
    out->height = requestData->logoPixelHeight;
    return true;
}

// ---------------------------------------------------------------------------------------------
// Animation
//
// WIC does not compose GIF frames: it hands out the raw sub-frame plus the metadata describing how
// to place it, so the canvas and the disposal handling are ours. The loop below follows the GIF
// model, which is also what Microsoft's own sample implements:
//
//   1. apply the *previous* frame's disposal to the canvas (0/1 keep, 2 clear its rectangle,
//      3 restore the snapshot taken before it was drawn);
//   2. if the current frame's disposal is 3, snapshot the canvas as it stands;
//   3. draw the current frame's pixels at its own (left, top).
//
// The canvas is scaled once per frame on the way out, never per sub-frame: scaling in sub-frame
// coordinates would introduce edge errors.
// ---------------------------------------------------------------------------------------------

struct FFWicAnimation {
    IWICImagingFactory* factory;
    IWICBitmapDecoder* decoder;
    IWICMetadataQueryReader* decoderReader; // container level; nullptr for formats without one

    uint32_t canvasWidth;
    uint32_t canvasHeight;
    uint8_t* canvas;   // RGBA8, the composed canvas
    uint8_t* snapshot; // the canvas as it was before a disposal=3 frame was drawn

    uint32_t outputWidth;
    uint32_t outputHeight;

    uint32_t previousLeft;
    uint32_t previousTop;
    uint32_t previousWidth;
    uint32_t previousHeight;
    uint32_t previousDisposal;
    uint32_t previousRawDelay; // centiseconds, as the source declares it
    bool hasPrevious;

    uint32_t nextIndex; // frames are handed out in order; frame i depends on 0..i-1
    int32_t minGap;     // 0, or 10 when every frame's raw delay is <= 0
};

static void clearRect(FFWicAnimation* animation, uint32_t left, uint32_t top, uint32_t width, uint32_t height) {
    const size_t rowBytes = (size_t) width * 4;
    for (uint32_t y = 0; y < height; ++y) {
        memset(animation->canvas + (((size_t) (top + y) * animation->canvasWidth) + left) * 4, 0, rowBytes);
    }
}

// Reads frame `index`'s metadata, applies the previous frame's disposal, snapshots if this frame
// asks for it, and draws the frame onto the canvas. Reports the frame's gap in milliseconds.
static bool composeFrame(FFWicAnimation* animation, uint32_t index, int32_t* delayMs, const char** error) {
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapFrameDecode* frame = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICMetadataQueryReader* reader = nullptr;

    if (FAILED(animation->decoder->GetFrame(index, &frame)) ||
        FAILED(frame->GetMetadataQueryReader(&reader))) {
        *error = "failed to read the frame metadata";
        return false;
    }

    uint32_t left = 0, top = 0, width = 0, height = 0, disposal = 0, rawDelay = 0;
    if (!readUint(reader, L"/imgdesc/Left", &left) || !readUint(reader, L"/imgdesc/Top", &top) ||
        !readUint(reader, L"/imgdesc/Width", &width) || !readUint(reader, L"/imgdesc/Height", &height)) {
        // Not a GIF: there is no placement metadata at all and the frame is the whole canvas.
        UINT frameWidth = 0, frameHeight = 0;
        if (FAILED(frame->GetSize(&frameWidth, &frameHeight))) {
            *error = "failed to read the frame rectangle";
            return false;
        }
        left = 0;
        top = 0;
        width = frameWidth;
        height = frameHeight;
    }

    if (width == 0 || height == 0 ||
        left + width > animation->canvasWidth || top + height > animation->canvasHeight) {
        *error = "the frame rectangle does not fit the canvas";
        return false;
    }

    // Both are absent on anything that is not a GIF, in which case the frame covers the canvas and
    // there is nothing to animate.
    readUint(reader, L"/grctlext/Disposal", &disposal);
    readUint(reader, L"/grctlext/Delay", &rawDelay);

    // kitty's mapping, which the design settled on: the raw delay is in centiseconds, the 100 ms
    // floor exists only for sources whose frames are *all* zero, and whatever is left at <= 0 means
    // "gapless" rather than 100 ms. Not the "<90 -> 90" floor of the WIC sample, which its own
    // comment admits destroys legitimate zero-gap frames.
    const int32_t gap = (int32_t) (rawDelay > (uint32_t) animation->minGap ? rawDelay : (uint32_t) animation->minGap) * 10;
    *delayMs = gap > 0 ? gap : -1;

    if (animation->hasPrevious) {
        if (animation->previousDisposal == 2 && animation->previousRawDelay != 0) {
            // No colour table is reachable through WIC - only BackgroundColorIndex, never a palette
            // - so "restore to background" can only mean transparent.
            //
            // A previous frame with a zero delay keeps its pixels instead: it was never displayed
            // for any length of time, so erasing it would only flicker. This is the rule the
            // reference compositor was validated against.
            clearRect(animation, animation->previousLeft, animation->previousTop,
                animation->previousWidth, animation->previousHeight);
        } else if (animation->previousDisposal == 3) {
            memcpy(animation->canvas, animation->snapshot,
                (size_t) animation->canvasWidth * animation->canvasHeight * 4);
        }
    }

    if (disposal == 3) {
        const size_t canvasSize = (size_t) animation->canvasWidth * animation->canvasHeight * 4;
        if (animation->snapshot == nullptr) {
            animation->snapshot = (uint8_t*) malloc(canvasSize);
            if (animation->snapshot == nullptr) {
                *error = "out of memory";
                return false;
            }
        }
        memcpy(animation->snapshot, animation->canvas, canvasSize);
    }

    // Decode the sub-frame at its own size; the canvas is scaled on the way out instead.
    FF_AUTO_FREE uint8_t* pixels = nullptr;
    if (!convertToRGBA(animation->factory, frame, width, height, &pixels, error)) {
        return false;
    }

    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t* source = pixels + (size_t) y * width * 4;
        uint8_t* target = animation->canvas + (((size_t) (top + y) * animation->canvasWidth) + left) * 4;

        for (uint32_t x = 0; x < width; ++x, source += 4, target += 4) {
            const uint8_t alpha = source[3];
            if (alpha == 0) {
                // GIF transparency is one bit, so "keep" is exact and needs no arithmetic at all.
                continue;
            }
            if (alpha == 255) {
                memcpy(target, source, 4);
                continue;
            }

            // Not a GIF. Composite over, so a source with real alpha still lands sanely; for the
            // one-bit alpha above this branch is never taken.
            for (uint32_t channel = 0; channel < 3; ++channel) {
                target[channel] = (uint8_t) (((uint32_t) source[channel] * alpha +
                    (uint32_t) target[channel] * (255 - alpha) + 127) / 255);
            }
            target[3] = (uint8_t) (alpha + (uint32_t) target[3] * (255 - alpha) / 255);
        }
    }

    animation->previousLeft = left;
    animation->previousTop = top;
    animation->previousWidth = width;
    animation->previousHeight = height;
    animation->previousDisposal = disposal;
    animation->previousRawDelay = rawDelay;
    animation->hasPrevious = true;
    return true;
}

// Scales the composed canvas to the output size and hands it over.
static bool outputCanvas(FFWicAnimation* animation, FFImageFrame* out, const char** error) {
    const UINT stride = animation->canvasWidth * 4;

    FF_AUTO_RELEASE_COM_OBJECT IWICBitmap* bitmap = nullptr;
    if (FAILED(animation->factory->CreateBitmapFromMemory(animation->canvasWidth, animation->canvasHeight,
            GUID_WICPixelFormat32bppRGBA, stride, stride * animation->canvasHeight, animation->canvas, &bitmap))) {
        *error = "failed to wrap the composed canvas";
        return false;
    }

    return convertToRGBA(animation->factory, bitmap, animation->outputWidth, animation->outputHeight,
        &out->data, error);
}

static bool wicGetFrame(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error) {
    FFWicAnimation* session = (FFWicAnimation*) ffImageAnimationGetImpl(animation);

    // The composition of frame i depends on frames 0..i-1, so they are walked in order and the
    // intermediate canvases are simply discarded.
    if (index < session->nextIndex) {
        *error = "animation frames must be taken in order";
        return false;
    }

    int32_t delayMs = -1;
    for (uint32_t i = session->nextIndex; i <= index; ++i) {
        if (!composeFrame(session, i, &delayMs, error)) {
            return false;
        }
        session->nextIndex = i + 1;
    }

    if (!outputCanvas(session, out, error)) {
        return false;
    }

    out->delayMs = delayMs;
    return true;
}

static void wicDestroyAnimation(FFImageAnimation* animation) {
    FFWicAnimation* session = (FFWicAnimation*) ffImageAnimationGetImpl(animation);
    if (session == nullptr) {
        return;
    }

    free(session->canvas);
    free(session->snapshot);
    if (session->decoderReader) session->decoderReader->Release();
    if (session->decoder) session->decoder->Release();
    if (session->factory) session->factory->Release();
    free(session);
}

// Reads one frame's raw delay, for the all-zero check that decides the 100 ms floor.
static bool readFrameDelay(IWICBitmapDecoder* decoder, uint32_t index, uint32_t* out) {
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapFrameDecode* frame = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICMetadataQueryReader* reader = nullptr;

    if (FAILED(decoder->GetFrame(index, &frame)) || FAILED(frame->GetMetadataQueryReader(&reader))) {
        return false;
    }

    return readUint(reader, L"/grctlext/Delay", out);
}

bool ffImageAnimationOpenWIC(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error) {
    const char* comError = ffInitCom();
    if (comError) {
        if (error) *error = comError;
        return false;
    }

    FF_AUTO_RELEASE_COM_OBJECT IWICImagingFactory* factory = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapDecoder* decoder = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICBitmapFrameDecode* firstFrame = nullptr;
    FF_AUTO_RELEASE_COM_OBJECT IWICMetadataQueryReader* decoderReader = nullptr;

    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory, (void**) &factory))) {
        if (error) *error = "WIC imaging factory is unavailable";
        return false;
    }

    if (!createDecoder(factory, &decoder, error)) {
        return false;
    }

    // Cheap: WICDecodeMetadataCacheOnDemand reads only the header, so the frame count - which a
    // negative --logo-animation-frame index is resolved against - needs no decoding.
    UINT frameCount = 0;
    if (FAILED(decoder->GetFrameCount(&frameCount)) || frameCount == 0) {
        if (error) *error = "the image source has no frames";
        return false;
    }

    if (FAILED(decoder->GetFrame(0, &firstFrame))) {
        if (error) *error = "failed to get the first frame";
        return false;
    }

    // Absent for every format without a query reader (PNG, JPEG, ...), which is not an error: the
    // frame is the canvas then.
    decoder->GetMetadataQueryReader(&decoderReader);

    // GetSize() on a frame gives the *sub-frame* rectangle, never the canvas, so the canvas has to
    // come from the logical screen descriptor.
    uint32_t canvasWidth = 0, canvasHeight = 0;
    if (!readUint(decoderReader, L"/logscrdesc/Width", &canvasWidth) ||
        !readUint(decoderReader, L"/logscrdesc/Height", &canvasHeight)) {
        UINT width = 0, height = 0;
        if (FAILED(firstFrame->GetSize(&width, &height)) || width == 0 || height == 0) {
            if (error) *error = "invalid image dimensions";
            return false;
        }
        canvasWidth = width;
        canvasHeight = height;
    }

    if (!resolveTargetSize(requestData, canvasWidth, canvasHeight, error)) {
        return false;
    }

    // 0 means "loop forever", -1 that the source declares no loop count at all.
    int32_t loopCount = -1;
    readLoopCount(decoderReader, &loopCount);

    // kitty's rule: the 100 ms floor applies only when every frame's raw delay is <= 0. One frame
    // with a delay settles it, so the normal case stops after the first read.
    int32_t minGap = 10;
    for (UINT i = 0; i < frameCount; ++i) {
        uint32_t rawDelay = 0;
        if (!readFrameDelay(decoder, i, &rawDelay) || rawDelay > 0) {
            minGap = 0;
            break;
        }
    }

    FFWicAnimation* session = (FFWicAnimation*) calloc(1, sizeof(*session));
    if (session == nullptr) {
        if (error) *error = "out of memory";
        return false;
    }

    session->canvas = (uint8_t*) calloc((size_t) canvasWidth * canvasHeight * 4, 1);
    if (session->canvas == nullptr) {
        free(session);
        if (error) *error = "out of memory";
        return false;
    }

    session->factory = factory;
    session->decoder = decoder;
    session->decoderReader = decoderReader;
    session->canvasWidth = canvasWidth;
    session->canvasHeight = canvasHeight;
    session->outputWidth = requestData->logoPixelWidth;
    session->outputHeight = requestData->logoPixelHeight;
    session->minGap = minGap;
    session->nextIndex = 0;

    FFImageAnimation* animation = ffImageAnimationCreate(frameCount, loopCount, session, wicGetFrame, wicDestroyAnimation);
    if (animation == nullptr) {
        free(session->canvas);
        free(session);
        if (error) *error = "out of memory";
        return false;
    }

    // The session owns these from here on, so the cleanup guards must not release them.
    factory = nullptr;
    decoder = nullptr;
    decoderReader = nullptr;
    *out = animation;
    return true;
}
