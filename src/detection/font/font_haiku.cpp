extern "C" {
#include "font.h"
}

#include <Application.h>
#include <Font.h>
#include <Menu.h>

extern "C" {
    const char* ffDetectFontImpl(FFFontResult* result);
}

static void generateString(FFFontResult* font)
{
    const char* types[] = { "Plain", "Menu", "Bold", "Mono" };
    for(uint32_t i = 0; i < ARRAY_SIZE(types); ++i)
    {
        if(i == 0 || !ffStrbufEqual(&font->fonts[i - 1], &font->fonts[i]))
        {
            if(i > 0)
                ffStrbufAppendS(&font->display, "], ");
            ffStrbufAppendF(&font->display, "%s [%s", font->fonts[i].chars, types[i]);
        }
        else
        {
            ffStrbufAppendS(&font->display, " / ");
            ffStrbufAppendS(&font->display, types[i]);
        }
    }
    ffStrbufAppendC(&font->display, ']');
}

const char* ffDetectFontImpl(FFFontResult* result)
{
    struct menu_info menuInfo;
    const BFont *f;
    // We need a valid be_app to query the app_server here.
    BApplication app("application/x-vnd.fastfetch-cli-fastfetch");

    if ((f = be_plain_font) != NULL)
    {
        f->GetFamilyAndStyle(&menuInfo.f_family, &menuInfo.f_style);
        ffStrbufAppendF(&result->fonts[0], "%s %s (%dpt)", menuInfo.f_family, menuInfo.f_style, (int)f->Size());
    }
    if (get_menu_info(&menuInfo) == B_OK)
    {
        ffStrbufAppendF(&result->fonts[1], "%s %s (%dpt)", menuInfo.f_family, menuInfo.f_style, (int)menuInfo.font_size);
    }
    if ((f = be_bold_font) != NULL)
    {
        f->GetFamilyAndStyle(&menuInfo.f_family, &menuInfo.f_style);
        ffStrbufAppendF(&result->fonts[2], "%s %s (%dpt)", menuInfo.f_family, menuInfo.f_style, (int)f->Size());
    }
    if ((f = be_fixed_font) != NULL)
    {
        f->GetFamilyAndStyle(&menuInfo.f_family, &menuInfo.f_style);
        ffStrbufAppendF(&result->fonts[3], "%s %s (%dpt)", menuInfo.f_family, menuInfo.f_style, (int)f->Size());
    }

    generateString(result);

    return NULL;
}
