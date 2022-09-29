#include "common/font.h"
#include "util/apple/cf_helpers.h"
#include "font.h"

#import <AppKit/AppKit.h>

static void detectFontForType(CTFontUIFontType uiType, FFstrbuf* font)
{
    CTFontRef ctFont = CTFontCreateUIFontForLanguage(uiType, 12, NULL);
    ffCfStrGetString(CTFontCopyFullName(ctFont), font);

    if(font->length == 0)
        return;

    FFstrbuf familyName;
    ffStrbufInit(&familyName);
    ffCfStrGetString(CTFontCopyFamilyName(ctFont), &familyName);

    if (ffStrbufComp(&familyName, font) != 0)
        ffStrbufAppendF(font, " (%*s)", familyName.length, familyName.chars);

    ffStrbufDestroy(&familyName);
}

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance);
    detectFontForType(kCTFontUIFontSystem, &result->fonts[0]);
    detectFontForType(kCTFontUIFontUser, &result->fonts[1]);
}
