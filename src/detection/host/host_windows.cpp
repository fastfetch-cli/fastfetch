extern "C" {
#include "host.h"
}
#include "util/windows/wmi.hpp"

extern "C" void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInit(&host->sysVendor);
    ffStrbufInit(&host->chassisType);
    ffStrbufInit(&host->chassisVendor);
    ffStrbufInit(&host->chassisVersion);

    FFWmiQuery query(L"SELECT Name, Version, SKUNumber, Vendor FROM Win32_ComputerSystemProduct", &host->error);
    if(!query)
        return;

    if(FFWmiRecord record = query.next())
    {
        record.getString(L"Name", &host->productName);
        record.getString(L"Version", &host->productVersion);
        record.getString(L"SKUNumber", &host->productSku);
        record.getString(L"Vendor", &host->sysVendor);
    }
    else
        ffStrbufAppendS(&host->error, "No Wmi result returned");
}
