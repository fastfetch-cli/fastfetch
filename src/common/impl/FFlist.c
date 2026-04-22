#include "common/FFlist.h"

#include <stdlib.h>
#include <string.h>

void* ffListAdd(FFlist* list, uint32_t elementSize) {
    if (list->length == list->capacity) {
        ffListReserve(list, elementSize, list->capacity == 0 ? FF_LIST_DEFAULT_ALLOC : list->capacity * 2);
    }

    ++list->length;
    return ffListGet(list, elementSize, list->length - 1);
}

bool ffListShift(FFlist* list, uint32_t elementSize, void* result) {
    if (list->length == 0) {
        return false;
    }

    memcpy(result, list->data, elementSize);
    memmove(list->data, list->data + elementSize, (size_t) elementSize * (list->length - 1));
    --list->length;
    return true;
}

bool ffListPop(FFlist* list, uint32_t elementSize, void* result) {
    if (list->length == 0) {
        return false;
    }

    memcpy(result, ffListGet(list, elementSize, list->length - 1), elementSize);
    --list->length;
    return true;
}
