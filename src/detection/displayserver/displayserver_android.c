#include "displayserver.h"
#include "common/android/api.h"
#include "common/android/binder.h"
#include "common/android/dex.h"
#include "common/arrutil.h"
#include "common/debug.h"
#include "common/settings.h"
#include "common/strutil.h"
#include "linux/displayserver_linux.h"

#include <math.h>

// ---------------------------------------------------------------------------------------------
// The binder route
// ---------------------------------------------------------------------------------------------
//
// `cmd display get-displays` and `dumpsys display` used to be the routes here, and both are a child
// process -- that fork was all of what this module cost: it took 16.0-26.5 ms on the test device,
// where `cmd display get-displays` itself takes 9.2 ms and a bare fork 8.6 ms. `cmd` does not exist
// before Android 13 either, and `dumpsys` is behind android.permission.DUMP, so on Android 11 and 12
// an app UID had no display at all. Both are gone: `display` is the service they sat on,
// android.hardware.display.IDisplayManager, it lives in system_server, an app UID may call it, and
// one `getDisplayInfo(0)` answers the whole `DisplayInfo` the dump prints, field for field. With the
// fork gone the module took 2.8-3.2 ms where `cmd` took 16.0-26.5 ms, and Android 11 -- the release
// `cmd` could not serve at all -- has been read this way on a device. What is left is dominated by
// the dex read below, not by the transaction.
//
// What it costs is the parse. The reply is a Java Parcelable and its field set moves with every
// release and every vendor fork -- `DisplayInfo` writes 34 fields on Android 11, 42 on 13 and 57 on
// 16 -- so neither a fixed offset nor a sequence written against one release survives. The sequence
// is read out of the device's own `framework.jar` instead, from the `iget` order of
// `DisplayInfo.writeToParcel`, and the walk below follows it: a field the release does not write is
// simply not in the sequence, which is what makes every one of them optional by construction. See
// common/android/dex.h for how the sequence is recovered and what it does not say.
//
// What the sequence does not say is how each field is framed: `writeString` and `writeString8` are
// both a `Ljava/lang/String;` on the wire and are not the same length. The type descriptor settles
// the primitives and both array shapes, a handful of names settle the rest, and every nested
// Parcelable is anchored on its class name. A layout this does not recognise fails the route rather
// than answering. There is no second route behind this one any more, so a build whose layout is not
// one this parser reads reports no display at all, rather than a plausible number out of the middle of
// a different field.

#define FF_DISPLAY_ANDROID_SERVICE "display"
#define FF_DISPLAY_ANDROID_DESCRIPTOR "android.hardware.display.IDisplayManager"

// The jar the reply's classes live in, and their dex type descriptors. All of them are asked for in
// one lookup: resolving a descriptor is dominated by a pass over the dex type table, and one pass
// answers every request whose class is named in it -- see common/android/dex.h.
#define FF_DISPLAY_ANDROID_JAR "/system/framework/framework.jar"

#define FF_DISPLAY_ANDROID_CLASS_INFO "Landroid/view/DisplayInfo;"
#define FF_DISPLAY_ANDROID_CLASS_ADDRESS "Landroid/view/DisplayAddress$Physical;"
#define FF_DISPLAY_ANDROID_CLASS_PRODUCT_INFO "Landroid/hardware/display/DeviceProductInfo;"
#define FF_DISPLAY_ANDROID_CLASS_MANUFACTURE_DATE "Landroid/hardware/display/DeviceProductInfo$ManufactureDate;"
#define FF_DISPLAY_ANDROID_CLASS_HDR_CAPABILITIES "Landroid/view/Display$HdrCapabilities;"
#define FF_DISPLAY_ANDROID_CLASS_MODE "Landroid/view/Display$Mode;"
#define FF_DISPLAY_ANDROID_CLASS_FRAME_RATE_CATEGORY_RATE "Landroid/view/FrameRateCategoryRate;"
#define FF_DISPLAY_ANDROID_CLASS_CUTOUT "Landroid/view/DisplayCutout;"

// The Java class names `writeParcelable` writes in front of each nested body, which is the anchor the
// walk hangs on. It writes the *runtime* class, so the abstract `DisplayAddress` a `DisplayInfo`
// declares arrives as its `$Physical` subclass.
#define FF_DISPLAY_ANDROID_NAME_ADDRESS "android.view.DisplayAddress$Physical"
#define FF_DISPLAY_ANDROID_NAME_PRODUCT_INFO "android.hardware.display.DeviceProductInfo"
#define FF_DISPLAY_ANDROID_NAME_MANUFACTURE_DATE "android.hardware.display.DeviceProductInfo$ManufactureDate"
#define FF_DISPLAY_ANDROID_NAME_HDR_CAPABILITIES "android.view.Display$HdrCapabilities"
#define FF_DISPLAY_ANDROID_NAME_FRAME_RATE_CATEGORY_RATE "android.view.FrameRateCategoryRate"

// `IDisplayManager$Stub`, as the device's own dex declares it. `getDisplayInfo` is the first method
// and has been since the interface was introduced -- it is an `@UnsupportedAppUsage` API and that
// order has never moved -- and `getDisplayIds` is the second. They are not counted out of the AOSP
// source: a vendor fork inserts methods freely, and the seven a fork added on the test device all sit
// behind these two.
#define FF_DISPLAY_ANDROID_TRANSACTION_GET_DISPLAY_INFO 1u
#define FF_DISPLAY_ANDROID_TRANSACTION_GET_DISPLAY_IDS 2u

// One display's reply is 2404 bytes on the test device, most of it the two mode lists, and it grows
// with the display. A reply that does not fit is a failure rather than a truncation.
#define FF_DISPLAY_ANDROID_REPLY_SIZE 8192
#define FF_DISPLAY_ANDROID_PARCEL_SIZE 256

// How many displays are read at most. The reply buffer holds one display, so this only bounds the id
// list and the array the parsed records are held in until every one of them has parsed.
#define FF_DISPLAY_ANDROID_MAX_DISPLAYS 8

// How many fields each write sequence can hold. A sequence that does not fit is reported as empty
// rather than truncated, which fails the route -- half a sequence would walk half a record. See
// common/android/dex.h.
#define FF_DISPLAY_ANDROID_INFO_FIELDS 64
#define FF_DISPLAY_ANDROID_NESTED_FIELDS 8
#define FF_DISPLAY_ANDROID_MODE_FIELDS 12

// The two instance fields of `DisplayCutout` that lengthen its body, in the order the request writes
// their results. `mCutoutPathParserInfo` arrived in Android 13 and adds nine writes, `mSideOverrides`
// in 16 and adds one; Android 11 declares neither.
typedef enum FFDisplayAndroidCutoutField : uint32_t {
    FF_DISPLAY_ANDROID_CUTOUT_PATH_PARSER_INFO,
    FF_DISPLAY_ANDROID_CUTOUT_SIDE_OVERRIDES,
    FF_DISPLAY_ANDROID_CUTOUT_FIELD_COUNT,
} FFDisplayAndroidCutoutField;

static const char* const ffDisplayAndroidCutoutFields[FF_DISPLAY_ANDROID_CUTOUT_FIELD_COUNT] = {
    "mCutoutPathParserInfo",
    "mSideOverrides",
};

// The write sequence of every class the reply is made of, read out of the device's own jar. It is
// what turns the parse from a sequence written against one release into a walk of the release that
// is running.
typedef struct FFDisplayAndroidLayout {
    FFDexParcelField displayInfo[FF_DISPLAY_ANDROID_INFO_FIELDS];
    uint32_t displayInfoCount;
    FFDexParcelField address[FF_DISPLAY_ANDROID_NESTED_FIELDS];
    uint32_t addressCount;
    FFDexParcelField productInfo[FF_DISPLAY_ANDROID_NESTED_FIELDS];
    uint32_t productInfoCount;
    FFDexParcelField manufactureDate[FF_DISPLAY_ANDROID_NESTED_FIELDS];
    uint32_t manufactureDateCount;
    FFDexParcelField hdrCapabilities[FF_DISPLAY_ANDROID_NESTED_FIELDS];
    uint32_t hdrCapabilitiesCount;
    FFDexParcelField mode[FF_DISPLAY_ANDROID_MODE_FIELDS];
    uint32_t modeCount;
    FFDexParcelField frameRateCategoryRate[FF_DISPLAY_ANDROID_NESTED_FIELDS];
    uint32_t frameRateCategoryRateCount;

    // `DisplayCutout` is the one class whose body is not field-driven -- `writeCutoutToParcel` reads
    // everything through compiler-synthesised accessors, so there is no `iget` to walk. What says how
    // long its body is, is which fields the class declares, and that is what is asked for here. They
    // are one array rather than two members because the request writes them as a run.
    bool cutoutFields[FF_DISPLAY_ANDROID_CUTOUT_FIELD_COUNT];
} FFDisplayAndroidLayout;

// Reads every sequence above, and `DisplayCutout`'s declared fields, out of the dex entries of
// `jarPath` -- in one lookup, because the walk behind it is what a lookup costs and the class data it
// reads along the way already holds everything asked for here.
//
// Every sequence but `FrameRateCategoryRate` is required: a class the jar does not declare leaves an
// empty sequence, and a walk without one has nothing to follow. `FrameRateCategoryRate` arrived in
// Android 15, so a jar that does not declare it is answering rather than failing -- the field is
// simply not in the sequence behind it, and the walk never asks.
//
// A jar that cannot be read at all fails too, and with nothing behind this route that leaves the
// module with no display to report. Which is the point of the dex being the source: a build this
// parser cannot read declines to answer, rather than reporting a plausible number read out of the
// middle of a different field.
static bool displayLayoutLoad(const char* jarPath, FFDisplayAndroidLayout* layout) {
    const FFDexParcelRequest requests[] = {
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_INFO,
            .methodName = "writeToParcel",
            .fields = layout->displayInfo,
            .capacity = FF_DISPLAY_ANDROID_INFO_FIELDS,
            .count = &layout->displayInfoCount,
        },
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_ADDRESS,
            .methodName = "writeToParcel",
            .fields = layout->address,
            .capacity = FF_DISPLAY_ANDROID_NESTED_FIELDS,
            .count = &layout->addressCount,
        },
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_PRODUCT_INFO,
            .methodName = "writeToParcel",
            .fields = layout->productInfo,
            .capacity = FF_DISPLAY_ANDROID_NESTED_FIELDS,
            .count = &layout->productInfoCount,
        },
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_MANUFACTURE_DATE,
            .methodName = "writeToParcel",
            .fields = layout->manufactureDate,
            .capacity = FF_DISPLAY_ANDROID_NESTED_FIELDS,
            .count = &layout->manufactureDateCount,
        },
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_HDR_CAPABILITIES,
            .methodName = "writeToParcel",
            .fields = layout->hdrCapabilities,
            .capacity = FF_DISPLAY_ANDROID_NESTED_FIELDS,
            .count = &layout->hdrCapabilitiesCount,
        },
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_MODE,
            .methodName = "writeToParcel",
            .fields = layout->mode,
            .capacity = FF_DISPLAY_ANDROID_MODE_FIELDS,
            .count = &layout->modeCount,
        },
        {
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_FRAME_RATE_CATEGORY_RATE,
            .methodName = "writeToParcel",
            .fields = layout->frameRateCategoryRate,
            .capacity = FF_DISPLAY_ANDROID_NESTED_FIELDS,
            .count = &layout->frameRateCategoryRateCount,
        },
        {
            // The one class whose body is not field-driven. `writeCutoutToParcel` reads everything
            // through compiler-synthesised accessors, so there is no `iget` to walk and no class name
            // on the wire either -- what says how long its body is, is which fields the class
            // declares. Asking here rather than by a call of its own is the point: the walk that
            // answers the seven sequences above is already holding this class's field list, and a
            // second call would repeat it from the mapping down for 7.0 ms of a 23.5 ms lookup.
            .classDescriptor = FF_DISPLAY_ANDROID_CLASS_CUTOUT,
            .instanceFieldNames = ffDisplayAndroidCutoutFields,
            .instanceFieldResults = layout->cutoutFields,
            .instanceFieldCount = ARRAY_SIZE(ffDisplayAndroidCutoutFields),
        },
    };

    const char* error = ffDexParcelFields(jarPath, requests, ARRAY_SIZE(requests));
    if (error != nullptr) {
        FF_DEBUG("Reading the display layout out of %s failed: %s", jarPath, error);
        return false;
    }
    if (layout->displayInfoCount == 0 || layout->addressCount == 0 || layout->productInfoCount == 0
        || layout->manufactureDateCount == 0 || layout->hdrCapabilitiesCount == 0 || layout->modeCount == 0) {
        FF_DEBUG("The classes a display is made of are not all declared in %s", jarPath);
        return false;
    }

    FF_DEBUG(
        "Display layout: %u fields in DisplayInfo, %u in a mode, cutout path parser info %s, side overrides %s",
        layout->displayInfoCount, layout->modeCount,
        layout->cutoutFields[FF_DISPLAY_ANDROID_CUTOUT_PATH_PARSER_INFO] ? "yes" : "no",
        layout->cutoutFields[FF_DISPLAY_ANDROID_CUTOUT_SIDE_OVERRIDES] ? "yes" : "no"
    );
    return true;
}

// `DisplayInfo.type`, i.e. `Display.TYPE_*`. A dump prints these as `INTERNAL` / `EXTERNAL` /
// `WIFI`; the numbers are what the parcel carries.
typedef enum FFDisplayAndroidType : int32_t {
    FF_DISPLAY_ANDROID_TYPE_UNKNOWN = 0,
    FF_DISPLAY_ANDROID_TYPE_INTERNAL = 1,
    FF_DISPLAY_ANDROID_TYPE_EXTERNAL = 2,
    FF_DISPLAY_ANDROID_TYPE_WIFI = 3,
    FF_DISPLAY_ANDROID_TYPE_OVERLAY = 4,
    FF_DISPLAY_ANDROID_TYPE_VIRTUAL = 5,
} FFDisplayAndroidType;

// The `Parcel.writeValue` tags `DeviceProductInfo` uses. The framework writes one of these in front
// of the value; the numbers are its own.
typedef enum FFDisplayAndroidValue : int32_t {
    FF_DISPLAY_ANDROID_VALUE_NULL = -1,
    FF_DISPLAY_ANDROID_VALUE_STRING = 0,
    FF_DISPLAY_ANDROID_VALUE_INTEGER = 1,
    FF_DISPLAY_ANDROID_VALUE_PARCELABLE = 4,
} FFDisplayAndroidValue;

// A cursor over one reply. Every read is bounds checked and every mismatch raises `failed` rather
// than returning a value: once a field has been read at the wrong place nothing behind it can be
// trusted, and the only safe answer left is to report nothing.
typedef struct FFDisplayAndroidReader {
    const uint8_t* data;
    size_t size;
    size_t offset;
    bool failed;
} FFDisplayAndroidReader;

static bool readerSkip(FFDisplayAndroidReader* reader, size_t bytes) {
    if (bytes > reader->size - reader->offset) {
        reader->failed = true;
        return false;
    }
    reader->offset += bytes;
    return true;
}

static int32_t readerI32(FFDisplayAndroidReader* reader) {
    if (reader->offset + sizeof(int32_t) > reader->size) {
        reader->failed = true;
        return 0;
    }
    int32_t value = 0;
    memcpy(&value, reader->data + reader->offset, sizeof(int32_t));
    reader->offset += sizeof(int32_t);
    return value;
}

static uint32_t readerU32(FFDisplayAndroidReader* reader) {
    return (uint32_t) readerI32(reader);
}

static uint64_t readerU64(FFDisplayAndroidReader* reader) {
    if (reader->offset + sizeof(uint64_t) > reader->size) {
        reader->failed = true;
        return 0;
    }
    uint64_t value = 0;
    memcpy(&value, reader->data + reader->offset, sizeof(uint64_t));
    reader->offset += sizeof(uint64_t);
    return value;
}

static float readerF32(FFDisplayAndroidReader* reader) {
    const uint32_t raw = readerU32(reader);
    float value = 0;
    memcpy(&value, &raw, sizeof(value));
    return value;
}

// A `writeBoolean`, which is an int32. Anything but 0 or 1 is a field read at the wrong place.
static bool readerBool(FFDisplayAndroidReader* reader) {
    const int32_t value = readerI32(reader);
    if (value != 0 && value != 1) {
        reader->failed = true;
        return false;
    }
    return value == 1;
}

// Whether the string16 at `offset` holds `expected`, which is a class name. The length has to be the
// name's own and every code unit has to spell it, so a body that happens to start with a plausible
// length does not pass for one.
static bool readerClassNameAt(const FFDisplayAndroidReader* reader, size_t offset, const char* expected) {
    const size_t length = strlen(expected);
    if (length > 128 || offset + sizeof(int32_t) + (length + 1) * 2 > reader->size) {
        return false;
    }
    int32_t declared = 0;
    memcpy(&declared, reader->data + offset, sizeof(int32_t));
    if ((size_t) declared != length) {
        return false;
    }
    const uint8_t* text = reader->data + offset + sizeof(int32_t);
    for (size_t i = 0; i < length; ++i) {
        if (text[i * 2] != (uint8_t) expected[i] || text[i * 2 + 1] != 0) {
            return false;
        }
    }
    return true;
}

// The class name in front of a Parcelable body, which `writeParcelable` writes and which is the
// anchor this whole parser hangs on: a field that moved puts something else here, and a name that
// does not decode is how the parser finds out instead of reading a length out of the middle of a
// body.
static bool readerClassName(FFDisplayAndroidReader* reader, const char* expected) {
    if (!readerClassNameAt(reader, reader->offset, expected)) {
        reader->failed = true;
        return false;
    }
    return readerSkip(reader, sizeof(int32_t) + (((strlen(expected) + 1) * 2 + 3) & ~(size_t) 3));
}

// A `writeString`, which is UTF-16 despite the name: an int32 count of code units, the code units, a
// NUL terminator and the padding behind it. A count of -1 is how the same field spells a null String
// and is not an error; `result` may be null where the text is only in the way.
static bool readerString16(FFDisplayAndroidReader* reader, FFstrbuf* result) {
    const int32_t length = readerI32(reader);
    if (reader->failed) {
        return false;
    }
    if (length < 0) {
        return true;
    }
    const size_t payload = ((size_t) length * 2 + 2 + 3) & ~(size_t) 3;
    if (payload > reader->size - reader->offset) {
        reader->failed = true;
        return false;
    }
    if (result != nullptr) {
        for (int32_t i = 0; i < length; ++i) {
            uint32_t codePoint = (uint32_t) reader->data[reader->offset + (size_t) i * 2]
                | ((uint32_t) reader->data[reader->offset + (size_t) i * 2 + 1] << 8);
            if (codePoint >= 0xD800 && codePoint <= 0xDBFF && i + 1 < length) {
                // Anything outside the basic plane arrives as a surrogate pair, and the low half is
                // what turns it back into one code point. A high surrogate without one is malformed
                // and is passed through as it stands rather than guessed at.
                const uint32_t low = (uint32_t) reader->data[reader->offset + (size_t) (i + 1) * 2]
                    | ((uint32_t) reader->data[reader->offset + (size_t) (i + 1) * 2 + 1] << 8);
                if (low >= 0xDC00 && low <= 0xDFFF) {
                    codePoint = 0x10000u + ((codePoint - 0xD800u) << 10) + (low - 0xDC00u);
                    ++i;
                }
            }
            ffStrbufAppendUtf32CodePoint(result, codePoint);
        }
    }
    reader->offset += payload;
    return true;
}

// A `writeString8`, which is the one that really is UTF-8: an int32 *byte* count, that many bytes, a
// NUL and the padding. This is what `DisplayInfo.name` and `DisplayInfo.uniqueId` use -- the built-in
// display's name is 12 bytes of UTF-8 where the same text is 4 code units as a string16.
static bool readerString8(FFDisplayAndroidReader* reader, FFstrbuf* result) {
    const int32_t length = readerI32(reader);
    if (reader->failed) {
        return false;
    }
    if (length < 0) {
        return true;
    }
    const size_t payload = ((size_t) length + 1 + 3) & ~(size_t) 3;
    if (payload > reader->size - reader->offset) {
        reader->failed = true;
        return false;
    }
    if (result != nullptr) {
        ffStrbufAppendNS(result, (uint32_t) length, (const char*) reader->data + reader->offset);
    }
    reader->offset += payload;
    return true;
}

// An array of primitives: a count and the values. A count of -1 is a null array, which carries none.
static bool readerSkipArray(FFDisplayAndroidReader* reader, size_t elementSize) {
    const int32_t count = readerI32(reader);
    if (reader->failed) {
        return false;
    }
    if (count < 0) {
        return true;
    }
    if (count > 1024) {
        reader->failed = true;
        return false;
    }
    return readerSkip(reader, (size_t) count * elementSize);
}

// A `writeValue` whose value is an Integer, or a null which reads as zero.
static int32_t readerValueInteger(FFDisplayAndroidReader* reader) {
    switch ((FFDisplayAndroidValue) readerI32(reader)) {
        case FF_DISPLAY_ANDROID_VALUE_NULL:
            return 0;
        case FF_DISPLAY_ANDROID_VALUE_INTEGER:
            return readerI32(reader);
        default:
            reader->failed = true;
            return 0;
    }
}

// A `writeValue` whose value is a String, an Integer or a null. Nothing here wants the value; the
// shapes it does not know are what it refuses to guess a length for.
static bool readerSkipValue(FFDisplayAndroidReader* reader) {
    switch ((FFDisplayAndroidValue) readerI32(reader)) {
        case FF_DISPLAY_ANDROID_VALUE_NULL:
            return true;
        case FF_DISPLAY_ANDROID_VALUE_STRING:
            return readerString16(reader, nullptr);
        case FF_DISPLAY_ANDROID_VALUE_INTEGER:
            (void) readerI32(reader);
            return !reader->failed;
        default:
            reader->failed = true;
            return false;
    }
}

// One field whose dex type descriptor settles how it is framed. `[I` and `[F` are the two array
// shapes the record carries and report their element count; the rest report their value.
typedef struct FFDisplayAndroidScalar {
    int32_t i32;
    float f32;
    int32_t count;
} FFDisplayAndroidScalar;

// Reads one field whose descriptor settles its framing -- an `I`, an `F`, a `J`, a `Z`, an `[I` or
// an `[F`. A descriptor with no rule here is a failure rather than a guess: `Ljava/lang/String;`
// alone can be a `writeString` or a `writeString8`, the two are not the same length on the wire, and
// stepping over it as if it were either would move every field behind it.
static bool readerScalar(FFDisplayAndroidReader* reader, const char* type, FFDisplayAndroidScalar* value) {
    *value = (FFDisplayAndroidScalar) {};
    if (ffStrEquals(type, "I")) {
        value->i32 = readerI32(reader);
        return !reader->failed;
    }
    if (ffStrEquals(type, "F")) {
        value->f32 = readerF32(reader);
        return !reader->failed;
    }
    if (ffStrEquals(type, "J")) {
        (void) readerU64(reader);
        return !reader->failed;
    }
    if (ffStrEquals(type, "Z")) {
        value->i32 = readerBool(reader) ? 1 : 0;
        return !reader->failed;
    }
    if (ffStrEquals(type, "[I") || ffStrEquals(type, "[F")) {
        const int32_t count = readerI32(reader);
        if (reader->failed || count < 0 || count > 1024) {
            reader->failed = true;
            return false;
        }
        value->count = count;
        return readerSkip(reader, (size_t) count * (ffStrEquals(type, "[I") ? sizeof(int32_t) : sizeof(float)));
    }
    reader->failed = true;
    return false;
}

// The class name in front of a nested body, checked against the class the field is expected to carry.
// `writeParcelable` writes the runtime class, so the abstract `DisplayAddress` a `DisplayInfo`
// declares arrives as its `$Physical` subclass; a null field is a single -1 where the class name
// would be, which is a value and not a failure.
static bool readerParcelableHead(FFDisplayAndroidReader* reader, const char* expected, bool* present) {
    const int32_t length = readerI32(reader);
    if (reader->failed) {
        return false;
    }
    if (length < 0) {
        *present = false;
        return true;
    }
    reader->offset -= sizeof(int32_t);
    *present = true;
    return readerClassName(reader, expected);
}

// A `Rect` written by `writeTypedObject` or by `writeTypedArray`: the non-null marker the typed write
// puts in front of the body and then four ints. A null is a marker and no ints behind it.
static bool readerRect(FFDisplayAndroidReader* reader) {
    const int32_t marker = readerI32(reader);
    if (reader->failed) {
        return false;
    }
    return marker < 0 || readerSkip(reader, 4 * sizeof(int32_t));
}

// `writeValue(mManufactureDate)`, the only Parcelable a `writeValue` carries here, and the one place
// the parcel does something the AOSP sources do not describe: on Android 13 and later the tag is
// followed by the size of everything behind it -- the class name and the body -- before the class
// name, which is why a plain `tag + class name` walk lands four bytes early; Android 11 has no size
// at all. Neither layout is assumed -- a value that decodes as the class name has no size, anything
// else is stepped over -- and where there is one the cursor is snapped to the end it promises, so a
// body this parser reads differently can not move the fields behind it. The size counts from its own
// end, not from its start.
static bool readerManufactureDate(FFDisplayAndroidReader* reader, const FFDisplayAndroidLayout* layout, uint16_t* year, uint16_t* week) {
    // `mManufactureDate` is `@Nullable`, so a display with no date writes a bare `VAL_NULL` and
    // nothing behind it -- and a date is not what the display is being read for. A missing one is
    // skipped rather than failed: everything the module needs is still in the record, and dropping
    // the display over an absent date would answer nothing at all. Only a tag that is neither a
    // Parcelable nor a null fails, because that is a framing this walk does not know how to step over.
    const int32_t tag = readerI32(reader);
    if (tag == FF_DISPLAY_ANDROID_VALUE_NULL) {
        *year = 0;
        *week = 0;
        return true;
    }
    if (tag != FF_DISPLAY_ANDROID_VALUE_PARCELABLE) {
        reader->failed = true;
        return false;
    }

    size_t bodyEnd = 0;
    if (!readerClassNameAt(reader, reader->offset, FF_DISPLAY_ANDROID_NAME_MANUFACTURE_DATE)) {
        const int32_t declared = readerI32(reader);
        if (declared <= 0) {
            reader->failed = true;
            return false;
        }
        bodyEnd = reader->offset + (size_t) declared;
    }
    if (!readerClassName(reader, FF_DISPLAY_ANDROID_NAME_MANUFACTURE_DATE)) {
        return false;
    }

    // `writeValue(mWeek)` and `writeValue(mYear)`, each an Integer or a null. Nothing is reported
    // unless both are there, so a class that stops writing one leaves the date at nothing rather
    // than at half of it.
    uint16_t weekValue = 0;
    uint16_t yearValue = 0;
    for (uint32_t i = 0; i < layout->manufactureDateCount; ++i) {
        const int32_t value = readerValueInteger(reader);
        if (reader->failed || value < 0 || value > 3000) {
            reader->failed = true;
            return false;
        }
        if (ffStrEquals(layout->manufactureDate[i].name, "mWeek")) {
            if (value > 53) {
                reader->failed = true;
                return false;
            }
            weekValue = (uint16_t) value;
        } else if (ffStrEquals(layout->manufactureDate[i].name, "mYear")) {
            yearValue = (uint16_t) value;
        }
    }
    *week = weekValue;
    *year = yearValue;

    if (bodyEnd > 0) {
        if (bodyEnd < reader->offset || bodyEnd > reader->size) {
            reader->failed = true;
            return false;
        }
        reader->offset = bodyEnd;
    }
    return true;
}

// `deviceProductInfo`, which is where a display's manufacture date is carried. Nothing else in it is
// used, but it sits in front of `name`, the density and the rotation, so it has to be walked whether
// or not its values are wanted -- and it is null on some displays.
//
// Three of its six fields are handed to `writeValue`, which frames them by their runtime type rather
// than by their declared one: `mProductId` is declared a `String` and goes through `writeValue` where
// `mName` next to it goes through `writeString`, so those are named rather than dispatched by type.
// The sixth is `mRelativeAddress`, an int array, on Android 11 and `mConnectionToSinkType`, an int,
// on 13 and later; the descriptor settles that one, which is what makes the two read the same code.
static bool readerDeviceProductInfo(FFDisplayAndroidReader* reader, const FFDisplayAndroidLayout* layout, uint16_t* year, uint16_t* week) {
    bool present = false;
    if (!readerParcelableHead(reader, FF_DISPLAY_ANDROID_NAME_PRODUCT_INFO, &present)) {
        return false;
    }
    if (!present) {
        return true;
    }

    for (uint32_t i = 0; i < layout->productInfoCount; ++i) {
        const FFDexParcelField* field = &layout->productInfo[i];
        if (ffStrEquals(field->name, "mName") || ffStrEquals(field->name, "mManufacturerPnpId")) {
            if (!readerString16(reader, nullptr)) {
                return false;
            }
        } else if (ffStrEquals(field->name, "mProductId") || ffStrEquals(field->name, "mModelYear")) {
            if (!readerSkipValue(reader)) {
                return false;
            }
        } else if (ffStrEquals(field->name, "mManufactureDate")) {
            if (!readerManufactureDate(reader, layout, year, week)) {
                return false;
            }
        } else {
            FFDisplayAndroidScalar value = {};
            if (!readerScalar(reader, field->type, &value)) {
                return false;
            }
        }
    }
    return true;
}

// `address`, which is null on some displays and a `DisplayAddress$Physical` on others. Its body is
// one `long` on every release seen -- the older sources describe a `long` plus an `int`, and that
// shape has not turned up on any of the three jars -- and it is walked out of the jar's own sequence
// rather than assumed, so a release that changes it is read rather than misread.
static bool readerDisplayAddress(FFDisplayAndroidReader* reader, const FFDisplayAndroidLayout* layout) {
    bool present = false;
    if (!readerParcelableHead(reader, FF_DISPLAY_ANDROID_NAME_ADDRESS, &present)) {
        return false;
    }
    if (!present) {
        return true;
    }
    for (uint32_t i = 0; i < layout->addressCount; ++i) {
        FFDisplayAndroidScalar value = {};
        if (!readerScalar(reader, layout->address[i].type, &value)) {
            return false;
        }
    }
    return true;
}

// A nested Parcelable whose values this module does not use, walked because the fields behind it sit
// on the far side of it. `frameRateCategoryRate` is the only one of those left before `uniqueId`.
static bool readerSkippedParcelable(FFDisplayAndroidReader* reader, const char* expected, const FFDexParcelField* fields, uint32_t count) {
    bool present = false;
    if (!readerParcelableHead(reader, expected, &present)) {
        return false;
    }
    if (!present) {
        return true;
    }
    for (uint32_t i = 0; i < count; ++i) {
        FFDisplayAndroidScalar value = {};
        if (!readerScalar(reader, fields[i].type, &value)) {
            return false;
        }
    }
    return true;
}

// `hdrCapabilities`, whose only value this module uses is how many HDR types the display supports --
// an empty list is the display saying it can not do HDR at all.
static bool readerHdrCapabilities(FFDisplayAndroidReader* reader, const FFDisplayAndroidLayout* layout, int32_t* hdrTypeCount) {
    bool present = false;
    if (!readerParcelableHead(reader, FF_DISPLAY_ANDROID_NAME_HDR_CAPABILITIES, &present)) {
        return false;
    }
    if (!present) {
        return true;
    }
    for (uint32_t i = 0; i < layout->hdrCapabilitiesCount; ++i) {
        FFDisplayAndroidScalar value = {};
        if (!readerScalar(reader, layout->hdrCapabilities[i].type, &value)) {
            return false;
        }
        if (ffStrEquals(layout->hdrCapabilities[i].name, "mSupportedHdrTypes")) {
            if (value.count > 16) {
                reader->failed = true;
                return false;
            }
            *hdrTypeCount = value.count;
        }
    }
    return true;
}

// `displayCutout`, the largest nested structure in the record and the one that is easiest to get
// wrong: it is serialized by `DisplayCutout$ParcelableWrapper.writeCutoutToParcel`, because
// `DisplayCutout` has no `writeToParcel` of its own, and that method reads everything through
// `-$$Nest$` accessors the compiler synthesised -- so there is no `iget` to walk and no class name on
// the wire either, and the write sequence the jar gives for every other class says nothing here.
//
// What says how long the body is, is which fields the class declares. Android 11 declares three --
// `mBounds`, `mSafeInsets`, `mWaterfallInsets` -- Android 13 adds `mCutoutPathParserInfo`, which is
// nine more writes, and Android 16 adds `mSideOverrides`, which is one more. Both are read from the
// jar rather than from a version number. None of it is reported; it is walked because `rotation` and
// everything behind it sit on the far side of it.
static bool readerDisplayCutout(FFDisplayAndroidReader* reader, const FFDisplayAndroidLayout* layout) {
    const int32_t marker = readerI32(reader);
    if (reader->failed) {
        return false;
    }
    if (marker != 1) {
        // 0 for a null cutout and -1 for `DisplayCutout.NO_CUTOUT`, which is a display with none at
        // all. Anything else is a field read at the wrong place.
        if (marker == 0 || marker == -1) {
            return true;
        }
        reader->failed = true;
        return false;
    }

    if (!readerRect(reader)) { // mSafeInsets
        return false;
    }
    const int32_t bounds = readerI32(reader); // mBounds, a typed array of Rects
    if (reader->failed || bounds < 0 || bounds > 64) {
        reader->failed = true;
        return false;
    }
    for (int32_t i = 0; i < bounds; ++i) {
        if (!readerRect(reader)) {
            return false;
        }
    }
    if (!readerRect(reader)) { // mWaterfallInsets
        return false;
    }

    const bool* cutout = layout->cutoutFields;
    if (cutout[FF_DISPLAY_ANDROID_CUTOUT_PATH_PARSER_INFO]) {
        // the display size the cutout was built for and then the physical one
        if (!readerSkip(reader, 4 * sizeof(int32_t))) {
            return false;
        }
        const float density = readerF32(reader);
        if (reader->failed || !(density > 0) || density > 1000) {
            reader->failed = true;
            return false;
        }
        // the cutout spec, e.g. `M -45,0 L -45,142 L 45,142 L 45,0 Z`, and then the rotation, the
        // scale and the physical pixel ratio, which are three plain four byte fields
        if (!readerString16(reader, nullptr) || !readerSkip(reader, 3 * sizeof(int32_t))) {
            return false;
        }
    }
    if (cutout[FF_DISPLAY_ANDROID_CUTOUT_SIDE_OVERRIDES]) {
        return readerSkipArray(reader, sizeof(int32_t)); // mSideOverrides
    }
    return true;
}

// One entry of `supportedModes`, which is `Display$Mode`.
typedef struct FFDisplayAndroidMode {
    int32_t id;
    uint32_t width;
    uint32_t height;
    float refreshRate;
} FFDisplayAndroidMode;

// Walks a `Display$Mode[]`: a count and then that many bare bodies, with no non-null marker per
// element, which is what `writeTypedArray` would put there -- the framework writes the length with
// `writeInt` and then one `writeToParcel` per element. The two ids the record carries are looked up
// as it goes, so the list does not have to be kept.
//
// The body is walked out of the jar's own sequence rather than read as a fixed run: `Display$Mode`
// writes four fields on Android 11, five on 13 and eight on 16, and the fourth -- the one the
// refresh rate is in -- is `mRefreshRate` on the first two and `mPeakRefreshRate` on the third.
static bool readerModeList(FFDisplayAndroidReader* reader, const FFDisplayAndroidLayout* layout, int32_t activeId, int32_t defaultId, FFDisplayAndroidMode* active, FFDisplayAndroidMode* preferred) {
    const int32_t count = readerI32(reader);
    if (reader->failed || count < 0 || count > 64) {
        reader->failed = true;
        return false;
    }
    for (int32_t i = 0; i < count; ++i) {
        FFDisplayAndroidMode mode = {};
        for (uint32_t j = 0; j < layout->modeCount; ++j) {
            const FFDexParcelField* field = &layout->mode[j];
            FFDisplayAndroidScalar value = {};
            if (!readerScalar(reader, field->type, &value)) {
                return false;
            }
            if (ffStrEquals(field->name, "mModeId")) {
                mode.id = value.i32;
            } else if (ffStrEquals(field->name, "mWidth")) {
                mode.width = (uint32_t) value.i32;
            } else if (ffStrEquals(field->name, "mHeight")) {
                mode.height = (uint32_t) value.i32;
            } else if (ffStrEquals(field->name, "mRefreshRate") || ffStrEquals(field->name, "mPeakRefreshRate")) {
                mode.refreshRate = value.f32;
            }
        }
        if (reader->failed
            || mode.id < 0 || mode.width == 0 || mode.width > 32768 || mode.height == 0 || mode.height > 32768
            || !(mode.refreshRate > 0) || mode.refreshRate > 1000) {
            reader->failed = true;
            return false;
        }
        if (active != nullptr && mode.id == activeId) {
            *active = mode;
        }
        if (preferred != nullptr && mode.id == defaultId) {
            *preferred = mode;
        }
    }
    return !reader->failed;
}

// One parsed record. It is held until every display has parsed, because a record this parser does not
// recognise says the layout is not the one it was written for, and half an answer from a layout it
// does not understand is worse than falling back.
typedef struct FFDisplayAndroidInfo {
    uint32_t width;
    uint32_t height;
    uint32_t preferredWidth;
    uint32_t preferredHeight;
    double preferredRefreshRate;
    double refreshRate;
    uint32_t density;
    double physicalXDpi;
    double physicalYDpi;
    uint32_t rotation;
    uint32_t displayId;
    int32_t type;
    int32_t modeId;
    int32_t defaultModeId;
    int32_t hdrTypeCount;
    float renderFrameRate;
    bool forceSdr;
    uint16_t manufactureYear;
    uint16_t manufactureWeek;
    uint64_t id;
    FFstrbuf name;
} FFDisplayAndroidInfo;

// One `DisplayInfo` walk in progress: the record being filled, the two modes the mode list is
// resolved against, and the id string -- which is not part of the record, it is only read to pull the
// display's id out of it, so it is held here rather than there.
typedef struct FFDisplayAndroidWalk {
    FFDisplayAndroidInfo* info;
    FFstrbuf* uniqueId;
    FFDisplayAndroidMode active;
    FFDisplayAndroidMode preferred;
} FFDisplayAndroidWalk;

// `uniqueId` identifies the display across reboots, e.g. `local:4630946557703207059` for the panel
// and `virtual:...` for a virtual one. The prefix says what kind of display it is, and the number
// behind the colon is the id.
static uint64_t displayUniqueId(const char* uniqueId) {
    const char* colon = strchr(uniqueId, ':');
    return (uint64_t) strtoull(colon != nullptr ? colon + 1 : uniqueId, nullptr, 10);
}

// One field of the `DisplayInfo` write sequence. The name says which value the field carries; the
// type descriptor says how it is framed, and settles that for every primitive and both array shapes.
//
// Four things the descriptor does not settle, and each is named:
//
//  * `displayCutout`, which has no write sequence to follow at all. See readerDisplayCutout.
//  * `frameRateCategoryRate`, a Parcelable whose values this module does not use.
//  * `supportedModes` and `appsSupportedModes`, arrays of Parcelables written by a loop rather than
//    by a `write*Array`, so the shape behind them is `Display$Mode`'s own.
//  * The `String` fields. `DisplayInfo` writes its own through `writeString8` -- a byte count and
//    UTF-8 -- while `DeviceProductInfo` writes its through `writeString`, which is a code-unit count
//    and UTF-16; the descriptor is `Ljava/lang/String;` either way. Within one class the framing is
//    the class's habit rather than the field's, so a `String` this does not know is read the way the
//    ones it does are, which is what a field a later release adds gets.
//
// A *name* this does not know is stepped over by its type, which is what makes a field a later
// release added cost its bytes and nothing else -- `displayGroupId`, `renderFrameRate`,
// `hasArrSupport`, `supportedRefreshRates`, `userPreferredModeId`, `appsSupportedModes`, `isForceSdr`
// and `committedState` are all of them. A *type* this does not know is a failure: the descriptor is
// the only thing that says how long the field is, and a `String` alone can be two different lengths.
static bool displayInfoField(FFDisplayAndroidReader* reader, const FFDexParcelField* field, const FFDisplayAndroidLayout* layout, FFDisplayAndroidWalk* walk) {
    FFDisplayAndroidInfo* info = walk->info;
    const char* name = field->name;

    if (ffStrEquals(name, "address")) {
        return readerDisplayAddress(reader, layout);
    }
    if (ffStrEquals(name, "deviceProductInfo")) {
        return readerDeviceProductInfo(reader, layout, &info->manufactureYear, &info->manufactureWeek);
    }
    if (ffStrEquals(name, "displayCutout")) {
        return readerDisplayCutout(reader, layout);
    }
    if (ffStrEquals(name, "hdrCapabilities")) {
        return readerHdrCapabilities(reader, layout, &info->hdrTypeCount);
    }
    if (ffStrEquals(name, "frameRateCategoryRate")) {
        return readerSkippedParcelable(reader, FF_DISPLAY_ANDROID_NAME_FRAME_RATE_CATEGORY_RATE, layout->frameRateCategoryRate, layout->frameRateCategoryRateCount);
    }
    if (ffStrEquals(name, "supportedModes") || ffStrEquals(name, "appsSupportedModes")) {
        // The two ids the list is resolved against are written in front of it on every release seen.
        // One that moved behind it would leave the list resolved against nothing, so the walk says so
        // rather than reporting the mode of whatever id happens to sit at the sentinel.
        const bool wanted = ffStrEquals(name, "supportedModes");
        if (wanted && (info->modeId == INT32_MIN || info->defaultModeId == INT32_MIN)) {
            reader->failed = true;
            return false;
        }
        return readerModeList(reader, layout, info->modeId, info->defaultModeId,
            wanted ? &walk->active : nullptr, wanted ? &walk->preferred : nullptr);
    }
    if (ffStrEquals(field->type, "Ljava/lang/String;")) {
        if (ffStrEquals(name, "name")) {
            return readerString8(reader, &info->name);
        }
        if (ffStrEquals(name, "uniqueId")) {
            return readerString8(reader, walk->uniqueId);
        }
        return readerString8(reader, nullptr); // `ownerPackageName`, and anything a later release adds
    }

    FFDisplayAndroidScalar value = {};
    if (!readerScalar(reader, field->type, &value)) {
        return false;
    }
    if (ffStrEquals(name, "type")) {
        info->type = value.i32;
    } else if (ffStrEquals(name, "displayId")) {
        info->displayId = (uint32_t) value.i32;
    } else if (ffStrEquals(name, "logicalWidth")) {
        info->width = (uint32_t) value.i32;
    } else if (ffStrEquals(name, "logicalHeight")) {
        info->height = (uint32_t) value.i32;
    } else if (ffStrEquals(name, "rotation")) {
        info->rotation = (uint32_t) value.i32;
    } else if (ffStrEquals(name, "modeId")) {
        info->modeId = value.i32;
    } else if (ffStrEquals(name, "defaultModeId")) {
        info->defaultModeId = value.i32;
    } else if (ffStrEquals(name, "renderFrameRate")) {
        if (!(value.f32 > 0) || value.f32 > 1000) {
            reader->failed = true;
            return false;
        }
        info->renderFrameRate = value.f32;
    } else if (ffStrEquals(name, "logicalDensityDpi")) {
        info->density = (uint32_t) value.i32;
    } else if (ffStrEquals(name, "physicalXDpi")) {
        info->physicalXDpi = value.f32;
    } else if (ffStrEquals(name, "physicalYDpi")) {
        info->physicalYDpi = value.f32;
    } else if (ffStrEquals(name, "isForceSdr")) {
        info->forceSdr = value.i32 != 0;
    }
    return true;
}

// One `getDisplayInfo` reply: the exception code the stub writes, the non-null marker
// `writeTypedObject` writes in front of the Parcelable, and the record itself -- walked in the order
// the device's own `DisplayInfo.writeToParcel` writes it, out of the sequence `displayLayoutLoad`
// read from that device's jar.
static bool displayInfoParse(const uint8_t* data, size_t size, const FFDisplayAndroidLayout* layout, FFDisplayAndroidInfo* info) {
    FFDisplayAndroidReader reader = { .data = data, .size = size, .offset = 0, .failed = false };

    const int32_t exception = readerI32(&reader);
    if (exception != 0) {
        FF_DEBUG("getDisplayInfo raised exception %d", exception);
        return false;
    }
    if (readerI32(&reader) != 1) {
        FF_DEBUG("getDisplayInfo answered no display");
        return false;
    }

    // Sentinels, so a field the release does not write can not be mistaken for one that was read as
    // zero. Every one of these is in the sequence on every release seen and the check below is what
    // says so; the two that are not -- `renderFrameRate` and `isForceSdr` -- have a defined fallback
    // and are the only ones left out of it.
    info->type = -1;
    info->displayId = UINT32_MAX;
    info->width = 0;
    info->height = 0;
    info->rotation = UINT32_MAX;
    info->modeId = INT32_MIN;
    info->defaultModeId = INT32_MIN;
    info->hdrTypeCount = 0;
    info->renderFrameRate = 0;
    info->density = 0;
    info->physicalXDpi = 0;
    info->physicalYDpi = 0;
    info->forceSdr = false;

    FF_STRBUF_AUTO_DESTROY uniqueId = ffStrbufCreate();
    FFDisplayAndroidWalk walk = { .info = info, .uniqueId = &uniqueId };

    // `uniqueId` is the last field this module reads. Everything behind it -- `removeMode`, the
    // brightness, the corners and shapes, the throttling -- would have to be walked for nothing, so
    // the walk stops here rather than implementing four more nested shapes no value comes out of.
    bool complete = false;
    for (uint32_t i = 0; i < layout->displayInfoCount; ++i) {
        const FFDexParcelField* field = &layout->displayInfo[i];
        if (!displayInfoField(&reader, field, layout, &walk)) {
            FF_DEBUG("getDisplayInfo(%u) answered a %s this parser does not recognise", info->displayId, field->name);
            return false;
        }
        if (ffStrEquals(field->name, "uniqueId")) {
            complete = true;
            break;
        }
    }
    if (!complete) {
        FF_DEBUG("getDisplayInfo(%u) answered a record with no uniqueId", info->displayId);
        return false;
    }

    if (info->type < 0 || info->type > FF_DISPLAY_ANDROID_TYPE_VIRTUAL
        || info->displayId == UINT32_MAX
        || info->width == 0 || info->width > 65536 || info->height == 0 || info->height > 65536
        || info->rotation > 3
        || info->modeId < 0 || info->defaultModeId < -1
        || info->density > 4000
        || !(info->physicalXDpi >= 0) || info->physicalXDpi > 2000
        || !(info->physicalYDpi >= 0) || info->physicalYDpi > 2000
        // A display with no name is still a display -- the key falls back to a number -- so only a
        // name that could not have come from a string is a reason to drop the record.
        || info->name.length > 256
        || uniqueId.length > 256) {
        FF_DEBUG(
            "getDisplayInfo(%u) answered type %d, %u x %u, rotation %u, mode %d, density %u, %f x %f dpi",
            info->displayId, info->type, info->width, info->height, info->rotation, info->modeId, info->density,
            info->physicalXDpi, info->physicalYDpi
        );
        return false;
    }

    info->preferredWidth = walk.preferred.width;
    info->preferredHeight = walk.preferred.height;
    info->preferredRefreshRate = walk.preferred.refreshRate;
    // The nominal rate of the active mode, which is what `Display.getRefreshRate()` reports and what
    // every other platform reports. `renderFrameRate` is only used when the list has no entry for
    // `modeId`, because it is a render cadence that follows the content rather than a property of the
    // display -- and it is not written at all before Android 15. `refreshRateOverride` is deliberately
    // not consulted either: it is what an app asked the framework to hold the panel at, not a
    // property of the display.
    info->refreshRate = walk.active.refreshRate > 0 ? walk.active.refreshRate : info->renderFrameRate;
    info->id = displayUniqueId(uniqueId.chars);
    return true;
}

static FFDisplayType displayType(int32_t type) {
    switch ((FFDisplayAndroidType) type) {
        case FF_DISPLAY_ANDROID_TYPE_INTERNAL:
            return FF_DISPLAY_TYPE_BUILTIN;
        case FF_DISPLAY_ANDROID_TYPE_EXTERNAL:
        case FF_DISPLAY_ANDROID_TYPE_WIFI:
            // A WIFI display is a wireless sink, which is as external as a wired one
            return FF_DISPLAY_TYPE_EXTERNAL;
        default:
            return FF_DISPLAY_TYPE_UNKNOWN;
    }
}

// `hdrCapabilities.mSupportedHdrTypes` is the display's own answer and an empty list means it can not
// do HDR at all. `isForceSdr` is the framework having turned every capability of this display off and
// `persist.sys.hdr_mode` is the vendor's switch, so `Supported` and `Enabled` stay two different
// questions: the list is what the panel can do, those two are whether anything is asking for it.
static FFDisplayHdrStatus displayHdrStatus(const FFDisplayAndroidInfo* info) {
    if (info->hdrTypeCount <= 0) {
        return FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
    }
    FF_STRBUF_AUTO_DESTROY hdrMode = ffStrbufCreate();
    if (!info->forceSdr && ffSettingsGetAndroidProperty("persist.sys.hdr_mode", &hdrMode) && ffStrbufToUInt(&hdrMode, 0) > 0) {
        return FF_DISPLAY_HDR_STATUS_ENABLED;
    }
    return FF_DISPLAY_HDR_STATUS_SUPPORTED;
}

// Reads every enabled display over binder. Returns false without appending anything when the service,
// the transaction or the layout is not what this parser was written for. There is no route behind
// this one, so that is also how the module ends up reporting no display.
static bool detectWithBinder(FFDisplayServerResult* ds) {
    // The write sequence first. It is what the whole parse is made of, and a build whose jar cannot be
    // read has nothing to walk -- so failing here costs one file open rather than a round trip to a
    // service whose answer could not be read anyway.
    FFDisplayAndroidLayout layout = {};
    if (!displayLayoutLoad(FF_DISPLAY_ANDROID_JAR, &layout)) {
        return false;
    }

    [[gnu::cleanup(ffBinderClose)]] FFBinder binder = { .fd = -1 };
    const char* error = ffBinderOpen(&binder);
    if (error != nullptr) {
        FF_DEBUG("Reading the display over binder failed: %s", error);
        return false;
    }

    // Released on every path out, including the early returns below.
    [[gnu::cleanup(ffBinderServiceHandleRelease)]] FFBinderServiceHandle service = { .binder = &binder };
    error = ffBinderLookupService(&binder, FF_DISPLAY_ANDROID_SERVICE, FF_BINDER_SM_GET_SERVICE, &service.handle);
    if (error != nullptr) {
        FF_DEBUG("Looking up the \"%s\" service failed: %s", FF_DISPLAY_ANDROID_SERVICE, error);
        return false;
    }

    uint8_t parcelBuffer[FF_DISPLAY_ANDROID_PARCEL_SIZE];
    uint8_t replyBuffer[FF_DISPLAY_ANDROID_REPLY_SIZE];

    // `getDisplayIds(false)` is the list of the enabled displays, which is the set a display dump
    // prints. It is a separate call rather than a loop over 0..n because the ids are not contiguous,
    // and it is twelve bytes.
    //
    // The argument is written unconditionally although the method takes none on Android 11: the
    // generated stub reads the parameters its own signature declares and leaves the rest of the
    // parcel alone, so the extra int is dropped on those releases.
    FFBinderParcel parcel = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
    ffBinderParcelPutInterfaceToken(&parcel, FF_DISPLAY_ANDROID_DESCRIPTOR);
    ffBinderParcelPutI32(&parcel, 0); // includeDisabled

    FFBinderReply reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
    error = ffBinderTransact(&binder, service.handle, FF_DISPLAY_ANDROID_TRANSACTION_GET_DISPLAY_IDS, 0, &parcel, &reply);
    if (error != nullptr) {
        FF_DEBUG("getDisplayIds failed: %s", error);
        return false;
    }
    if (ffBinderReplyIsStatus(&reply)) {
        FF_DEBUG("getDisplayIds answered status %d instead of a parcel", (int) ffBinderReadI32(reply.data, reply.size, 0));
        return false;
    }
    const int32_t exception = ffBinderReadI32(reply.data, reply.size, 0);
    if (exception != 0) {
        FF_DEBUG("getDisplayIds raised exception %d", exception);
        return false;
    }
    const int32_t count = ffBinderReadI32(reply.data, reply.size, sizeof(int32_t));
    if (count <= 0 || count > FF_DISPLAY_ANDROID_MAX_DISPLAYS) {
        FF_DEBUG("getDisplayIds answered %d displays", count);
        return false;
    }

    int32_t displayIds[FF_DISPLAY_ANDROID_MAX_DISPLAYS] = {};
    for (int32_t i = 0; i < count; ++i) {
        displayIds[i] = ffBinderReadI32(reply.data, reply.size, (size_t) (2 + i) * sizeof(int32_t));
        if (displayIds[i] < 0) {
            FF_DEBUG("getDisplayIds answered display id %d", displayIds[i]);
            return false;
        }
    }

    // Every display has to parse before any of them is reported, so the records are held here until
    // the loop has been through all of them.
    FFDisplayAndroidInfo infos[FF_DISPLAY_ANDROID_MAX_DISPLAYS] = {};
    int32_t parsed = 0;
    for (; parsed < count; ++parsed) {
        FFDisplayAndroidInfo* info = &infos[parsed];
        ffStrbufInit(&info->name);

        FFBinderParcel request = ffBinderParcelCreate(parcelBuffer, sizeof(parcelBuffer));
        ffBinderParcelPutInterfaceToken(&request, FF_DISPLAY_ANDROID_DESCRIPTOR);
        ffBinderParcelPutI32(&request, displayIds[parsed]);

        reply = ffBinderReplyCreate(replyBuffer, sizeof(replyBuffer));
        error = ffBinderTransact(&binder, service.handle, FF_DISPLAY_ANDROID_TRANSACTION_GET_DISPLAY_INFO, 0, &request, &reply);
        if (error != nullptr) {
            FF_DEBUG("getDisplayInfo(%d) failed: %s", displayIds[parsed], error);
            break;
        }
        if (ffBinderReplyIsStatus(&reply)) {
            FF_DEBUG("getDisplayInfo(%d) answered status %d instead of a parcel", displayIds[parsed], (int) ffBinderReadI32(reply.data, reply.size, 0));
            break;
        }
        if (!displayInfoParse(reply.data, reply.size, &layout, info)) {
            FF_DEBUG("getDisplayInfo(%d) answered a record this parser does not recognise", displayIds[parsed]);
            break;
        }
    }

    if (parsed != count) {
        for (int32_t i = 0; i <= parsed && i < count; ++i) {
            ffStrbufDestroy(&infos[i].name);
        }
        return false;
    }

    for (int32_t i = 0; i < count; ++i) {
        FFDisplayAndroidInfo* info = &infos[i];

        // The physical dpi describes the panel itself and does not change with the logical display
        // size, so the physical size has to be derived from the native resolution. Nothing in the
        // record carries it directly.
        uint32_t physicalWidth = 0, physicalHeight = 0;
        if (info->physicalXDpi > 0) {
            physicalWidth = (uint32_t) ((info->preferredWidth ? info->preferredWidth : info->width) * 25.4 / info->physicalXDpi + 0.5);
        }
        if (info->physicalYDpi > 0) {
            physicalHeight = (uint32_t) ((info->preferredHeight ? info->preferredHeight : info->height) * 25.4 / info->physicalYDpi + 0.5);
        }

        FFDisplayResult* display = ffdsAppendDisplay(ds,
            info->width,
            info->height,
            info->refreshRate,
            info->density * 96 / 160,
            info->preferredWidth,
            info->preferredHeight,
            info->preferredRefreshRate,
            info->rotation,
            &info->name, // moved
            displayType(info->type),
            info->displayId == 0, // Display 0 is the default one
            info->id,
            physicalWidth,
            physicalHeight,
            "binder");
        if (display == nullptr) {
            // `ffdsAppendDisplay` moves `name` only when it keeps the display, so it is still ours
            ffStrbufDestroy(&info->name);
            continue;
        }
        display->manufactureYear = info->manufactureYear;
        display->manufactureWeek = info->manufactureWeek;
        display->hdrStatus = displayHdrStatus(info);
    }
    return ds->displays.length > 0;
}

// Several vendors embed the UI name and its version in `ro.build.display.id` without any
// separator, e.g. `MyOS12.0.14_A2121` or `RedMagicOS10.0.24_NX779J`.
static bool detectDEFromDisplayId(FFDisplayServerResult* ds, const char* const* names, uint32_t count) {
    FF_STRBUF_AUTO_DESTROY displayId = ffStrbufCreate();
    if (!ffSettingsGetAndroidProperty("ro.build.display.id", &displayId)) {
        return false;
    }

    for (uint32_t i = 0; i < count; i++) {
        uint32_t length = (uint32_t) strlen(names[i]);
        if (!ffStrbufStartsWithS(&displayId, names[i])) {
            continue;
        }

        ffStrbufSubstrBeforeFirstC(&displayId, '_'); // Drop the model suffix
        if (displayId.length > length) {
            ffStrbufInsertNC(&displayId, length, 1, ' ');
        }
        ffStrbufSet(&ds->dePrettyName, &displayId);
        return true;
    }

    return false;
}

static bool detectDE(FFDisplayServerResult* ds) {
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY brand = ffStrbufCreate();

    // `ffSettingsGetAndroidProperty` appends to the given buffer, so the brand has to be read
    // into its own buffer exactly once: reading it into a shared buffer repeatedly would
    // concatenate the values and break every comparison against it.
    ffSettingsGetAndroidProperty("ro.product.brand", &brand);

    // vivo reports the marketing name and version in `ro.vivo.os.build.display.id`,
    // separated by an underscore (`Funtouch OS_10`, `OriginOS 5`), and the build number
    // in `ro.vivo.product.version`.
    if (ffSettingsGetAndroidProperty("ro.vivo.os.build.display.id", &ds->dePrettyName)) {
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        if (ffSettingsGetAndroidProperty("ro.vivo.product.version", &buffer)) {
            ffStrbufAppendC(&ds->dePrettyName, ' ');
            ffStrbufAppend(&ds->dePrettyName, &buffer);
        }
        return true;
    }

    // HarmonyOS 2.0 - 4.x is built on top of the Android framework and keeps reporting an
    // EMUI version, but it is HarmonyOS. `ro.build.ohos.devicetype` is set only by those
    // builds, and `hw_sc.build.platform.version` is the HarmonyOS version, while
    // `ro.build.version.emui` only carries the EMUI compatibility version
    // (HarmonyOS 2.0 == EMUI 12, 3.0 == 13, 4.0 == 14, 4.2 == 14.2, 4.3 == 15).
    if (ffSettingsGetAndroidProperty("ro.build.ohos.devicetype", &buffer)) {
        ffStrbufClear(&buffer);
        if (!ffSettingsGetAndroidProperty("hw_sc.build.platform.version", &buffer) &&
            ffSettingsGetAndroidProperty("ro.huawei.build.display.id", &buffer)) {
            // A few builds leave `hw_sc.build.platform.version` unset and only expose the
            // version through `ro.huawei.build.display.id`, e.g. `JKM-AL00 2.0.0.263(C00E260R4P3)`
            ffStrbufSubstrAfterFirstC(&buffer, ' ');
            ffStrbufSubstrBeforeFirstC(&buffer, '(');
            ffStrbufSubstrBeforeLastC(&buffer, '.');
        }
        if (buffer.length > 0) {
            ffStrbufSetF(&ds->dePrettyName, "HarmonyOS %s", buffer.chars);
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "HarmonyOS");
        }
        return true;
    }

    // HarmonyOS NEXT (5.0 and newer) drops the Android framework. It is the only Huawei
    // family reporting `ro.build.display.id` as `System 104.5.0.001(60J9)`, and none of its
    // properties carries the marketing version, so it can only be named without one.
    if (ffStrbufIgnCaseEqualS(&brand, "HUAWEI") &&
        ffSettingsGetAndroidProperty("ro.build.display.id", &buffer) &&
        ffStrbufStartsWithS(&buffer, "System ")) {
        ffStrbufSetStatic(&ds->dePrettyName, "HarmonyOS NEXT");
        return true;
    }

    // HONOR reports MagicOS / MagicUI in `ro.build.version.magic`. MagicUI 3.x stores a
    // bare version number (`3.0.1`) instead of a `MagicUI_x.y.z` string.
    if (ffSettingsGetAndroidProperty("ro.build.version.magic", &ds->dePrettyName)) {
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        if (!ffStrbufStartsWithS(&ds->dePrettyName, "Magic")) {
            ffStrbufPrependS(&ds->dePrettyName, "MagicUI ");
        }
        return true;
    }

    if (ffSettingsGetAndroidProperty("ro.build.version.emui", &ds->dePrettyName)) {
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        return true;
    }

    // Xiaomi HyperOS. `ro.mi.os.version.incremental` is `OS1.0.10.0.TLDCNXM`, while
    // `ro.build.version.incremental` still starts with `V816` on HyperOS 1.0, so it can
    // not be used to tell HyperOS and MIUI apart.
    if (ffSettingsGetAndroidProperty("ro.mi.os.version.name", &ds->dePrettyName)) {
        ffStrbufClear(&ds->dePrettyName);
        if (ffSettingsGetAndroidProperty("ro.mi.os.version.incremental", &ds->dePrettyName) &&
            ffStrbufStartsWithS(&ds->dePrettyName, "OS")) {
            ffStrbufSubstrAfter(&ds->dePrettyName, 1); // Drop the leading "OS"
            ffStrbufPrependS(&ds->dePrettyName, "HyperOS ");
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "HyperOS");
        }
        return true;
    }

    // Black Shark runs JOYUI, a MIUI fork, and therefore also sets
    // `ro.miui.ui.version.name`. `ro.build.version.incremental` is `V11.0.4.0.JOYUI`.
    if (ffStrbufIgnCaseEqualS(&brand, "blackshark")) {
        ffStrbufClear(&ds->dePrettyName);
        if (ffSettingsGetAndroidProperty("ro.build.version.incremental", &ds->dePrettyName)) {
            ffStrbufSubstrBeforeLastC(&ds->dePrettyName, '.'); // Drop the trailing "JOYUI"
            if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
                ffStrbufSubstrAfter(&ds->dePrettyName, 0);
            }
            ffStrbufPrependS(&ds->dePrettyName, "JOYUI ");
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "JOYUI");
        }
        return true;
    }

    // MIUI. `ro.build.version.incremental` is `V14.0.1.0.TJJCNXM` on stable builds and a
    // bare release date (`21.11.17`) on beta builds.
    if (ffSettingsGetAndroidProperty("ro.miui.ui.version.name", &buffer)) {
        ffStrbufClear(&ds->dePrettyName);
        if (ffSettingsGetAndroidProperty("ro.build.version.incremental", &ds->dePrettyName)) {
            if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
                ffStrbufSubstrAfter(&ds->dePrettyName, 0);
            }
            ffStrbufPrependS(&ds->dePrettyName, "MiUI ");
        } else {
            ffStrbufSetStatic(&ds->dePrettyName, "MiUI");
        }
        return true;
    }

    // realme UI is a ColorOS fork and reports both `ro.build.version.realmeui` and
    // `ro.build.version.oplusrom`; the former is the realme UI version.
    if (ffSettingsGetAndroidProperty("ro.build.version.realmeui", &ds->dePrettyName)) {
        if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
            ffStrbufSubstrAfter(&ds->dePrettyName, 0);
        }
        ffStrbufPrependS(&ds->dePrettyName, "realme UI ");
        return true;
    }

    // ColorOS 12 and newer report `ro.build.version.oplusrom`, ColorOS 11 and older
    // report `ro.build.version.opporom`.
    if (ffSettingsGetAndroidProperty("ro.build.version.oplusrom", &ds->dePrettyName) ||
        ffSettingsGetAndroidProperty("ro.build.version.opporom", &ds->dePrettyName)) {
        if (ffStrbufStartsWithS(&ds->dePrettyName, "V")) {
            ffStrbufSubstrAfter(&ds->dePrettyName, 0);
        }
        ffStrbufPrependS(&ds->dePrettyName, "ColorOS ");
        return true;
    }

    if (ffSettingsGetAndroidProperty("ro.oxygen.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "OxygenOS ");
        return true;
    }

    // HydrogenOS is the Chinese counterpart of OxygenOS and reports `ro.rom.version`.
    if (ffStrbufEqualS(&brand, "OnePlus") &&
        ffSettingsGetAndroidProperty("ro.rom.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "H2OS ");
        return true;
    }

    if (ffSettingsGetAndroidProperty("ro.build.version.oneui", &ds->dePrettyName)) {
        // [ro.build.version.oneui]: [50101] => One UI 5.1.1
        // Samsung encodes the version with a base-100 carry: `major * 10000 + minor * 100 + patch`
        uint32_t version = (uint32_t) ffStrbufToUInt(&ds->dePrettyName, 0);
        uint32_t major = version / 10000;
        uint32_t minor = version / 100 % 100;
        uint32_t patch = version % 100;
        if (major == 0) {
            // Unexpected format, show the raw value
            ffStrbufPrependS(&ds->dePrettyName, "OneUI ");
        } else if (patch > 0) {
            ffStrbufSetF(&ds->dePrettyName, "OneUI %u.%u.%u", major, minor, patch);
        } else {
            ffStrbufSetF(&ds->dePrettyName, "OneUI %u.%u", major, minor);
        }
        return true;
    }

    // Flyme. `ro.build.display.id` is `Flyme 10.5.0.1A`, where the trailing `A` marks a
    // stable release. `ro.flyme.version.id` holds the same string, except on Flyme 12,
    // where it is an Android build id instead.
    if (ffStrbufIgnCaseEqualS(&brand, "meizu")) {
        if (!ffSettingsGetAndroidProperty("ro.build.display.id", &ds->dePrettyName) ||
            !ffStrbufStartsWithS(&ds->dePrettyName, "Flyme")) {
            ffStrbufSetStatic(&ds->dePrettyName, "Flyme");
        }
        return true;
    }

    // SmartisanOS. `ro.smartisan.version` is `4.2.6-201808311713-user-511` or
    // `6.6.6.2_TNT-201904101033-user-oce`.
    if (ffSettingsGetAndroidProperty("ro.smartisan.version", &ds->dePrettyName)) {
        ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '_');
        ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '-');
        ffStrbufPrependS(&ds->dePrettyName, "SmartisanOS ");
        return true;
    }

    // ZUI / ZUXOS. `ro.com.zui.version` is the internal ZUI version (`17.0` for
    // ZUXOS 1.1.10.138), while `ro.build.display.id` embeds the marketing name and
    // version: `TB321FU_CN_OPEN_USER_Q00011.0_V_ZUXOS_1.1.10.138_ST_250626`.
    if (ffSettingsGetAndroidProperty("ro.com.zui.version", &ds->dePrettyName)) {
        FF_STRBUF_AUTO_DESTROY displayId = ffStrbufCreate();
        const char* name = nullptr;
        if (ffSettingsGetAndroidProperty("ro.build.display.id", &displayId)) {
            if (ffStrbufSubstrAfterFirstS(&displayId, "_ZUXOS_")) {
                name = "ZUXOS";
            } else if (ffStrbufSubstrAfterFirstS(&displayId, "_ZUI_")) {
                name = "ZUI";
            }
            if (name) {
                ffStrbufSubstrBeforeFirstC(&displayId, '_'); // Drop the trailing "_ST_250626"
                ffStrbufSetF(&ds->dePrettyName, "%s %s", name, displayId.chars);
                return true;
            }
        }
        ffStrbufPrependS(&ds->dePrettyName, "ZUI "); // Moto builds only expose `ro.com.zui.version`
        return true;
    }

    // 360 OS (QiKU) reports `ro.build.uiversion` as `360UI:V3.0`.
    if (ffSettingsGetAndroidProperty("ro.build.uiversion", &ds->dePrettyName)) {
        uint32_t index = ffStrbufFirstIndexC(&ds->dePrettyName, ':');
        if (index < ds->dePrettyName.length) {
            // `360UI:V3.0` -> `360UI 3.0`
            ffStrbufRemoveSubstr(&ds->dePrettyName, index, index + 1);
            if (ds->dePrettyName.chars[index] == 'V') {
                ffStrbufRemoveSubstr(&ds->dePrettyName, index, index + 1);
            }
            ffStrbufInsertNC(&ds->dePrettyName, index, 1, ' ');
        }
        return true;
    }

    // LeEco EUI reports `ro.letv.release.version` as `6.0.030S`, where the trailing `S`
    // marks a stable release.
    if (ffSettingsGetAndroidProperty("ro.letv.release.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "EUI ");
        return true;
    }

    // nubia / ZTE
    if (ffStrbufIgnCaseEqualS(&brand, "nubia") || ffStrbufIgnCaseEqualS(&brand, "zte")) {
        // ObricUI reports `ro.os.ota.version`, e.g.
        // `1.8.0.2-20260204-125753-RELEASE-user-pacific-b911`
        if (ffSettingsGetAndroidProperty("ro.os.ota.version", &ds->dePrettyName)) {
            ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '-');
            ffStrbufPrependS(&ds->dePrettyName, "ObricUI ");
            return true;
        }

        // MyOS, NebulaAIOS and RedMagicOS embed the name and version in `ro.build.display.id`
        static const char* const displayIdNames[] = { "RedMagicOS", "NebulaAIOS", "MyOS" };
        if (detectDEFromDisplayId(ds, displayIdNames, ARRAY_SIZE(displayIdNames))) {
            return true;
        }

        // The older nubiaUI stores them in `ro.build.nubia.rom.name` and `.code`
        if (ffSettingsGetAndroidProperty("ro.build.nubia.rom.name", &ds->dePrettyName)) {
            if (ffSettingsGetAndroidProperty("ro.build.nubia.rom.code", &buffer)) {
                if (ffStrbufStartsWithS(&buffer, "V")) {
                    ffStrbufSubstrAfter(&buffer, 0); // `V1.0` -> `1.0`
                }
                ffStrbufAppendC(&ds->dePrettyName, ' ');
                ffStrbufAppend(&ds->dePrettyName, &buffer);
            }
            return true;
        }

        // MiFavor is the UI of the pre-MyOS ZTE phones; `ro.build.MiFavor_version` is a
        // bare version number (`4.0`). Note that MyOS, NebulaAIOS and RedMagicOS reuse
        // this property for their own version, so they are checked first.
        if (ffSettingsGetAndroidProperty("ro.build.MiFavor_version", &ds->dePrettyName)) {
            ffStrbufPrependS(&ds->dePrettyName, "MiFavor ");
            return true;
        }
    }

    // LineageOS and other AOSP-based distributions
    if (ffSettingsGetAndroidProperty("ro.lineage.build.version", &ds->dePrettyName)) {
        ffStrbufPrependS(&ds->dePrettyName, "LineageOS ");
        return true;
    }
    if (ffSettingsGetAndroidProperty("org.pixelexperience.version.display", &ds->dePrettyName)) {
        // PixelExperience_Plus_whyred-13.0-20230325-0421-OFFICIAL
        ffStrbufSubstrBeforeFirstC(&ds->dePrettyName, '-');
        ffStrbufSubstrBeforeLastC(&ds->dePrettyName, '_');
        ffStrbufReplaceAllC(&ds->dePrettyName, '_', ' ');
        return true;
    }

    if (ffStrbufEqualS(&brand, "asus") &&
        ffSettingsGetAndroidProperty("ro.build.version.incremental", &ds->dePrettyName)) {
        return true;
    }
    if (ffSettingsGetAndroidProperty("ro.build.display.id", &ds->dePrettyName)) {
        // Google Pixel and other devices running native Android
        return true;
    }

    return false;
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds) {
    // Keep the same display-server preference as Linux: Wayland provides the
    // richest output information, followed by XCB and XRandR.
    if (instance.config.general.dsForceDrm == FF_DS_FORCE_DRM_TYPE_FALSE) {
        ffdsConnectWayland(ds);

        if (ds->displays.length == 0) {
            ffdsConnectXcbRandr(ds);
        }

        if (ds->displays.length == 0) {
            ffdsConnectXrandr(ds);
        }

        if (ds->displays.length > 0) {
            ffdsDetectWMDE(ds);
            return;
        }
    }

    // https://source.android.com/docs/core/graphics/surfaceflinger-windowmanager
    ffStrbufSetStatic(&ds->wmProcessName, "system_server");
    ffStrbufSetStatic(&ds->wmPrettyName, "WindowManager"); // A system service managed by system_server
    ffStrbufSetStatic(&ds->wmProtocolName, FF_WM_PROTOCOL_SURFACEFLINGER);

    detectWithBinder(ds);

    detectDE(ds);
}
