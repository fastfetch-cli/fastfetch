#include "fastfetch.h"
#include "common/printing.h"
#include "util/textModifier.h"

void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat)
{
    ffLogoPrintLine(instance);

    //This is used by --set-keyless, in this case we wan't neither the module name nor the separator
    if(moduleName == NULL)
        return;

    if(!instance->config.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_RESET FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
        ffPrintColor(&instance->config.colorKeys);
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
        FFstrbuf key;
        ffStrbufInit(&key);
        ffParseFormatString(&key, customKeyFormat, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT8, &moduleIndex}
        });
        ffPrintUserString(key.chars);
        ffStrbufDestroy(&key);
    }

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    ffStrbufWriteTo(&instance->config.keyValueSeparator, stdout);

    if(!instance->config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}

void ffPrintFormatString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* format, uint32_t numArgs, const FFformatarg* arguments)
{
    FFstrbuf buffer;
    ffStrbufInitA(&buffer, 256);

    ffParseFormatString(&buffer, format, numArgs, arguments);

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);
        ffPrintUserString(buffer.chars);
        putchar('\n');
    }

    ffStrbufDestroy(&buffer);
}

void ffPrintFormat(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, uint32_t numArgs, const FFformatarg* arguments)
{
    ffPrintFormatString(instance, moduleName, moduleIndex, &moduleArgs->key, &moduleArgs->outputFormat, numArgs, arguments);
}

static void printError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customErrorFormat, const char* message, va_list arguments)
{
    bool hasCustomErrorFormat = customErrorFormat != NULL && customErrorFormat->length > 0;
    if(!hasCustomErrorFormat && !instance->config.showErrors)
        return;

    if(hasCustomErrorFormat)
    {
        FFstrbuf error;
        ffStrbufInit(&error);
        ffStrbufAppendVF(&error, message, arguments);

        ffPrintFormatString(instance, moduleName, moduleIndex, customKeyFormat, customErrorFormat, 1, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &error}
        });

        ffStrbufDestroy(&error);
    }
    else
    {
        ffPrintLogoAndKey(instance, moduleName, moduleIndex, customKeyFormat);

        if(!instance->config.pipe)
            fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);

        vprintf(message, arguments);

        if(!instance->config.pipe)
            fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

        putchar('\n');
    }
}

void ffPrintErrorString(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat, const FFstrbuf* customErrorFormat, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(instance, moduleName, moduleIndex, customKeyFormat, customErrorFormat, message, arguments);
    va_end(arguments);
}

void ffPrintError(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(instance, moduleName, moduleIndex, &moduleArgs->key, &moduleArgs->errorFormat, message, arguments);
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

void ffPrintUserString(const char* value)
{
    while(*value != '\0')
    {
        if(*value != '\\')
        {
            putchar(*value);
            ++value;
            continue;
        }

        ++value;

        if(*value == 'n')
            putchar('\n');
        else if(*value == 't')
            putchar('\t');
        else if(*value == 'e')
            putchar('\e');
        else if(*value == '\\')
            putchar('\\');
        else
        {
            putchar('\\');
            putchar(*value);
        }

        ++value;
    }
}
