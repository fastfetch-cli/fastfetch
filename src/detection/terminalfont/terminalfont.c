#include "terminalfont.h"
#include "common/io.h"
#include "common/properties.h"
#include "common/processing.h"
#include "common/debug.h"
#include "detection/terminalshell/terminalshell.h"

static void detectAlacritty(FFTerminalFontResult* terminalFont) {
    // Maybe using a toml parser to read the config file is better?
    // https://github.com/cktan/tomlc17

    // Doc: https://alacritty.org/config-alacritty.html#s26
    FF_STRBUF_AUTO_DESTROY fontNormal = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontFamily = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontStyle = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    do {
        FFpropquery fontQueryToml[] = {
            { "normal =", &fontNormal },
            { "size =", &fontSize },
        };

        // alacritty parses config files in this order
        if (ffParsePropFileConfigValues("alacritty/alacritty.toml", 2, fontQueryToml)) {
            break;
        }
        if (ffParsePropFileConfigValues("alacritty.toml", 2, fontQueryToml)) {
            break;
        }
        if (ffParsePropFileConfigValues(".alacritty.toml", 2, fontQueryToml)) {
            break;
        }
    } while (false);

    if (fontNormal.length > 0) {
        // { family = "Fira Code", style = "Medium" }
        ffStrbufTrimSpace(&fontNormal);
        ffStrbufTrimRight(&fontNormal, '}');
        ffStrbufTrimLeft(&fontNormal, '{');
        ffStrbufTrimSpace(&fontNormal);

        // family = "Fira Code", style = "Medium"
        ffStrbufReplaceAllC(&fontNormal, ',', '\n'); // Assume no commas in font names
        ffParsePropLines(fontNormal.chars, "family =", &fontFamily);
        ffParsePropLines(fontNormal.chars, "style =", &fontStyle);
    }

    if (fontFamily.length == 0) {
#if __APPLE__
        ffStrbufSetStatic(&fontFamily, "Menlo");
#elif _WIN32
        ffStrbufSetStatic(&fontFamily, "Consolas");
#else
        ffStrbufSetStatic(&fontFamily, "monospace");
#endif
    }
    if (fontStyle.length == 0) {
        ffStrbufSetStatic(&fontStyle, "Regular");
    }

    if (fontSize.length == 0) {
        ffStrbufSetStatic(&fontSize, "11.25");
    }

    ffFontInitMoveValues(&terminalFont->font, &fontFamily, &fontSize, &fontStyle);
}

// Maximum number of `config-file` directives to follow, guarding against runaway includes
#define FF_GHOSTTY_MAX_CONFIG_FILES 16

static void parseGhosttyConfig(const FFstrbuf* path, FFstrbuf* fontName, FFstrbuf* fontNameFallback, FFstrbuf* fontSize, FFlist* configFiles /* list of FFstrbuf */) {
    FF_DEBUG("parsing config: %s", path->chars);

    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY temp = ffStrbufCreate();
    if (!ffAppendFileBuffer(path->chars, &buffer)) {
        FF_DEBUG("cannot read config: %s", path->chars);
        return;
    }

    char* line = nullptr;
    size_t len = 0;
    while (ffStrbufGetline(&line, &len, &buffer)) {
        if (ffParsePropLine(line, "font-family =", &temp)) {
            FF_DEBUG("found font-family='%s' in %s", temp.chars, path->chars);
            // Latter overrides former; former becomes the fallback font
            if (fontName->length > 0) {
                ffStrbufDestroy(fontNameFallback);
                ffStrbufInitMove(fontNameFallback, fontName);
            }
            ffStrbufDestroy(fontName);
            ffStrbufInitMove(fontName, &temp);
        } else if (ffParsePropLine(line, "font-size =", &temp)) {
            FF_DEBUG("found font-size='%s' in %s", temp.chars, path->chars);
            // Latter overrides former
            ffStrbufDestroy(fontSize);
            ffStrbufInitMove(fontSize, &temp);
        } else if (ffParsePropLine(line, "config-file =", &temp)) {
            // Doc: https://ghostty.org/docs/config/reference#config-file
            // A leading `?` suppresses errors if the file doesn't exist; missing files are skipped here either way
            ffStrbufTrimLeft(&temp, '?');
            ffStrbufTrimLeft(&temp, '"');
            ffStrbufTrimRight(&temp, '"');
            if (temp.length == 0) {
                continue;
            }

            if (!ffStrbufStartsWithC(&temp, '/')) {
                // Relative paths are relative to the file containing the `config-file` directive
                FF_STRBUF_AUTO_DESTROY absolutePath = ffStrbufCreateCopy(path);
                ffStrbufSubstrBeforeLastC(&absolutePath, '/');
                ffStrbufAppendC(&absolutePath, '/');
                ffStrbufAppend(&absolutePath, &temp);
                ffStrbufDestroy(&temp);
                ffStrbufInitMove(&temp, &absolutePath);
            }

            // Each unique file is only loaded once, which also prevents include cycles
            bool loaded = ffStrbufEqual(&temp, path);
            if (!loaded) {
                FF_LIST_FOR_EACH(FFstrbuf, it, *configFiles) {
                    if (ffStrbufEqual(it, &temp)) {
                        loaded = true;
                        break;
                    }
                }
            }

            if (loaded) {
                FF_DEBUG("config-file '%s' was already loaded, skipping to avoid cycles", temp.chars);
            } else if (configFiles->length >= FF_GHOSTTY_MAX_CONFIG_FILES) {
                FF_DEBUG("too many config-file directives, ignoring '%s'", temp.chars);
            } else {
                FF_DEBUG("found config-file='%s' in %s", temp.chars, path->chars);
                ffStrbufInitMove(FF_LIST_ADD(FFstrbuf, *configFiles), &temp);
            }
            ffStrbufClear(&temp);
        }
    }
}

static void detectGhostty(FFTerminalFontResult* terminalFont, [[maybe_unused]] const char* configPathMac, const char* configPathUnix) {
    FF_DEBUG("detectGhostty: start");
    FF_STRBUF_AUTO_DESTROY configPath = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontNameFallback = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();
    FF_LIST_AUTO_DESTROY configFiles = ffListCreate(); // list of FFstrbuf

    // Ghostty loads the XDG config first, then (on macOS) the Application Support config,
    // so values in the latter override the former
    if (instance.state.platform.configDirs.length > 0) {
        ffStrbufSet(&configPath, FF_LIST_FIRST(FFstrbuf, instance.state.platform.configDirs));
        ffStrbufAppendS(&configPath, configPathUnix); // ghostty/config
        parseGhosttyConfig(&configPath, &fontName, &fontNameFallback, &fontSize, &configFiles);
    }

#if __APPLE__
    ffStrbufSet(&configPath, &instance.state.platform.homeDir);
    ffStrbufAppendS(&configPath, "Library/Application Support/");
    ffStrbufAppendS(&configPath, configPathMac); // com.mitchellh.ghostty/config
    parseGhosttyConfig(&configPath, &fontName, &fontNameFallback, &fontSize, &configFiles);
#endif

    // Files referenced by `config-file` don't take effect until the whole configuration is loaded,
    // so they are parsed after all root config files, in the order they were found
    for (uint32_t i = 0; i < configFiles.length; ++i) {
        // Copy the path as parseGhosttyConfig may grow the list, invalidating pointers into it
        ffStrbufSet(&configPath, FF_LIST_GET(FFstrbuf, configFiles, i));
        parseGhosttyConfig(&configPath, &fontName, &fontNameFallback, &fontSize, &configFiles);
    }

    FF_LIST_FOR_EACH(FFstrbuf, it, configFiles) {
        ffStrbufDestroy(it);
    }

    if (fontName.length == 0) {
        ffStrbufAppendS(&fontName, "JetBrainsMono Nerd Font");
        FF_DEBUG("using default family='%s'", fontName.chars);
    }

    if (fontSize.length == 0) {
        ffStrbufAppendS(&fontSize,
#if __APPLE__
            "13"
#else
            "12"
#endif
        );
        FF_DEBUG("using default size='%s'", fontSize.chars);
    }

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);
    if (fontNameFallback.length > 0) {
        FF_DEBUG("applying fallback family='%s'", fontNameFallback.chars);
        ffFontInitValues(&terminalFont->fallback, fontNameFallback.chars, nullptr);
    }
    FF_DEBUG("result family='%s' size='%s'%s", fontName.chars, fontSize.chars, fontNameFallback.length ? " (with fallback)" : "");
    FF_DEBUG("detectGhostty: end");
}

[[maybe_unused]] static void detectTTY(FFTerminalFontResult* terminalFont) {
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

    ffParsePropFile(FASTFETCH_TARGET_DIR_ETC "/vconsole.conf", "Font =", &fontName);

    if (fontName.length == 0) {
        ffStrbufAppendS(&fontName, "VGA default kernel font ");
        ffProcessAppendStdOut(&fontName, (char* const[]){ "showconsolefont", "--info", nullptr });

        ffStrbufTrimRight(&fontName, ' ');
    }

    if (fontName.length > 0) {
        ffFontInitCopy(&terminalFont->font, fontName.chars);
    } else {
        ffStrbufAppendS(&terminalFont->error, "Couldn't find Font in " FASTFETCH_TARGET_DIR_ETC "/vconsole.conf");
    }
}

[[maybe_unused]] static bool detectKitty(const FFstrbuf* exe, FFTerminalFontResult* result) {
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    char fontHex[512] = "", sizeHex[512] = "";
    // https://github.com/fastfetch-cli/fastfetch/discussions/1030#discussioncomment-9845233
    if (ffGetTerminalResponse(
            "\eP+q6b697474792d71756572792d666f6e745f66616d696c79;6b697474792d71756572792d666f6e745f73697a65\e\\", // kitty-query-font_family;kitty-query-font_size
            2,
            "\eP1+r%*[^=]=%511[^\e]\e\\\eP1+r%*[^=]=%511[^\e]\e\\",
            fontHex,
            sizeHex) == nullptr &&
        *fontHex && *sizeHex) {
        // decode hex string
        for (const char* p = fontHex; p[0] && p[1]; p += 2) {
            unsigned value;
            if (sscanf(p, "%2x", &value) == 1) {
                ffStrbufAppendC(&fontName, (char) value);
            }
        }
        for (const char* p = sizeHex; p[0] && p[1]; p += 2) {
            unsigned value;
            if (sscanf(p, "%2x", &value) == 1) {
                ffStrbufAppendC(&fontSize, (char) value);
            }
        }
    } else {
        FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
        if (!ffProcessAppendStdOut(&buf, (char* const[]){
                                             exe->chars,
                                             "+kitten",
                                             "query-terminal",
                                             nullptr,
                                         })) {
            ffParsePropLines(buf.chars, "font_family: ", &fontName);
            ffParsePropLines(buf.chars, "font_size: ", &fontSize);
        } else {
            FFpropquery fontQuery[] = {
                { "font_family ", &fontName },
                { "font_size ", &fontSize },
            };

            ffParsePropFileConfigValues("kitty/kitty.conf", 2, fontQuery);

            if (fontName.length == 0) {
                ffStrbufSetS(&fontName, "monospace");
            }
            if (fontSize.length == 0) {
                ffStrbufSetS(&fontSize, "11.0");
            }
        }
    }

    ffFontInitValues(&result->font, fontName.chars, fontSize.chars);

    return true;
}

static bool detectWezterm(const FFstrbuf* exe, FFTerminalFontResult* result) {
    FF_STRBUF_AUTO_DESTROY cli = ffStrbufCreateCopy(exe);
    ffStrbufSubstrBeforeLastC(&cli, '-');

#ifdef _WIN32
    ffStrbufAppendS(&cli, ".exe");
#endif

    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();

    ffStrbufSetS(&result->error, ffProcessAppendStdOut(&fontName, (char* const[]){ cli.chars, "ls-fonts", "--text", "a", nullptr }));
    if (result->error.length) {
        return false;
    }

    // LeftToRight
    //  0 a    \u{61}       x_adv=7  cells=1  glyph=a,180  wezterm.font("JetBrains Mono", {weight="Regular", stretch="Normal", style="Normal"})
    //                                       <built-in>, BuiltIn
    ffStrbufSubstrAfterFirstC(&fontName, '"');
    ffStrbufSubstrBeforeFirstC(&fontName, '"');

    if (!fontName.length) {
        return false;
    }

    ffFontInitCopy(&result->font, fontName.chars);
    return true;
}

static bool detectTabby(FFTerminalFontResult* result) {
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    FFpropquery fontQuery[] = {
        { "font: ", &fontName },
        { "fontSize: ", &fontSize },
    };

    if (!ffParsePropFileConfigValues("tabby/config.yaml", 2, fontQuery)) {
        return false;
    }

    if (fontName.length == 0) {
        ffStrbufSetS(&fontName, "monospace");
    }
    if (fontSize.length == 0) {
        ffStrbufSetS(&fontSize, "14");
    }

    ffFontInitValues(&result->font, fontName.chars, fontSize.chars);

    return true;
}

static bool detectContour(const FFstrbuf* exe, FFTerminalFontResult* result) {
    FF_STRBUF_AUTO_DESTROY buf = ffStrbufCreate();
    if (ffProcessAppendStdOut(&buf, (char* const[]){ exe->chars, "font-locator", nullptr })) {
        ffStrbufAppendS(&result->error, "`contour font-locator` failed");
        return false;
    }

    //[error] Missing key .logging.enabled. Using default: false.
    //[error] ...
    // Matching fonts using  : Fontconfig
    // Font description      : (family=Sarasa Term SC Nerd weight=Regular slant=Roman spacing=Monospace, strict_spacing=yes)
    // Number of fonts found : 49
    //  path /usr/share/fonts/google-noto/NotoSansMono-Regular.ttf Regular Roman
    //  path ...

    uint32_t index = ffStrbufFirstIndexS(&buf, "Font description      : (family=");
    if (index >= buf.length) {
        return false;
    }
    index += (uint32_t) strlen("Font description      : (family=");
    ffStrbufSubstrBefore(&buf, ffStrbufNextIndexS(&buf, index, " weight="));
    ffFontInitCopy(&result->font, buf.chars + index);
    return true;
}

static bool detectRio(FFTerminalFontResult* terminalFont) {
    FF_STRBUF_AUTO_DESTROY fontName = ffStrbufCreate();
    FF_STRBUF_AUTO_DESTROY fontSize = ffStrbufCreate();

    FFpropquery fontQueryToml[] = {
        { "family =", &fontName },
        { "size =", &fontSize },
    };

    ffParsePropFileConfigValues("rio/config.toml", 2, fontQueryToml);

    if (fontName.length == 0) {
        ffStrbufAppendS(&fontName, "Cascadia Code");
    }

    if (fontSize.length == 0) {
        ffStrbufAppendS(&fontSize, "18");
    }

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    return true;
}

bool ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont);

static bool detectTerminalFontCommon(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont) {
    if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "alacritty")) {
        detectAlacritty(terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "wezterm-gui")) {
        detectWezterm(&terminal->exe, terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "tabby")) {
        detectTabby(terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "contour")) {
        detectContour(&terminal->exe, terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "ghostty")) {
        detectGhostty(terminalFont, "com.mitchellh.ghostty/config", "ghostty/config");
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "Muxy")) {
        detectGhostty(terminalFont, "Muxy/ghostty.conf", "muxy/ghostty.conf");
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->processName, "rio")) {
        detectRio(terminalFont);
    }
#ifndef _WIN32
    else if (ffStrbufStartsWithIgnCaseS(&terminal->exe, "/dev/pts/")) {
        ffStrbufAppendS(&terminalFont->error, "Terminal font detection is not supported on PTS");
    } else if (ffStrbufIgnCaseEqualS(&terminal->processName, "kitty")) {
        detectKitty(&terminal->exe, terminalFont);
    } else if (ffStrbufStartsWithIgnCaseS(&terminal->exe, "/dev/tty")) {
        detectTTY(terminalFont);
    }
#endif

    else {
        return false;
    }

    return true;
}

bool ffDetectTerminalFont(FFTerminalFontResult* result) {
    const FFTerminalResult* terminal = ffDetectTerminal();

    if (terminal->processName.length == 0) {
        ffStrbufAppendS(&result->error, "Terminal font needs successful terminal detection");
    }

    else if (!detectTerminalFontCommon(terminal, result)) {
        ffDetectTerminalFontPlatform(terminal, result);
    }

    if (result->error.length == 0 && result->font.pretty.length == 0) {
        ffStrbufAppendF(&result->error, "Unknown terminal: %s", terminal->processName.chars);
    }

    return result->error.length == 0;
}
