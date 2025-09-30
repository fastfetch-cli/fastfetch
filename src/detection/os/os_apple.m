#include "os.h"
#include "common/io/io.h"
#include "common/sysctl.h"
#include "util/stringUtils.h"
#include "util/mallocHelper.h"

#include <stdlib.h>
#include <string.h>
#import <Foundation/Foundation.h>

static bool parseSystemVersion(FFOSResult* os)
{
    NSError* error;
    NSString* fileName = @"file:///System/Library/CoreServices/SystemVersion.plist";
    NSDictionary* dict = [NSDictionary dictionaryWithContentsOfURL:[NSURL URLWithString:fileName]
                                       error:&error];
    if(error)
        return false;

    NSString* value;

    if((value = dict[@"ProductName"]))
        ffStrbufInitS(&os->name, value.UTF8String);
    if((value = dict[@"ProductUserVisibleVersion"]))
        ffStrbufInitS(&os->version, value.UTF8String);
    if((value = dict[@"ProductBuildVersion"]))
        ffStrbufInitS(&os->buildID, value.UTF8String);
    if (ffStrbufStartsWithS(&os->version, "16."))
    {
        // macOS 26 Tahoe. #1809
        os->version.chars[0] = '2';
    }

    return true;
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
        case 26:
        case 16: ffStrbufSetStatic(&os->codename, "Tahoe"); return true;
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
                case 11: ffStrbufSetStatic(&os->codename, "El Capitan"); return true;
                case 10: ffStrbufSetStatic(&os->codename, "Yosemite"); return true;
                case 9: ffStrbufSetStatic(&os->codename, "Mavericks"); return true;
                case 8: ffStrbufSetStatic(&os->codename, "Mountain Lion"); return true;
                case 7: ffStrbufSetStatic(&os->codename, "Lion"); return true;
                case 6: ffStrbufSetStatic(&os->codename, "Snow Leopard"); return true;
                case 5: ffStrbufSetStatic(&os->codename, "Leopard"); return true;
                case 4: ffStrbufSetStatic(&os->codename, "Tiger"); return true;
                case 3: ffStrbufSetStatic(&os->codename, "Panther"); return true;
                case 2: ffStrbufSetStatic(&os->codename, "Jaguar"); return true;
                case 1: ffStrbufSetStatic(&os->codename, "Puma"); return true;
                case 0: ffStrbufSetStatic(&os->codename, "Cheetah"); return true;
            }
        }
    }

    return false;
}

void ffDetectOSImpl(FFOSResult* os)
{
    parseSystemVersion(os);

    ffStrbufSetStatic(&os->id, "macos");

    if(__builtin_expect(os->name.length == 0, 0))
        ffStrbufSetStatic(&os->name, "macOS");

    if(__builtin_expect(os->version.length == 0, 0))
        ffSysctlGetString("kern.osproductversion", &os->version);

    if(__builtin_expect(os->buildID.length == 0, 0))
        ffSysctlGetString("kern.osversion", &os->buildID);

    ffStrbufAppend(&os->versionID, &os->version);

    detectOSCodeName(os);

    ffStrbufSetF(&os->prettyName, "%s %s %s (%s)", os->name.chars, os->codename.chars, os->version.chars, os->buildID.chars);
}
