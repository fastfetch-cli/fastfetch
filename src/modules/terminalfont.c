#include "fastfetch.h"
#include "common/properties.h"
#include "common/printing.h"
#include "common/font.h"
#include "common/processing.h"
#include "terminalfont.h"

void ffPrintTerminalFontResult(FFinstance* instance, const char* raw, FFfont* font)
{
    if(font->pretty.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Terminal font is an empty value");
        return;
    }

    if(instance->config.terminalFont.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont.key);
        ffStrbufPutTo(&font->pretty, stdout);
    }
    else
    {
        ffPrintFormat(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, FF_TERMFONT_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, raw},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->name},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->size},
            {FF_FORMAT_ARG_TYPE_LIST,   &font->styles},
            {FF_FORMAT_ARG_TYPE_STRBUF, &font->pretty}
        });
    }
}

static void printAlacritty(FFinstance* instance) {
    FFstrbuf fontName;
    FFstrbuf fontSize;
    ffStrbufInit(&fontName);
    ffStrbufInit(&fontSize);

    FFpropquery fontQuery[] = {
        {"family:", &fontName},
        {"size:", &fontSize},
    };

    // alacritty parses config files in this order
    ffParsePropFileConfigValues(instance, "alacritty/alacritty.yml", 2, fontQuery);
    if(fontName.length == 0 || fontSize.length == 0)
        ffParsePropFileConfigValues(instance, "alacritty.yml", 2, fontQuery);
    if(fontName.length == 0 || fontSize.length == 0)
        ffParsePropFileConfigValues(instance, ".alacritty.yml", 2, fontQuery);

    //by default alacritty uses its own font called alacritty
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "alacritty");

    // the default font size is 11
    if(fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "11");

    FFfont font;
    ffFontInitValues(&font, fontName.chars, fontSize.chars);
    ffPrintTerminalFontResult(instance, fontName.chars, &font);
    ffFontDestroy(&font);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

static void printTTY(FFinstance* instance)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    ffParsePropFile(FASTFETCH_TARGET_DIR_ROOT"/etc/vconsole.conf", "Font =", &fontName);

    if(fontName.length == 0)
    {
        ffStrbufAppendS(&fontName, "VGA default kernel font ");
        ffProcessAppendStdOut(&fontName, (char* const[]){
            "showconsolefont",
            "--info",
            NULL
        });
    }

    ffStrbufTrimRight(&fontName, ' ');

    FFfont font;
    ffFontInitCopy(&font, fontName.chars);
    ffPrintTerminalFontResult(instance, fontName.chars, &font);
    ffFontDestroy(&font);
    ffStrbufDestroy(&fontName);
}

void ffPrintTerminalFont(FFinstance* instance)
{
    const FFTerminalShellResult* result = ffDetectTerminalShell(instance);

    if(result->terminalProcessName.length == 0)
    {
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Terminal font needs successfull terminal detection");
        return;
    }

    if(ffPrintTerminalFontPlatform(instance, result))
        return;
    if(ffStrbufStartsWithIgnCaseS(&result->terminalExe, "/dev/tty"))
        printTTY(instance);
    else if(ffStrbufIgnCaseCompS(&result->terminalProcessName, "alacritty") == 0)
        printAlacritty(instance);
    else
        ffPrintError(instance, FF_TERMFONT_MODULE_NAME, 0, &instance->config.terminalFont, "Unknown terminal: %s", result->terminalProcessName.chars);
}
