#pragma once

#include "common/percent.h"
#include "util/FFstrbuf.h"

typedef enum __attribute__((__packed__)) FFSizeBinaryPrefixType
{
    FF_SIZE_BINARY_PREFIX_TYPE_IEC,   // 1024 Bytes = 1 KiB, 1024 KiB = 1 MiB, ... (standard)
    FF_SIZE_BINARY_PREFIX_TYPE_SI,    // 1000 Bytes = 1 kB, 1000 kB = 1 MB, ...
    FF_SIZE_BINARY_PREFIX_TYPE_JEDEC, // 1024 Bytes = 1 KB, 1024 KB = 1 MB, ...
} FFSizeBinaryPrefixType;

typedef enum __attribute__((__packed__)) FFTemperatureUnit
{
    FF_TEMPERATURE_UNIT_DEFAULT,
    FF_TEMPERATURE_UNIT_CELSIUS,
    FF_TEMPERATURE_UNIT_FAHRENHEIT,
    FF_TEMPERATURE_UNIT_KELVIN,
} FFTemperatureUnit;

typedef enum __attribute__((__packed__)) FFSpaceBeforeUnitType
{
    FF_SPACE_BEFORE_UNIT_DEFAULT,
    FF_SPACE_BEFORE_UNIT_ALWAYS,
    FF_SPACE_BEFORE_UNIT_NEVER,
} FFSpaceBeforeUnitType;

typedef enum __attribute__((__packed__)) FFFractionTrailingZerosType
{
    FF_FRACTION_TRAILING_ZEROS_TYPE_DEFAULT,
    FF_FRACTION_TRAILING_ZEROS_TYPE_ALWAYS,
    FF_FRACTION_TRAILING_ZEROS_TYPE_NEVER,
} FFFractionTrailingZerosType;

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
    #ifndef NDEBUG
    bool debugMode;
    #endif
    bool disableLinewrap;
    bool durationAbbreviation;
    FFSpaceBeforeUnitType durationSpaceBeforeUnit;
    bool hideCursor;
    FFSizeBinaryPrefixType sizeBinaryPrefix;
    uint8_t sizeNdigits;
    uint8_t sizeMaxPrefix;
    FFSpaceBeforeUnitType sizeSpaceBeforeUnit;
    FFTemperatureUnit tempUnit;
    uint8_t tempNdigits;
    FFstrbuf tempColorGreen;
    FFstrbuf tempColorYellow;
    FFstrbuf tempColorRed;
    FFSpaceBeforeUnitType tempSpaceBeforeUnit;
    FFstrbuf barCharElapsed;
    FFstrbuf barCharTotal;
    FFstrbuf barBorderLeft;
    FFstrbuf barBorderRight;
    FFstrbuf barBorderLeftElapsed;
    FFstrbuf barBorderRightElapsed;
    FFstrbuf barColorElapsed; // "auto" for auto selection from percent config; empty for no custom color (inherits)
    FFstrbuf barColorTotal; // empty for no custom color (inherits)
    FFstrbuf barColorBorder; // empty for no custom color (inherits)
    uint8_t barWidth;
    FFPercentageTypeFlags percentType;
    uint8_t percentNdigits;
    FFstrbuf percentColorGreen;
    FFstrbuf percentColorYellow;
    FFstrbuf percentColorRed;
    FFSpaceBeforeUnitType percentSpaceBeforeUnit;
    uint8_t percentWidth;
    bool noBuffer;
    FFModuleKeyType keyType;
    uint16_t keyWidth;
    uint16_t keyPaddingLeft;
    int8_t freqNdigits;
    FFSpaceBeforeUnitType freqSpaceBeforeUnit;
    int8_t fractionNdigits;
    FFFractionTrailingZerosType fractionTrailingZeros;

    FFlist constants; // list of FFstrbuf
} FFOptionsDisplay;

const char* ffOptionsParseDisplayJsonConfig(FFOptionsDisplay* options, yyjson_val* root);
bool ffOptionsParseDisplayCommandLine(FFOptionsDisplay* options, const char* key, const char* value);
void ffOptionsInitDisplay(FFOptionsDisplay* options);
void ffOptionsDestroyDisplay(FFOptionsDisplay* options);
void ffOptionsGenerateDisplayJsonConfig(FFOptionsDisplay* options, yyjson_mut_doc* doc);
