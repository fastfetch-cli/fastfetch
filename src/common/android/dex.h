#pragma once

// AIDL gives every method of an interface a transaction code at build time:
//
//     static final int TRANSACTION_<method> = IBinder.FIRST_CALL_TRANSACTION + <declaration index>;
//
// so the number is a property of the `.aidl` of the release that built the device's jar, not of any
// public header. Counting the methods in an AOSP checkout only describes that checkout, and it goes
// wrong the moment a release or a vendor fork inserts a method ahead of the one being looked for:
// `IWifiManager.getConnectionInfo` is 29 on Android 11 and 41 on Android 16. There is no negotiation
// and no discovery -- a wrong code reaches a different method, or none.
//
// Being a compile-time constant, the value lands in the `static_values` array of the `X$Stub` class
// in the jar's dex, paired position by position with that class's static field list. Reading the two
// together gives the mapping of the build that is actually on the device, on any release, with
// nothing to keep in sync.
//
// `libdexfile.so` cannot do this for us. It is dlopen-able from an app UID through the ART apex, but
// it is not in the NDK and its only C ABI -- the `ADexFile_*` family -- covers methods, not fields;
// the field and class-data APIs are C++ with no ABI guarantee.
//
// `ffReadFileBuffer` would work but pulls the whole jar through the heap for one integer. The jar is
// mapped instead, and only the pages actually read are faulted in.

#include "fastfetch.h"

// What a request is written when the jar does not answer it, on every path out of ffDexStaticInts --
// a jar that cannot be opened and a field the class does not declare both leave it behind, so a
// caller reads a result rather than what its own stack happened to hold there. No transaction code
// is negative: AIDL numbers them from `IBinder.FIRST_CALL_TRANSACTION`, which is 1.
//
// This is what keeps the reader a lookup and nothing more. Whether a code is one the caller cannot do
// without or one it only uses when the build declares it, whether what came back is enough to carry
// on, and whether the module reports a value or an error are the caller's decisions, made by reading
// the results. The reader finds numbers and says which ones it could not.
#define FF_DEX_STATIC_INT_UNRESOLVED (-1)

// One `<classDescriptor>.<fieldName>` lookup, and where its value is written. `classDescriptor` is
// the dex type descriptor, e.g. "Landroid/net/wifi/IWifiManager$Stub;".
typedef struct FFDexStaticIntRequest {
    const char* classDescriptor;
    const char* fieldName;
    int32_t* result;
} FFDexStaticIntRequest;

// Resolves the int value of every request out of the dex entries of `jarPath`, walking `classes.dex`,
// `classes2.dex`, ... in order until one of them defines the class.
//
// Asking for several fields at once is not a convenience. A lookup is dominated by the type table,
// which lists every type the dex references -- 8428 descriptors in `framework.jar`'s `classes.dex`,
// each one read from a different part of a 51 MB mapping -- and a single pass answers every request
// whose class is named in it. Measured over three devices' own `framework.jar`, four fields of two
// classes cost one pass rather than four.
//
// Every request is written a code or FF_DEX_STATIC_INT_UNRESOLVED. A request is never allowed to
// take the others down with it: the classes a caller asks for are looked for independently, and one
// the jar does not declare leaves its own result at the sentinel while the rest are answered.
//
// Not finding a code is not a failure. A jar that is readable answers the call, whether it declares
// every field asked for, some of them, or none -- the results say which, and what that means for the
// module is the caller's to decide. The return value is therefore reserved for the one thing the
// caller cannot read out of the results: the jar itself, which is reported when it cannot be opened
// or does not hold a dex that can be read. nullptr means the lookup ran.
[[gnu::nonnull(1, 2), nodiscard]] const char* ffDexStaticInts(const char* jarPath, const FFDexStaticIntRequest* requests, uint32_t count);

// ---------------------------------------------------------------------------------------------
// Reading a Parcelable's wire layout
// ---------------------------------------------------------------------------------------------
//
// A Parcelable's `writeToParcel` puts its fields on the wire in the order the method reads them, and
// that order is the only thing that says where each one is. It is not in the class's declaration: a
// dex stores a class's instance fields sorted by field index, which is sorted by *name*, so the
// declaration order a reader would want is not recorded anywhere but the method body.
//
// Recovering it therefore means walking the bytecode. The compiler emits
//
//     iget v0, p0, LDisplayInfo;->logicalWidth:I
//     invoke-virtual {p1, v0}, Landroid/os/Parcel;->writeInt(I)V
//
// once per field, in write order, so the `iget` opcodes in a linear pass over the instruction grid
// are the sequence. Two things have to be got right for that pass to stay on the grid, and both are
// easy to get wrong in a way that still produces a plausible-looking list:
//
//  * Instructions are 1, 2, 3, 4 or 5 code units wide, and the width is a property of the opcode.
//    The table below is ART's `Instruction::SizeInCodeUnits`.
//  * The payload pseudo-instructions a `switch` or a `fill-array-data` leaves in the stream have a
//    low byte of zero and a high byte of 1, 2 or 3. They are not instructions, they sit behind every
//    reachable instruction, and stepping into one desynchronises the grid -- so the walk stops at the
//    first of them rather than trying to step over it.
//
// An array field is read twice: once for its `length`, which is the count that goes on the wire, and
// again inside the loop that writes the elements. A class cannot declare two instance fields under
// one name, so a repeat is always that reload, and the first occurrence is where the field is
// written. The reader de-duplicates by name for that reason.
//
// What is *not* recovered is how each field is framed -- `writeString` against `writeString8`,
// `writeParcelable` against `writeTypedObject`. That is a property of the method the field is handed
// to, not of the field, and two `String` fields of the same class can differ: `DisplayInfo.name` goes
// through `writeString8` while `DeviceProductInfo.mName` goes through `writeString`. Telling them
// apart would mean resolving every invoke target, so the type descriptor is reported instead and the
// caller frames the fields its descriptor settles -- an `I`, a `J`, an `[F` -- and names the few it
// does not. Measured over three releases' own `framework.jar`, that is four names per class.
//
// The point of asking the device rather than carrying a table is that the field set moves: `DisplayInfo`
// has 34 instance fields on Android 11, 42 on 13 and 59 on 16, and the fields that arrived in between
// (`displayGroupId`, `renderFrameRate`, `appsSupportedModes`, `committedState`, ...) are exactly the
// ones a parser written against one release reads out of the middle of another. Reading the sequence
// from the build that is running makes every one of them optional by construction.

// A field name and a type descriptor are ASCII and short -- the longest name in `DisplayInfo` is 28
// bytes and the longest descriptor `Landroid/hardware/display/DeviceProductInfo$ManufactureDate;` is
// 60. A longer one is a class this reader was not written for, and is reported as unresolved rather
// than truncated: half a name would silently match nothing, which is indistinguishable from the field
// not being there.
#define FF_DEX_PARCEL_FIELD_NAME_MAX 40
#define FF_DEX_PARCEL_FIELD_TYPE_MAX 72

// One field of the write sequence. `type` is the dex type descriptor, e.g. "I", "[F",
// "Ljava/lang/String;" or "Landroid/view/DisplayAddress;".
typedef struct FFDexParcelField {
    char name[FF_DEX_PARCEL_FIELD_NAME_MAX];
    char type[FF_DEX_PARCEL_FIELD_TYPE_MAX];
} FFDexParcelField;

// One `<classDescriptor>.<methodName>` write sequence, and where it is written.
typedef struct FFDexParcelRequest {
    const char* classDescriptor;
    const char* methodName;
    FFDexParcelField* fields;
    uint32_t capacity;
    uint32_t* count; // how many fields the sequence holds, or 0 when the jar cannot answer
} FFDexParcelRequest;

// Resolves the write sequence of every request out of the dex entries of `jarPath`, walking
// `classes.dex`, `classes2.dex`, ... in order until one of them defines the class -- the same walk,
// and the same one pass over the type table, that ffDexStaticInts makes.
//
// Every request is written a count or 0. A class the jar does not declare, a method it does not
// declare, and a sequence longer than the caller's `capacity` all leave 0 behind, and none of them
// holds back the others: they are independent questions about the same pass. A count of 0 is not a
// failure but an answer -- this build does not write that class, so a caller reading it has nothing
// to walk. The return value is reserved for the jar itself, as in ffDexStaticInts.
[[gnu::nonnull(1, 2), nodiscard]] const char* ffDexParcelFields(const char* jarPath, const FFDexParcelRequest* requests, uint32_t count);

// Whether `<classDescriptor>` declares an instance field named `<fieldName>`.
//
// This is the question a Parcelable whose layout is not field-driven needs. `DisplayCutout` is one:
// its `writeCutoutToParcel` reads everything through `-$$Nest$` accessors the compiler synthesised,
// so there is no `iget` to walk and no class name on the wire either. What does say how long its body
// is, is which fields the class carries -- Android 11 declares three, Android 13 adds
// `mCutoutPathParserInfo` and Android 16 adds `mSideOverrides`, and each one adds a known run of
// writes behind the three both have. The declaration is a field set rather than an order, which is
// exactly what this answers.
typedef struct FFDexInstanceFieldRequest {
    const char* classDescriptor;
    const char* fieldName;
    bool* result;
} FFDexInstanceFieldRequest;

// Writes `false` for every request the jar cannot answer, on every path out -- a class the jar does
// not declare and a field the class does not declare are the same answer here, because both mean the
// field is not in the parcel. The return value is reserved for the jar itself.
[[gnu::nonnull(1, 2), nodiscard]] const char* ffDexInstanceFields(const char* jarPath, const FFDexInstanceFieldRequest* requests, uint32_t count);
