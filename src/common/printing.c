#include "fastfetch.h"

void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, uint32_t numFormatArgs, const char* message, ...)
{
    if(!instance->config.showErrors)
        return;

    va_list arguments;
    va_start(arguments, message);

    if(formatString == NULL || formatString->length == 0)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);
        vprintf(message, arguments);
        puts(FASTFETCH_TEXT_MODIFIER_RESET);
    }
    else
    {
        FF_STRBUF_CREATE(error);
        ffStrbufAppendVF(&error, message, arguments);

        // calloc sets all to 0 and FF_FORMAT_ARG_TYPE_NULL also has value 0 so we don't need to explictly set it
        FFformatarg* nullArgs = calloc(numFormatArgs, sizeof(FFformatarg));

        ffPrintFormatString(instance, moduleName, moduleIndex, customKeyFormat, formatString, &error, numFormatArgs, nullArgs);

        free(nullArgs);
        ffStrbufDestroy(&error);
    }

    va_end(arguments);
}

void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* formatString, const FFstrbuf* error, uint32_t numArgs, const FFformatarg* arguments)
{
    FFstrbuf buffer;
    ffStrbufInitA(&buffer, 256);

    ffParseFormatString(&buffer, formatString, error, numArgs, arguments);

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        ffStrbufPutTo(&buffer, stdout);
    }

    ffStrbufDestroy(&buffer);
}

void ffPrintColor(const FFstrbuf* colorValue)
{
    fputs("\033[", stdout);
    ffStrbufWriteTo(colorValue, stdout);
    fputc('m', stdout);
}

void ffPrintCharTimes(char c, uint32_t times)
{
    for(uint32_t i = 0; i < times; i++)
        putchar(c);
}
