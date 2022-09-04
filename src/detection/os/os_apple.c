#include "os.h"
#include "common/properties.h"
#include "common/settings.h"

#include <stdlib.h>
#include <sys/sysctl.h>

typedef enum PListKey
{
    PLIST_KEY_NAME,
    PLIST_KEY_VERSION,
    PLIST_KEY_BUILD,
    PLIST_KEY_OTHER
} PListKey;

static void parseFile(FFOSResult* os)
{
    FILE* plist = fopen("/System/Library/CoreServices/SystemVersion.plist", "r");
    if(plist == NULL)
    return;

    char* line = NULL;
    size_t len = 0;
    PListKey key = PLIST_KEY_OTHER;

    FFstrbuf keyBuffer;
    ffStrbufInit(&keyBuffer);

    while(getline(&line, &len, plist) != EOF)
    {
        if(ffParsePropLine(line, "<key>", &keyBuffer))
        {
            if(ffStrbufIgnCaseCompS(&keyBuffer, "ProductName") == 0)
                key = PLIST_KEY_NAME;
            else if(ffStrbufIgnCaseCompS(&keyBuffer, "ProductUserVisibleVersion") == 0)
                key = PLIST_KEY_VERSION;
            else if(ffStrbufIgnCaseCompS(&keyBuffer, "ProductBuildVersion") == 0)
                key = PLIST_KEY_BUILD;
            else
                key = PLIST_KEY_OTHER;

            ffStrbufClear(&keyBuffer);
            continue;
        }

        if(key == PLIST_KEY_NAME)
            ffParsePropLine(line, "<string>", &os->name);
        else if(key == PLIST_KEY_VERSION)
            ffParsePropLine(line, "<string>", &os->version);
        else if(key == PLIST_KEY_BUILD)
            ffParsePropLine(line, "<string>", &os->buildID);
    }

    ffStrbufDestroy(&keyBuffer);

    if(line != NULL)
        free(line);

    fclose(plist);
}

void ffDetectOSImpl(FFOSResult* os, const FFinstance* instance)
{
    FF_UNUSED(instance);

    ffStrbufInit(&os->name);
    ffStrbufInit(&os->version);
    ffStrbufInit(&os->buildID);
    ffStrbufInit(&os->id);
    ffStrbufInit(&os->prettyName);
    ffStrbufInit(&os->versionID);
    ffStrbufInit(&os->systemName);
    ffStrbufInit(&os->architecture);

    ffStrbufInitA(&os->codename, 0);
    ffStrbufInitA(&os->idLike, 0);
    ffStrbufInitA(&os->variant, 0);
    ffStrbufInitA(&os->variantID, 0);

    parseFile(os);

    if(ffStrbufStartsWithIgnCaseS(&os->name, "MacOS"))
        ffStrbufAppendS(&os->id, "macos");

    if(os->version.length == 0)
        ffSettingsGetAppleProperty("kern.osproductversion", &os->version);

    //TODO map version to pretty name
    ffStrbufAppend(&os->prettyName, &os->name);
    ffStrbufAppend(&os->versionID, &os->version);
    ffSettingsGetAppleProperty("kern.ostype", &os->systemName);
    ffSettingsGetAppleProperty("hw.machine", &os->architecture);
}
