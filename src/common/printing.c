#include "fastfetch.h"
#include "common/printing.h"
#include "util/textModifier.h"

void ffPrintLogoAndKey(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor)
{
    ffLogoPrintLine();

    //This is used by --set-keyless, in this case we wan't neither the module name nor the separator
    if(moduleName == NULL)
        return;

    if(!instance.config.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
        if (instance.config.brightColor)
            fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);

        if(customKeyColor != NULL && customKeyColor->length > 0)
            ffPrintColor(customKeyColor);
        else
            ffPrintColor(&instance.config.colorKeys);
    }

    //NULL check is required for modules with custom keys, e.g. disk with the folder path
    if(customKeyFormat == NULL || customKeyFormat->length == 0)
    {
        fputs(moduleName, stdout);

        if(moduleIndex > 0)
            printf(" %hhu", moduleIndex);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
        ffParseFormatString(&key, customKeyFormat, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT8, &moduleIndex}
        });
        ffStrbufWriteTo(&key, stdout);
    }

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    ffStrbufWriteTo(&instance.config.keyValueSeparator, stdout);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    if (instance.config.keyWidth > 0)
        printf("\e[%uG", (unsigned) (instance.config.keyWidth + instance.state.logoWidth));
}

void ffPrintFormatString(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor, const FFstrbuf* format, uint32_t numArgs, const FFformatarg* arguments)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateA(256);
    ffParseFormatString(&buffer, format, numArgs, arguments);

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(moduleName, moduleIndex, customKeyFormat, customKeyColor);
        ffStrbufPutTo(&buffer, stdout);
    }
}

void ffPrintFormat(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, uint32_t numArgs, const FFformatarg* arguments)
{
    ffPrintFormatString(moduleName, moduleIndex, &moduleArgs->key, &moduleArgs->keyColor, &moduleArgs->outputFormat, numArgs, arguments);
}

static void printError(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor, const char* message, va_list arguments)
{
    if(!instance.config.showErrors)
        return;

    ffPrintLogoAndKey(moduleName, moduleIndex, customKeyFormat, customKeyColor);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);

    vprintf(message, arguments);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    putchar('\n');
}

void ffPrintErrorString(const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customKeyColor, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(moduleName, moduleIndex, customKeyFormat, customKeyColor, message, arguments);
    va_end(arguments);
}

void ffPrintError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(moduleName, moduleIndex, &moduleArgs->key, &moduleArgs->keyColor, message, arguments);
    va_end(arguments);
}

void ffPrintColor(const FFstrbuf* colorValue)
{
    //If the color is not set, this would reset in \033[m, which resets everything.
    //So we only print it, if the main color is at least one char.
    if(colorValue->length == 0)
        return;

    fputs("\033[", stdout);
    ffStrbufWriteTo(colorValue, stdout);
    fputc('m', stdout);
}

void ffPrintCharTimes(char c, uint32_t times)
{
    if(times == 0)
        return;

    char str[32];
    memset(str, c, sizeof(str)); //2 instructions when compiling with AVX2 enabled
    for(uint32_t i = sizeof(str); i <= times; i += (uint32_t)sizeof(str))
        fwrite(str, 1, sizeof(str), stdout);
    uint32_t remaining = times % sizeof(str);
    if(remaining > 0)
        fwrite(str, 1, remaining, stdout);
}
