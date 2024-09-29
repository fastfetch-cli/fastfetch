#include "cursor.h"

#import <Foundation/Foundation.h>

static void appendColor(FFstrbuf* str, NSDictionary* color)
{
    int r = (int) (((NSNumber*) color[@"red"]).doubleValue * 255);
    int g = (int) (((NSNumber*) color[@"green"]).doubleValue * 255);
    int b = (int) (((NSNumber*) color[@"blue"]).doubleValue * 255);
    int a = (int) (((NSNumber*) color[@"alpha"]).doubleValue * 255);

    if (r == 255 && g == 255 && b == 255 && a == 255)
        ffStrbufAppendS(str, "White");
    else if (r == 0 && g == 0 && b == 0 && a == 255)
        ffStrbufAppendS(str, "Black");
    else
        ffStrbufAppendF(str, "#%02X%02X%02X%02X", r, g, b, a);
}

void ffDetectCursor(FFCursorResult* result)
{
    NSError* error;
    NSString* fileName = [NSString stringWithFormat:@"file://%s/Library/Preferences/com.apple.universalaccess.plist", instance.state.platform.homeDir.chars];
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
    {
        ffStrbufAppendS(&result->error, error.localizedDescription.UTF8String);
        return;
    }

    NSDictionary* color;

    ffStrbufAppendS(&result->theme, "Fill - ");
    if ((color = dict[@"cursorFill"]))
        appendColor(&result->theme, color);
    else
        ffStrbufAppendS(&result->theme, "Black");

    ffStrbufAppendS(&result->theme, ", Outline - ");

    if ((color = dict[@"cursorOutline"]))
        appendColor(&result->theme, color);
    else
        ffStrbufAppendS(&result->theme, "White");

    NSNumber* mouseDriverCursorSize = dict[@"mouseDriverCursorSize"];
    if (mouseDriverCursorSize)
        ffStrbufAppendF(&result->size, "%d", (int) (mouseDriverCursorSize.doubleValue * 32));
    else
        ffStrbufAppendS(&result->size, "32");
}
