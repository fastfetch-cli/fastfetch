#include "terminalfont.h"
#include "common/io/io.h"
#include "common/properties.h"
#include "common/processing.h"
#include "detection/terminalshell/terminalshell.h"

static void detectAlacritty(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    do {
        // Latest alacritty uses toml instead of yaml
        FFpropquery fontQueryToml[] = {
            {"family =", &fontName},
            {"size =", &fontSize},
        };

        // alacritty parses config files in this order
        if(ffParsePropFileConfigValues("alacritty/alacritty.toml", 2, fontQueryToml))
            break;
        if(ffParsePropFileConfigValues("alacritty.toml", 2, fontQueryToml))
            break;
        if(ffParsePropFileConfigValues(".alacritty.toml", 2, fontQueryToml))
            break;

        FFpropquery fontQueryYaml[] = {
            {"family:", &fontName},
            {"size:", &fontSize},
        };

        if(ffParsePropFileConfigValues("alacritty/alacritty.yml", 2, fontQueryYaml))
            break;
        if(ffParsePropFileConfigValues("alacritty.yml", 2, fontQueryYaml))
            break;
        if(ffParsePropFileConfigValues(".alacritty.yml", 2, fontQueryYaml))
            break;
    } while (false);

    //by default alacritty uses its own font called alacritty
    if(fontName.length == 0)
        ffStrbufAppendS(&fontName, "alacritty");

    // the default font size is 11
    if(fontSize.length == 0)
        ffStrbufAppendS(&fontSize, "11");

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
}

FF_MAYBE_UNUSED static void detectTTY(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

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
}

static bool queryKittyTerm(const char* query, FFstrbuf* res)
{
    // https://github.com/fastfetch-cli/fastfetch/discussions/1030#discussioncomment-9845233
    char buffer[256] = "";
    if (ffGetTerminalResponse(
        query, // kitty-query-font_family;kitty-query-font_size
        "\eP1+r%*[^=]=%255[^\e]\e\\", buffer) == NULL)
    {
        // decode hex string
        for (const char* p = buffer; p[0] && p[1]; p += 2)
        {
            unsigned value;
            if (sscanf(p, "%2x", &value) == 1)
                ffStrbufAppendC(res, (char) value);
        }
        return true;
    }
    return false;
}

FF_MAYBE_UNUSED static bool detectKitty(const FFstrbuf* exe, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    // Kitty generates response independently even if we query font family and size in one query
    // which may result in short read in `ffGetTerminalResponse`
    if (queryKittyTerm("\eP+q6b697474792d71756572792d666f6e745f66616d696c79\e\\", &fontName)) // kitty-query-font_family
        queryKittyTerm("\eP+q6b697474792d71756572792d666f6e745f73697a65\e\\", &fontSize); // kitty-query-font_size
    else
    {
        FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
        if(!ffProcessAppendStdOut(&buf, (char* const[]){
            exe->chars,
            "+kitten",
            "query-terminal",
            NULL,
        }))
        {
            ffParsePropLines(buf.chars, "font_family: ", &fontName);
            ffParsePropLines(buf.chars, "font_size: ", &fontSize);
        }
        else
        {
            FFpropquery fontQuery[] = {
                {"font_family ", &fontName},
                {"font_size ", &fontSize},
            };

            ffParsePropFileConfigValues("kitty/kitty.conf", 2, fontQuery);

            if(fontName.length == 0)
                ffStrbufSetS(&fontName, "monospace");
            if(fontSize.length == 0)
                ffStrbufSetS(&fontSize, "11.0");
        }
    }

    ffFontInitValues(&result->font, fontName.chars, fontSize.chars);

    return true;
}

static bool detectWezterm(const FFstrbuf* exe, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY cli = ffStrbufCreateCopy(exe);
    ffStrbufSubstrBeforeLastC(&cli, '-');

    #ifdef _WIN32
    ffStrbufAppendS(&cli, ".exe");
    #endif

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

    ffStrbufSetS(&result->error, ffProcessAppendStdOut(&fontName, (char* const[]){
        cli.chars,
        "ls-fonts",
        "--text",
        "a",
        NULL
    }));
    if(result->error.length)
        return false;

    //LeftToRight
    // 0 a    \u{61}       x_adv=7  cells=1  glyph=a,180  wezterm.font("JetBrains Mono", {weight="Regular", stretch="Normal", style="Normal"})
    //                                      <built-in>, BuiltIn
    ffStrbufSubstrAfterFirstC(&fontName, '"');
    ffStrbufSubstrBeforeFirstC(&fontName, '"');

    if(!fontName.length)
        return false;

    ffFontInitCopy(&result->font, fontName.chars);
    return true;
}

static bool detectTabby(FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    FFpropquery fontQuery[] = {
        {"font: ", &fontName},
        {"fontSize: ", &fontSize},
    };

    if(!ffParsePropFileConfigValues("tabby/config.yaml", 2, fontQuery))
        return false;

    if(fontName.length == 0)
        ffStrbufSetS(&fontName, "monospace");
    if(fontSize.length == 0)
        ffStrbufSetS(&fontSize, "14");

    ffFontInitValues(&result->font, fontName.chars, fontSize.chars);

    return true;
}

static bool detectContour(const FFstrbuf* exe, FFTerminalFontResult* result)
{
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if(ffProcessAppendStdOut(&buf, (char* const[]){
        exe->chars,
        "font-locator",
        NULL
    }))
    {
        ffStrbufAppendS(&result->error, "`contour font-locator` failed");
        return false;
    }

    //[error] Missing key .logging.enabled. Using default: false.
    //[error] ...
    //Matching fonts using  : Fontconfig
    //Font description      : (family=Sarasa Term SC Nerd weight=Regular slant=Roman spacing=Monospace, strict_spacing=yes)
    //Number of fonts found : 49
    //  path /usr/share/fonts/google-noto/NotoSansMono-Regular.ttf Regular Roman
    //  path ...

    uint32_t index = ffStrbufFirstIndexS(&buf, "Font description      : (family=");
    if(index >= buf.length) return false;
    index += (uint32_t) strlen("Font description      : (family=");
    ffStrbufSubstrBefore(&buf, ffStrbufNextIndexS(&buf, index, " weight="));
    ffFontInitCopy(&result->font, buf.chars + index);
    return true;
}

void ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont);

static bool detectTerminalFontCommon(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "alacritty"))
        detectAlacritty(terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "wezterm-gui"))
        detectWezterm(&terminal->exe, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "tabby"))
        detectTabby(terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->processName, "contour"))
        detectContour(&terminal->exe, terminalFont);

    #ifndef _WIN32
    else if(ffStrbufStartsWithIgnCaseS(&terminal->exe, "/dev/pts/"))
        ffStrbufAppendS(&terminalFont->error, "Terminal font detection is not supported on PTS");
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "kitty"))
        detectKitty(&terminal->exe, terminalFont);
    else if(ffStrbufStartsWithIgnCaseS(&terminal->exe, "/dev/tty"))
        detectTTY(terminalFont);
    #endif

    else
        return false;

    return true;
}

bool ffDetectTerminalFont(FFTerminalFontResult* result)
{
    const FFTerminalResult* terminal = ffDetectTerminal();

    if(terminal->processName.length == 0)
        ffStrbufAppendS(&result->error, "Terminal font needs successful terminal detection");

    else if(!detectTerminalFontCommon(terminal, result))
        ffDetectTerminalFontPlatform(terminal, result);

    if(result->error.length == 0 && result->font.pretty.length == 0)
        ffStrbufAppendF(&result->error, "Unknown terminal: %s", terminal->processName.chars);

    return result->error.length == 0;
}
