#include "common/FFlist.h"

#include <stdlib.h>
#include <string.h>

bool ffListShift(FFlist* list, uint32_t elementSize, void* __restrict result) {
    if (list->length == 0) {
        return false;
    }

    memcpy(result, list->data, elementSize);
    memmove(list->data, list->data + elementSize, (size_t) elementSize * (list->length - 1));
    --list->length;
    return true;
}

bool ffListPop(FFlist* list, uint32_t elementSize, void* __restrict result) {
    if (list->length == 0) {
        return false;
    }

    memcpy(result, ffListGet(list, elementSize, list->length - 1), elementSize);
    --list->length;
    return true;
}
