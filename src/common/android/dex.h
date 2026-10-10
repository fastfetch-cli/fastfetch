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
