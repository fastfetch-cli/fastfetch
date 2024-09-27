#include "terminalfont.h"
#include "common/font.h"
#include "detection/terminalshell/terminalshell.h"
#include "util/apple/osascript.h"

#include <stdlib.h>
#include <string.h>
#import <Foundation/Foundation.h>

static void detectIterm2(FFTerminalFontResult* terminalFont)
{
    const char* profile = getenv("ITERM_PROFILE");
    if (profile == NULL)
    {
        ffStrbufAppendS(&terminalFont->error, "environment variable ITERM_PROFILE not set");
        return;
    }

    NSError* error;
    NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Preferences/com.googlecode.iterm2.plist", instance.state.platform.homeDir.chars];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
    {
        ffStrbufAppendS(&terminalFont->error, error.localizedDescription.UTF8String);
        return;
    }

    for(NSDictionary* bookmark in dict[@"New Bookmarks"])
    {
        if(![bookmark[@"Name"] isEqualToString:@(profile)])
            continue;

        NSString* normalFont = bookmark[@"Normal Font"];
        if(!normalFont)
        {
            ffStrbufAppendF(&terminalFont->error, "`Normal Font` key in profile `%s` doesn't exist", profile);
            return;
        }
        ffFontInitWithSpace(&terminalFont->font, normalFont.UTF8String);

        NSNumber* useNonAsciiFont = bookmark[@"Use Non-ASCII Font"];
        if(useNonAsciiFont.boolValue)
        {
            NSString* nonAsciiFont = bookmark[@"Non Ascii Font"];
            if (nonAsciiFont)
                ffFontInitWithSpace(&terminalFont->fallback, nonAsciiFont.UTF8String);
        }
        return;
    }

    ffStrbufAppendF(&terminalFont->error, "find profile `%s` bookmark failed", profile);
}

static void detectAppleTerminal(FFTerminalFontResult* terminalFont)
{
    FF_STRBUF_AUTO_DESTROY font = ffStrbufCreate();
    ffOsascript("tell application \"Terminal\" to font name of window frontmost & \" \" & font size of window frontmost", &font);

    if(font.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "executing osascript failed");
        return;
    }

    ffFontInitWithSpace(&terminalFont->font, font.chars);
}

static void detectWarpTerminal(FFTerminalFontResult* terminalFont)
{
    NSError* error;
    NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Preferences/dev.warp.Warp-Stable.plist", instance.state.platform.homeDir.chars];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
    {
        ffStrbufAppendS(&terminalFont->error, error.localizedDescription.UTF8String);
        return;
    }

    NSString* fontName = dict[@"FontName"];
    if(!fontName)
        fontName = @"Hack";
    else
        fontName = [fontName stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"\""]];

    NSString* fontSize = dict[@"FontSize"];
    if(!fontSize)
        fontSize = @"13";

    ffFontInitValues(&terminalFont->font, fontName.UTF8String, fontSize.UTF8String);
}

void ffDetectTerminalFontPlatform(const FFTerminalResult* terminal, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseEqualS(&terminal->processName, "iterm.app") ||
        ffStrbufStartsWithIgnCaseS(&terminal->processName, "iTermServer-"))
        detectIterm2(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "Apple_Terminal"))
        detectAppleTerminal(terminalFont);
    else if(ffStrbufIgnCaseEqualS(&terminal->processName, "WarpTerminal"))
        detectWarpTerminal(terminalFont);
}
