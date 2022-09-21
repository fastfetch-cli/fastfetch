#include "os.h"
#include "common/properties.h"
#include "common/sysctl.h"

#include <stdlib.h>
#include <string.h>

typedef enum PListKey
{
    PLIST_KEY_NAME,
    PLIST_KEY_VERSION,
    PLIST_KEY_BUILD,
    PLIST_KEY_OTHER
} PListKey;

static void parseSystemVersion(FFOSResult* os)
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

void parseOSXSoftwareLicense(FFOSResult* os)
{
    FILE* rtf = fopen("/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf", "r");
    if(rtf == NULL)
        return;

    char* line = NULL;
    size_t len = 0;
    const char* searchStr = "\\f0\\b SOFTWARE LICENSE AGREEMENT FOR macOS ";
    const size_t searchLen = strlen(searchStr);
    while(getline(&line, &len, rtf) != EOF)
    {
        if (strncmp(line, searchStr, searchLen) == 0)
        {
            ffStrbufAppendS(&os->codename, line + searchLen);
            ffStrbufTrimRight(&os->codename, '\n');
            ffStrbufTrimRight(&os->codename, '\\');
            break;
        }
    }

    if(line != NULL)
        free(line);

    fclose(rtf);
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

    parseSystemVersion(os);

    if(ffStrbufStartsWithIgnCaseS(&os->name, "MacOS"))
        ffStrbufAppendS(&os->id, "macos");

    if(os->version.length == 0)
        ffSysctlGetString("kern.osproductversion", &os->version);

    if(os->buildID.length == 0)
        ffSysctlGetString("kern.osversion", &os->buildID);

    ffStrbufAppend(&os->prettyName, &os->name);
    ffStrbufAppend(&os->versionID, &os->version);
    ffSysctlGetString("kern.ostype", &os->systemName);
    ffSysctlGetString("hw.machine", &os->architecture);

    parseOSXSoftwareLicense(os);
}
