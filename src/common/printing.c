#include "fastfetch.h"

void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat)
{
    ffPrintLogoLine(instance);

    if(!instance->config.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_RESET FASTFETCH_TEXT_MODIFIER_BOLT, stdout);

        //If the main color is not set (e.g. none logo), this would reset in \033[m, which resets everything,
        //including the wanted bolt from above. So we only print it, if the main color is at least one char.
        if(instance->config.mainColor.length > 0)
            ffPrintColor(&instance->config.mainColor);
    }

    if(customKeyFormat == NULL || customKeyFormat->length == 0)
    {
        fputs(moduleName, stdout);

        if(moduleIndex > 0)
            printf(" %hhu", moduleIndex);
    }
    else
    {
        FFstrbuf key;
        ffStrbufInit(&key);
        ffParseFormatString(&key, customKeyFormat, NULL, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT8, &moduleIndex}
        });
        ffStrbufWriteTo(&key, stdout);
        ffStrbufDestroy(&key);
    }

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    ffStrbufWriteTo(&instance->config.separator, stdout);

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}

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
