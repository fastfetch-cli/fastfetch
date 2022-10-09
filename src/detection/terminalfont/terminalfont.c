#include "terminalfont.h"
#include "common/properties.h"
#include "common/processing.h"
#include "detection/internal.h"
#include "detection/terminalshell/terminalshell.h"

static void detectAlacritty(const FFinstance* instance, FFTerminalFontResult* terminalFont) {
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    FFstrbuf fontSize;
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

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

static void detectTTY(FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);

    ffParsePropFile(FASTFETCH_TARGET_DIR_ETC"/vconsole.conf", "Font =", &fontName);

    if(fontName.length == 0)
    {
        ffStrbufAppendS(&fontName, "VGA default kernel font ");
        ffProcessAppendStdOut(&fontName, (char* const[]){
            "showconsolefont",
            "--info",
            NULL
        });

        ffStrbufTrimRight(&fontName, ' ');
    }

    if(fontName.length > 0)
        ffFontInitCopy(&terminalFont->font, fontName.chars);
    else
        ffStrbufAppendS(&terminalFont->error, "Couldn't find Font in "FASTFETCH_TARGET_DIR_ETC"/vconsole.conf");

    ffStrbufDestroy(&fontName);
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont);

const FFTerminalFontResult* ffDetectTerminalFont(const FFinstance* instance)
{
    FF_DETECTION_INTERNAL_GUARD(FFTerminalFontResult,
        ffStrbufInitA(&result.error, 0);

        const FFTerminalShellResult* terminalShell = ffDetectTerminalShell(instance);

        if(terminalShell->terminalProcessName.length == 0)
            ffStrbufAppendS(&result.error, "Terminal font needs successfull terminal detection");
        else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "alacritty") == 0)
            detectAlacritty(instance, &result);
        else if(ffStrbufStartsWithIgnCaseS(&terminalShell->terminalExe, "/dev/tty"))
            detectTTY(&result);
        else
            ffDetectTerminalFontPlatform(instance, terminalShell, &result);

        if(result.error.length == 0 && result.font.pretty.length == 0)
            ffStrbufAppendF(&result.error, "Unknown terminal: %s", terminalShell->terminalProcessName.chars);
    );
}
