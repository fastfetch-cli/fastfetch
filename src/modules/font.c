#include "fastfetch.h"
#include "common/printing.h"
#include "detection/font/font.h"
#include "detection/displayserver/displayserver.h"

#define FF_FONT_MODULE_NAME "Font"
#define FF_FONT_NUM_FORMAT_ARGS 4

#if defined(__linux__) || defined(__FreeBSD__)
#include "common/parsing.h"

static void printFont(const FFFontResult* font)
{
    FFstrbuf gtk;
    ffStrbufInit(&gtk);
    ffParseGTK(&gtk, &font->fonts[1], &font->fonts[2], &font->fonts[3]);

    if(font->fonts[0].length > 0)
    {
        printf("%s [QT]", font->fonts[0].chars);
        if(gtk.length > 0)
            fputs(", ", stdout);
    }

    ffStrbufWriteTo(&gtk, stdout);
    ffStrbufDestroy(&gtk);
}

#elif defined(__APPLE__)

static void printFont(const FFFontResult* font)
{
    if(font->fonts[0].length > 0)
    {
        printf("%s [System]", font->fonts[0].chars);
        if(font->fonts[1].length > 0)
            fputs(", ", stdout);
    }

    if(font->fonts[1].length > 0)
        printf("%s [User]", font->fonts[1].chars);
}

#elif defined(_WIN32)

static void printFont(const FFFontResult* font)
{
    if(font->fonts[0].length > 0)
    {
        printf("%s [Desktop]", font->fonts[0].chars);
    }
}

#else

static void printFont(const FFFontResult* font)
{
    FF_UNUSED(font);
}

#endif

void ffPrintFont(FFinstance* instance)
{
    assert(FF_DETECT_FONT_NUM_FONTS == FF_FONT_NUM_FORMAT_ARGS);

    const FFDisplayServerResult* wmde = ffConnectDisplayServer(instance);

    if(ffStrbufIgnCaseCompS(&wmde->wmProtocolName, "TTY") == 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "Font isn't supported in TTY");
        return;
    }

    const FFFontResult* font = ffDetectFont(instance);

    if(font->error.length > 0)
    {
        ffPrintError(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, "%s", font->error.chars);
        return;
    }

    if(instance->config.font.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font.key);
        printFont(font);
        putchar('\n');
    }
    else
    {
        ffPrintFormat(instance, FF_FONT_MODULE_NAME, 0, &instance->config.font, FF_FONT_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[0]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[1]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[2]},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->fonts[3]}
        });
    }
}
