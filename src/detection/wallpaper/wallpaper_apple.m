#include "wallpaper.h"
#include "common/settings.h"
#include "common/apple/osascript.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

const char* detectFromPlist(FFstrbuf* result) {
    // For Sonoma and later (macOS 14.0+)
    // https://github.com/JohnCoates/Aerial/issues/1332
    NSError* error;
    NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Application Support/com.apple.wallpaper/Store/Index.plist", instance.state.platform.homeDir.chars];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                                             error:&error];

    if (error) {
        return "Failed to read wallpaper plist file";
    }

    NSArray* choices = [dict valueForKeyPath:@"SystemDefault.Desktop.Content.Choices"];
    if (choices.count > 0) {
        NSDictionary* choice = choices[0];

        NSArray* files = choice[@"Files"];
        if (files.count > 0) {
            NSString* file = files[0][@"relative"];
            ffStrbufSetS(result, [NSURL URLWithString:file].path.UTF8String);
        }

        if (result->length == 0) {
            NSData* configData = choice[@"Configuration"];
            if (configData && configData.length > 0) {
                NSError* plistError = nil;
                NSDictionary* configPlist = [NSPropertyListSerialization propertyListWithData:configData
                                                                                      options:NSPropertyListImmutable
                                                                                       format:NULL
                                                                                        error:&plistError];

                if (!plistError && [configPlist isKindOfClass:NSDictionary.class]) {
                    NSDictionary* urlDict = configPlist[@"url"];
                    if ([urlDict isKindOfClass:NSDictionary.class]) {
                        NSString* relativeUrlString = urlDict[@"relative"];
                        if ([relativeUrlString isKindOfClass:[NSString class]]) {
                            NSURL* fileUrl = [NSURL URLWithString:relativeUrlString];
                            if (fileUrl.fileURL) {
                                ffStrbufSetS(result, fileUrl.path.UTF8String);
                            }
                        }
                    }
                }
            }
        }

        if (result->length == 0) {
            NSString* provider = choice[@"Provider"];
            if ([provider isKindOfClass:NSString.class]) {
                NSString* builtinPrefix = @"com.apple.wallpaper.choice.";
                if ([provider hasPrefix:builtinPrefix]) {
                    provider = [provider substringFromIndex:builtinPrefix.length];
                }

                // macOS internal wallpapers
                if ([provider isEqualToString:@"aerials"]) { // Most builtin aerial wallpapers are private
                    ffStrbufSetStatic(result, "Built-in aerial photography");
                } else if ([provider isEqualToString:@"default"]) {
                    ffStrbufSetStatic(result, "macOS Default Wallpaper");
                } else {
                    ffStrbufSetF(result, "Built-in %s wallpaper", provider.UTF8String);
                }
            }
        }
    }
    if (result->length == 0) {
        return "Failed to detect wallpaper from plist";
    }
    return nullptr;
}

#ifdef FF_HAVE_SQLITE3
const char* detectFromSQLite(FFstrbuf* result) {
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
            "WHERE display_id=1 AND space_id=1 AND key=1",
            result)) {
        return nullptr;
    }
    return "Failed to detect wallpaper from SQLite database";
}
#endif

const char* detectFromNSWorkspace(FFstrbuf* result) {
    // Reliable for user-picked static images.
    NSScreen* mainScreen = NSScreen.mainScreen;
    if (!mainScreen) {
        return "Failed to detect wallpaper from NSWorkspace: No main screen found";
    }

    NSURL* url = [NSWorkspace.sharedWorkspace desktopImageURLForScreen:mainScreen];
    if (url.fileURL) {
        ffStrbufSetS(result, url.path.UTF8String);
        return nullptr;
    }

    return "Failed to detect wallpaper from NSWorkspace";
}

const char* ffDetectWallpaper(FFstrbuf* result) {
    const char* error;

    if (@available(macOS 14.0, *)) {
        error = detectFromPlist(result);
    } else {
#ifdef FF_HAVE_SQLITE3
        error = detectFromSQLite(result);
#else
        error = detectFromNSWorkspace(result);
#endif
    }
    return error;
}
