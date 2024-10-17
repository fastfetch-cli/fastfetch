#include "os.h"
#include "common/library.h"
#include "util/windows/unicode.h"
#include "util/stringUtils.h"

#include <windows.h>

PWSTR WINAPI BrandingFormatString(PCWSTR format);

void ffDetectOSImpl(FFOSResult* os)
{
    //https://dennisbabkin.com/blog/?t=how-to-tell-the-real-version-of-windows-your-app-is-running-on#ver_string
    const wchar_t* rawName = BrandingFormatString(L"%WINDOWS_LONG%");
    ffStrbufSetWS(&os->variant, rawName);
    GlobalFree((HGLOBAL)rawName);
    ffStrbufTrimRight(&os->variant, ' ');

    //WMI returns the "Microsoft" prefix while BrandingFormatString doesn't. Make them consistent.
    if(ffStrbufStartsWithS(&os->variant, "Microsoft "))
        ffStrbufSubstrAfter(&os->variant, strlen("Microsoft ") - 1);

    if(ffStrbufStartsWithS(&os->variant, "Windows "))
    {
        ffStrbufAppendS(&os->name, "Windows");
        ffStrbufAppendS(&os->prettyName, "Windows");

        ffStrbufSubstrAfter(&os->variant, strlen("Windows ") - 1);

        if(ffStrbufStartsWithS(&os->variant, "Server "))
        {
            ffStrbufAppendS(&os->name, " Server");
            ffStrbufAppendS(&os->prettyName, " Server");
            ffStrbufSubstrAfter(&os->variant, strlen(" Server") - 1);
        }

        uint32_t index = ffStrbufFirstIndexC(&os->variant, ' ');
        ffStrbufAppendNS(&os->version, index, os->variant.chars);
        ffStrbufSubstrAfter(&os->variant, index);

        // Windows Server 20xx Rx
        if(ffStrbufEndsWithC(&os->prettyName, 'r'))
        {
            if(os->variant.chars[0] == 'R' &&
                ffCharIsDigit(os->variant.chars[1]) &&
                (os->variant.chars[2] == '\0' || os->variant.chars[2] == ' '))
            {
                ffStrbufAppendF(&os->version, " R%c", os->variant.chars[1]);
                ffStrbufSubstrAfter(&os->variant, strlen("Rx ") - 1);
            }
        }
    }
    else
    {
        // Unknown Windows name, please report this
        ffStrbufAppend(&os->name, &os->variant);
        ffStrbufClear(&os->variant);
    }

    ffStrbufAppendF(&os->id, "%*s %*s", os->prettyName.length, os->prettyName.chars, os->version.length, os->version.chars);
    ffStrbufSetStatic(&os->idLike, "Windows");
}
