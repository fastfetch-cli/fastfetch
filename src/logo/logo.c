#include "logo.h"
#include "common/io.h"
#include "common/printing.h"
#include "detection/os/os.h"
#include "detection/terminalshell/terminalshell.h"
#include "util/textModifier.h"

#include <ctype.h>
#include <string.h>

static void ffLogoPrintCharsRaw(FFinstance* instance, const char* data, size_t length)
{
    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
    ffPrintCharTimes('\n', instance->config.logo.paddingTop);
    ffPrintCharTimes(' ', instance->config.logo.paddingLeft);
    fwrite(data, length, 1, stdout);
    instance->state.logoHeight = instance->config.logo.paddingTop + instance->config.logo.height;
    instance->state.logoWidth = instance->config.logo.paddingLeft + instance->config.logo.width + instance->config.logo.paddingRight;
    printf("\033[9999999D\n\033[%uA", instance->state.logoHeight);
}

void ffLogoPrintChars(FFinstance* instance, const char* data, bool doColorReplacement)
{
    uint32_t currentlineLength = 0;

    fputs(FASTFETCH_TEXT_MODIFIER_BOLT, stdout);
    ffPrintCharTimes('\n', instance->config.logo.paddingTop);
    ffPrintCharTimes(' ', instance->config.logo.paddingLeft);

    instance->state.logoHeight = instance->config.logo.paddingTop;

    //Use logoColor[0] as the default color
    if(doColorReplacement)
        ffPrintColor(&instance->config.logo.colors[0]);

    while(*data != '\0')
    {
        //We are at the end of a line. Print paddings and update max line length
        if(*data == '\n' || (*data == '\r' && *(data + 1) == '\n'))
        {
            ffPrintCharTimes(' ', instance->config.logo.paddingRight);

            //We have \r\n, skip the \r
            if(*data == '\r')
                ++data;

            putchar('\n');
            ++data;

            ffPrintCharTimes(' ', instance->config.logo.paddingLeft);

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
                ffPrintColor(&instance->config.logo.colors[index]);
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

    ffPrintCharTimes(' ', instance->config.logo.paddingRight);
    fputs(FASTFETCH_TEXT_MODIFIER_RESET, stdout);

    //Happens if the last line is the longest
    if(currentlineLength > instance->state.logoWidth)
        instance->state.logoWidth = currentlineLength;

    instance->state.logoWidth += instance->config.logo.paddingLeft + instance->config.logo.paddingRight;

    //Go to the leftmost position
    fputs("\033[9999999D", stdout);

    //If the logo height is > 1, go up the height
    if(instance->state.logoHeight > 0)
        printf("\033[%uA", instance->state.logoHeight);
}

static void logoApplyColors(FFinstance* instance, const FFlogo* logo)
{
    if(instance->config.colorKeys.length == 0)
        ffStrbufAppendS(&instance->config.colorKeys, logo->colorKeys);

    if(instance->config.colorTitle.length == 0)
        ffStrbufAppendS(&instance->config.colorTitle, logo->colorTitle);
}

static bool logoHasName(const FFlogo* logo, const char* name)
{
    const char** logoName = logo->names;

    while(*logoName != NULL)
    {
        if(strcasecmp(*logoName, name) == 0)
            return true;
        ++logoName;
    }

    return false;
}

static const FFlogo* logoGetBuiltin(const char* name)
{
    GetLogoMethod* methods = ffLogoBuiltinGetAll();

    while(*methods != NULL)
    {
        const FFlogo* logo = (*methods)();

        if(logoHasName(logo, name))
            return logo;

        ++methods;
    }

    return NULL;
}

static const FFlogo* logoGetBuiltinDetected(FFinstance* instance)
{
    const FFOSResult* os = ffDetectOS(instance);

    const FFlogo* logo = logoGetBuiltin(os->id.chars);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(os->name.chars);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(os->prettyName.chars);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(os->idLike.chars);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(os->systemName.chars);
    if(logo != NULL)
        return logo;

    return ffLogoBuiltinGetUnknown();
}

static inline void logoApplyColorsDetected(FFinstance* instance)
{
    logoApplyColors(instance, logoGetBuiltinDetected(instance));
}

static void logoPrintStruct(FFinstance* instance, const FFlogo* logo)
{
    logoApplyColors(instance, logo);

    const char** colors = logo->builtinColors;
    for(int i = 0; *colors != NULL && i < FASTFETCH_LOGO_MAX_COLORS; i++, colors++)
    {
        if(instance->config.logo.colors[i].length == 0)
            ffStrbufAppendS(&instance->config.logo.colors[i], *colors);
    }

    ffLogoPrintChars(instance, logo->data, true);
}

static void logoPrintNone(FFinstance* instance)
{
    logoApplyColorsDetected(instance);
    instance->state.logoHeight = 0;
    instance->state.logoWidth = 0;
}

static bool logoPrintBuiltinIfExists(FFinstance* instance, const char* name)
{
    if(strcasecmp(name, "none") == 0)
    {
        logoPrintNone(instance);
        return true;
    }

    const FFlogo* logo = logoGetBuiltin(name);
    if(logo == NULL)
        return false;

    logoPrintStruct(instance, logo);

    return true;
}

static inline void logoPrintDetected(FFinstance* instance)
{
    logoPrintStruct(instance, logoGetBuiltinDetected(instance));
}

static void logoPrintData(FFinstance* instance, bool doColorReplacement) {
    ffLogoPrintChars(instance, instance->config.logo.source.chars, doColorReplacement);
    logoApplyColorsDetected(instance);
}

static bool logoPrintFileIfExists(FFinstance* instance, bool doColorReplacement, bool raw)
{
    FFstrbuf content;
    ffStrbufInit(&content);

    if(!ffAppendFileBuffer(instance->config.logo.source.chars, &content))
    {
        ffStrbufDestroy(&content);
        fputs("Logo: Failed to load file content from logo source\n", stderr);
        return false;
    }

    logoApplyColorsDetected(instance);
    if(raw)
        ffLogoPrintCharsRaw(instance, content.chars, content.length);
    else
        ffLogoPrintChars(instance, content.chars, doColorReplacement);
    ffStrbufDestroy(&content);
    return true;
}

static bool logoPrintImageIfExists(FFinstance* instance, FFLogoType logo)
{
    if(!ffLogoPrintImageIfExists(instance, logo))
        return false;

    logoApplyColorsDetected(instance);
    return true;
}

static void logoPrintKnownType(FFinstance* instance)
{
    bool successfull = true;

    if(instance->config.logo.type == FF_LOGO_TYPE_BUILTIN)
        successfull = logoPrintBuiltinIfExists(instance, instance->config.logo.source.chars);
    else if(instance->config.logo.type == FF_LOGO_TYPE_FILE)
        successfull = logoPrintFileIfExists(instance, true, false);
    else if(instance->config.logo.type == FF_LOGO_TYPE_FILE_RAW)
        successfull = logoPrintFileIfExists(instance, false, false);
    else if(instance->config.logo.type == FF_LOGO_TYPE_DATA)
        logoPrintData(instance, true);
    else if(instance->config.logo.type == FF_LOGO_TYPE_DATA_RAW)
        logoPrintData(instance, false);
    else if(instance->config.logo.type == FF_LOGO_TYPE_IMAGE_RAW)
    {
        if(instance->config.logo.width == 0 || instance->config.logo.height == 0)
        {
            fputs("both `--logo-width` and `--logo-height` must be specified\n", stderr);
            successfull = false;
        }
        else
            successfull = logoPrintFileIfExists(instance, false, true);
    }
    else if(instance->config.logo.type == FF_LOGO_TYPE_NONE)
        logoApplyColorsDetected(instance);
    else //image
        successfull = logoPrintImageIfExists(instance, instance->config.logo.type);

    if(!successfull)
        logoPrintDetected(instance);
}

void ffLogoPrint(FFinstance* instance)
{
    //In pipe mode, we don't have a logo or padding.
    //We also don't need to set main color, because it won't be printed anyway.
    //So we can return quickly here.
    if(instance->config.pipe)
    {
        instance->state.logoHeight = 0;
        instance->state.logoWidth = 0;
        return;
    }

    //If the source is not set, we can directly print the detected logo.
    if(instance->config.logo.source.length == 0)
    {
        logoPrintDetected(instance);
        return;
    }

    //If the source and source type is set to something else than auto, always print with the set type.
    if(instance->config.logo.source.length > 0 && instance->config.logo.type != FF_LOGO_TYPE_AUTO)
    {
        logoPrintKnownType(instance);
        return;
    }

    //If source matches the name of a builtin logo, print it and return.
    if(logoPrintBuiltinIfExists(instance, instance->config.logo.source.chars))
        return;

    const FFTerminalShellResult* terminalShell = ffDetectTerminalShell(instance);

    //Terminal emulators that support kitty graphics protocol.
    bool supportsKitty =
        ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "kitty") == 0 ||
        ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "konsole") == 0 ||
        ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "wezterm") == 0 ||
        ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "wayst") == 0;

    //Try to load the logo as an image. If it succeeds, print it and return.
    if(logoPrintImageIfExists(instance, supportsKitty ? FF_LOGO_TYPE_IMAGE_KITTY : FF_LOGO_TYPE_IMAGE_CHAFA))
        return;

    //Try to load the logo as a file. If it succeeds, print it and return.
    if(logoPrintFileIfExists(instance, true, false))
        return;

    logoPrintDetected(instance);
}

void ffLogoPrintLine(FFinstance* instance)
{
    if(instance->state.logoWidth > 0)
        printf("\033[%uC", instance->state.logoWidth);

    ++instance->state.keysHeight;
}

void ffLogoPrintRemaining(FFinstance* instance)
{
    while(instance->state.keysHeight <= instance->state.logoHeight)
    {
        ffLogoPrintLine(instance);
        putchar('\n');
    }
}

void ffLogoBuiltinPrint(FFinstance* instance)
{
    GetLogoMethod* methods = ffLogoBuiltinGetAll();

    while(*methods != NULL)
    {
        const FFlogo* logo = (*methods)();
        printf("\033[%sm%s:\033[0m\n", logo->builtinColors[0], logo->names[0]);
        logoPrintStruct(instance, logo);
        ffLogoPrintRemaining(instance);

        //reset everything
        instance->state.logoHeight = 0;
        instance->state.keysHeight = 0;
        for(uint8_t i = 0; i < FASTFETCH_LOGO_MAX_COLORS; i++)
            ffStrbufClear(&instance->config.logo.colors[i]);

        puts("\n");
        ++methods;
    }
}

void ffLogoBuiltinList()
{
    GetLogoMethod* methods = ffLogoBuiltinGetAll();

    uint32_t counter = 0;

    while(*methods != NULL)
    {
        const FFlogo* logo = (*methods)();
        const char** names = logo->names;

        printf("%u)%s ", counter, counter < 10 ? " " : "");
        ++counter;

        while(*names != NULL)
        {
            printf("\"%s\" ", *names);
            ++names;
        }

        putchar('\n');
        ++methods;
    }
}

void ffLogoBuiltinListAutocompletion()
{
    GetLogoMethod* methods = ffLogoBuiltinGetAll();

    while(*methods != NULL)
    {
        printf("%s\n", (*methods)()->names[0]);
        ++methods;
    }
}
