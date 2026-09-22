#pragma once

#include "../logo.h"

#if defined(FF_HAVE_IMAGEMAGICK7) || defined(FF_HAVE_IMAGEMAGICK6) || defined(_WIN32) || defined(__APPLE__) || defined(__ANDROID__) || defined(FF_HAVE_SIXEL)

typedef enum FFLogoImageResult: uint8_t {
    FF_LOGO_IMAGE_RESULT_SUCCESS,    // Logo printed
    FF_LOGO_IMAGE_RESULT_INIT_ERROR, // Failed to load library, try again with next IM version
    FF_LOGO_IMAGE_RESULT_RUN_ERROR   // Failed to load / convert image, cancel whole sixel code
} FFLogoImageResult;

typedef struct FFLogoRequestData {
    FFLogoType type;
    FFstrbuf cacheDir;

    double characterPixelWidth;
    double characterPixelHeight;

    uint32_t logoPixelWidth;
    uint32_t logoPixelHeight;

    uint32_t logoCharacterHeight;
    uint32_t logoCharacterWidth;
} FFLogoRequestData;

// Decoded and resized image: RGBA8, straight (unassociated) alpha, no row padding
typedef struct FFImageBuffer {
    uint8_t* data;
    uint32_t width;
    uint32_t height;
} FFImageBuffer;

// Backend contract. Both functions update requestData->logoPixelWidth / logoPixelHeight
// with the real dimensions of the produced image, so the caller can derive the character
// dimensions afterwards.

// Decode the image source and resize it to the requested pixel size
bool ffImageCreate(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
void ffImageDestroy(FFImageBuffer* buffer);

// Encode the image source as a sixel byte stream (including the DCS envelope).
// The encoder belongs to the backend: ImageMagick's SIXEL coder off Windows,
// the embedded libsixel on Windows.
bool ffImageSixelEncode(FFLogoRequestData* requestData, FFstrbuf* out, const char** error);

// The same, for pixels the caller already has. The selected-frame path needs this: it composes the
// frame itself, and ffImageSixelEncode would go back to the source and encode a different one.
bool ffImageSixelEncodeBuffer(const FFImageBuffer* buffer, FFstrbuf* out, const char** error);

// One frame of an animation: a fully composed canvas, RGBA8, straight (unassociated) alpha,
// no row padding, exactly requestData->logoPixelWidth x logoPixelHeight.
typedef struct FFImageFrame {
    uint8_t* data;
    int32_t delayMs; // >0: wait this many milliseconds; -1: gapless; 0: unspecified
} FFImageFrame;

void ffImageFrameDestroy(FFImageFrame* frame);

// Animation session. Opaque outside src/logo/image/.
//
// Contract: the backend hands out *composed* full-canvas frames through a sequential iterator.
// Composition belongs to the backend (ImageIO already does it, WIC does not), never to the
// output layer. Taking a frame transfers its ownership to the caller, so the caller never holds
// more than one frame. Frames must be taken in non-decreasing index order: the composition of
// frame i depends on frames 0..i-1.
//
// That says nothing about what the backend holds, though. ImageMagick's CoalesceImages composes
// every frame up front and keeps the list, so its peak is the whole animation rather than one
// frame -- 60 frames of 500x500 measured 532 MiB. Bounding that would mean writing the compositor
// by hand, which is the cost reusing the library's composition exists to avoid, so it is accepted
// and recorded instead. See doc/kitty-animation.md 5.4 and 11-E.
typedef struct FFImageAnimation FFImageAnimation;

// Open a session. Like ffImageCreate, it must fill in requestData->logoPixelWidth /
// logoPixelHeight, because every frame has those dimensions. frameCount / loopCount must be
// known without decoding a frame, because a negative --logo-animation-frame index is resolved
// before the first frame is taken.
bool ffImageAnimationOpen(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error);
uint32_t ffImageAnimationFrameCount(const FFImageAnimation* animation);
int32_t ffImageAnimationLoopCount(const FFImageAnimation* animation); // 0 = infinite; -1 = the source declares none
bool ffImageAnimationGetFrame(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error);
void ffImageAnimationClose(FFImageAnimation* animation);

// Called by the backends once they have a session ready. `destroy` must release the session and
// its backend state; `impl` is handed back to them untouched.
FFImageAnimation* ffImageAnimationCreate(uint32_t frameCount, int32_t loopCount, void* impl,
    bool (*getFrame)(FFImageAnimation* animation, uint32_t index, FFImageFrame* out, const char** error),
    void (*destroy)(FFImageAnimation* animation));

// The session is opaque to the backends; this is how the callbacks they supply get their own
// state back out of it. Never null for a session that ffImageAnimationCreate built.
void* ffImageAnimationGetImpl(const FFImageAnimation* animation);

#endif

#ifdef FF_HAVE_IMAGEMAGICK7
bool ffImageCreateIM7(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageSixelEncodeIM7(FFLogoRequestData* requestData, FFstrbuf* out, const char** error);
bool ffImageSixelEncodeBufferIM7(const FFImageBuffer* buffer, FFstrbuf* out, const char** error);
// ImageMagick 7 is the only ImageMagick version that gets animation: 6 would need its own port
// (PixelPacket / opacity instead of PixelInfo / alpha) and there is no way to test it here, so it
// is left unimplemented rather than written blind. See doc/kitty-animation.md §11-E.
bool ffImageAnimationOpenIM7(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error);
#endif

#ifdef FF_HAVE_IMAGEMAGICK6
bool ffImageCreateIM6(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageSixelEncodeIM6(FFLogoRequestData* requestData, FFstrbuf* out, const char** error);
#endif

#ifdef _WIN32
bool ffImageCreateWIC(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageAnimationOpenWIC(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error);
#endif

#ifdef __APPLE__
bool ffImageCreateImageIO(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageAnimationOpenImageIO(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error);
#endif

#ifdef __ANDROID__
bool ffImageCreateAID(FFLogoRequestData* requestData, FFImageBuffer* out, const char** error);
bool ffImageAnimationOpenAID(FFLogoRequestData* requestData, FFImageAnimation** out, const char** error);
#endif

#ifdef FF_HAVE_SIXEL
bool ffSixelEncode(const FFImageBuffer* buffer, FFstrbuf* result, const char** error);
#endif
