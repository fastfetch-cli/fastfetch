#include "fastfetch.h"
#include "detection/datetime/datetime.h"
#include "common/thread.h"

#include <time.h>

const FFDateTimeResult* ffDetectDateTime(void)
{
    static FFDateTimeResult result;
    static FFThreadMutex mutex = FF_THREAD_MUTEX_INITIALIZER;
    static bool init = false;

    ffThreadMutexLock(&mutex);
    if (init)
    {
        ffThreadMutexUnlock(&mutex);
        return &result;
    }
    init = true;

    const time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    result.year = (uint16_t) (tm->tm_year + 1900);
    result.yearShort = (uint8_t) (result.year % 100);
    result.month = (uint8_t) (tm->tm_mon + 1);

    ffStrbufInitA(&result.monthPretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.monthPretty.length = (uint32_t) strftime(result.monthPretty.chars, ffStrbufGetFree(&result.monthPretty), "%m", tm);

    ffStrbufInitA(&result.monthName, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.monthName.length = (uint32_t) strftime(result.monthName.chars, ffStrbufGetFree(&result.monthName), "%B", tm);

    ffStrbufInitA(&result.monthNameShort, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.monthNameShort.length = (uint32_t) strftime(result.monthNameShort.chars, ffStrbufGetFree(&result.monthNameShort), "%b", tm);

    result.week = (uint8_t) (tm->tm_yday / 7 + 1);

    ffStrbufInitA(&result.weekday, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.weekday.length = (uint32_t) strftime(result.weekday.chars, ffStrbufGetFree(&result.weekday), "%A", tm);

    ffStrbufInitA(&result.weekdayShort, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.weekdayShort.length = (uint32_t) strftime(result.weekdayShort.chars, ffStrbufGetFree(&result.weekdayShort), "%a", tm);

    result.dayInYear = (uint8_t) (tm->tm_yday + 1);
    result.dayInMonth = (uint8_t) tm->tm_mday;
    result.dayInWeek = tm->tm_wday == 0 ? 7 : (uint8_t) tm->tm_wday;

    result.hour = (uint8_t) tm->tm_hour;

    ffStrbufInitA(&result.hourPretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.hourPretty.length = (uint32_t) strftime(result.hourPretty.chars, ffStrbufGetFree(&result.hourPretty), "%H", tm);

    result.hour12 = (uint8_t) (result.hour % 12);

    ffStrbufInitA(&result.hour12Pretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.hour12Pretty.length = (uint32_t) strftime(result.hour12Pretty.chars, ffStrbufGetFree(&result.hour12Pretty), "%I", tm);

    result.minute = (uint8_t) tm->tm_min;

    ffStrbufInitA(&result.minutePretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.minutePretty.length = (uint32_t) strftime(result.minutePretty.chars, ffStrbufGetFree(&result.minutePretty), "%M", tm);

    result.second = (uint8_t) tm->tm_sec;

    ffStrbufInitA(&result.secondPretty, FASTFETCH_STRBUF_DEFAULT_ALLOC);
    result.secondPretty.length = (uint32_t) strftime(result.secondPretty.chars, ffStrbufGetFree(&result.secondPretty), "%S", tm);

    ffThreadMutexUnlock(&mutex);
    return &result;
}
