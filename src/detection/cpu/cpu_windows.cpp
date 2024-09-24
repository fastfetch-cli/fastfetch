#include "util/windows/wmi.hpp"

extern "C"
const char* detectThermalTemp(double* current, double* critical)
{
    // Requires Administrator privileges
    // https://wutils.com/wmi/root/wmi/msacpi_thermalzonetemperature/#properties
    FFWmiQuery query(L"SELECT CurrentTemperature, CriticalTripPoint FROM MSAcpi_ThermalZoneTemperature WHERE Active = TRUE", nullptr, FFWmiNamespace::WMI);
    if(!query)
        return "Query WMI service failed";

    if(FFWmiRecord record = query.next())
    {
        if (current)
        {
            if(auto vtCurrent = record.get(L"CurrentTemperature"))
                *current = vtCurrent.get<int32_t>() / 10 - 273.15; // In tenth of degrees Kelvin
            else
                *current = 0.0/0.0;
        }

        if (critical)
        {
            if(auto vtCritical = record.get(L"CriticalTripPoint"))
                *critical = vtCritical.get<int32_t>() / 10 - 273.15; // In tenth of degrees Kelvin
            else
                *critical = 0.0/0.0;
        }
    }

    return "No WMI result returned";
}
