extern "C" {
#include "wallpaper.h"
#include "common/mallocHelper.h"
}

#include <Application.h>
#include <FindDirectory.h>
#include <InterfaceDefs.h>
#include <Node.h>
#include <Path.h>
#include <Screen.h>
#include <fs_attr.h>
#include <be_apps/Tracker/Background.h>

const char* ffDetectWallpaper(FFstrbuf* result) {
    BMessage backgrounds;
    BPath pDesktop;
    BString path;

    if (find_directory(B_DESKTOP_DIRECTORY, &pDesktop) < B_OK) {
        return "find_directory(B_DESKTOP_DIRECTORY) failed";
    }

    // We need a valid be_app to query the app_server here.
    BApplication app("application/x-vnd.fastfetch-cli-fastfetch");

    BNode nDesktop(pDesktop.Path());
    if (nDesktop.InitCheck() == B_OK) {
        struct attr_info ai;
        if (nDesktop.GetAttrInfo(B_BACKGROUND_INFO, &ai) == B_OK && ai.size > 0) {
            FF_AUTO_FREE char* pAttr = (char*) malloc(ai.size);
            if (nDesktop.ReadAttr(B_BACKGROUND_INFO, ai.type, 0LL, pAttr, (size_t) ai.size) >= B_OK) {
                if (backgrounds.Unflatten(pAttr) == B_OK) {
                    for (int i = 0; backgrounds.FindString(B_BACKGROUND_IMAGE, i, &path) == B_OK; i++) {
                        int32 ws;
                        if (backgrounds.FindInt32(B_BACKGROUND_WORKSPACES, i, &ws) == B_OK) {
                            if (ws & (1 << current_workspace())) {
                                // We try to match the one for the current workspace
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (path.Length() < 1) {
        return "Failed to detect the current wallpaper path";
    }

    ffStrbufAppendS(result, path.String());
    return NULL;
}
