#include "wallpaper.h"
#include "common/settings.h"
#include "util/apple/osascript.h"

#import <Foundation/Foundation.h>

const char* ffDetectWallpaper(FFstrbuf* result)
{
    {
        // For Sonoma
        // https://github.com/JohnCoates/Aerial/issues/1332
        NSError* error;
        NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Application Support/com.apple.wallpaper/Store/Index.plist", instance.state.platform.homeDir.chars];
        NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                        error:&error];
        if (!error)
        {
            NSArray* choices = [dict valueForKeyPath:@"SystemDefault.Desktop.Content.Choices"];
            if (choices.count > 0)
            {
                NSDictionary* choice = choices[0];
                NSArray* files = choice[@"Files"];
                if (files.count > 0)
                {
                    NSString* file = files[0][@"relative"];
                    ffStrbufAppendS(result, [NSURL URLWithString:file].path.UTF8String);
                }
                else
                {
                    NSString* provider = choice[@"Provider"];
                    NSString* builtinPrefix = @"com.apple.wallpaper.choice.";
                    if ([provider hasPrefix:builtinPrefix])
                        provider = [provider substringFromIndex:builtinPrefix.length];
                    if ([provider isEqualToString:@"sonoma"])
                        ffStrbufSetStatic(result, "macOS Sonoma");
                    else if ([provider isEqualToString:@"aerials"]) // Most builtin aerial wallpapers are private
                        ffStrbufSetStatic(result, "Built-in aerial photography");
                    else
                        ffStrbufAppendF(result, "Built-in %s wallpaper", provider.UTF8String);
                }
            }
            if (result->length > 0)
                return NULL;
        }
    }

    #ifdef FF_HAVE_SQLITE3

    {
        // For Ventura
        // https://stackoverflow.com/questions/301215/getting-desktop-background-on-mac
        FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateCopy(&instance.state.platform.homeDir);
        ffStrbufAppendS(&path, "Library/Application Support/Dock/desktoppicture.db");
        if (ffSettingsGetSQLite3String(path.chars,
            "SELECT value\n"
            "FROM preferences\n"
            "JOIN data ON preferences.data_id=data.ROWID\n"
            "JOIN pictures ON preferences.picture_id=pictures.ROWID\n"
            "JOIN displays ON pictures.display_id=displays.ROWID\n"
            "JOIN spaces ON pictures.space_id=spaces.ROWID\n"
            "WHERE display_id=1 AND space_id=1 AND key=1", result)
        )
            return NULL;
    }

    #endif

    if (ffOsascript("tell application \"Finder\" to get POSIX path of (get desktop picture as alias)", result))
        return NULL;

    return "All detection methods failed";
}
