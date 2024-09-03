#include "fastfetch.h"
#include "common/printing.h"
#include "util/textModifier.h"

void ffPrintLogoAndKey(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType)
{
    ffLogoPrintLine();

    //This is used by --set-keyless, in this case we want neither the module name nor the separator
    if(moduleName == NULL)
        return;

    //This is used as a magic value for hiding keys
    if (!(moduleArgs && ffStrbufEqualS(&moduleArgs->key, " ")) && instance.config.display.keyType != FF_MODULE_KEY_TYPE_NONE)
    {
        ffPrintCharTimes(' ', instance.config.display.keyPaddingLeft);

        if(!instance.config.display.pipe)
        {
            fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
            if (instance.config.display.brightColor)
                fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);

            if(moduleArgs && !(printType & FF_PRINT_TYPE_NO_CUSTOM_KEY_COLOR) && moduleArgs->keyColor.length > 0)
                ffPrintColor(&moduleArgs->keyColor);
            else
                ffPrintColor(&instance.config.display.colorKeys);
        }

        bool hasIcon = false;
        if (instance.config.display.keyType & FF_MODULE_KEY_TYPE_ICON && moduleArgs && moduleArgs->keyIcon.length > 0)
        {
            ffStrbufWriteTo(&moduleArgs->keyIcon, stdout);
            hasIcon = true;
        }

        if (instance.config.display.keyType & FF_MODULE_KEY_TYPE_STRING)
        {
            if(hasIcon)
                putchar(' ');

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
                FF_PARSE_FORMAT_STRING_CHECKED(&key, &moduleArgs->key, 2, ((FFformatarg[]){
                    FF_FORMAT_ARG(moduleIndex, "index"),
                    FF_FORMAT_ARG(moduleArgs->keyIcon, "icon"),
                }));
                ffStrbufWriteTo(&key, stdout);
            }
        }

        if(!instance.config.display.pipe)
        {
            fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
            ffPrintColor(&instance.config.display.colorSeparator);
        }

        ffStrbufWriteTo(&instance.config.display.keyValueSeparator, stdout);

        if(!instance.config.display.pipe && instance.config.display.colorSeparator.length)
            fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

        if (!(printType & FF_PRINT_TYPE_NO_CUSTOM_KEY_WIDTH))
        {
            uint32_t keyWidth = moduleArgs && moduleArgs->keyWidth > 0 ? moduleArgs->keyWidth : instance.config.display.keyWidth;
            if (keyWidth > 0)
                printf("\e[%uG", (unsigned) (keyWidth + instance.state.logoWidth));
        }
    }

    if(!instance.config.display.pipe)
    {
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
        if (moduleArgs && moduleArgs->outputColor.length)
            ffPrintColor(&moduleArgs->outputColor);
        else if (instance.config.display.colorOutput.length)
            ffPrintColor(&instance.config.display.colorOutput);
    }
}

void ffPrintFormat(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, uint32_t numArgs, const FFformatarg* arguments)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (moduleArgs)
        ffParseFormatString(&buffer, &moduleArgs->outputFormat, numArgs, arguments);
    else
        ffStrbufAppendS(&buffer, "unknown");

    ffPrintLogoAndKey(moduleName, moduleIndex, moduleArgs, printType);
    ffStrbufPutTo(&buffer, stdout);
}

static void printError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, va_list arguments)
{
    if(!instance.config.display.showErrors)
        return;

    ffPrintLogoAndKey(moduleName, moduleIndex, moduleArgs, printType);

    if(!instance.config.display.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_ERROR, stdout);

    vprintf(message, arguments);

    if(!instance.config.display.pipe)
        fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    putchar('\n');
}

void ffPrintError(const char* moduleName, uint8_t moduleIndex, const FFModuleArgs* moduleArgs, FFPrintType printType, const char* message, ...)
{
    va_list arguments;
    va_start(arguments, message);
    printError(moduleName, moduleIndex, moduleArgs, printType, message, arguments);
    va_end(arguments);
}

void ffPrintColor(const FFstrbuf* colorValue)
{
    //If the color is not set, this would reset in \033[m, which resets everything.
    //So we only print it, if the main color is at least one char.
    if(colorValue->length == 0)
        return;

    printf("\e[%sm", colorValue->chars);
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

void ffPrintModuleFormatHelp(const char* name, const char* def, uint32_t numArgs, const char* args[])
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreateS(name);
    ffStrbufLowerCase(&buffer);
    printf("--%s-format:\n", buffer.chars);
    printf("Sets the format string for %s output.\n", name);
    puts("To see how a format string is constructed, take a look at \"fastfetch --help format\".");
    puts("The following values are passed:");

    for(unsigned i = 0; i < numArgs; i++)
        printf("        {%u}: %s\n", i + 1, args[i]);

    printf("The default is something similar to \"%s\".\n", def);
}
