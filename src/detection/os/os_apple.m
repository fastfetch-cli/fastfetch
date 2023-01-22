#include "os.h"
#include "common/sysctl.h"

#include <stdlib.h>
#include <string.h>
#import <Foundation/Foundation.h>

static void parseSystemVersion(FFOSResult* os)
{
    NSError* error;
    NSString* fileName = @"file:///System/Library/CoreServices/SystemVersion.plist";
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
        return;

    NSString* value;

    if((value = [dict valueForKey:@"ProductName"]))
        ffStrbufInitS(&os->name, value.UTF8String);
    if((value = [dict valueForKey:@"ProductUserVisibleVersion"]))
        ffStrbufInitS(&os->version, value.UTF8String);
    if((value = [dict valueForKey:@"ProductBuildVersion"]))
        ffStrbufInitS(&os->buildID, value.UTF8String);
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

    parseOSXSoftwareLicense(os);
}
