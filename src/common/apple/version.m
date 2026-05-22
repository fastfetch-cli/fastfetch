#include "common/apple/version.h"
#include "common/stringUtils.h"

#import <Foundation/Foundation.h>

#define APP_CONTENTS_MACOS ".app/Contents/MacOS"

bool ffGetAppNameAndVersion(const char* exePath, FFstrbuf* retName, FFstrbuf* retVersion) {
    char* lastSlash = strrchr(exePath, '/');
    if (!lastSlash) {
        return false;
    }
    if ((size_t) (lastSlash - exePath) > strlen("X" APP_CONTENTS_MACOS) && memcmp(lastSlash - strlen(APP_CONTENTS_MACOS), APP_CONTENTS_MACOS, strlen(APP_CONTENTS_MACOS)) != 0) {
        return false;
    }

    lastSlash -= strlen("MacOS");
    char infoPlistPath[PATH_MAX];
    memcpy(infoPlistPath, exePath, lastSlash - exePath);
    memcpy(infoPlistPath + (lastSlash - exePath), "Info.plist", sizeof("Info.plist")); // X.app/Contents/Info.plist
    NSError* error;
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@(infoPlistPath)]
                                                             error:&error];
    if (!dict) {
        return false;
    }

    if (retName) {
        NSString* bundleName = dict[@"CFBundleDisplayName"] ?: dict[@"CFBundleName"];
        if (bundleName) {
            ffStrbufSetS(retName, bundleName.UTF8String);
        }
    }

    if (retVersion) {
        NSString* bundleVersion = dict[@"CFBundleShortVersionString"];
        if (bundleVersion) {
            ffStrbufSetS(retVersion, bundleVersion.UTF8String);
        }
    }

    return true;
}
