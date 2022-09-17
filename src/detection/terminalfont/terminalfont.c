#include "fastfetch.h"
#include "common/properties.h"
#include "common/processing.h"
#include "common/font.h"
#include "terminalfont.h"

static const char* detectAlacritty(FFinstance* instance, FFfont* font)
{
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

    ffFontInitValues(font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);

    return NULL;
}

static const char* detectTTY(FFinstance* instance, FFfont* font)
{
    FF_UNUSED(instance);

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

    ffFontInitCopy(font, fontName.chars);
    ffStrbufDestroy(&fontName);

    return NULL;
}

const char* ffDetectTerminalFontCommon(FFinstance* instance, FFfont* font)
{
    const FFTerminalShellResult* shellInfo = ffDetectTerminalShell(instance);

    if(shellInfo->terminalProcessName.length == 0)
        return "Terminal font needs successfull terminal detection";

    const char* error = ffDetectTerminalFontPlatform(instance, shellInfo, font);
    if (error == NULL || *error != '\0')
        return error;

    if(ffStrbufStartsWithIgnCaseS(&shellInfo->terminalExe, "/dev/tty"))
        return detectTTY(instance, font);

    if(ffStrbufIgnCaseCompS(&shellInfo->terminalProcessName, "alacritty") == 0)
        return detectAlacritty(instance, font);

    return "Unknown terminal";
}
