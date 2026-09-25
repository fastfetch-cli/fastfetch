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

// Resolves the int value of `<classDescriptor>.<fieldName>` out of the `classes.dex` of `jarPath`.
// `classDescriptor` is the dex type descriptor, e.g. "Landroid/net/wifi/IWifiManager$Stub;".
// Returns nullptr on success, a static message otherwise.
[[gnu::nonnull(1, 2, 3, 4), nodiscard]] const char* ffDexStaticInt(const char* jarPath, const char* classDescriptor, const char* fieldName, int32_t* result);
