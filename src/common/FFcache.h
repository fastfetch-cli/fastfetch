#pragma once

#include "fastfetch.h"

// A cache for a value that is built at most once per generation.
//
// The value itself is owned by the entry owner (usually a file scope `static`); the entry only
// remembers whether that value is still valid and knows how to drop it. `--dynamic-interval`
// starts a new generation for every round, dropping all the values that have been built so far,
// so the modules re-detect them instead of replaying the first round's snapshot.
//
// An entry registers itself the first time it is used. There is therefore no central list to keep
// in sync, and a cache cannot be forgotten in it. Do not use this type for state that is carried
// across calls to derive a rate (cpuusage, diskio, netio, top): that is a baseline rather than a
// snapshot, and dropping it makes the first sample of a round read as zero.
//
// The storage pointed to by `storage` must stay valid for the whole run; only the value behind it
// is dropped by an invalidation.
//
// Not thread safe: entries are only touched from a single thread, on a frame boundary, while the
// modules are not running.
typedef struct FFcacheEntry {
    const char* name;               // For diagnostics.
    void* storage;                  // Owned by the entry owner. May be nullptr.
    void (*init)(void* storage);    // Optional. Runs whenever the entry is stale.
    void (*destroy)(void* storage); // Optional. Must be idempotent, and tolerate a partially initialized storage.
    bool initialized;
    bool registered;
    struct FFcacheEntry* next; // Internal: registry link.
} FFcacheEntry;

// Returns `entry->storage`, running `init` on it first when the entry is stale. The entry is marked
// initialized before `init` runs, so a call made from within `init` returns the partially built
// value instead of recursing.
[[gnu::nonnull(1)]] void* ffCacheGet(FFcacheEntry* entry);

// Staleness test for entries whose initialization needs arguments that are not part of the cache
// identity (such as the cover file request of `ffDetectMedia`) and therefore cannot live behind
// `init(void*)`. Returns true exactly once per generation: the caller must then build the value
// itself.
[[gnu::nonnull(1), nodiscard]] bool ffCacheBeginInit(FFcacheEntry* entry);

// Drops the cached value of one entry, so that the next request builds it again. Idempotent, and
// safe on an entry that has never been used.
[[gnu::nonnull(1)]] void ffCacheInvalidate(FFcacheEntry* entry);

// Drops every value that has been built so far. Called once per `--dynamic-interval` round.
void ffCacheInvalidateAll(void);
