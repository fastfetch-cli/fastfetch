#include "osascript.h"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <CoreData/CoreData.h>

bool ffOsascript(const char* input, FFstrbuf* result) {
    NSString* appleScript = [NSString stringWithUTF8String: input];
    NSAppleScript* script = [[NSAppleScript alloc] initWithSource:appleScript];
    NSDictionary* errInfo = nil;
    NSAppleEventDescriptor* descriptor = [script executeAndReturnError:&errInfo];
    if (errInfo)
        return false;

    ffStrbufSetS(result, [[descriptor stringValue] cStringUsingEncoding:NSUTF8StringEncoding]);
    return true;
}
