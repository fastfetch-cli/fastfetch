#include "logo/logo.h"
#include "common/io/io.h"
#include "common/printing.h"
#include "detection/os/os.h"
#include "detection/terminalshell/terminalshell.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <string.h>

typedef enum FFLogoSize
{
    FF_LOGO_SIZE_UNKNOWN,
    FF_LOGO_SIZE_NORMAL,
    FF_LOGO_SIZE_SMALL,
} FFLogoSize;

static void ffLogoPrintCharsRaw(const char* data, size_t length)
{
    FFOptionsLogo* options = &instance.config.logo;
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();

    if (!options->width || !options->height)
    {
        ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH",
            (unsigned) options->paddingTop,
            (unsigned) options->paddingLeft
        );
        ffStrbufAppendNS(&buf, (uint32_t) length, data);
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

        uint16_t X = 0, Y = 0;
        const char* error = ffGetTerminalResponse("\e[6n", "\e[%hu;%huR", &Y, &X);
        if (error)
        {
            fprintf(stderr, "\nLogo (image-raw): fail to query cursor position: %s\n", error);
            return;
        }
        instance.state.logoWidth = X + instance.config.logo.paddingRight;
        instance.state.logoHeight = Y;
        fputs("\e[H", stdout);
    }
    else
    {
        ffStrbufAppendNC(&buf, options->paddingTop, '\n');
        ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        ffStrbufAppendNS(&buf, (uint32_t) length, data);
        instance.state.logoHeight = options->paddingTop + options->height;
        instance.state.logoWidth = options->paddingLeft + options->width + options->paddingRight;
        ffStrbufAppendF(&buf, "\n\e[%uA", instance.state.logoHeight);
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
    }
}

void ffLogoPrintChars(const char* data, bool doColorReplacement)
{
    FFOptionsLogo* options = &instance.config.logo;

    uint32_t currentlineLength = 0;

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(2048);

    if (!instance.config.display.pipe && instance.config.display.brightColor)
        ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_BOLT);

    ffStrbufAppendNC(&result, options->paddingTop, '\n');
    ffStrbufAppendNC(&result, options->paddingLeft, ' ');

    instance.state.logoHeight = options->paddingTop;

    //Use logoColor[0] as the default color
    if(doColorReplacement && !instance.config.display.pipe)
        ffStrbufAppendF(&result, "\e[%sm", options->colors[0].chars);

    while(*data != '\0')
    {
        //We are at the end of a line. Print paddings and update max line length
        if(*data == '\n' || (*data == '\r' && *(data + 1) == '\n'))
        {
            ffStrbufAppendNC(&result, options->paddingRight, ' ');

            //We have \r\n, skip the \r
            if(*data == '\r')
                ++data;

            ffStrbufAppendC(&result, '\n');
            ++data;

            ffStrbufAppendNC(&result, options->paddingLeft, ' ');

            if(currentlineLength > instance.state.logoWidth)
                instance.state.logoWidth = currentlineLength;

            currentlineLength = 0;
            ++instance.state.logoHeight;
            continue;
        }

        //Always print tabs as 4 spaces, to have consistent spacing
        if(*data == '\t')
        {
            ffStrbufAppendNC(&result, 4, ' ');
            ++data;
            continue;
        }

        //We have an escape sequence directly as bytes. We print it, but don't increase the line length
        if(*data == '\e' && *(data + 1) == '[')
        {
            const char* start = data;

            ffStrbufAppendS(&result, "\e[");
            data += 2;

            while(isdigit(*data) || *data == ';')
                ffStrbufAppendC(&result, *data++); // number

            //We have a valid control sequence, print it and continue with next char
            if(isascii(*data))
            {
                ffStrbufAppendC(&result, *data++); // single letter, end of control sequence
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
                ffStrbufAppendC(&result, '$');
                ++currentlineLength;
                ++data;
                continue;
            }

            if(!instance.config.display.pipe)
            {
                //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
                int index = ((int) *data) - 49;

                //If the index is valid, print the color. Otherwise continue as normal
                if(index < 0 || index >= FASTFETCH_LOGO_MAX_COLORS)
                {
                    ffStrbufAppendC(&result, '$');
                    ++currentlineLength;
                    //Don't continue here, we want to print the current char as unicode
                }
                else
                {
                    ffStrbufAppendF(&result, "\e[%sm", options->colors[index].chars);
                    ++data;
                    continue;
                }
            }
            else
            {
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

            ffStrbufAppendC(&result, *data++);
        }
    }

    if(!instance.config.display.pipe)
        ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_RESET);

    if(!options->separate)
    {
        //Happens if the last line is the longest
        if(currentlineLength > instance.state.logoWidth)
            instance.state.logoWidth = currentlineLength;

        instance.state.logoWidth += options->paddingLeft + options->paddingRight;

        //Go to the leftmost position and go up the height
        ffStrbufAppendF(&result, "\e[1G\e[%uA", instance.state.logoHeight);
    }
    else
    {
        instance.state.logoWidth = instance.state.logoHeight = 0;
        ffStrbufAppendNC(&result, options->paddingRight, '\n');
    }

    ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &result);
}

static void logoApplyColors(const FFlogo* logo)
{
    if(instance.config.display.colorTitle.length == 0)
        ffStrbufAppendS(&instance.config.display.colorTitle, logo->colorTitle ? logo->colorTitle : logo->colors[0]);

    if(instance.config.display.colorKeys.length == 0)
        ffStrbufAppendS(&instance.config.display.colorKeys, logo->colorKeys ? logo->colorKeys : logo->colors[1]);
}

static bool logoHasName(const FFlogo* logo, const FFstrbuf* name, bool small)
{
    for(
        const char* const* logoName = logo->names;
        *logoName != NULL && logoName <= &logo->names[FASTFETCH_LOGO_MAX_NAMES];
        ++logoName
    ) {
        if(small)
        {
            uint32_t logoNameLength = (uint32_t) (strlen(*logoName) - strlen("_small"));
            if(name->length == logoNameLength && strncasecmp(*logoName, name->chars, logoNameLength) == 0) return true;
        }
        if(ffStrbufIgnCaseEqualS(name, *logoName))
            return true;
    }

    return false;
}

static const FFlogo* logoGetBuiltin(const FFstrbuf* name, FFLogoSize size)
{
    if (name->length == 0 || !isalpha(name->chars[0]))
        return NULL;

    for(const FFlogo* logo = ffLogoBuiltins[toupper(name->chars[0]) - 'A']; *logo->names; ++logo)
    {
        switch (size)
        {
            // Never use alternate logos
            case FF_LOGO_SIZE_NORMAL:
                if(logo->type != FF_LOGO_LINE_TYPE_NORMAL) continue;
                break;
            case FF_LOGO_SIZE_SMALL:
                if(logo->type != FF_LOGO_LINE_TYPE_SMALL_BIT) continue;
                break;
            default:
                break;
        }

        if(logoHasName(logo, name, size == FF_LOGO_SIZE_SMALL))
            return logo;
    }

    return NULL;
}

static const FFlogo* logoGetBuiltinDetected(FFLogoSize size)
{
    const FFOSResult* os = ffDetectOS();

    const FFlogo* logo = logoGetBuiltin(&os->id, size);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(&os->name, size);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(&os->prettyName, size);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(&os->idLike, size);
    if(logo != NULL)
        return logo;

    logo = logoGetBuiltin(&instance.state.platform.systemName, size);
    if(logo != NULL)
        return logo;

    return &ffLogoUnknown;
}

static inline void logoApplyColorsDetected(void)
{
    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL));
}

static void logoPrintStruct(const FFlogo* logo)
{
    logoApplyColors(logo);

    FFOptionsLogo* options = &instance.config.logo;

    const char* const* colors = logo->colors;
    for(int i = 0; *colors != NULL && i < FASTFETCH_LOGO_MAX_COLORS; i++, colors++)
    {
        if(options->colors[i].length == 0)
            ffStrbufAppendS(&options->colors[i], *colors);
    }

    ffLogoPrintChars(logo->lines, true);
}

static void logoPrintNone(void)
{
    logoApplyColorsDetected();
    instance.state.logoHeight = 0;
    instance.state.logoWidth = 0;
}

static bool logoPrintBuiltinIfExists(const FFstrbuf* name, FFLogoSize size)
{
    if(ffStrbufIgnCaseEqualS(name, "none"))
    {
        logoPrintNone();
        return true;
    }

    const FFlogo* logo = logoGetBuiltin(name, size);
    if(logo == NULL)
        return false;

    logoPrintStruct(logo);

    return true;
}

static inline void logoPrintDetected(FFLogoSize size)
{
    logoPrintStruct(logoGetBuiltinDetected(size));
}

static bool logoPrintData(bool doColorReplacement)
{
    FFOptionsLogo* options = &instance.config.logo;
    if(options->source.length == 0)
        return false;

    ffLogoPrintChars(options->source.chars, doColorReplacement);
    logoApplyColorsDetected();
    return true;
}

static void updateLogoPath(void)
{
    FFOptionsLogo* options = &instance.config.logo;

    if(ffPathExists(options->source.chars, FF_PATHTYPE_FILE))
        return;

    FF_STRBUF_AUTO_DESTROY fullPath = ffStrbufCreate();
    if (ffPathExpandEnv(options->source.chars, &fullPath) && ffPathExists(fullPath.chars, FF_PATHTYPE_FILE))
    {
        ffStrbufSet(&options->source, &fullPath);
        return;
    }

    FF_LIST_FOR_EACH(FFstrbuf, dataDir, instance.state.platform.dataDirs)
    {
        //We need to copy it, because multiple threads might be using dataDirs at the same time
        ffStrbufSet(&fullPath, dataDir);
        ffStrbufAppendS(&fullPath, "fastfetch/logos/");
        ffStrbufAppend(&fullPath, &options->source);

        if(ffPathExists(fullPath.chars, FF_PATHTYPE_FILE))
        {
            ffStrbufSet(&options->source, &fullPath);
            break;
        }
    }
}

static bool logoPrintFileIfExists(bool doColorReplacement, bool raw)
{
    FFOptionsLogo* options = &instance.config.logo;

    FF_STRBUF_AUTO_DESTROY content = ffStrbufCreate();

    if(ffStrbufEqualS(&options->source, "-")
        ? !ffAppendFDBuffer(FFUnixFD2NativeFD(STDIN_FILENO), &content)
        : !ffAppendFileBuffer(options->source.chars, &content)
    )
    {
        if (instance.config.display.showErrors)
            fprintf(stderr, "Logo: Failed to load file content from logo source: %s \n", options->source.chars);
        return false;
    }

    logoApplyColorsDetected();
    if(raw)
        ffLogoPrintCharsRaw(content.chars, content.length);
    else
        ffLogoPrintChars(content.chars, doColorReplacement);

    return true;
}

static bool logoPrintImageIfExists(FFLogoType logo, bool printError)
{
    if(!ffLogoPrintImageIfExists(logo, printError))
        return false;

    logoApplyColorsDetected();
    return true;
}

static bool logoTryKnownType(void)
{
    FFOptionsLogo* options = &instance.config.logo;

    if(options->type == FF_LOGO_TYPE_NONE)
    {
        logoApplyColorsDetected();
        return true;
    }

    if(options->type == FF_LOGO_TYPE_BUILTIN)
        return logoPrintBuiltinIfExists(&options->source, FF_LOGO_SIZE_UNKNOWN);

    if(options->type == FF_LOGO_TYPE_SMALL)
        return logoPrintBuiltinIfExists(&options->source, FF_LOGO_SIZE_SMALL);

    if(options->type == FF_LOGO_TYPE_DATA)
        return logoPrintData(true);

    if(options->type == FF_LOGO_TYPE_DATA_RAW)
        return logoPrintData(false);

    updateLogoPath(); //We sure have a file, resolve relative paths

    if(options->type == FF_LOGO_TYPE_FILE)
        return logoPrintFileIfExists(true, false);

    if(options->type == FF_LOGO_TYPE_FILE_RAW)
        return logoPrintFileIfExists(false, false);

    if(options->type == FF_LOGO_TYPE_IMAGE_RAW)
        return logoPrintFileIfExists(false, true);

    return logoPrintImageIfExists(options->type, instance.config.display.showErrors);
}

void ffLogoPrint(void)
{
    //When generate JSON result, we don't have a logo or padding.
    //We also don't need to set main color, because it won't be printed anyway.
    //So we can return quickly here.
    if(instance.state.resultDoc)
    {
        instance.state.logoHeight = 0;
        instance.state.logoWidth = 0;
        return;
    }

    const FFOptionsLogo* options = &instance.config.logo;

    if (options->type == FF_LOGO_TYPE_NONE)
    {
        logoPrintNone();
        return;
    }

    //If the source is not set, we can directly print the detected logo.
    if(options->source.length == 0)
    {
        logoPrintDetected(options->type == FF_LOGO_TYPE_SMALL ? FF_LOGO_SIZE_SMALL : FF_LOGO_SIZE_NORMAL);
        return;
    }

    //If the source and source type is set to something else than auto, always print with the set type.
    if(options->source.length > 0 && options->type != FF_LOGO_TYPE_AUTO)
    {
        if(!logoTryKnownType())
        {
            if (instance.config.display.showErrors)
            {
                // Image logo should have been handled
                if(options->type == FF_LOGO_TYPE_BUILTIN || options->type == FF_LOGO_TYPE_SMALL)
                    fprintf(stderr, "Logo: Failed to load %s logo: %s \n", options->type == FF_LOGO_TYPE_BUILTIN ? "builtin" : "builtin small", options->source.chars);
            }

            logoPrintDetected(FF_LOGO_SIZE_UNKNOWN);
        }
        return;
    }

    //If source matches the name of a builtin logo, print it and return.
    if(logoPrintBuiltinIfExists(&options->source, FF_LOGO_SIZE_UNKNOWN))
        return;

    //Make sure the logo path is set correctly.
    updateLogoPath();

    const FFTerminalResult* terminal = ffDetectTerminal();

    //Terminal emulators that support kitty graphics protocol.
    bool supportsKitty =
        ffStrbufIgnCaseEqualS(&terminal->processName, "kitty") ||
        ffStrbufIgnCaseEqualS(&terminal->processName, "konsole") ||
        ffStrbufIgnCaseEqualS(&terminal->processName, "wezterm") ||
        ffStrbufIgnCaseEqualS(&terminal->processName, "wayst");

    //Try to load the logo as an image. If it succeeds, print it and return.
    if(logoPrintImageIfExists(supportsKitty ? FF_LOGO_TYPE_IMAGE_KITTY : FF_LOGO_TYPE_IMAGE_CHAFA, false))
        return;

    //Try to load the logo as a file. If it succeeds, print it and return.
    if(logoPrintFileIfExists(true, false))
        return;

    logoPrintDetected(FF_LOGO_SIZE_UNKNOWN);
}

void ffLogoPrintLine(void)
{
    if(instance.state.logoWidth > 0)
        printf("\033[%uC", instance.state.logoWidth);

    ++instance.state.keysHeight;
}

void ffLogoPrintRemaining(void)
{
    if (instance.state.keysHeight <= instance.state.logoHeight)
        ffPrintCharTimes('\n', instance.state.logoHeight - instance.state.keysHeight + 1);
    instance.state.keysHeight = instance.state.logoHeight + 1;
}

void ffLogoBuiltinPrint(void)
{
    FFOptionsLogo* options = &instance.config.logo;

    for(uint8_t ch = 0; ch < 26; ++ch)
    {
        for(const FFlogo* logo = ffLogoBuiltins[ch]; *logo->names; ++logo)
        {
            printf("\033[%sm%s:\033[0m\n", logo->colors[0], logo->names[0]);
            logoPrintStruct(logo);
            ffLogoPrintRemaining();

            //reset everything
            instance.state.logoHeight = 0;
            instance.state.keysHeight = 0;
            for(uint8_t i = 0; i < FASTFETCH_LOGO_MAX_COLORS; i++)
                ffStrbufClear(&options->colors[i]);

            putchar('\n');
        }
    }
}

void ffLogoBuiltinList(void)
{
    uint32_t counter = 0;
    for(uint8_t ch = 0; ch < 26; ++ch)
    {
        for(const FFlogo* logo = ffLogoBuiltins[ch]; *logo->names; ++logo)
        {
            ++counter;
            printf("%u)%s ", counter, counter < 10 ? " " : "");

            for(
                const char* const* names = logo->names;
                *names != NULL && names <= &logo->names[FASTFETCH_LOGO_MAX_NAMES];
                ++names
            )
                printf("\"%s\" ", *names);

            putchar('\n');
        }
    }
}

void ffLogoBuiltinListAutocompletion(void)
{
    for(uint8_t ch = 0; ch < 26; ++ch)
    {
        for(const FFlogo* logo = ffLogoBuiltins[ch]; *logo->names; ++logo)
            printf("%s\n", logo->names[0]);
    }
}
