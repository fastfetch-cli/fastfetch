#pragma once

#include "common/FFstrbuf.h"
#include <stdbool.h>

typedef struct FFKeeneticInfo
{
    bool available;
    FFstrbuf vendor;       // "Keenetic"
    FFstrbuf vendor_full;  // "Keenetic Ltd."
    FFstrbuf title;        // "5.1.6"
    FFstrbuf model;        // "Giant (KN-2610)"
    FFstrbuf version;      // "5.01.C.6.0-1"
    const char* id;        // "keenetic"
    const char* idLike;    // "keeneticos"
    const char* family;    // "linux"
} FFKeeneticInfo;

const FFKeeneticInfo* ffDetectKeenetic(void);

void ffKeeneticFormatOS(FFstrbuf* result);     // "Keenetic OS"
void ffKeeneticFormatOSFull(FFstrbuf* result); // "Keenetic OS 5.1.6"
void ffKeeneticFormatHost(FFstrbuf* result);   // "Keenetic Giant (KN-2610)"
