#include <stdint.h>
#include <malloc.h>
#include <string.h>

#include "FFvaluestore.h"

void ffValuestoreInit(FFvaluestore* vs)
{
    vs->capacity = 32;
    vs->pairs = calloc(vs->capacity, sizeof(FFvaluestorePair));
    vs->size = 0;
}

void ffValuestoreSet(FFvaluestore* vs, const char* name, const char* value)
{
    for(uint32_t i = 0; i < vs->size; i++)
    {
        if(strcasecmp(vs->pairs[i].name, name) == 0)
        {
            strcpy(vs->pairs[i].value, value);
            return;
        }
    }

    if(vs->size == vs->capacity)
    {
        FFvaluestorePair* pairs = (FFvaluestorePair*) calloc(vs->capacity * 2, sizeof(FFvaluestorePair));
        memcpy(pairs, vs->pairs, sizeof(FFvaluestorePair) * vs->size);
        free(vs->pairs);
        vs->pairs = pairs;
        vs->capacity *= 2;
    }

    strcpy(vs->pairs[vs->size].name, name);
    strcpy(vs->pairs[vs->size].value, value);

    ++vs->size;
}

const char* ffValuestoreGet(FFvaluestore* vs, const char* name)
{
    for(uint32_t i = 0; i < vs->size; i++)
    {
        if(strcasecmp(vs->pairs[i].name, name) == 0)
            return vs->pairs[i].value;
    }

    return NULL;
}

bool ffValuestoreContains(FFvaluestore* vs, const char* name)
{
    return ffValuestoreGet(vs, name) != NULL;
}

void ffValuestoreDelete(FFvaluestore* vs)
{
    free(vs->pairs);
}