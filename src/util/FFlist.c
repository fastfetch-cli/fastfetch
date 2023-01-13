#include "FFlist.h"

#include <stdlib.h>
#include <string.h>

void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity)
{
    list->elementSize = elementSize;
    list->capacity = capacity;
    list->length = 0;
    list->data = capacity == 0 ? NULL : malloc((size_t)list->capacity * list->elementSize);
}

void* ffListAdd(FFlist* list)
{
    if(list->length == list->capacity)
    {
        list->capacity = list->capacity == 0 ? FF_LIST_DEFAULT_ALLOC : list->capacity * 2;
        // realloc(NULL, newSize) is same as malloc(newSize)
        list->data = realloc(list->data, (size_t)list->capacity * list->elementSize);
    }

    ++list->length;
    return ffListGet(list, list->length - 1);
}

uint32_t ffListFirstIndexComp(const FFlist* list, void* compElement, bool(*compFunc)(const void*, const void*))
{
    for(uint32_t i = 0; i < list->length; i++)
    {
        if(compFunc(ffListGet(list, i), compElement))
            return i;
    }

    return list->length;
}

bool ffListShift(FFlist* list, void* result)
{
    if(list->length == 0)
        return false;

    memcpy(result, list->data, list->elementSize);
    memmove(list->data, list->data + list->elementSize, list->elementSize * (list->length - 1));
    --list->length;
    return true;
}

bool ffListPop(FFlist* list, void* result)
{
    if(list->length == 0)
        return false;

    memcpy(result, ffListGet(list, list->length - 1), list->elementSize);
    --list->length;
    return result;
}

void ffListDestroy(FFlist* list)
{
    //Avoid free-after-use. These 3 assignments are cheap so don't remove them
    list->capacity = list->length = 0;
    free(list->data);
    list->data = NULL;
}
