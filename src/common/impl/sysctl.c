#include "common/sysctl.h"

#include <stdlib.h>

#ifdef __OpenBSD__
const char* ffSysctlGetString(int mib1, int mib2, FFstrbuf* result) {
    size_t neededLength;
    if (sysctl((int[]) { mib1, mib2 }, 2, nullptr, &neededLength, nullptr, 0) != 0 || neededLength == 1) { // neededLength is 1 for empty strings, because of the null terminator
        return "sysctl() length query failed";
    }

    ffStrbufEnsureFree(result, (uint32_t) neededLength - 1);

    if (sysctl((int[]) { mib1, mib2 }, 2, result->chars + result->length, &neededLength, nullptr, 0) != 0) {
        return "sysctl() failed to retrieve string data";
    }

    result->length += (uint32_t) neededLength - 1;
    result->chars[result->length] = '\0';

    return nullptr;
}

int ffSysctlGetInt(int mib1, int mib2, int defaultValue) {
    alignas(int) uint8_t result[sizeof(int)];
    size_t neededLength = sizeof(result);
    if (sysctl((int[]) { mib1, mib2 }, 2, &result, &neededLength, nullptr, 0) != 0) {
        return defaultValue;
    }
    switch (neededLength) {
        case sizeof(int32_t):
            return *(int32_t*)result;
        case sizeof(int16_t):
            return *(int16_t*)result;
        case sizeof(int8_t):
            return *(int8_t*)result;
        default:
            return defaultValue;
    }
}

int64_t ffSysctlGetInt64(int mib1, int mib2, int64_t defaultValue) {
    alignas(int64_t) uint8_t result[sizeof(int64_t)];
    size_t neededLength = sizeof(result);
    if (sysctl((int[]) { mib1, mib2 }, 2, &result, &neededLength, nullptr, 0) != 0) {
        return defaultValue;
    }
    switch (neededLength) {
        case sizeof(int64_t):
            return *(int64_t*)result;
        case sizeof(int32_t):
            return *(int32_t*)result;
        case sizeof(int16_t):
            return *(int16_t*)result;
        case sizeof(int8_t):
            return *(int8_t*)result;
        default:
            return defaultValue;
    }
}
#else
const char* ffSysctlGetString(const char* propName, FFstrbuf* result) {
    size_t neededLength;
    if (sysctlbyname(propName, nullptr, &neededLength, nullptr, 0) != 0 || neededLength == 1) { // neededLength is 1 for empty strings, because of the null terminator
        return "sysctlbyname() failed";
    }

    ffStrbufEnsureFree(result, (uint32_t) neededLength - 1);

    if (sysctlbyname(propName, result->chars + result->length, &neededLength, nullptr, 0) != 0) {
        return "sysctlbyname() failed to retrieve string data";
    }

    result->length += (uint32_t) neededLength - 1;

    result->chars[result->length] = '\0';

    return nullptr;
}

int ffSysctlGetInt(const char* propName, int defaultValue) {
    alignas(int) uint8_t result[sizeof(int)];
    size_t neededLength = sizeof(result);
    if (sysctlbyname(propName, &result, &neededLength, nullptr, 0) != 0) {
        return defaultValue;
    }
    switch (neededLength) {
        case sizeof(int32_t):
            return *(int32_t*)result;
        case sizeof(int16_t):
            return *(int16_t*)result;
        case sizeof(int8_t):
            return *(int8_t*)result;
        default:
            return defaultValue;
    }
}

int64_t ffSysctlGetInt64(const char* propName, int64_t defaultValue) {
    alignas(int64_t) uint8_t result[sizeof(int64_t)];
    size_t neededLength = sizeof(result);
    if (sysctlbyname(propName, &result, &neededLength, nullptr, 0) != 0) {
        return defaultValue;
    }
    switch (neededLength) {
        case sizeof(int64_t):
            return *(int64_t*)result;
        case sizeof(int32_t):
            return *(int32_t*)result;
        case sizeof(int16_t):
            return *(int16_t*)result;
        case sizeof(int8_t):
            return *(int8_t*)result;
        default:
            return defaultValue;
    }
}
#endif // OpenBSD

void* ffSysctlGetData(int* request, u_int requestLength, size_t* resultLength) {
    if (sysctl(request, requestLength, nullptr, resultLength, nullptr, 0) != 0) {
        return nullptr;
    }

    void* data = malloc(*resultLength);

    if (sysctl(request, requestLength, data, resultLength, nullptr, 0) != 0) {
        free(data);
        return nullptr;
    }

    return data;
}
