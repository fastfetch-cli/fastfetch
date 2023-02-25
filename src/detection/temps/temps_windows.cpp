extern "C"
{
#include "temps_windows.h"
}
#include "util/windows/wmi.hpp"

extern "C"
const char* ffDetectSmbiosTemp(double* current, double* critical)
{
    // Requires Administrator priviledges
    // https://wutils.com/wmi/root/wmi/msacpi_thermalzonetemperature/#properties
    FFWmiQuery query(L"SELECT CurrentTemperature, CriticalTripPoint FROM MSAcpi_ThermalZoneTemperature WHERE Active = TRUE", nullptr, FFWmiNamespace::WMI);
    if(!query)
        return "Query WMI service failed";

    if(FFWmiRecord record = query.next())
    {
        if (current && record.getReal(L"CurrentTemperature", current)) // In tenth of degrees Kelvin
            *current = *current / 10 - 273.15;
        if (critical && record.getReal(L"CriticalTripPoint", critical)) // In tenth of degrees Kelvin
            *critical = *critical / 10 - 273.15;
    }

    return "No WMI result returned";
}
