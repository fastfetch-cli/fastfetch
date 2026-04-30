#include "osascript.h"

#import <Foundation/Foundation.h>

bool ffOsascript(const char* input, FFstrbuf* result)
{
    NSAppleScript* script = [NSAppleScript.alloc initWithSource:@(input)];
    NSDictionary* errInfo = nil;
    NSAppleEventDescriptor* descriptor = [script executeAndReturnError:&errInfo];
    if (errInfo)
        return false;

    ffStrbufSetS(result, descriptor.stringValue.UTF8String);
    return true;
}
