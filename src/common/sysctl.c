#include "sysctl.h"

#include <stdlib.h>

void ffSysctlGetString(const char* propName, FFstrbuf* result)
{
    size_t neededLength;
    if(sysctlbyname(propName, NULL, &neededLength, NULL, 0) != 0 || neededLength == 1) //neededLength is 1 for empty strings, because of the null terminator
        return;

    ffStrbufEnsureFree(result, (uint32_t) neededLength - 1);

    if(sysctlbyname(propName, result->chars + result->length, &neededLength, NULL, 0) == 0)
        result->length += (uint32_t) neededLength - 1;

    result->chars[result->length] = '\0';
}

int ffSysctlGetInt(const char* propName, int defaultValue)
{
    int result;
    size_t neededLength = sizeof(result);
    if(sysctlbyname(propName, &result, &neededLength, NULL, 0) != 0)
        return defaultValue;
    return result;
}

int64_t ffSysctlGetInt64(const char* propName, int64_t defaultValue)
{
    int64_t result;
    size_t neededLength = sizeof(result);
    if(sysctlbyname(propName, &result, &neededLength, NULL, 0) != 0)
        return defaultValue;
    return result;
}

void* ffSysctlGetData(int* request, u_int requestLength, size_t* resultLength)
{
    if(sysctl(request, requestLength, NULL, resultLength, NULL, 0) != 0)
        return NULL;

    void* data = malloc(*resultLength);
    if(data == NULL)
        return NULL;

    if(sysctl(request, requestLength, data, resultLength, NULL, 0) != 0)
    {
        free(data);
        return NULL;
    }

    return data;
}
