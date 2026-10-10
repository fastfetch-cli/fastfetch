#include "common/FFcache.h"

// Entries are file scope statics owned by the cache owner, so they can be linked into a list
// without any allocation. The list holds every entry that has been used at least once: an entry
// that was never used has nothing to drop.
static FFcacheEntry* entries = nullptr;

static void registerEntry(FFcacheEntry* entry) {
    if (entry->registered) {
        return;
    }

    entry->registered = true;
    entry->next = entries;
    entries = entry;
}

void* ffCacheGet(FFcacheEntry* entry) {
    if (ffCacheBeginInit(entry) && entry->init) {
        entry->init(entry->storage);
    }

    return entry->storage;
}

bool ffCacheBeginInit(FFcacheEntry* entry) {
    registerEntry(entry);

    if (entry->initialized) {
        return false;
    }

    entry->initialized = true;
    return true;
}

void ffCacheInvalidate(FFcacheEntry* entry) {
    if (!entry->initialized) {
        return;
    }

    entry->initialized = false;

    if (entry->destroy) {
        entry->destroy(entry->storage);
    }
}

void ffCacheInvalidateAll(void) {
    for (FFcacheEntry* entry = entries; entry; entry = entry->next) {
        ffCacheInvalidate(entry);
    }
}
