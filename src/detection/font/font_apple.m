#include "common/font.h"
#include "common/io.h"
#include "util/apple/cf_helpers.h"
#include "font.h"

#import <AppKit/AppKit.h>

static void detectFontForType(CTFontUIFontType uiType, FFstrbuf* font)
{
    CTFontRef ctFont = CTFontCreateUIFontForLanguage(uiType, 12, NULL);
    ffCfStrGetString(CTFontCopyFamilyName(ctFont), font);
}

void ffDetectFontImpl(const FFinstance* instance, FFFontResult* result)
{
    FF_UNUSED(instance);
    ffSuppressIO(true);
    detectFontForType(kCTFontUIFontSystem, &result->fonts[0]);
    detectFontForType(kCTFontUIFontUser, &result->fonts[1]);
    detectFontForType(kCTFontUIFontUserFixedPitch, &result->fonts[2]);
    detectFontForType(kCTFontUIFontApplication, &result->fonts[3]);
    ffSuppressIO(false);
}
