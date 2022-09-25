#include "common/font.h"
#include "util/apple/cf_helpers.h"
#include "font.h"

#import <AppKit/AppKit.h>

#define FF_FONT_MODULE_NAME "Font"

static void detectFontForType(CTFontUIFontType uiType, FFFontResult* result)
{
    CTFontRef ctFont = CTFontCreateUIFontForLanguage(uiType, 12, NULL);

    ffStrbufInit(&result->fontPretty);
    ffCfStrGetString(CTFontCopyFullName(ctFont), &result->fontPretty);

    FFstrbuf familyName;
    ffStrbufInit(&familyName);
    ffCfStrGetString(CTFontCopyFamilyName(ctFont), &familyName);

    if (ffStrbufComp(&familyName, &result->fontPretty) != 0)
    {
        ffStrbufAppendF(&result->fontPretty, " (%*s)", familyName.length, familyName.chars);
    }

    ffStrbufDestroy(&familyName);
}

const char* ffDetectFontImpl(FFinstance* instance, FFlist* result)
{
    FF_UNUSED(instance);

    FFFontResult* resultSystem = (FFFontResult*)ffListAdd(result);
    resultSystem->type = "SYS";
    detectFontForType(kCTFontUIFontSystem, resultSystem);

    FFFontResult* resultUser = (FFFontResult*)ffListAdd(result);
    resultUser->type = "USER";
    detectFontForType(kCTFontUIFontUser, resultUser);

    return NULL;
}
