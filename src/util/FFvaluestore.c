#include "FFvaluestore.h"

#include <string.h>

typedef char KeyType[33]; //32 byte key + \0

void ffValuestoreInit(FFvaluestore* vs, uint32_t valueSize)
{
    ffListInit(vs, (uint32_t) (sizeof(KeyType) + valueSize));
}

void* ffValuestoreGet(FFvaluestore* vs, const char* key)
{
    for(uint32_t i = 0; i < vs->length; i++)
    {
        char* element = ffListGet(vs, i);
        if(strcmp(element, key) == 0)
            return element + sizeof(KeyType);
    }

    return NULL;
}

void* ffValuestoreSet(FFvaluestore* vs, const char* key, bool* created)
{
    char* element = ffValuestoreGet(vs, key);
    if(element != NULL)
    {
        if(created != NULL)
            *created = false;
        return element;
    }

    element = ffListAdd(vs);
    strncpy(element, key, sizeof(KeyType) - 1);
    element[sizeof(KeyType) - 1] = '\0';

    if(created != NULL)
        *created = true;
    return element + sizeof(KeyType);
}

void ffValuestoreDestroy(FFvaluestore* vs)
{
    ffListDestroy(vs);
}
