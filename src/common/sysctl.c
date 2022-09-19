#include "sysctl.h"

#include <sys/sysctl.h>

void ffSysctlGetString(const char* propName, FFstrbuf* result)
{
    size_t neededLength;
    if(sysctlbyname(propName, NULL, &neededLength, NULL, 0) != 0 || neededLength == 0)
        return;

    ffStrbufEnsureFree(result, (uint32_t) neededLength);

    if(sysctlbyname(propName, result->chars + result->length, &neededLength, NULL, 0) == 0)
        result->length += (uint32_t) neededLength;

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

void* ffSysctlGetData(int* request, int requestLength, size_t* resultLength)
{
    if(sysctl(request, requestLength, NULL, &length, NULL, 0) != 0)
        return NULL;

    void* data = malloc(*length);
    if(data == NULL)
        return NULL;

    if(sysctl(request, requestLength, data, length, NULL, 0) != 0)
    {
        free(data);
        return NULL;
    }

    return data;
}
