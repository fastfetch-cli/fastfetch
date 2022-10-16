extern "C" {
#include "bios.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectBios(FFBiosResult* bios)
{
    ffStrbufInit(&bios->error);

    ffStrbufInit(&bios->biosDate);
    ffStrbufInit(&bios->biosRelease);
    ffStrbufInit(&bios->biosVendor);
    ffStrbufInit(&bios->biosVersion);

    FFWmiQuery query(L"SELECT Name, ReleaseDate, Version, Manufacturer FROM Win32_BIOS", &bios->error);
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        record.getString(L"Name", &bios->biosRelease);
        record.getString(L"ReleaseDate", &bios->biosDate);
        record.getString(L"Version", &bios->biosVersion);
        record.getString(L"Manufacturer", &bios->biosVendor);
    }
    else
        ffStrbufInitS(&bios->error, "No Wmi result returned");
}
