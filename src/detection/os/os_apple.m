#include "os.h"
#include "common/io/io.h"
#include "common/sysctl.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

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
        case 15: ffStrbufSetStatic(&os->codename, "Sequoia"); return true;
        case 14: ffStrbufSetStatic(&os->codename, "Sonoma"); return true;
        case 13: ffStrbufSetStatic(&os->codename, "Ventura"); return true;
        case 12: ffStrbufSetStatic(&os->codename, "Monterey"); return true;
        case 11: ffStrbufSetStatic(&os->codename, "Big Sur"); return true;
        case 10: {
            version = str_end + 1;
            num = strtoul(version, &str_end, 10);
            if (str_end == version) return false;

            switch (num)
            {
                case 16: ffStrbufSetStatic(&os->codename, "Big Sur"); return true;
                case 15: ffStrbufSetStatic(&os->codename, "Catalina"); return true;
                case 14: ffStrbufSetStatic(&os->codename, "Mojave"); return true;
                case 13: ffStrbufSetStatic(&os->codename, "High Sierra"); return true;
                case 12: ffStrbufSetStatic(&os->codename, "Sierra"); return true;
                case 11: ffStrbufSetStatic(&os->codename, "El Capitan"); ffStrbufSetStatic(&os->prettyName, "OS X"); return true;
                case 10: ffStrbufSetStatic(&os->codename, "Yosemite"); ffStrbufSetStatic(&os->prettyName, "OS X"); return true;
                case 9: ffStrbufSetStatic(&os->codename, "Mavericks"); ffStrbufSetStatic(&os->prettyName, "OS X"); return true;
                case 8: ffStrbufSetStatic(&os->codename, "Mountain Lion"); ffStrbufSetStatic(&os->prettyName, "OS X"); return true;
                case 7: ffStrbufSetStatic(&os->codename, "Lion"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 6: ffStrbufSetStatic(&os->codename, "Snow Leopard"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 5: ffStrbufSetStatic(&os->codename, "Leopard"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 4: ffStrbufSetStatic(&os->codename, "Tiger"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 3: ffStrbufSetStatic(&os->codename, "Panther"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 2: ffStrbufSetStatic(&os->codename, "Jaguar"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 1: ffStrbufSetStatic(&os->codename, "Puma"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
                case 0: ffStrbufSetStatic(&os->codename, "Cheetah"); ffStrbufSetStatic(&os->prettyName, "Mac OS X"); return true;
            }
        }
    }

    return false;
}

static void parseOSXSoftwareLicense(FFOSResult* os)
{
    FF_AUTO_CLOSE_FILE FILE* rtf = fopen("/System/Library/CoreServices/Setup Assistant.app/Contents/Resources/en.lproj/OSXSoftwareLicense.rtf", "r");
    if(rtf == NULL)
        return;

    FF_AUTO_FREE char* line = NULL;
    size_t len = 0;
    const char* searchStr = "\\f0\\b SOFTWARE LICENSE AGREEMENT FOR macOS ";
    while(getline(&line, &len, rtf) != EOF)
    {
        if (ffStrStartsWith(line, searchStr))
        {
            ffStrbufAppendS(&os->codename, line + strlen(searchStr));
            ffStrbufTrimRight(&os->codename, '\n');
            ffStrbufTrimRight(&os->codename, '\\');
            break;
        }
    }
}

void ffDetectOSImpl(FFOSResult* os)
{
    parseSystemVersion(os);

    ffStrbufSetStatic(&os->id, "macos");

    if(os->version.length == 0)
        ffSysctlGetString("kern.osproductversion", &os->version);

    if(os->buildID.length == 0)
        ffSysctlGetString("kern.osversion", &os->buildID);

    if(os->prettyName.length == 0)
        ffStrbufSetStatic(&os->prettyName, "macOS");
    ffStrbufAppend(&os->versionID, &os->version);

    if(!detectOSCodeName(os))
        parseOSXSoftwareLicense(os);
}
