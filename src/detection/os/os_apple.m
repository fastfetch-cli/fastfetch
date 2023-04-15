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

static bool detectOSCodeName(FFOSResult* os)
{
    // https://en.wikipedia.org/wiki/MacOS_version_history
    char* str_end;
    const char* version = os->version.chars;
    unsigned long num = strtoul(version, &str_end, 10);
    if (str_end == version) return false;

    switch (num)
    {
        case 14: ffStrbufAppendS(&os->codename, "Sonoma"); return true;
        case 13: ffStrbufAppendS(&os->codename, "Ventura"); return true;
        case 12: ffStrbufAppendS(&os->codename, "Monterey"); return true;
        case 11: ffStrbufAppendS(&os->codename, "Big Sur"); return true;
        case 10: {
            version = str_end + 1;
            num = strtoul(version, &str_end, 10);
            if (str_end == version) return false;

            switch (num)
            {
                case 16: ffStrbufAppendS(&os->codename, "Big Sur"); return true;
                case 15: ffStrbufAppendS(&os->codename, "Catalina"); return true;
                case 14: ffStrbufAppendS(&os->codename, "Mojave"); return true;
                case 13: ffStrbufAppendS(&os->codename, "High Sierra"); return true;
                case 12: ffStrbufAppendS(&os->codename, "Sierra"); return true;
                case 11: ffStrbufAppendS(&os->codename, "El Capitan"); return true;
                case 10: ffStrbufAppendS(&os->codename, "Yosemite"); return true;
                case 9: ffStrbufAppendS(&os->codename, "Mavericks"); return true;
                case 8: ffStrbufAppendS(&os->codename, "Mountain Lion"); return true;
                case 7: ffStrbufAppendS(&os->codename, "Lion"); return true;
                case 6: ffStrbufAppendS(&os->codename, "Snow Leopard"); return true;
                case 5: ffStrbufAppendS(&os->codename, "Leopard"); return true;
                case 4: ffStrbufAppendS(&os->codename, "Tiger"); return true;
                case 3: ffStrbufAppendS(&os->codename, "Panther"); return true;
                case 2: ffStrbufAppendS(&os->codename, "Jaguar"); return true;
                case 1: ffStrbufAppendS(&os->codename, "Puma"); return true;
                case 0: ffStrbufAppendS(&os->codename, "Cheetah"); return true;
            }
        }
    }

    return false;
}

static void parseOSXSoftwareLicense(FFOSResult* os)
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

    ffStrbufInit(&os->codename);
    ffStrbufInit(&os->idLike);
    ffStrbufInit(&os->variant);
    ffStrbufInit(&os->variantID);

    parseSystemVersion(os);

    if(ffStrbufStartsWithIgnCaseS(&os->name, "MacOS"))
        ffStrbufAppendS(&os->id, "macos");

    if(os->version.length == 0)
        ffSysctlGetString("kern.osproductversion", &os->version);

    if(os->buildID.length == 0)
        ffSysctlGetString("kern.osversion", &os->buildID);

    ffStrbufAppend(&os->prettyName, &os->name);
    ffStrbufAppend(&os->versionID, &os->version);

    if(!detectOSCodeName(os))
        parseOSXSoftwareLicense(os);
}
