#include "fastfetch.h"
#include "common/printing.h"
#include "util/textModifier.h"

void ffPrintLogoAndKey(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType)
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

        if(moduleArgs && !(printType & FF_PRINT_TYPE_NO_CUSTOM_KEY_COLOR) && moduleArgs->keyColor.length > 0)
            ffPrintColor(&moduleArgs->keyColor);
        else
            ffPrintColor(&instance.config.colorKeys);
    }

    //NULL check is required for modules with custom keys, e.g. disk with the folder path
    if((printType & FF_PRINT_TYPE_NO_CUSTOM_KEY) || !moduleArgs || moduleArgs->key.length == 0)
    {
        fputs(moduleName, stdout);

        if(moduleIndex > 0)
            printf(" %hhu", moduleIndex);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
        ffParseFormatString(&key, &moduleArgs->key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT8, &moduleIndex}
        });
        ffStrbufWriteTo(&key, stdout);
    }

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    ffStrbufWriteTo(&instance.config.keyValueSeparator, stdout);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);


    if (!instance.config.pipe && !(printType & FF_PRINT_TYPE_NO_CUSTOM_KEY_WIDTH))
    {
        uint32_t keyWidth = moduleArgs && moduleArgs->keyWidth > 0 ? moduleArgs->keyWidth : instance.config.keyWidth;
        if (keyWidth > 0)
            printf("\e[%uG", (unsigned) (keyWidth + instance.state.logoWidth));
    }
}

void ffPrintFormatString(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, uint32_t numArgs, const FFformatarg* arguments)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (moduleArgs)
        ffParseFormatString(&buffer, &moduleArgs->outputFormat, numArgs, arguments);
    else
        ffStrbufAppendS(&buffer, "unknown");

    if(buffer.length > 0)
    {
        ffPrintLogoAndKey(moduleName, moduleIndex, moduleArgs, printType);
        ffStrbufPutTo(&buffer, stdout);
    }
}

static void printError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, va_list arguments)
{
    if(!instance.config.showErrors)
        return;

    ffPrintLogoAndKey(moduleName, moduleIndex, moduleArgs, printType);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);

    vprintf(message, arguments);

    if(!instance.config.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    putchar('\n');
}

void ffPrintErrorString(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(moduleName, moduleIndex, moduleArgs, printType, message, arguments);
    va_end(arguments);
}

void ffPrintError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(moduleName, moduleIndex, moduleArgs, FF_PRINT_TYPE_DEFAULT, message, arguments);
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
