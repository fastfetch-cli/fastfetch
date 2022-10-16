#include "util/windows/wmi.hpp"
extern "C" {
    #include "utsname.h"
}

int uname(struct utsname *name)
{
    memset(name, 0, sizeof(*name));

    strncpy(name->sysname, "Windows_NT", UTSNAME_MAXLENGTH);

    FFWmiQuery query(L"SELECT Version, CSName, OSArchitecture FROM Win32_OperatingSystem");
    if(!query)
        return -1;

    if(FFWmiRecord record = query.next())
    {
        FFstrbuf value;
        ffStrbufInit(&value);
        record.getString(L"Version", &value);
        strncpy(name->release, value.chars, UTSNAME_MAXLENGTH);

        ffStrbufClear(&value);
        record.getString(L"CSName", &value);
        strncpy(name->nodename, value.chars, UTSNAME_MAXLENGTH);

        ffStrbufClear(&value);
        record.getString(L"OSArchitecture", &value);
        strncpy(name->machine, value.chars, UTSNAME_MAXLENGTH);

        ffStrbufDestroy(&value);
    }

    return 0;
}
