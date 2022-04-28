#include "logo.h"

#include <ctype.h>

void ffLogoPrint(FFinstance* instance, const char* data, bool doColorReplacement)
{
    uint32_t currentlineLength = 0;

    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
    ffPrintCharTimes(' ', instance->config.logoPaddingLeft);

    //Use logoColor[0] as the default color
    if(doColorReplacement)
        ffPrintColor(&instance->config.logoColors[0]);

    while(*data != '\0')
    {
        //We are at the end of a line. Print paddings and update max line length
        if(*data == '\n' || (*data == '\r' && *(data + 1) == '\n'))
        {
            ffPrintCharTimes(' ', instance->config.logoPaddingRight);

            //We have \r\n, skip the \r
            if(*data == '\r')
                ++data;

            putchar('\n');
            ++data;

            ffPrintCharTimes(' ', instance->config.logoPaddingLeft);

            if(currentlineLength > instance->state.logoWidth)
                instance->state.logoWidth = currentlineLength;

            currentlineLength = 0;
            ++instance->state.logoHeight;
            continue;
        }

        //Always print tabs as 4 spaces, to have consistent spacing
        if(*data == '\t')
        {
            ffPrintCharTimes(' ', 4);
            ++data;
            continue;
        }

        //We have an escape sequence direclty as bytes. We print it, but don't increase the line length
        if(*data == '\033' && *(data + 1) == '[')
        {
            const char* start = data;

            fputs("\033[", stdout);
            data += 2;

            while(isdigit(*data) || *data == ';')
                putchar(*data++); // number

            //We have a valid control sequence, print it and continue with next char
            if(isascii(*data))
            {
                putchar(*data++); // single letter, end of control sequence
                continue;
            }

            //Invalid control sequence, try to get most accurate length
            currentlineLength += (uint32_t) (data - start - 1); //-1 for \033 which for sure doesn't take any space

            //Don't continue here, print the char after the letters with the unicode printing
        }

        //We have a fastfetch color placeholder. Replace it with the esacape sequence, don't increase the line length
        if(doColorReplacement && *data == '$')
        {
            ++data;

            //If we have $$, or $\0, print it as single $
            if(*data == '$' || *data == '\0')
            {
                putchar('$');
                ++currentlineLength;
                ++data;
                continue;
            }

            //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
            int index = ((int) *data) - 49;

            //If the index is valid, print the color. Otherwise continue as normal
            if(index < 0 || index >= FASTFETCH_LOGO_MAX_COLORS)
            {
                putchar('$');
                ++currentlineLength;
                //Don't continue here, we want to print the current char as unicode
            }
            else
            {
                ffPrintColor(&instance->config.logoColors[index]);
                ++data;
                continue;
            }
        }

        //Do the printing, respecting unicode

        ++currentlineLength;

        int codepoint = (unsigned char) *data;
        uint8_t bytes;

        if(codepoint <= 127)
            bytes = 1;
        else if((codepoint & 0xE0) == 0xC0)
            bytes = 2;
        else if((codepoint & 0xF0) == 0xE0)
            bytes = 3;
        else if((codepoint & 0xF8) == 0xF0)
            bytes = 4;
        else
            bytes = 1; //Invalid utf8, print it as is, byte by byte

        for(uint8_t i = 0; i < bytes; ++i)
        {
            if(*data == '\0')
                break;

            putchar(*data++);
        }
    }

    ffPrintCharTimes(' ', instance->config.logoPaddingRight);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    //Happens if the last line is the longest
    if(currentlineLength > instance->state.logoWidth)
        instance->state.logoWidth = currentlineLength;

    instance->state.logoWidth += instance->config.logoPaddingLeft + instance->config.logoPaddingRight;

    //Go to the leftmost position
    fputs("\033[9999999D", stdout);

    //If the logo height is > 1, go up the height
    if(instance->state.logoHeight > 0)
        printf("\033[%uA", instance->state.logoHeight);
}

static void logoPrintFile(FFinstance* instance, bool doColorReplacement)
{
    FFstrbuf content;
    ffStrbufInitA(&content, 2047);

    if(ffAppendFileContent(instance->config.logoSource.chars, &content))
        ffLogoPrint(instance, content.chars, doColorReplacement);
    else
        ffLogoPrintUnknown(instance);
}

static void logoPrintDetected(FFinstance* instance)
{
    if(instance->config.logoSource.length > 0)
    {
        if(
            !ffLogoPrintBuiltinIfExists(instance) &&
            !ffLogoPrintImageIfExists(instance, FF_LOGO_TYPE_KITTY) &&
            !ffLogoPrintImageIfExists(instance, FF_LOGO_TYPE_CHAFA)
        ) logoPrintFile(instance, true);
        return;
    }

    ffLogoPrintBuiltinDetected(instance);
}

void ffPrintLogo(FFinstance* instance)
{
    if(instance->config.mainColor.length == 0)
        ffLogoSetMainColor(instance);

    if(( //Logo type needs set logo name, but nothing was given. Print question mark
        instance->config.logoType == FF_LOGO_TYPE_BUILTIN ||
        instance->config.logoType == FF_LOGO_TYPE_FILE ||
        instance->config.logoType == FF_LOGO_TYPE_RAW ||
        instance->config.logoType == FF_LOGO_TYPE_SIXEL ||
        instance->config.logoType == FF_LOGO_TYPE_KITTY ||
        instance->config.logoType == FF_LOGO_TYPE_CHAFA
    ) && instance->config.logoSource.length == 0)
        ffLogoPrintUnknown(instance);
    else if(instance->config.logoType == FF_LOGO_TYPE_BUILTIN)
    {
        if(!ffLogoPrintBuiltinIfExists(instance))
            ffLogoPrintUnknown(instance);
    }
    else if(instance->config.logoType == FF_LOGO_TYPE_FILE)
        logoPrintFile(instance, true);
    else if(instance->config.logoType == FF_LOGO_TYPE_RAW)
        logoPrintFile(instance, false);
    else if(instance->config.logoType == FF_LOGO_TYPE_SIXEL || instance->config.logoType == FF_LOGO_TYPE_KITTY || instance->config.logoType == FF_LOGO_TYPE_CHAFA)
    {
        if(!ffLogoPrintImageIfExists(instance, instance->config.logoType))
            ffLogoPrintBuiltinDetected(instance);
    }
    else
        logoPrintDetected(instance);
}

static inline void printLogoWidth(const FFinstance* instance)
{
    if(instance->state.logoWidth > 0)
        printf("\033[%uC", instance->state.logoWidth);
}

void ffPrintRemainingLogo(FFinstance* instance)
{
    if(!instance->config.logoPrintRemaining)
        return;

    for(uint32_t i = instance->state.keysHeight; i <= instance->state.logoHeight; i++)
    {
        printLogoWidth(instance);
        putchar('\n');
    }
}

void ffPrintLogoLine(FFinstance* instance)
{
    printLogoWidth(instance);
    ++instance->state.keysHeight;
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}

void ffPrintLogoAndKey(FFinstance* instance, const char* moduleName, uint8_t moduleIndex, const FFstrbuf* customKeyFormat)
{
    ffPrintLogoLine(instance);

    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
    ffPrintColor(&instance->config.mainColor);

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

    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
    ffStrbufWriteTo(&instance->config.separator, stdout);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);
}
