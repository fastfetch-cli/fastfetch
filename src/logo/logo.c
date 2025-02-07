#include "logo/logo.h"
#include "common/io/io.h"
#include "common/printing.h"
#include "detection/os/os.h"
#include "detection/terminalshell/terminalshell.h"
#include "util/textModifier.h"
#include "util/stringUtils.h"

#include <ctype.h>
#include <string.h>

typedef enum __attribute__((__packed__)) FFLogoSize
{
    FF_LOGO_SIZE_UNKNOWN,
    FF_LOGO_SIZE_NORMAL,
    FF_LOGO_SIZE_SMALL,
} FFLogoSize;

static bool ffLogoPrintCharsRaw(const char* data, size_t length, bool printError)
{
    FFOptionsLogo* options = &instance.config.logo;
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();

    if (!options->width || !options->height)
    {
        if (options->position == FF_LOGO_POSITION_LEFT)
        {
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;%uH",
                (unsigned) options->paddingTop + 1,
                (unsigned) options->paddingLeft + 1
            );
        }
        else if (options->position == FF_LOGO_POSITION_TOP)
        {
            ffStrbufAppendNC(&buf, options->paddingTop, '\n');
            ffStrbufAppendNC(&buf, options->paddingLeft, ' ');
        }
        else if (options->position == FF_LOGO_POSITION_RIGHT)
        {
            if (!options->width)
            {
                if (printError)
                    fputs("Logo (image-raw): Must set logo width when using position right\n", stderr);
                return false;
            }
            ffStrbufAppendF(&buf, "\e[2J\e[3J\e[%u;9999999H\e[%uD", (unsigned) options->paddingTop + 1, (unsigned) options->paddingRight + options->width);
        }
        ffStrbufAppendNS(&buf, (uint32_t) length, data);
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);

        if (options->position == FF_LOGO_POSITION_LEFT || options->position == FF_LOGO_POSITION_RIGHT)
        {
            uint16_t X = 0, Y = 0;
            // Windows Terminal doesn't report `\e` for some reason
            const char* error = ffGetTerminalResponse("\e[6n", 2, "%*[^0-9]%hu;%huR", &Y, &X); // %*[^0-9]: ignore optional \e[
            if (error)
            {
                if (printError)
                    fprintf(stderr, "\nLogo (image-raw): fail to query cursor position: %s\n", error);
                return true;
            }
            if (options->position == FF_LOGO_POSITION_LEFT)
            {
                if (options->width + options->paddingLeft > X)
                    X = (uint16_t) (options->width + options->paddingLeft);
                instance.state.logoWidth = X + instance.config.logo.paddingRight - 1;
            }
            instance.state.logoHeight = Y;
            fputs("\e[H", stdout);
        }
        else if (options->position == FF_LOGO_POSITION_TOP)
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffPrintCharTimes('\n', options->paddingRight);
        }
    }
    else
    {
        ffStrbufAppendNC(&buf, options->paddingTop, '\n');

        if (options->position == FF_LOGO_POSITION_RIGHT)
            ffStrbufAppendF(&buf, "\e[9999999C\e[%uD", (unsigned) options->paddingRight + options->width);
        else if (options->paddingLeft)
            ffStrbufAppendF(&buf, "\e[%uC", (unsigned) options->paddingLeft);

        ffStrbufAppendNS(&buf, (uint32_t) length, data);
        ffStrbufAppendC(&buf, '\n');

        if (options->position == FF_LOGO_POSITION_LEFT)
        {
            instance.state.logoWidth = options->width + options->paddingLeft + options->paddingRight;
            instance.state.logoHeight = options->paddingTop + options->height;
            ffStrbufAppendF(&buf, "\e[%uA", (unsigned) instance.state.logoHeight);
        }
        else if (options->position == FF_LOGO_POSITION_TOP)
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffStrbufAppendNC(&buf, options->paddingRight, '\n');
        }
        else if (options->position == FF_LOGO_POSITION_RIGHT)
        {
            instance.state.logoWidth = instance.state.logoHeight = 0;
            ffStrbufAppendF(&buf, "\e[%uA", (unsigned) options->height);
        }
        ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &buf);
    }

    return true;
}

// If result is NULL, calculate logo width
// Returns logo height
static uint32_t logoAppendChars(const char* data, bool doColorReplacement, FFstrbuf* result)
{
    FFOptionsLogo* options = &instance.config.logo;
    uint32_t currentlineLength = 0;
    uint32_t logoHeight = 0;

    if (result)
    {
        if (options->position != FF_LOGO_POSITION_RIGHT)
            ffStrbufAppendNC(result, options->paddingLeft, ' ');
        else
            ffStrbufAppendF(result, "\e[9999999C\e[%dD", options->paddingRight + instance.state.logoWidth);
    }

    while(*data != '\0')
    {
        //We are at the end of a line. Print paddings and update max line length
        if(*data == '\n' || (*data == '\r' && *(data + 1) == '\n'))
        {
            //We have \r\n, skip the \r
            if(*data == '\r')
                ++data;

            if(result) ffStrbufAppendC(result, '\n');
            ++data;

            if (result)
            {
                if (options->position != FF_LOGO_POSITION_RIGHT)
                    ffStrbufAppendNC(result, options->paddingLeft, ' ');
                else
                    ffStrbufAppendF(result, "\e[9999999C\e[%dD", options->paddingRight + instance.state.logoWidth);
            }

            if(currentlineLength > instance.state.logoWidth)
                instance.state.logoWidth = currentlineLength;

            currentlineLength = 0;
            ++logoHeight;
            continue;
        }

        //Always print tabs as 4 spaces, to have consistent spacing
        if(*data == '\t')
        {
            if(result) ffStrbufAppendNC(result, 4, ' ');
            ++data;
            continue;
        }

        //We have an escape sequence directly as bytes. We print it, but don't increase the line length
        if(*data == '\e' && *(data + 1) == '[')
        {
            const char* start = data;

            if(result) ffStrbufAppendS(result, "\e[");
            data += 2;

            while(ffCharIsDigit(*data) || *data == ';')
            {
                if(result) ffStrbufAppendC(result, *data); // number
                ++data;
            }

            //We have a valid control sequence, print it and continue with next char
            if(isascii(*data))
            {
                if(result) ffStrbufAppendC(result, *data); // single letter, end of control sequence
                ++data;
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
                if(result) ffStrbufAppendC(result, '$');
                ++currentlineLength;
                ++data;
                continue;
            }

            if(!instance.config.display.pipe)
            {
                //Map the number to an array index, so that '1' -> 0, '2' -> 1, etc.
                int index = *data - '1';

                //If the index is valid, print the color. Otherwise continue as normal
                if(index < 0 || index >= FASTFETCH_LOGO_MAX_COLORS)
                {
                    if(result) ffStrbufAppendC(result, '$');
                    ++currentlineLength;
                    //Don't continue here, we want to print the current char as unicode
                }
                else
                {
                    if(result) ffStrbufAppendF(result, "\e[%sm", options->colors[index].chars);
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

            if(result) ffStrbufAppendC(result, *data);
            ++data;
        }
    }
    //Happens if the last line is the longest
    if(currentlineLength > instance.state.logoWidth)
        instance.state.logoWidth = currentlineLength;

    return logoHeight;
}

void ffLogoPrintChars(const char* data, bool doColorReplacement)
{
    FFOptionsLogo* options = &instance.config.logo;

    if (options->position == FF_LOGO_POSITION_RIGHT)
        logoAppendChars(data, doColorReplacement, NULL);

    FF_STRBUF_AUTO_DESTROY result = ffStrbufCreateA(2048);

    if (!instance.config.display.pipe && instance.config.display.brightColor)
        ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_BOLT);

    ffStrbufAppendNC(&result, options->paddingTop, '\n');

    //Use logoColor[0] as the default color
    if(doColorReplacement && !instance.config.display.pipe)
        ffStrbufAppendF(&result, "\e[%sm", options->colors[0].chars);

    instance.state.logoHeight = options->paddingTop + logoAppendChars(data, doColorReplacement, &result);

    if(!instance.config.display.pipe)
        ffStrbufAppendS(&result, FASTFETCH_TEXT_MODIFIER_RESET);

    if(options->position == FF_LOGO_POSITION_LEFT)
    {
        instance.state.logoWidth += options->paddingLeft + options->paddingRight;

        //Go to the leftmost position and go up the height
        ffStrbufAppendF(&result, "\e[1G\e[%uA", instance.state.logoHeight);
    }
    else if(options->position == FF_LOGO_POSITION_RIGHT)
    {
        instance.state.logoWidth = 0;

        //Go to the leftmost position and go up the height
        ffStrbufAppendF(&result, "\e[1G\e[%uA", instance.state.logoHeight);
    }
    else if (options->position == FF_LOGO_POSITION_TOP)
    {
        instance.state.logoWidth = instance.state.logoHeight = 0;
        ffStrbufAppendNC(&result, options->paddingRight, '\n');
    }

    ffWriteFDBuffer(FFUnixFD2NativeFD(STDOUT_FILENO), &result);
}

static void logoApplyColors(const FFlogo* logo, bool replacement)
{
    if(instance.config.display.colorTitle.length == 0)
        ffStrbufAppendS(&instance.config.display.colorTitle, logo->colorTitle ? logo->colorTitle : logo->colors[0]);

    if(instance.config.display.colorKeys.length == 0)
        ffStrbufAppendS(&instance.config.display.colorKeys, logo->colorKeys ? logo->colorKeys : logo->colors[1]);

    if (replacement)
    {
        FFOptionsLogo* options = &instance.config.logo;

        const char* const* colors = logo->colors;
        for(int i = 0; *colors != NULL && i < FASTFETCH_LOGO_MAX_COLORS; i++, colors++)
        {
            if(options->colors[i].length == 0)
                ffStrbufAppendS(&options->colors[i], *colors);
        }
    }
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

    if (ffStrbufContainC(&os->idLike, ' '))
    {
        FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
        for (
            uint32_t start = 0, end = ffStrbufFirstIndexC(&os->idLike, ' ');
            true;
            start = end + 1, end = ffStrbufNextIndexC(&os->idLike, start, ' ')
        )
        {
            ffStrbufSetNS(&buf, end - start, os->idLike.chars + start);
            logo = logoGetBuiltin(&buf, size);
            if(logo != NULL)
                return logo;

            if (end >= os->idLike.length)
                break;
        }
    }
    else
    {
        logo = logoGetBuiltin(&os->idLike, size);
        if(logo != NULL)
            return logo;
    }

    logo = logoGetBuiltin(&instance.state.platform.sysinfo.name, size);
    if(logo != NULL)
        return logo;

    return &ffLogoUnknown;
}

static void logoPrintStruct(const FFlogo* logo)
{
    logoApplyColors(logo, true);

    ffLogoPrintChars(logo->lines, true);
}

static void logoPrintNone(void)
{
    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), false);
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

    const FFlogo* logo = ffStrbufEqualS(name, "?") ? &ffLogoUnknown : logoGetBuiltin(name, size);
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

    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), doColorReplacement);
    ffLogoPrintChars(options->source.chars, doColorReplacement);
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

    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), doColorReplacement);
    if(raw)
        return ffLogoPrintCharsRaw(content.chars, content.length, instance.config.display.showErrors);

    ffLogoPrintChars(content.chars, doColorReplacement);
    return true;
}

static bool logoPrintImageIfExists(FFLogoType logo, bool printError)
{
    if(!ffLogoPrintImageIfExists(logo, printError))
        return false;

    logoApplyColors(logoGetBuiltinDetected(FF_LOGO_SIZE_NORMAL), false);
    return true;
}

static bool logoTryKnownType(void)
{
    FFOptionsLogo* options = &instance.config.logo;

    if(options->type == FF_LOGO_TYPE_NONE)
    {
        logoPrintNone();
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
        ffStrbufIgnCaseEqualS(&terminal->processName, "wayst") ||
        ffStrbufIgnCaseEqualS(&terminal->processName, "ghostty");

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
