#include "fastfetch.h"
#include "detection/displayserver/displayserver.h"
#include "wmtheme.h"

#import <Foundation/Foundation.h>

static bool detectQuartzCompositor(FFinstance* instance, FFstrbuf* themeOrError)
{
    FF_UNUSED(instance);

    NSError* error;
    NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Preferences/.GlobalPreferences.plist", instance->state.platform.homeDir.chars];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
    {
        ffStrbufAppendS(themeOrError, error.localizedDescription.UTF8String);
        return false;
    }

    NSNumber* wmThemeColor = [dict valueForKey:@"AppleAccentColor"];
    if(!wmThemeColor)
        ffStrbufAppendS(themeOrError, "Multicolor");
    else
    {
        switch(wmThemeColor.intValue)
        {
            case -1: ffStrbufAppendS(themeOrError, "Graphite"); break;
            case 0: ffStrbufAppendS(themeOrError, "Red"); break;
            case 1: ffStrbufAppendS(themeOrError, "Orange"); break;
            case 2: ffStrbufAppendS(themeOrError, "Yellow"); break;
            case 3: ffStrbufAppendS(themeOrError, "Green"); break;
            case 4: ffStrbufAppendS(themeOrError, "Blue"); break;
            case 5: ffStrbufAppendS(themeOrError, "Purple"); break;
            case 6: ffStrbufAppendS(themeOrError, "Pink"); break;
            default: ffStrbufAppendS(themeOrError, "Unknown"); break;
        }
    }

    NSString* wmTheme = [dict valueForKey:@"AppleInterfaceStyle"];
    ffStrbufAppendF(themeOrError, " (%s)", wmTheme ? wmTheme.UTF8String : "Light");
    return true;
}

bool ffDetectWmTheme(FFinstance* instance, FFstrbuf* themeOrError)
{
    const FFDisplayServerResult* wm = ffConnectDisplayServer(instance);

    if(wm->wmPrettyName.length == 0)
    {
        ffStrbufAppendS(themeOrError, "WM Theme needs sucessfull WM detection");
        return false;
    }

    if(ffStrbufIgnCaseCompS(&wm->wmPrettyName, "Quartz Compositor") == 0)
        return detectQuartzCompositor(instance, themeOrError);

    ffStrbufAppendS(themeOrError, "Unknown WM: ");
    ffStrbufAppend(themeOrError, &wm->dePrettyName);
    return false;
}
