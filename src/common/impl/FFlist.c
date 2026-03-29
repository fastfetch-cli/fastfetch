#include "common/FFlist.h"

#include <stdlib.h>
#include <string.h>

void* ffListAdd(FFlist* list) {
    if (list->length == list->capacity) {
        ffListReserve(list, list->capacity == 0 ? FF_LIST_DEFAULT_ALLOC : list->capacity * 2);
    }

    ++list->length;
    return ffListGet(list, list->length - 1);
}

bool ffListShift(FFlist* list, void* result) {
    if (list->length == 0) {
        return false;
    }

    memcpy(result, list->data, list->elementSize);
    memmove(list->data, list->data + list->elementSize, (size_t) list->elementSize * (list->length - 1));
    --list->length;
    return true;
}

bool ffListPop(FFlist* list, void* result) {
    if (list->length == 0) {
        return false;
    }

    memcpy(result, ffListGet(list, list->length - 1), list->elementSize);
    --list->length;
    return true;
}
