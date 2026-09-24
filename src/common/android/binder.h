#pragma once

// Android exposes no public C API for the system services that carry the data fastfetch wants: the
// NDK has no BatteryManager / WifiManager, libbinder_ndk only speaks to AIDL HALs (it refuses to
// prepare a transaction for a binder that has no NDK class), and `/system/bin/service call` costs a
// fork/exec worth 15-20 ms. What is left is /dev/binder itself, which is all that `service` uses
// underneath. This header speaks that protocol directly: open() + mmap() + BINDER_WRITE_READ, no
// extra shared library, no child process, ~0.13 ms per call once a handle is cached.
//
// Three details are easy to get wrong. All three were confirmed against the device's own
// libbinder.so and with a ptrace trace of `service`, and getting any of them wrong makes the
// service answer BAD_TYPE (0x80000001) or BR_FAILED_REPLY (0x7211):
//
//   * A request parcel must start with three int32 values -- strict mode policy, work source and
//     'SYST'. Parcel::markForBinder() writes nothing for kernel binder, but the receiving side's
//     Parcel::enforceInterface() still compares the third word against its own header.
//   * Parcel::writeString16() writes the length, the UTF-16 code units, a UTF-16 NUL terminator and
//     then pads to four bytes. Skipping the terminator shifts every following field by four bytes.
//   * A handle arriving in a reply is owned by that reply buffer. It has to be acquired
//     (BC_ACQUIRE + BC_INCREFS) before BC_FREE_BUFFER; otherwise only a weak reference survives and
//     every later transaction on it fails, because the kernel looks up a strong reference.
//
// See .workbuddy-ai/android-binder-raw-client.md for the full write-up and the tooling.

#include "fastfetch.h" // IWYU pragma: keep

// The structures and command words below are not defined here: bionic generates this header from
// bionic/libc/kernel/uapi/linux/android/binder.h and every NDK ships it, so it is the
// authoritative copy of the binder ABI -- struct binder_write_read, struct binder_transaction_data,
// struct flat_binder_object, BINDER_WRITE_READ / BINDER_VERSION, the BC_* and BR_* command words,
// TF_* and BINDER_TYPE_*. Using it beats transcribing it: a second copy can only be wrong, and if
// the header itself is wrong that is the NDK's problem to fix, not something fastfetch could have
// caught by keeping its own.
#include <linux/android/binder.h>

#include <string.h>
#include <sys/ioctl.h>

#define FF_BINDER_DEVICE "/dev/binder"

// The kernel allocates reply payloads out of the region handed to mmap(); 1 MiB is what libbinder
// asks for. Replies themselves are small -- `listServices` on this device is the largest at a few
// hundred bytes -- but the region also carries the per-process buffer pool.
#define FF_BINDER_SHARED_SIZE (1024 * 1024)

// A command word plus the binder_transaction_data that follows it. The payload never lands here:
// it lives in the mmap region, and BR_REPLY only hands over a pointer to it.
#define FF_BINDER_REPLY_BUFFER_SIZE 1024

// How many handles a reply may transfer to us. Replies that carry more than this still succeed, but
// the surplus handles are left unacquired and will fail on first use.
#define FF_BINDER_MAX_HANDLES 4

// How many file descriptors a reply may hand over. A descriptor is not a handle: the kernel installs
// it in the caller's own fd table as it delivers the reply, so it is owned rather than borrowed and
// the caller closes it. A reply carrying more than this still succeeds, and the surplus is closed
// again by ffBinderTransact() because nothing would be left holding it.
#define FF_BINDER_MAX_FDS 4

// The three int32 values Parcel::writeInterfaceToken() puts in front of the descriptor, and that
// Parcel::enforceInterface() checks the last of. These come from libbinder rather than from the
// kernel, which is why they are spelled out here.
#define FF_BINDER_STRICT_MODE_PENALTY_GATHER 0x80000000u // (1 << 31), Parcel.cpp
#define FF_BINDER_UNSET_WORK_SOURCE 0xffffffffu          // IPCThreadState::kUnsetWorkSource
#define FF_BINDER_HEADER 0x53595354u                     // B_PACK_CHARS('S', 'Y', 'S', 'T')

typedef struct FFBinder {
    int fd;
    uint8_t* shared;
    size_t sharedSize;
    int32_t protocolVersion;
} FFBinder;

// Opens /dev/binder, checks the protocol version and maps the shared region. Returns nullptr on
// success, a static message otherwise; the device is left closed on failure.
[[gnu::nonnull(1), nodiscard]] const char* ffBinderOpen(FFBinder* binder);

// Usable as a cleanup attribute. Safe on a zeroed or already closed FFBinder.
[[gnu::nonnull(1)]] void ffBinderClose(FFBinder* binder);

// ---------------------------------------------------------------------------------------------
// Parcel writing. The caller owns the buffer; anything that does not fit is dropped and recorded
// in `truncated`, which ffBinderTransact() then reports as an error.
// ---------------------------------------------------------------------------------------------

typedef struct FFBinderParcel {
    uint8_t* data;
    size_t capacity;
    size_t size;
    bool truncated;
} FFBinderParcel;

[[nodiscard]] static inline FFBinderParcel ffBinderParcelCreate(uint8_t* data, size_t capacity) {
    return (FFBinderParcel) { .data = data, .capacity = capacity, .size = 0, .truncated = false };
}

[[gnu::nonnull(1), nodiscard]] static inline uint8_t* ffBinderParcelReserve(FFBinderParcel* parcel, size_t bytes) {
    if (bytes > parcel->capacity - parcel->size) {
        parcel->truncated = true;
        return nullptr;
    }
    uint8_t* result = parcel->data + parcel->size;
    parcel->size += bytes;
    return result;
}

[[gnu::nonnull(1)]] static inline void ffBinderParcelPutU32(FFBinderParcel* parcel, uint32_t value) {
    uint8_t* dst = ffBinderParcelReserve(parcel, sizeof(uint32_t));
    if (dst != nullptr) {
        memcpy(dst, &value, sizeof(uint32_t));
    }
}

[[gnu::nonnull(1)]] static inline void ffBinderParcelPutI32(FFBinderParcel* parcel, int32_t value) {
    uint8_t* dst = ffBinderParcelReserve(parcel, sizeof(int32_t));
    if (dst != nullptr) {
        memcpy(dst, &value, sizeof(int32_t));
    }
}

[[gnu::nonnull(1)]] static inline void ffBinderParcelPutU64(FFBinderParcel* parcel, uint64_t value) {
    uint8_t* dst = ffBinderParcelReserve(parcel, sizeof(uint64_t));
    if (dst != nullptr) {
        memcpy(dst, &value, sizeof(uint64_t));
    }
}

// AIDL string16. The descriptors and service names fastfetch asks for are ASCII, so one byte is one
// UTF-16 code unit; anything else would have to be encoded properly here.
[[gnu::nonnull(1, 2)]] static inline void ffBinderParcelPutString16(FFBinderParcel* parcel, const char* value) {
    const size_t length = strlen(value);
    const size_t payload = sizeof(uint32_t) + (length + 1) * 2; // length, code units, NUL terminator
    const size_t padded = (payload + 3) & ~(size_t) 3;
    uint8_t* dst = ffBinderParcelReserve(parcel, padded);
    if (dst == nullptr) {
        return;
    }

    memset(dst, 0, padded);
    const uint32_t length32 = (uint32_t) length;
    memcpy(dst, &length32, sizeof(uint32_t));
    for (size_t i = 0; i < length; i++) {
        dst[sizeof(uint32_t) + i * 2] = (uint8_t) value[i];
    }
    // The two bytes after the last code unit stay zero: that is the UTF-16 NUL terminator, and
    // Parcel::readInplace() advances by (length + 1) * sizeof(char16_t) to account for it.
}

[[gnu::nonnull(1, 2)]] static inline void ffBinderParcelPutInterfaceToken(FFBinderParcel* parcel, const char* descriptor) {
    ffBinderParcelPutU32(parcel, FF_BINDER_STRICT_MODE_PENALTY_GATHER);
    ffBinderParcelPutU32(parcel, FF_BINDER_UNSET_WORK_SOURCE);
    ffBinderParcelPutU32(parcel, FF_BINDER_HEADER);
    ffBinderParcelPutString16(parcel, descriptor);
}

// ---------------------------------------------------------------------------------------------
// Transactions
// ---------------------------------------------------------------------------------------------

typedef struct FFBinderReply {
    uint8_t* data;    // caller owned; receives the payload
    size_t capacity;  // ditto
    size_t size;      // payload length in bytes
    uint32_t code;    // transaction code the service echoed back
    uint32_t flags;   // TF_* of the reply
    uint32_t handleCount;
    uint32_t handles[FF_BINDER_MAX_HANDLES];
    uint32_t fdCount;
    int32_t fds[FF_BINDER_MAX_FDS]; // owned by the caller, which closes them
} FFBinderReply;

[[nodiscard]] static inline FFBinderReply ffBinderReplyCreate(uint8_t* data, size_t capacity) {
    return (FFBinderReply) { .data = data, .capacity = capacity, .size = 0, .code = 0, .flags = 0, .handleCount = 0, .handles = {}, .fdCount = 0, .fds = {} };
}

// A reply carrying TF_STATUS_CODE is not an AIDL parcel at all: the first four bytes are the raw
// status_t that onTransact() returned (-74 being UNKNOWN_TRANSACTION, i.e. no such method). Check
// this before interpreting anything else.
[[gnu::nonnull(1), nodiscard]] static inline bool ffBinderReplyIsStatus(const FFBinderReply* reply) {
    return (reply->flags & TF_STATUS_CODE) != 0;
}

[[gnu::nonnull(1), nodiscard]] static inline uint32_t ffBinderReadU32(const uint8_t* data, size_t size, size_t offset) {
    uint32_t value = 0;
    if (offset + sizeof(uint32_t) <= size) {
        memcpy(&value, data + offset, sizeof(uint32_t));
    }
    return value;
}

[[gnu::nonnull(1), nodiscard]] static inline int32_t ffBinderReadI32(const uint8_t* data, size_t size, size_t offset) {
    int32_t value = 0;
    if (offset + sizeof(int32_t) <= size) {
        memcpy(&value, data + offset, sizeof(int32_t));
    }
    return value;
}

[[gnu::nonnull(1), nodiscard]] static inline uint64_t ffBinderReadU64(const uint8_t* data, size_t size, size_t offset) {
    uint64_t value = 0;
    if (offset + sizeof(uint64_t) <= size) {
        memcpy(&value, data + offset, sizeof(uint64_t));
    }
    return value;
}

// Synchronous transaction. Returns nullptr on success, a static message otherwise.
//
// `flags` carries the TF_* values for the request, and one of them is not cosmetic: a reply that
// carries a file descriptor is only delivered at all when the request asked with TF_ACCEPT_FDS. A
// request that did not gets BR_FAILED_REPLY instead, which reads like a service that is not running
// rather than like a missing flag. TF_ONE_WAY is the other one the kernel acts on, and it is what
// makes a call fire-and-forget -- there is then no reply to fill `reply` from.
//
// Handles the reply carries are acquired and added to `reply->handles` before the reply buffer is
// freed; descriptors are added to `reply->fds`, which the caller owns and closes.
//
// `handle` 0 is the service manager; every other handle comes from ffBinderLookupService(). A
// service that never replies blocks this call forever -- none of the services fastfetch reads does,
// but there is no timeout here to fall back on.
[[gnu::nonnull(1, 5, 6), nodiscard]] const char* ffBinderTransact(FFBinder* binder, uint32_t handle, uint32_t code, uint32_t flags, const FFBinderParcel* parcel, FFBinderReply* reply);

// ---------------------------------------------------------------------------------------------
// Service manager
// ---------------------------------------------------------------------------------------------

#define FF_BINDER_SERVICE_MANAGER_HANDLE 0u
#define FF_BINDER_SM_DESCRIPTOR "android.os.IServiceManager"

// Entry points of android.os.IServiceManager, as this device answers them. The numbers do not match
// the AOSP sources: checkService is 4 here, not 3. FF_BINDER_SM_GET_SERVICE returns a nullable
// IBinder and is the simplest reply to parse; FF_BINDER_SM_CHECK_SERVICE is what `service` itself
// calls and answers with a parcelable instead.
#define FF_BINDER_SM_GET_SERVICE 1u
#define FF_BINDER_SM_GET_SERVICE2 2u
#define FF_BINDER_SM_CHECK_SERVICE 4u
#define FF_BINDER_SM_LIST_SERVICES 6u

// Resolves a service name to a handle through the service manager. Returns nullptr on success.
[[gnu::nonnull(1, 2, 4), nodiscard]] const char* ffBinderLookupService(FFBinder* binder, const char* name, uint32_t transactionCode, uint32_t* handle);

// A handle resolved through the service manager, held together with the binder it came from so that a
// single cleanup attribute can give it back on every path out of the function that took it. Zeroed --
// or left at handle 0 by a lookup that has not run or did not succeed -- it is a no-op, which is what
// makes it safe to declare above the lookup itself.
typedef struct FFBinderServiceHandle {
    FFBinder* binder;
    uint32_t handle;
} FFBinderServiceHandle;

// Returns the strong and weak references ffBinderLookupService() acquired on the handle. Usable as a
// cleanup attribute; safe on a zeroed or already released FFBinderServiceHandle.
[[gnu::nonnull(1)]] void ffBinderServiceHandleRelease(FFBinderServiceHandle* service);
