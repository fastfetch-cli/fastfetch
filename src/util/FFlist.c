#include "FFlist.h"

#include <stdlib.h>

void ffListInit(FFlist* list, uint32_t elementSize)
{
    ffListInitA(list, elementSize, FF_LIST_DEFAULT_ALLOC);
}

void ffListInitA(FFlist* list, uint32_t elementSize, uint32_t capacity)
{
    list->elementSize = elementSize;
    list->capacity = capacity;
    list->length = 0;
    list->data = capacity == 0 ? NULL : malloc((size_t)list->capacity * list->elementSize);
}

void* ffListGet(const FFlist* list, uint32_t index)
{
    return list->data + (index * list->elementSize);
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

void ffListDestroy(FFlist* list)
{
    //Avoid free-after-use. These 3 assignments are cheap so don't remove them
    list->capacity = list->length = 0;
    free(list->data);
    list->data = NULL;
}
