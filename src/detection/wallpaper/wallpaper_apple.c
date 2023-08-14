#include "wallpaper.h"
#include "common/settings.h"
#include "util/apple/osascript.h"

const char* ffDetectWallpaper(FFstrbuf* result)
{
    // https://stackoverflow.com/questions/301215/getting-desktop-background-on-mac

    #ifdef FF_HAVE_SQLITE3

    FF_STRBUF_AUTO_DESTROY path = ffStrbufCreateCopy(&instance.state.platform.homeDir);
    ffStrbufAppendS(&path, "Library/Application Support/Dock/desktoppicture.db");
    if (ffSettingsGetSQLite3String(path.chars,
        "SELECT value\n"
        "FROM preferences\n"
        "JOIN data ON preferences.data_id=data.ROWID\n"
        "JOIN pictures ON preferences.picture_id=pictures.ROWID\n"
        "JOIN displays ON pictures.display_id=displays.ROWID\n"
        "JOIN spaces ON pictures.space_id=spaces.ROWID\n"
        "WHERE display_id=1 AND space_id=1 AND key=1", result)
    )
        return NULL;

    #endif

    if (!ffOsascript("tell application \"Finder\" to get POSIX path of (get desktop picture as alias)", result))
        return "ffOsascript() failed";

    return NULL;
}
