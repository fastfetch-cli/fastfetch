#include "common/time.h"

#include <stdio.h>

char ffTimeInternalBuffer[64]; // Reduce memory usage and prevent redundant allocations

#ifdef _WIN32
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat"
#endif

const char* ffTimeToFullStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);
    const struct tm* tm = localtime(&tsec);

    uint32_t len = 0;
    len += (uint32_t) strftime(ffTimeInternalBuffer, ARRAY_SIZE(ffTimeInternalBuffer) - len, "%FT%T", tm);
    len += (uint32_t) snprintf(ffTimeInternalBuffer + len, ARRAY_SIZE(ffTimeInternalBuffer) - len, ".%03u", (unsigned) (msec % 1000));
    len += (uint32_t) strftime(ffTimeInternalBuffer + len, ARRAY_SIZE(ffTimeInternalBuffer) - len, "%z", tm);
    return ffTimeInternalBuffer;
}

const char* ffTimeToShortStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);

    strftime(ffTimeInternalBuffer, ARRAY_SIZE(ffTimeInternalBuffer), "%F %T", localtime(&tsec));
    return ffTimeInternalBuffer;
}

const char* ffTimeToTimeStr(uint64_t msec)
{
    if (msec == 0) return "";
    time_t tsec = (time_t) (msec / 1000);

    uint32_t len = (uint32_t) strftime(ffTimeInternalBuffer, ARRAY_SIZE(ffTimeInternalBuffer), "%T", localtime(&tsec));
    sprintf(ffTimeInternalBuffer + len, ".%03u", (unsigned) (msec % 1000));
    return ffTimeInternalBuffer;
}

#ifdef _WIN32
    #pragma GCC diagnostic pop
#endif

FFTimeGetAgeResult ffTimeGetAge(uint64_t birthMs, uint64_t nowMs)
{
    FFTimeGetAgeResult result = {};
    if (__builtin_expect(birthMs == 0 || nowMs < birthMs, 0))
        return result;

    time_t birth_s = (time_t) (birthMs / 1000);
    struct tm birth_tm;
    #ifdef _WIN32
    localtime_s(&birth_tm, &birth_s);
    #else
    localtime_r(&birth_s, &birth_tm);
    #endif

    time_t now_s = (time_t) (nowMs / 1000);
    struct tm now_tm;
    #ifdef _WIN32
    localtime_s(&now_tm, &now_s);
    #else
    localtime_r(&now_s, &now_tm);
    #endif

    result.years = (uint32_t) (now_tm.tm_year - birth_tm.tm_year);
    if (now_tm.tm_yday < birth_tm.tm_yday)
        result.years--;

    birth_tm.tm_year += (int) result.years;
    birth_s = mktime(&birth_tm);
    uint32_t diff_s = (uint32_t) (now_s - birth_s);
    result.daysOfYear = diff_s / (24 * 60 * 60);

    birth_tm.tm_year += 1;
    result.yearsFraction = (double) diff_s / (double) (mktime(&birth_tm) - birth_s) + result.years;

    return result;
}

#ifdef _WIN32
    double ffQpcMultiplier;

    __attribute__((constructor))
    static void ffTimeInitQpcMultiplier(void)
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        ffQpcMultiplier = 1000. / (double) frequency.QuadPart;
    }
#endif
