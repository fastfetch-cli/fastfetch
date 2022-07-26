#pragma once

#ifndef FF_INCLUDED_detection_datetime
#define FF_INCLUDED_detection_datetime

#include "fastfetch.h"

typedef struct FFDateTimeResult
{
    //Examples for 21.02.2022 - 15:18:37
    uint16_t year; //2022
    uint8_t yearShort; //22
    uint8_t month; //2
    FFstrbuf monthPretty; //02
    FFstrbuf monthName; //February
    FFstrbuf monthNameShort; //Feb
    uint8_t week; //8
    FFstrbuf weekday; //Monday
    FFstrbuf weekdayShort; //Mon
    uint16_t dayInYear; //52
    uint8_t dayInMonth; //21
    uint8_t dayInWeek; //1
    uint8_t hour; //15
    FFstrbuf hourPretty; //15
    uint8_t hour12; //3
    FFstrbuf hour12Pretty; //03
    uint8_t minute; //18
    FFstrbuf minutePretty; //18
    uint8_t second; //37
    FFstrbuf secondPretty; //37
} FFDateTimeResult;

const FFDateTimeResult* ffDetectDateTime(const FFinstance* instance);

#endif
