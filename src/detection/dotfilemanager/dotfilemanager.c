#include "common/io/io.h"
#include "dotfilemanager.h"

static bool detectBareGitRepository(FFDotfileManagerResult* result)
{
    FF_STRBUF_AUTO_DESTROY fullPath = ffStrbufCreate();
    ffStrbufSet(&fullPath, &instance.state.platform.homeDir);
    ffStrbufAppendS(&fullPath, ".git");

    if (ffPathExists(fullPath.chars, FF_PATHTYPE_DIRECTORY)) {
        ffStrbufSetS(&result->name, "bare git repository");
        return true;
    }

    return false;
}

static bool detectChezmoi(FFDotfileManagerResult* result)
{
    FF_LIST_FOR_EACH(FFstrbuf, dataDir, instance.state.platform.dataDirs)
    {
        FF_STRBUF_AUTO_DESTROY fullPath = ffStrbufCreate();
        ffStrbufSet(&fullPath, dataDir);
        ffStrbufAppendS(&fullPath, "chezmoi");

        if (ffPathExists(fullPath.chars, FF_PATHTYPE_DIRECTORY)) {
            ffStrbufSetS(&result->name, "chezmoi");
            return true;
        }
    }

    return false;
}

static bool detectYADM(FFDotfileManagerResult* result)
{
    FF_LIST_FOR_EACH(FFstrbuf, configDir, instance.state.platform.configDirs)
    {
        FF_STRBUF_AUTO_DESTROY fullPath = ffStrbufCreate();
        ffStrbufSet(&fullPath, configDir);
        ffStrbufAppendS(&fullPath, "yadm");

        if (ffPathExists(fullPath.chars, FF_PATHTYPE_DIRECTORY)) {
            ffStrbufSetS(&result->name, "yadm");
            return true;
        }
    }

    return false;
}

const char* ffDetectDotfileManager(FFDotfileManagerResult* result)
{
    if (detectBareGitRepository(result)) return NULL;
    if (detectChezmoi(result)) return NULL;
    if (detectYADM(result)) return NULL;

    return NULL;
}
