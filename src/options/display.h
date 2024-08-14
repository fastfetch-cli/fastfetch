#pragma once

#include "util/FFstrbuf.h"

typedef enum FFSizeBinaryPrefixType
{
    FF_SIZE_BINARY_PREFIX_TYPE_IEC,   // 1024 Bytes = 1 KiB, 1024 KiB = 1 MiB, ... (standard)
    FF_SIZE_BINARY_PREFIX_TYPE_SI,    // 1000 Bytes = 1 KB, 1000 KB = 1 MB, ...
    FF_SIZE_BINARY_PREFIX_TYPE_JEDEC, // 1024 Bytes = 1 kB, 1024 kB = 1 MB, ...
} FFSizeBinaryPrefixType;

typedef enum FFTemperatureUnit
{
    FF_TEMPERATURE_UNIT_CELSIUS,
    FF_TEMPERATURE_UNIT_FAHRENHEIT,
    FF_TEMPERATURE_UNIT_KELVIN,
} FFTemperatureUnit;

typedef struct FFOptionsDisplay
{
    //If one of those is empty, ffLogoPrint will set them
    FFstrbuf colorKeys;
    FFstrbuf colorTitle;
    FFstrbuf colorOutput;
    FFstrbuf colorSeparator;

    bool brightColor;

    FFstrbuf keyValueSeparator;

    int32_t stat; // <0: disable stat; 0: no threshold; >0: threshold in ms
    bool pipe; //disables all escape sequences
    bool showErrors;
    bool disableLinewrap;
    bool hideCursor;
    FFSizeBinaryPrefixType sizeBinaryPrefix;
    uint8_t sizeNdigits;
    uint8_t sizeMaxPrefix;
    FFTemperatureUnit tempUnit;
    uint8_t tempNdigits;
    FFstrbuf tempColorGreen;
    FFstrbuf tempColorYellow;
    FFstrbuf tempColorRed;
    FFstrbuf barCharElapsed;
    FFstrbuf barCharTotal;
    FFstrbuf barBorderLeft;
    FFstrbuf barBorderRight;
    uint8_t barWidth;
    uint8_t percentType;
    uint8_t percentNdigits;
    FFstrbuf percentColorGreen;
    FFstrbuf percentColorYellow;
    FFstrbuf percentColorRed;
    bool noBuffer;
    FFModuleKeyType keyType;
    uint16_t keyWidth;
    uint16_t keyPaddingLeft;
    int8_t freqNdigits;
    FFlist constants; // list of FFstrbuf
} FFOptionsDisplay;

const char* ffOptionsParseDisplayJsonConfig(FFOptionsDisplay* options, yyjson_val* root);
bool ffOptionsParseDisplayCommandLine(FFOptionsDisplay* options, const char* key, const char* value);
void ffOptionsInitDisplay(FFOptionsDisplay* options);
void ffOptionsDestroyDisplay(FFOptionsDisplay* options);
void ffOptionsGenerateDisplayJsonConfig(FFOptionsDisplay* options, yyjson_mut_doc* doc);
