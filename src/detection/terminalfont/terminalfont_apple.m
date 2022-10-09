#include "terminalfont.h"
#include "common/font.h"
#include "detection/terminalshell/terminalshell.h"
#include "util/apple/osascript.h"

#include <stdlib.h>
#include <string.h>
#import <Foundation/Foundation.h>

static void detectIterm2(const FFinstance* instance, FFTerminalFontResult* terminalFont)
{
    const char* profile = getenv("ITERM_PROFILE");
    if (profile == NULL)
    {
        ffStrbufAppendS(&terminalFont->error, "environment variable ITERM_PROFILE not set");
        return;
    }

    NSError* error;
    NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Preferences/com.googlecode.iterm2.plist", instance->state.passwd->pw_dir];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
    {
        ffStrbufAppendS(&terminalFont->error, [error localizedDescription].UTF8String);
        return;
    }

    for(NSDictionary* bookmark in [dict valueForKey:@"New Bookmarks"])
    {
        if(![[bookmark valueForKey:@"Name"] isEqualToString:@(profile)])
            continue;

        NSString* normalFont = [bookmark valueForKey:@"Normal Font"];
        if(!normalFont)
        {
            ffStrbufAppendF(&terminalFont->error, "`Normal Font` key in profile `%s` doesn't exist", profile);
            return;
        }
        ffFontInitWithSpace(&terminalFont->font, normalFont.UTF8String);
        return;
    }

    ffStrbufAppendF(&terminalFont->error, "find profile `%s` bookmark failed", profile);
}

static void detectAppleTerminal(FFTerminalFontResult* terminalFont)
{
    FFstrbuf fontName;
    ffStrbufInit(&fontName);
    ffOsascript("tell application \"Terminal\" to font name of window frontmost", &fontName);

    if(fontName.length == 0)
    {
        ffStrbufAppendS(&terminalFont->error, "executing osascript failed");
        ffStrbufDestroy(&fontName);
        return;
    }

    FFstrbuf fontSize;
    ffStrbufInit(&fontSize);
    ffOsascript("tell application \"Terminal\" to font size of window frontmost", &fontSize);

    ffFontInitValues(&terminalFont->font, fontName.chars, fontSize.chars);

    ffStrbufDestroy(&fontName);
    ffStrbufDestroy(&fontSize);
}

void ffDetectTerminalFontPlatform(const FFinstance* instance, const FFTerminalShellResult* terminalShell, FFTerminalFontResult* terminalFont)
{
    if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "iterm.app") == 0)
        detectIterm2(instance, terminalFont);
    else if(ffStrbufIgnCaseCompS(&terminalShell->terminalProcessName, "Apple_Terminal") == 0)
        detectAppleTerminal(terminalFont);
}
