#include "stringUtils.h"

#include <ctype.h>

bool ffStrSet(const char* str)
{
    if(str == NULL)
        return false;

    while(isspace(*str))
        str++;

    return *str != '\0';
}

bool ffStrHasNChars(const char* str, char c, uint32_t n)
{
    for(uint32_t i = 0; i < n; i++)
    {
        char* current = strchr(str, c);
        if(current == NULL)
            return false;

        str = current + 1;
    }

    return true;
}
