extern "C" {
#include "fastfetch.h"
#include "wmtheme.h"
}

#include <Application.h>
#include <private/interface/DecorInfo.h>

bool ffDetectWmTheme(FFstrbuf* themeOrError)
{
    // We need a valid be_app to query the app_server here.
    BApplication app("application/x-vnd.fastfetch-cli-fastfetch");

    DecorInfoUtility *util = new DecorInfoUtility();
    DecorInfo* decor = NULL;

    if (util) {
        decor = util->CurrentDecorator();
        if (decor) {
            ffStrbufAppendS(themeOrError, decor->Name().String());
        }
        delete util;
        if (decor) {
            return true;
        }
    }

    ffStrbufAppendS(themeOrError, "Failed to get DecorInfo");
    return false;
}
