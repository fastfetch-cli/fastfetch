#include "media.h"
#include "common/android/api.h"
#include "common/android/binder.h"
#include "common/debug.h"
#include "common/io.h"
#include "common/mallocHelper.h"
#include "detection/displayserver/displayserver.h"

#include <android/bitmap.h>
#include <android/data_space.h>

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

// Android's "now playing" belongs to the media_session system service. There is no C API for it --
// the NDK exposes neither the service nor a Java bridge -- and `/system/bin/dumpsys media_session`
// costs a fork/exec (measured at ~70 ms on this device) while answering with less. This file speaks
// to the service over /dev/binder through common/android/binder.h instead, the same way the wallpaper
// and wifi modules do, and pays ~0.1 ms per call.
//
// The gate is a single method. `ISessionManager.getSessions(ComponentName, int)` is the only entry
// point that checks a permission: MediaSessionService.enforceMediaPermissions() admits the system UI,
// a holder of android.permission.MEDIA_CONTENT_CONTROL and an enabled notification listener, and
// refuses everything else -- measured on an Android 16 device, an app UID opens binder, finds the
// service and reaches onTransact, and is then answered with a SecurityException carrying the server's
// own stack (`enforceMediaPermissions` <- `verifySessionsRequest` <- `getSessions`), while the shell
// UID gets the session list from the same transaction code. Nothing in this file tests the uid: the
// service is asked and its answer is believed, so a device whose policy is looser, or one where a
// notification listener is registered, works without a change here.
//
// Only one of the two routes is ever asked, and which one it is depends on the display server. The
// binder route is Android's, and the session it reads is the one the phone is playing only while
// SurfaceFlinger is the display server; a Termux:X11 or Wayland session runs a real compositor on top
// of the device, and the players running there are not Android media sessions at all -- so that case
// is handed to the MPRIS implementation in media_linux.c, which is compiled alongside this file for
// exactly this reason.
//
// Under SurfaceFlinger the Android session is still not the whole story. When it has nothing to say --
// an app UID, or a phone with nothing registered -- the module falls back to that same MPRIS
// implementation, over the session bus. That bus is open to every process in the session, so the
// fallback is the only route an app UID has and a real one for the shell UID too.
//
// What the service answers with is worth the trouble, and more than the dump carries. The reply to
// getSessions is a list of session tokens, and each token *is* an ISessionController binder -- there
// is no getController() hop -- and every argument-less getter on that interface was measured to
// answer without a permission check, so the one gate above is the whole story. getMetadata yields the
// title, the artist, the album and, unlike the dump, the track length; getPlaybackState yields the
// state, the position and the playback speed; and the artwork comes across as a Bitmap whose pixels
// arrive as an ashmem descriptor, which is what makes a real cover possible here.
//
// Two traps, both measured:
//
//   * A request must ask with TF_ACCEPT_FDS, or a reply that carries a descriptor is refused with
//     BR_FAILED_REPLY -- which reads exactly like a service that is not running. The failure is
//     asymmetric and that is what makes it expensive: only metadata carrying artwork has a
//     descriptor, so a probe tried against one player passes while the same probe fails against the
//     next one.
//   * A Bundle value is written by Parcel.writeValue, and for a Parcelable that means an int32 byte
//     length in front of the class name (Parcel.isLengthPrefixed). Reading the class name without
//     accounting for it, or reading a Bitmap as if it were a string, desynchronises every field after
//     it. The length is also the one thing that makes an unknown Parcelable skippable.
//
#define FF_MEDIA_ANDROID_SERVICE "media_session"

#define FF_MEDIA_ANDROID_SESSIONS_DESCRIPTOR "android.media.session.ISessionManager"
#define FF_MEDIA_ANDROID_CONTROLLER_DESCRIPTOR "android.media.session.ISessionController"

// Transaction codes as this device's own framework.jar declares them, read out of its dex rather than
// transcribed from AOSP -- a vendor ROM adds and drops methods as it likes, and a wrong code reaches
// a different method or none at all. getSessions is the second declared method of ISessionManager;
// the numbers on ISessionController are sparse because most of that interface is setters this module
// never calls.
#define FF_MEDIA_ANDROID_GET_SESSIONS 2u
#define FF_MEDIA_ANDROID_GET_PACKAGE_NAME 5u
#define FF_MEDIA_ANDROID_GET_METADATA 32u
#define FF_MEDIA_ANDROID_GET_PLAYBACK_STATE 33u

// The largest reply any of these getters produced is a metadata Bundle at ~1.6 KB, and the artwork
// never lands here -- it arrives as a descriptor. 16 KB leaves room for a Bundle of unusually long
// strings; a reply that did not fit would be reported rather than misread.
#define FF_MEDIA_ANDROID_REPLY_SIZE (16 * 1024)

// An interface token plus, for getSessions, two int32s.
#define FF_MEDIA_ANDROID_REQUEST_SIZE 128

// Lossless WebP reads quality as effort rather than as fidelity: 0 is "compress quickly and accept a
// larger file", 100 is "spend the time". Measured on a 363x363 cover, the whole range is worth 1.2%
// of the file for 2.4 times the time, so there is nothing at the top to buy.
#define FF_MEDIA_ANDROID_COVER_QUALITY 0

// One reply buffer for every call in this file: the calls are sequential, and the metadata of the
// winning session is the last one to need it.
static uint8_t s_reply[FF_MEDIA_ANDROID_REPLY_SIZE];

// ---------------------------------------------------------------------------------------------
// Reading a Parcel
// ---------------------------------------------------------------------------------------------

// android.os.Parcel value types, from Parcel.VAL_*. A MediaMetadata Bundle only ever holds the four
// below; anything else stops the walk there rather than being guessed at, and the caller is told.
typedef enum FFMediaAndroidValueType : int32_t {
    FF_MEDIA_ANDROID_VAL_NULL = -1,
    FF_MEDIA_ANDROID_VAL_STRING = 0,
    FF_MEDIA_ANDROID_VAL_INTEGER = 1,
    FF_MEDIA_ANDROID_VAL_PARCELABLE = 4,
    FF_MEDIA_ANDROID_VAL_LONG = 6,
    FF_MEDIA_ANDROID_VAL_CHAR_SEQUENCE = 10,
} FFMediaAndroidValueType;

// Bundle.writeToParcel writes this in front of the entry count. It is the byte pattern 'BNDL', and
// checking it is what tells a bundle apart from the length word of something else.
#define FF_MEDIA_ANDROID_BUNDLE_MAGIC 0x4C444E42u

#define FF_MEDIA_ANDROID_CLASS_BITMAP "android.graphics.Bitmap"

#define FF_MEDIA_ANDROID_KEY_PREFIX "android.media.metadata."
#define FF_MEDIA_ANDROID_KEY_TITLE FF_MEDIA_ANDROID_KEY_PREFIX "TITLE"
#define FF_MEDIA_ANDROID_KEY_DISPLAY_TITLE FF_MEDIA_ANDROID_KEY_PREFIX "DISPLAY_TITLE"
#define FF_MEDIA_ANDROID_KEY_ARTIST FF_MEDIA_ANDROID_KEY_PREFIX "ARTIST"
#define FF_MEDIA_ANDROID_KEY_DISPLAY_SUBTITLE FF_MEDIA_ANDROID_KEY_PREFIX "DISPLAY_SUBTITLE"
#define FF_MEDIA_ANDROID_KEY_ALBUM FF_MEDIA_ANDROID_KEY_PREFIX "ALBUM"
#define FF_MEDIA_ANDROID_KEY_DURATION FF_MEDIA_ANDROID_KEY_PREFIX "DURATION"
#define FF_MEDIA_ANDROID_KEY_MEDIA_URI FF_MEDIA_ANDROID_KEY_PREFIX "MEDIA_URI"
#define FF_MEDIA_ANDROID_KEY_ART FF_MEDIA_ANDROID_KEY_PREFIX "ART"
#define FF_MEDIA_ANDROID_KEY_ALBUM_ART FF_MEDIA_ANDROID_KEY_PREFIX "ALBUM_ART"
#define FF_MEDIA_ANDROID_KEY_DISPLAY_ICON FF_MEDIA_ANDROID_KEY_PREFIX "DISPLAY_ICON"

// MediaMetadata carries the artwork under three keys, and Android documents them as: ART is "the
// artwork for the media", ALBUM_ART "the artwork for the album it is from", DISPLAY_ICON "a small
// icon representing the media". They are usually the same image; ART is the one the system UI puts on
// the lock screen, so it wins when more than one is present.
typedef enum FFMediaAndroidCoverPriority : uint32_t {
    FF_MEDIA_ANDROID_COVER_DISPLAY_ICON = 1,
    FF_MEDIA_ANDROID_COVER_ALBUM_ART = 2,
    FF_MEDIA_ANDROID_COVER_ART = 3,
} FFMediaAndroidCoverPriority;

typedef struct FFMediaAndroidReader {
    const uint8_t* data;
    size_t size;
    size_t position;
} FFMediaAndroidReader;

[[gnu::nonnull(1, 2)]] static bool androidReadI32(FFMediaAndroidReader* reader, int32_t* value) {
    if (reader->position + sizeof(int32_t) > reader->size) {
        return false;
    }
    memcpy(value, reader->data + reader->position, sizeof(int32_t));
    reader->position += sizeof(int32_t);
    return true;
}

[[gnu::nonnull(1, 2)]] static bool androidReadI64(FFMediaAndroidReader* reader, int64_t* value) {
    if (reader->position + sizeof(int64_t) > reader->size) {
        return false;
    }
    memcpy(value, reader->data + reader->position, sizeof(int64_t));
    reader->position += sizeof(int64_t);
    return true;
}

// AIDL string16: an int32 code-unit count, that many UTF-16LE code units, a UTF-16 NUL terminator,
// and padding to four bytes. The terminator is walked over even though it is not part of the string,
// and so is the padding -- getting either wrong shifts every following field. A count of -1 is how
// the same field spells null.
[[gnu::nonnull(1, 2)]] static bool androidReadString16(FFMediaAndroidReader* reader, FFstrbuf* result) {
    int32_t length = 0;
    if (!androidReadI32(reader, &length)) {
        return false;
    }
    if (length < 0) {
        ffStrbufClear(result);
        return true;
    }

    const size_t units = (size_t) length * 2;
    const size_t padded = (units + 2 + 3) & ~(size_t) 3; // the code units, the two byte NUL, then padding
    if (reader->position + padded > reader->size) {
        return false;
    }

    const uint8_t* codeUnits = reader->data + reader->position;
    reader->position += padded;

    ffStrbufClear(result);
    for (size_t i = 0; i < (size_t) length; i++) {
        uint32_t codepoint = (uint32_t) codeUnits[i * 2] | ((uint32_t) codeUnits[i * 2 + 1] << 8);
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF && i + 1 < (size_t) length) {
            const uint32_t low = (uint32_t) codeUnits[(i + 1) * 2] | ((uint32_t) codeUnits[(i + 1) * 2 + 1] << 8);
            if (low >= 0xDC00 && low <= 0xDFFF) {
                codepoint = 0x10000u + ((codepoint - 0xD800u) << 10) + (low - 0xDC00u);
                ++i;
            }
        }
        ffStrbufAppendUtf32CodePoint(result, codepoint);
    }
    return true;
}

// ---------------------------------------------------------------------------------------------
// The artwork
// ---------------------------------------------------------------------------------------------

typedef struct FFMediaAndroidCover {
    int fd; // a descriptor the reply handed over; whoever read the reply closes it
    uint32_t width;
    uint32_t height;
    uint32_t priority;
    bool found;
} FFMediaAndroidCover;

// The pixels of a Bitmap cross the transaction as an ashmem descriptor, and the descriptor's length is
// the one thing that ties the blob back to its Bitmap: Parcel::writeBlob writes the byte count in
// front of the descriptor, so that count and the descriptor's own size must agree -- and the same
// identity (`width * height * 4`) is what makes the two dimensions recoverable without knowing the
// rest of Bitmap's parcel layout, which is written by native code and has changed between releases.
//
// Nothing is assumed about where in the body the dimensions sit: the walk is over the whole body and
// the arithmetic is the filter, so a wrong candidate is rejected rather than believed. A Bitmap whose
// pixels were small enough to be written inline has no descriptor at all and answers nothing here.
[[gnu::nonnull(1, 3, 4)]] static bool androidTakeBitmap(const uint8_t* body, size_t length, const FFBinderReply* reply, FFMediaAndroidCover* cover) {
    for (uint32_t i = 0; i < reply->fdCount; i++) {
        const off_t size = lseek(reply->fds[i], 0, SEEK_END);
        if (size <= 0 || size > (off_t) UINT32_MAX || (size % 4) != 0) {
            continue;
        }

        size_t blobOffset = SIZE_MAX;
        for (size_t offset = 0; offset + sizeof(int32_t) <= length; offset += sizeof(int32_t)) {
            int32_t word = 0;
            memcpy(&word, body + offset, sizeof(word));
            if (word > 0 && (uint32_t) word == (uint32_t) size) {
                blobOffset = offset;
                break;
            }
        }
        if (blobOffset == SIZE_MAX) {
            continue;
        }

        for (size_t offset = 0; offset + 2 * sizeof(int32_t) <= blobOffset; offset += sizeof(int32_t)) {
            int32_t width = 0;
            int32_t height = 0;
            memcpy(&width, body + offset, sizeof(width));
            memcpy(&height, body + offset + sizeof(width), sizeof(height));
            if (width < 1 || height < 1 || width > 8192 || height > 8192) {
                continue;
            }
            if ((uint64_t) width * (uint64_t) height * 4 != (uint64_t) size) {
                continue;
            }

            cover->fd = reply->fds[i];
            cover->width = (uint32_t) width;
            cover->height = (uint32_t) height;
            cover->found = true;
            FF_DEBUG("The cover is %dx%d, %lld bytes of pixels", width, height, (long long) size);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------------------------
// Writing the cover out
// ---------------------------------------------------------------------------------------------

[[gnu::nonnull(2)]]
static bool androidWriteAll(FFNativeFD fd, const void* data, size_t length) {
    const uint8_t* bytes = data;
    while (length > 0) {
        const ssize_t written = write(fd, bytes, length);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        bytes += (size_t) written;
        length -= (size_t) written;
    }
    return true;
}

// The callback the encoder writes through. Its contract is that returning false stops it, which is
// what makes a full disk come out as no file rather than a truncated one.
typedef struct FFMediaAndroidCoverSink {
    FFNativeFD fd;
    size_t written;
    bool failed;
} FFMediaAndroidCoverSink;

[[gnu::nonnull(1)]]
static bool androidCoverSinkWrite(void* userContext, const void* data, size_t size) {
    FFMediaAndroidCoverSink* sink = userContext;
    if (!androidWriteAll(sink->fd, data, size)) {
        sink->failed = true;
        return false;
    }
    sink->written += size;
    return true;
}

// A Bitmap is premultiplied, and artwork without transparency is the same bytes either way -- so
// saying which it is costs one pass over the mapping and saves the encoder a bigger one. Declared
// premultiplied, AndroidBitmap_compress makes a pass over all 527 KB to unpremultiply pixels it then
// finds no alpha in: 41.1 ms against 37.6, for a file that came out byte for byte identical
// (sha256 943a6215... on all three of premultiplied, opaque and unpremultiplied). A cover that does
// carry transparency keeps the premultiplied description, and its alpha survives.
[[gnu::nonnull(1)]] static bool androidCoverIsOpaque(const uint8_t* pixels, size_t length) {
    for (size_t i = 3; i < length; i += 4) {
        if (pixels[i] != 0xFF) {
            return false;
        }
    }
    return true;
}

// API 30. The pixels are handed over as the mapping itself: the encoder reads them strided, so the
// cover never takes a second copy on the way in.
FF_ANDROID_REQUIRES_API(30)
static bool androidCompressCover(FFNativeFD fd, const void* pixels, uint32_t width, uint32_t height, size_t* outLength) {
    // Everything the description needs was already proved by the descriptor's own size, bar one
    // thing: the pixels are RGBA8888 and tightly packed, which is what a Bitmap carries, while
    // whether there is an alpha worth keeping is not in the size and has to be looked for.
    const AndroidBitmapInfo info = {
        .width = width,
        .height = height,
        .stride = width * 4,
        .format = ANDROID_BITMAP_FORMAT_RGBA_8888,
        .flags = androidCoverIsOpaque(pixels, (size_t) width * height * 4)
            ? ANDROID_BITMAP_FLAGS_ALPHA_OPAQUE
            : ANDROID_BITMAP_FLAGS_ALPHA_PREMUL,
    };

    FFMediaAndroidCoverSink sink = { .fd = fd };
    const int result = AndroidBitmap_compress(&info, ADATASPACE_SRGB, pixels, ANDROID_BITMAP_COMPRESS_FORMAT_WEBP_LOSSLESS, FF_MEDIA_ANDROID_COVER_QUALITY, &sink, androidCoverSinkWrite);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        FF_DEBUG("AndroidBitmap_compress returned %d", result);
        return false;
    }

    *outLength = sink.written;
    return !sink.failed;
}

// The descriptor holds the Bitmap's pixels and nothing else: no header, no pixel format and no
// length beyond the region's own size. Encoding them is the one thing the session service cannot do
// for this module, and the platform has the encoder -- see androidCompressCover above.
//
// What that encoder costs and produces, measured on this device with a real 363x363 cover (527076
// bytes of premultiplied RGBA) and the clock around the call alone:
//
//   format           quality    output      time
//   ---------------  --------   ----------  -------
//   PNG              any        203675 B    ~370 ms   quality is ignored, and all four settings
//                                                     produced byte-identical files
//   WebP lossless    0          146884 B      41 ms
//   WebP lossless    100        145064 B      98 ms   the whole effort knob is worth 1.2%
//   WebP lossy       80           22060 B     125 ms
//   JPEG             90           39973 B      11 ms
//
// PNG is what the artwork usually arrives as and the worst choice here: the header documents that
// quality is ignored for it, and Skia pays for an adaptive filter pass a single image cannot
// amortise. Compressing by hand instead -- a deflate stream of stored blocks, which needs no library
// at all -- costs 2 ms but writes 527490 bytes, so the platform encoder trades 39 ms for a file 3.6
// times smaller.
//
// Declaring the pixels opaque rather than premultiplied, which the scan in androidCompressCover
// makes a truthful thing to say, is worth another 3.5 ms of the 41; the two files are identical, so
// what is being bought is only the encoder's pass in search of an alpha that is not there.
//
// Lossless WebP is the format, for three measured reasons: it is nine times faster than PNG, it is
// 3.6 times smaller than what the hand-written encoder produced, and being lossless it keeps alpha --
// which matters, because a Bitmap is premultiplied and an encoder that dropped the alpha without
// compositing would leave the premultiplied colour behind and turn what was transparent black. Every
// reader here takes it as well: the iTerm and kitty logo paths hand the file over as it is for the
// terminal to decode, and AImageDecoder, which reads it back inside fastfetch, handles it too.
//
// TMPDIR is what Termux exports, and it is the one directory the terminal that will draw the logo
// can also read; /data/local/tmp is the answer when fastfetch was started by something that exports
// nothing. That second one belongs to the shell UID and root, and Termux runs this module as an app
// UID for which it is not writable, so the candidates are tested rather than trusted.
[[gnu::nonnull(1)]] static const char* androidCoverPath(char* buffer, size_t size) {
    const char* directories[] = { getenv("TMPDIR"), "/data/local/tmp" };
    for (size_t i = 0; i < sizeof(directories) / sizeof(directories[0]); i++) {
        const char* directory = directories[i];
        if (directory == nullptr || directory[0] != '/') {
            continue;
        }
        if (access(directory, W_OK) != 0) {
            continue;
        }
        const int length = snprintf(buffer, size, "%s/fastfetch-media-cover-%d.webp", directory, (int) getpid());
        if (length <= 0 || (size_t) length >= size) {
            continue;
        }
        return buffer;
    }
    return nullptr;
}

// The file belongs to this process, and this is the only place that knows its name: media.c removes
// it once the logo is drawn, which is what removeCoverAfterUse asks for, and a half-written file
// would otherwise never be removed by anyone.
[[gnu::nonnull(1)]] static void androidWriteCover(FFMediaResult* result, int sourceFD, uint32_t width, uint32_t height) {
    const off_t size = lseek(sourceFD, 0, SEEK_END);
    if (size <= 0 || (uint64_t) size != (uint64_t) width * (uint64_t) height * 4) {
        return;
    }

    // AndroidBitmap_compress is API 30, and the build reaches it through a weak reference, so this
    // is a run-time branch rather than a compile-time constant. There is no second encoder behind
    // it: AImageDecoder, the only thing on Android that can read a cover back, is API 30 as well, so
    // a file written below that would be one whose reader does not exist.
    if (FF_ANDROID_API_AT_LEAST(30)) {
        void* map = mmap(nullptr, (size_t) size, PROT_READ, MAP_SHARED, sourceFD, 0);
        if (map == MAP_FAILED) {
            return;
        }

        char path[256];
        const char* file = androidCoverPath(path, sizeof(path));
        FF_AUTO_CLOSE_FD FFNativeFD fd = file != nullptr ? open(file, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600) : -1;

        size_t length = 0;
        const bool written = ffIsValidNativeFD(fd) && androidCompressCover(fd, map, width, height, &length);

        munmap(map, (size_t) size);

        if (written) {
            ffStrbufSetS(&result->cover, file);
            result->removeCoverAfterUse = true;
            FF_DEBUG("The %ux%u cover was written to %s, %zu bytes", width, height, file, (size_t) length);
        } else if (file != nullptr) {
            ffRemoveFile(file);
        }
    }
}

// ---------------------------------------------------------------------------------------------
// The session
// ---------------------------------------------------------------------------------------------

// Every getter on ISessionController takes no argument, so a call is the interface token and nothing
// else; `flags` is passed through because one of them needs TF_ACCEPT_FDS.
[[gnu::nonnull(1, 5)]] static const char* androidCall(FFBinder* binder, uint32_t handle, uint32_t code, uint32_t flags, FFBinderReply* reply) {
    uint8_t request[FF_MEDIA_ANDROID_REQUEST_SIZE];
    FFBinderParcel parcel = ffBinderParcelCreate(request, sizeof(request));
    ffBinderParcelPutInterfaceToken(&parcel, FF_MEDIA_ANDROID_CONTROLLER_DESCRIPTOR);

    const char* error = ffBinderTransact(binder, handle, code, flags, &parcel, reply);
    if (error != nullptr) {
        return error;
    }
    if (ffBinderReplyIsStatus(reply)) {
        // onTransact() answered with a raw status_t instead of a parcel. -74 is UNKNOWN_TRANSACTION,
        // which is what a code this build of the interface does not have comes back as.
        return ffBinderReadI32(reply->data, reply->size, 0) == -74
            ? "The session does not have that method"
            : "The session rejected the request";
    }

    // Every reply starts with the exception code AIDL writes, so nothing past it is read before this
    // has been checked.
    const int32_t exception = ffBinderReadI32(reply->data, reply->size, 0);
    if (exception != 0) {
        FF_DEBUG("Transaction %u came back as exception %d over %zu bytes", code, exception, reply->size);
        return "The session raised an exception";
    }

    return nullptr;
}

[[gnu::nonnull(1, 4, 5, 6, 7)]]
static const char* androidReadMetadataEntry(FFMediaAndroidReader* reader, size_t bundleEnd, int32_t type, const FFstrbuf* key, FFMediaResult* result, const FFBinderReply* reply, FFMediaAndroidCover* cover) {
    switch (type) {
        case FF_MEDIA_ANDROID_VAL_NULL:
            return nullptr;

        case FF_MEDIA_ANDROID_VAL_STRING:
        case FF_MEDIA_ANDROID_VAL_CHAR_SEQUENCE: {
            FF_STRBUF_AUTO_DESTROY value = ffStrbufCreate();
            if (!androidReadString16(reader, &value)) {
                return "The metadata carries a truncated string";
            }
            if (value.length == 0) {
                return nullptr;
            }

            // DISPLAY_TITLE and DISPLAY_SUBTITLE are the fallbacks Android documents for a session
            // whose structured metadata is incomplete -- a radio stream, typically -- and an empty
            // TITLE is exactly the case they exist for.
            if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_TITLE)) {
                ffStrbufSet(&result->song, &value);
            } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_DISPLAY_TITLE)) {
                if (result->song.length == 0) {
                    ffStrbufSet(&result->song, &value);
                }
            } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_ARTIST)) {
                ffStrbufSet(&result->artist, &value);
            } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_DISPLAY_SUBTITLE)) {
                if (result->artist.length == 0) {
                    ffStrbufSet(&result->artist, &value);
                }
            } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_ALBUM)) {
                ffStrbufSet(&result->album, &value);
            } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_MEDIA_URI)) {
                ffStrbufSet(&result->url, &value);
            }
            return nullptr;
        }

        case FF_MEDIA_ANDROID_VAL_LONG: {
            int64_t value = 0;
            if (!androidReadI64(reader, &value)) {
                return "The metadata carries a truncated long";
            }
            // METADATA_KEY_DURATION is milliseconds, where MPRIS answers microseconds -- the one
            // field that has to be read differently on each platform.
            if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_DURATION) && value > 0 && value <= (int64_t) UINT32_MAX) {
                result->length = (uint32_t) value;
            }
            return nullptr;
        }

        case FF_MEDIA_ANDROID_VAL_INTEGER: {
            int32_t value = 0;
            if (!androidReadI32(reader, &value)) {
                return "The metadata carries a truncated int";
            }
            return nullptr;
        }

        case FF_MEDIA_ANDROID_VAL_PARCELABLE: {
            // Parcel.writeValue length-prefixes a Parcelable with the byte size of everything that
            // follows, and that size is what makes an unknown class skippable: the walk resumes at the
            // end of the payload instead of having to understand it.
            int32_t payloadLength = 0;
            if (!androidReadI32(reader, &payloadLength) || payloadLength < 0) {
                return "The metadata carries a malformed parcelable";
            }
            const size_t payloadEnd = reader->position + (size_t) payloadLength;
            if (payloadEnd > bundleEnd) {
                return "The metadata carries a parcelable past the end of the bundle";
            }

            FF_STRBUF_AUTO_DESTROY className = ffStrbufCreate();
            if (!androidReadString16(reader, &className)) {
                return "The metadata carries a truncated parcelable name";
            }

            if (ffStrbufEqualS(&className, FF_MEDIA_ANDROID_CLASS_BITMAP)) {
                uint32_t priority = 0;
                if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_ART)) {
                    priority = FF_MEDIA_ANDROID_COVER_ART;
                } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_ALBUM_ART)) {
                    priority = FF_MEDIA_ANDROID_COVER_ALBUM_ART;
                } else if (ffStrbufEqualS(key, FF_MEDIA_ANDROID_KEY_DISPLAY_ICON)) {
                    priority = FF_MEDIA_ANDROID_COVER_DISPLAY_ICON;
                }

                if (priority > cover->priority) {
                    FFMediaAndroidCover candidate = { .fd = -1 };
                    if (androidTakeBitmap(reader->data + reader->position, payloadEnd - reader->position, reply, &candidate)) {
                        candidate.priority = priority;
                        *cover = candidate;
                    }
                }
            }

            reader->position = payloadEnd;
            return nullptr;
        }

        default:
            // Anything else -- a map, a list, an array -- would have to be walked by shape to be
            // skipped, and a MediaMetadata Bundle does not carry one. Stopping here is what keeps the
            // walk from reading a wrong number of bytes and carrying on with nonsense.
            FF_DEBUG("The metadata carries an unsupported value type %d", type);
            return "The metadata carries a value this build cannot read";
    }
}

[[gnu::nonnull(1, 2, 3, 4)]]
static const char* androidReadMetadataBundle(FFMediaAndroidReader* reader, FFMediaResult* result, const FFBinderReply* reply, FFMediaAndroidCover* cover) {
    int32_t bundleLength = 0;
    if (!androidReadI32(reader, &bundleLength) || bundleLength <= 0) {
        return "The metadata bundle is empty";
    }
    const size_t bundleEnd = reader->position + (size_t) bundleLength;
    if (bundleEnd > reader->size) {
        return "The metadata bundle runs past the end of the reply";
    }

    int32_t magic = 0;
    if (!androidReadI32(reader, &magic) || (uint32_t) magic != FF_MEDIA_ANDROID_BUNDLE_MAGIC) {
        return "The metadata is not a bundle";
    }
    int32_t count = 0;
    if (!androidReadI32(reader, &count)) {
        return "The metadata bundle has no entry count";
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    int32_t read = 0;
    for (; read < count && reader->position < bundleEnd; read++) {
        if (!androidReadString16(reader, &key)) {
            return "The metadata carries a truncated key";
        }
        int32_t type = 0;
        if (!androidReadI32(reader, &type)) {
            return "The metadata carries a value without a type";
        }
        const char* error = androidReadMetadataEntry(reader, bundleEnd, type, &key, result, reply, cover);
        if (error != nullptr) {
            return error;
        }
    }

    // The count in the header is the check on the walk: a discrepancy means a type this build reads the
    // wrong width for, and a partially read title is worse than none.
    if (read != count) {
        FF_DEBUG("The metadata bundle declared %d entries but only %d could be read", count, read);
        return "The metadata bundle did not read back in full";
    }

    return nullptr;
}

// PlaybackState.STATE_*, which is what getPlaybackState answers with. The three names MPRIS uses are
// kept identical -- Playing, Paused, Stopped -- so that a format string written for the Linux or the
// Windows implementation keeps working.
static const char* androidPlaybackStateName(int32_t state) {
    switch (state) {
        case 0: return "None";
        case 1: return "Stopped";
        case 2: return "Paused";
        case 3: return "Playing";
        case 4: return "FastForwarding";
        case 5: return "Rewinding";
        case 6: return "Buffering";
        case 7: return "Error";
        case 8: return "Connecting";
        case 9: return "SkippingToPrevious";
        case 10: return "SkippingToNext";
        case 11: return "SkippingToQueueItem";
        default: return "Unknown";
    }
}

static uint32_t androidPlaybackStateScore(int32_t state) {
    // Playing first, then the states that are on their way to it, then paused. A session that is
    // stopped or has no state at all is only reported when nothing else is there.
    switch (state) {
        case 3: return 4;                                                   // PLAYING
        case 4: case 5: case 6: case 8: case 9: case 10: case 11: return 3; // moving, buffering, connecting
        case 2: return 2;                                                   // PAUSED
        case 1: case 7: return 1;                                           // STOPPED, ERROR
        default: return 0;
    }
}

// SystemClock.elapsedRealtime(), which is the clock PlaybackState.updateTime is stamped with. It is
// CLOCK_BOOTTIME and not CLOCK_MONOTONIC: a phone suspends with music playing often enough that the
// two differ by hours, and the difference would be read as a position that never advances.
static int64_t androidElapsedRealtime(void) {
    struct timespec now = {};
    if (clock_gettime(CLOCK_BOOTTIME, &now) != 0) {
        return 0;
    }
    return (int64_t) now.tv_sec * 1000 + (int64_t) (now.tv_nsec / 1000000);
}

// The reply is walked only as far as the speed: everything past it is a typed list of custom actions,
// an item id, a CharSequence and a Bundle, none of which this module reports. `length` therefore has
// to have been read already -- the clamping below needs it -- which is why the metadata pass runs
// first.
[[gnu::nonnull(1, 2)]] static const char* androidReadPlaybackState(FFMediaAndroidReader* reader, FFMediaResult* result) {
    int32_t state = 0;
    if (!androidReadI32(reader, &state) || state == 0) {
        return nullptr; // the session has no state yet
    }

    int64_t position = 0;
    if (!androidReadI64(reader, &position)) {
        return "The playback state is truncated";
    }

    float speed = 0;
    if (reader->position + sizeof(float) > reader->size) {
        return "The playback state is truncated";
    }
    memcpy(&speed, reader->data + reader->position, sizeof(speed));
    reader->position += sizeof(speed);

    int64_t updateTime = 0;
    if (!androidReadI64(reader, &updateTime)) {
        return "The playback state is truncated";
    }

    // `position` is the position *at* updateTime, so a snapshot taken now has to be advanced by the
    // time that has passed since -- otherwise a player that only refreshes its state once per track
    // would report the same position for the whole of it. Only a playing session advances: a paused
    // one is exactly where it says it is.
    int64_t current = position;
    if (state == 3 && speed > 0) {
        const int64_t now = androidElapsedRealtime();
        if (now > updateTime) {
            current += (int64_t) ((double) (now - updateTime) * (double) speed);
        }
    }

    if (current < 0) {
        current = 0;
    }
    if (result->length > 0 && current > (int64_t) result->length) {
        current = result->length;
    }
    if (current > 0 && current <= (int64_t) UINT32_MAX) {
        result->position = (uint32_t) current;
    }

    ffStrbufSetS(&result->status, androidPlaybackStateName(state));
    return nullptr;
}

static void androidReadPackageName(FFBinder* binder, uint32_t token, FFMediaResult* result) {
    FFBinderReply reply = ffBinderReplyCreate(s_reply, sizeof(s_reply));
    const char* error = androidCall(binder, token, FF_MEDIA_ANDROID_GET_PACKAGE_NAME, 0, &reply);
    if (error != nullptr) {
        FF_DEBUG("getPackageName failed: %s", error);
        return;
    }

    FFMediaAndroidReader reader = { .data = reply.data, .size = reply.size, .position = 4 };
    if (!androidReadString16(&reader, &result->playerId)) {
        return;
    }

    // `player` wants the name the app shows, and there is no way to get one from here: `cmd package
    // dump` for this app is 1.2 MB and carries `labelRes`, a resource id, rather than a name, and
    // resolving it would mean reading the app's resource table. The package name is what the
    // notification and the settings screen fall back to as well, so both fields get it -- the same
    // fallback media_linux.c uses when MPRIS answers without an Identity.
    ffStrbufSet(&result->player, &result->playerId);
}

static void androidReadMetadata(FFBinder* binder, uint32_t token, FFMediaResult* result, bool saveCover) {
    FFBinderReply reply = ffBinderReplyCreate(s_reply, sizeof(s_reply));

    // TF_ACCEPT_FDS is not a hint and not optional here: the artwork arrives as a descriptor, and a
    // request that did not ask for one is answered with BR_FAILED_REPLY -- which reads like a service
    // that is not running rather than like a missing flag. It is passed unconditionally, because the
    // alternative is a failure that depends on whether the player happened to publish artwork.
    const char* error = androidCall(binder, token, FF_MEDIA_ANDROID_GET_METADATA, TF_ACCEPT_FDS, &reply);

    FFMediaAndroidCover cover = { .fd = -1 };
    if (error != nullptr) {
        FF_DEBUG("getMetadata failed: %s", error);
    } else if (ffBinderReadI32(reply.data, reply.size, 4) != 0) {
        FFMediaAndroidReader reader = { .data = reply.data, .size = reply.size, .position = 8 };
        const char* parseError = androidReadMetadataBundle(&reader, result, &reply, &cover);
        if (parseError != nullptr) {
            FF_DEBUG("The metadata bundle could not be read: %s", parseError);
        }
    }

    if (cover.found && saveCover) {
        androidWriteCover(result, cover.fd, cover.width, cover.height);
    }

    // Every descriptor the reply carried is this side's to close, the cover's included: the pixels
    // have been read out by now, and the one no cover came from would otherwise leak.
    for (uint32_t i = 0; i < reply.fdCount; i++) {
        close(reply.fds[i]);
    }
}

static void androidReadSession(FFBinder* binder, uint32_t token, FFMediaResult* result, bool saveCover) {
    androidReadPackageName(binder, token, result);

    // The length has to be known before the position is placed, so the metadata comes first.
    androidReadMetadata(binder, token, result, saveCover);

    FFBinderReply reply = ffBinderReplyCreate(s_reply, sizeof(s_reply));
    const char* error = androidCall(binder, token, FF_MEDIA_ANDROID_GET_PLAYBACK_STATE, 0, &reply);
    if (error == nullptr && ffBinderReadI32(reply.data, reply.size, 4) != 0) {
        FFMediaAndroidReader reader = { .data = reply.data, .size = reply.size, .position = 8 };
        error = androidReadPlaybackState(&reader, result);
    }
    if (error != nullptr) {
        FF_DEBUG("getPlaybackState failed: %s", error);
    }
}

// ---------------------------------------------------------------------------------------------

// Everything that goes through the media_session service. A reason is returned when it came up empty;
// a song in `media` means it did not.
//
// There is deliberately no uid test in front of this. `getSessions` is refused for an app UID, but that
// is the service's decision to make and it is not the only way in -- a notification listener is let
// through too, an OEM can loosen the check, a release can change it -- so the module asks and reads the
// answer. That costs one transaction, and it keeps the module working wherever the check happens to
// pass, instead of hard-coding today's policy into the client.
static const char* androidDetectMediaSession(FFMediaResult* media, bool saveCover) {
    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    const char* error = ffBinderOpen(&binder);
    if (error != nullptr) {
        return error;
    }

    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle service = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_MEDIA_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &service.handle);
    if (error != nullptr) {
        FF_DEBUG("Looking up \"%s\" failed: %s", FF_MEDIA_ANDROID_SERVICE, error);
        return error;
    }

    // A phone can have several sessions at once -- a music player and a video one, say -- and only one
    // of them is the answer. Which one is decided first, by state alone, so that the metadata and the
    // artwork are fetched exactly once, for the winner.
    uint32_t tokens[FF_BINDER_MAX_HANDLES];
    uint32_t tokenCount = 0;
    {
        uint8_t request[FF_MEDIA_ANDROID_REQUEST_SIZE];
        FFBinderParcel parcel = ffBinderParcelCreate(request, sizeof(request));
        ffBinderParcelPutInterfaceToken(&parcel, FF_MEDIA_ANDROID_SESSIONS_DESCRIPTOR);
        ffBinderParcelPutI32(&parcel, 0); // ComponentName: null asks for every session
        ffBinderParcelPutI32(&parcel, 0); // userId: the single user a phone has

        FFBinderReply reply = ffBinderReplyCreate(s_reply, sizeof(s_reply));
        error = ffBinderTransact(&binder, service.handle, FF_MEDIA_ANDROID_GET_SESSIONS, 0, &parcel, &reply);
        if (error != nullptr) {
            return error;
        }
        if (ffBinderReplyIsStatus(&reply)) {
            return "The media session service rejected the request";
        }
        const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
        if (exception != 0) {
            // -1 is EX_SECURITY, which is what a UID the service will not read sessions for gets. The
            // number goes to the debug log rather than into the message: what tells the two cases apart
            // for the user is which UID is asking.
            FF_DEBUG("getSessions came back as exception %d%s", exception, exception == -1 ? " (EX_SECURITY)" : "");
            return ffAndroidIsRootOrShell(instance.state.platform.uid)
                ? "The media session service refused the request"
                : "Reading the media session needs the shell UID or root: an app UID is refused by the media_session service";
        }

        // The tokens arrive as flat binder objects, which ffBinderTransact() collects in payload order
        // -- the same order the descriptors of a metadata reply arrive in, which is what makes the two
        // line up without walking the list itself.
        tokenCount = reply.handleCount;
        if (tokenCount > FF_BINDER_MAX_HANDLES) {
            tokenCount = FF_BINDER_MAX_HANDLES;
        }
        for (uint32_t i = 0; i < tokenCount; i++) {
            tokens[i] = reply.handles[i];
        }
    }

    if (tokenCount == 0) {
        return "No active media session";
    }

    uint32_t best = 0;
    uint32_t bestScore = 0;
    for (uint32_t i = 0; i < tokenCount; i++) {
        FFBinderReply reply = ffBinderReplyCreate(s_reply, sizeof(s_reply));
        if (androidCall(&binder, tokens[i], FF_MEDIA_ANDROID_GET_PLAYBACK_STATE, 0, &reply) != nullptr) {
            continue;
        }
        // The presence marker first: a session with no state yet answers a null parcelable, and its
        // state field is not there to be read.
        const int32_t state = ffBinderReadI32(reply.data, reply.size, 4) != 0 ? ffBinderReadI32(reply.data, reply.size, 8) : 0;
        const uint32_t score = androidPlaybackStateScore(state);
        if (score > bestScore) {
            bestScore = score;
            best = i;
        }
    }

    androidReadSession(&binder, tokens[best], media, saveCover);

    // The tokens were acquired by ffBinderTransact() and are given back one by one; releasing them is
    // what keeps a short-lived fastfetch from holding a session open for the life of the process.
    for (uint32_t i = 0; i < tokenCount; i++) {
        FFBinderServiceHandle token = { .binder = &binder, .handle = tokens[i] };
        ffBinderServiceHandleRelease(&token);
    }

    return nullptr;
}

// ---------------------------------------------------------------------------------------------

// The MPRIS implementation in media_linux.c, whose entry point is renamed on Android because this file
// owns ffDetectMediaImpl -- the same arrangement as wallpaper_linux.c and terminalfont_linux.c.
void ffDetectMediaLinuxImpl(FFMediaResult* media, bool saveCover);

void ffDetectMediaImpl(FFMediaResult* media, bool saveCover) {
    // The binder route below reads Android's own now-playing session, which is the one the phone is
    // playing only while SurfaceFlinger is the display server. A Termux:X11 or Wayland session runs a
    // real compositor on top of the device, and what is playing there belongs to that desktop, not to
    // Android -- so that case is handed to the Linux implementation, which knows the MPRIS bus. Only
    // one of the two answers, never merged.
    const FFDisplayServerResult* wm = ffConnectDisplayServer();
    if (!ffStrbufIgnCaseEqualS(&wm->wmProtocolName, FF_WM_PROTOCOL_SURFACEFLINGER)) {
        FF_DEBUG("The display server is \"%s\", so the Linux implementation answers", wm->wmProtocolName.chars);
        ffDetectMediaLinuxImpl(media, saveCover);
        return;
    }

    const char* error = androidDetectMediaSession(media, saveCover);
    if (media->song.length > 0) {
        return; // the phone's own now-playing session answered
    }

    // Nothing came out of the media_session service, so try the session bus. That is worth trying even
    // with SurfaceFlinger on screen: the service refuses an app UID outright, and a player inside a
    // chroot is not an Android media session at all, so nothing else here would ever see it.
    //
    // The reason is copied out first, because the fallback writes to media->error itself.
    char sessionError[160] = "";
    if (error != nullptr) {
        snprintf(sessionError, sizeof(sessionError), "%s", error);
    }
    FF_DEBUG("The media session gave nothing (%s), falling back to MPRIS", sessionError[0] != '\0' ? sessionError : "no session");

    // Artwork without a song to go with it: the file belongs to the track MPRIS is about to report, so
    // it is removed here rather than at exit, where media.c would by then have forgotten the path.
    if (media->cover.length > 0) {
        ffRemoveFile(media->cover.chars);
        ffStrbufClear(&media->cover);
        media->removeCoverAfterUse = false;
    }

    ffStrbufClear(&media->error);
    ffDetectMediaLinuxImpl(media, saveCover);

    // The phone is what was asked about, so its reason is the one to print: it names something a user
    // can act on, while "Failed to connect to DBus" only explains why the second opinion was never
    // taken -- and most phones have no session bus at all, which would make that the usual answer.
    // MPRIS never reports anything when it merely finds no player, so a silent fallback here means
    // nothing is playing anywhere.
    if (media->song.length == 0) {
        ffStrbufSetS(&media->error, sessionError[0] != '\0' ? sessionError : "No active media session");
    }
}
