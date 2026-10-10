#include "common/FFcache.h"
#include "common/textModifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

typedef struct TestStorage {
    int value;
    int initCalls;
    int destroyCalls;
} TestStorage;

static TestStorage storageA, storageB, storageC, storageD, storageUnused;

static void initStorage(void* storage) {
    TestStorage* test = storage;
    ++test->initCalls;
    test->value = 42;
}

static void destroyStorage(void* storage) {
    TestStorage* test = storage;
    ++test->destroyCalls;
    test->value = -1;
}

static FFcacheEntry entryA = { .name = "A", .storage = &storageA, .init = initStorage, .destroy = destroyStorage };
static FFcacheEntry entryB = { .name = "B", .storage = &storageB, .init = initStorage, .destroy = destroyStorage };
static FFcacheEntry entryC = { .name = "C", .storage = &storageC, .init = initStorage, .destroy = destroyStorage };
static FFcacheEntry entryD = { .name = "D", .storage = &storageD };
static FFcacheEntry entryNoStorage = { .name = "no storage" };
static FFcacheEntry entryUnused = { .name = "unused", .storage = &storageUnused, .init = initStorage, .destroy = destroyStorage };

noreturn static void testFailed(const char* expression, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, expression);
    exit(1);
}

noreturn static void testFailedInt(int expected, int actual, const char* what, int lineNo) {
    fprintf(stderr, FASTFETCH_TEXT_MODIFIER_ERROR "[%d] %s: expected %d, got %d\n" FASTFETCH_TEXT_MODIFIER_RESET, lineNo, what, expected, actual);
    exit(1);
}

#define VERIFY(expression) \
    if (!(expression)) testFailed(#expression, __LINE__)

#define VERIFY_INT(expected, actual, what) \
    if ((expected) != (actual)) testFailedInt((expected), (actual), (what), __LINE__)

int main(void) {
    // A stale entry is built on the first get, and only then
    VERIFY(ffCacheGet(&entryA) == &storageA);
    VERIFY_INT(1, storageA.initCalls, "A init calls after the first get");
    VERIFY_INT(42, storageA.value, "A value after the first get");

    VERIFY(ffCacheGet(&entryA) == &storageA);
    VERIFY_INT(1, storageA.initCalls, "A init calls after the second get");

    // Nothing happens to the entry that has not been used yet
    VERIFY_INT(0, storageUnused.initCalls, "unused init calls");

    // Invalidating drops the value and rearms the entry, and doing it twice is a no-op
    ffCacheInvalidate(&entryA);
    VERIFY_INT(1, storageA.destroyCalls, "A destroy calls after invalidating");
    ffCacheInvalidate(&entryA);
    VERIFY_INT(1, storageA.destroyCalls, "A destroy calls after invalidating twice");

    VERIFY(ffCacheGet(&entryA) == &storageA);
    VERIFY_INT(2, storageA.initCalls, "A init calls after rebuilding");
    VERIFY_INT(42, storageA.value, "A value after rebuilding");

    // An entry that has never been used has nothing to drop
    ffCacheInvalidate(&entryC);
    VERIFY_INT(0, storageC.destroyCalls, "C destroy calls after invalidating an unused entry");
    VERIFY(ffCacheGet(&entryC) == &storageC);
    VERIFY_INT(1, storageC.initCalls, "C init calls");

    // An entry without an initializer tracks staleness on its own
    VERIFY(ffCacheGet(&entryD) == &storageD);
    VERIFY(!ffCacheBeginInit(&entryD));
    ffCacheInvalidate(&entryD);
    VERIFY(ffCacheBeginInit(&entryD));
    VERIFY(!ffCacheBeginInit(&entryD));

    // An entry without storage and without callbacks is tolerated
    VERIFY(ffCacheGet(&entryNoStorage) == nullptr);
    ffCacheInvalidate(&entryNoStorage);
    VERIFY(ffCacheGet(&entryNoStorage) == nullptr);

    // An entry whose owner builds the value itself (which is what `ffCacheBeginInit` is for) is only
    // marked, never built by the cache; and it must survive a `destroy` on a storage that the owner
    // never filled in
    VERIFY(ffCacheBeginInit(&entryB));
    VERIFY_INT(0, storageB.initCalls, "B init calls (the owner builds the value itself)");

    // Invalidating everything drops exactly the entries that have been used so far
    ffCacheInvalidateAll();
    VERIFY_INT(2, storageA.destroyCalls, "A destroy calls after invalidating all");
    VERIFY_INT(1, storageC.destroyCalls, "C destroy calls after invalidating all");
    VERIFY_INT(0, storageD.destroyCalls, "D destroy calls"); // D has no callback
    VERIFY_INT(1, storageB.destroyCalls, "B destroy calls after invalidating a value that was never built");
    VERIFY_INT(0, entryB.initialized, "B must be stale again after invalidating all");
    VERIFY_INT(0, storageUnused.initCalls, "unused init calls after invalidating all");

    // Rebuilding after invalidating all
    VERIFY(ffCacheGet(&entryA) == &storageA);
    VERIFY_INT(3, storageA.initCalls, "A init calls after invalidating all");
    VERIFY_INT(42, storageA.value, "A value after invalidating all");

    // An entry joins the registry when it is first used, not when it is created
    VERIFY(ffCacheGet(&entryUnused) == &storageUnused);
    VERIFY_INT(1, storageUnused.initCalls, "unused init calls after its first get");
    ffCacheInvalidateAll();
    VERIFY_INT(1, storageUnused.destroyCalls, "unused destroy calls after invalidating all");
    VERIFY_INT(3, storageA.destroyCalls, "A destroy calls after the second invalidating all");

    // Success
    puts("\033[32mAll tests passed!" FASTFETCH_TEXT_MODIFIER_RESET);
}
