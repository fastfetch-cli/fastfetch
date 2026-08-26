#include "lm.h"

#import <Foundation/Foundation.h>

const char* ffDetectLM(FFLMResult* result) {
    ffStrbufSetStatic(&result->service, "loginwindow");
    ffStrbufSetStatic(&result->prettyName, "Login Window");

    NSError* error;
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL fileURLWithPath:@"/System/Library/CoreServices/loginwindow.app/Contents/Info.plist" isDirectory:NO]
                                                                error:&error];

    if (dict) {
        ffStrbufSetS(&result->version, ((NSString*) dict[@"CFBundleShortVersionString"]).UTF8String);
    }

    return nullptr;
}
