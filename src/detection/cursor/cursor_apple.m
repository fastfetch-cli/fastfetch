#include "cursor.h"

#import <Foundation/Foundation.h>

static void appendColor(FFstrbuf* str, NSDictionary* dict)
{
    uint32_t r = (uint32_t) (((NSNumber*) dict[@"red"]).doubleValue * 255 + .5);
    uint32_t g = (uint32_t) (((NSNumber*) dict[@"green"]).doubleValue * 255 + .5);
    uint32_t b = (uint32_t) (((NSNumber*) dict[@"blue"]).doubleValue * 255 + .5);
    uint32_t a = (uint32_t) (((NSNumber*) dict[@"alpha"]).doubleValue * 255 + .5);
    uint32_t color = (r << 24) | (g << 16) | (b << 8) | a;

    switch (color)
    {
        case 0x000000FF: ffStrbufAppendS(str, "Black"); return;
        case 0x0433FFFF: ffStrbufAppendS(str, "Blue"); return;
        case 0xAA7942FF: ffStrbufAppendS(str, "Brown"); return;
        case 0x00FDFFFF: ffStrbufAppendS(str, "Cyan"); return;
        case 0x00F900FF: ffStrbufAppendS(str, "Green"); return;
        case 0xFF40FFFF: ffStrbufAppendS(str, "Magenta"); return;
        case 0xFF9300FF: ffStrbufAppendS(str, "Orange"); return;
        case 0x942192FF: ffStrbufAppendS(str, "Purple"); return;
        case 0xFF2600FF: ffStrbufAppendS(str, "Red"); return;
        case 0xFFFB00FF: ffStrbufAppendS(str, "Yellow"); return;
        case 0xFFFFFFFF: ffStrbufAppendS(str, "White"); return;
        case 0x00000000: ffStrbufAppendS(str, "Transparent"); return;
        default: ffStrbufAppendF(str, "#%08X", color); return;
    }
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
        ffStrbufAppendF(&result->size, "%d", (int) (mouseDriverCursorSize.doubleValue * 32 + 0.5));
    else
        ffStrbufAppendS(&result->size, "32");
}
