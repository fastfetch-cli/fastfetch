#include "host.h"
#include "common/sysctl.h"
#include "util/apple/cf_helpers.h"
#include "util/stringUtils.h"

#include <IOKit/IOKitLib.h>

static const char* getProductNameWithHwModel(const FFstrbuf* hwModel)
{
    // Macbook Pro: https://support.apple.com/en-us/HT201300
    // Macbook Air: https://support.apple.com/en-us/HT201862
    // Mac mini:    https://support.apple.com/en-us/HT201894
    // iMac:        https://support.apple.com/en-us/HT201634
    // Mac Pro:     https://support.apple.com/en-us/HT202888
    // Mac Studio:  https://support.apple.com/en-us/HT213073

    if(ffStrbufStartsWithS(hwModel, "MacBookPro"))
    {
        const char* version = hwModel->chars + strlen("MacBookPro");
        if(ffStrEquals(version, "18,3") ||
           ffStrEquals(version, "18,4"))        return "MacBook Pro (14-inch, 2021)";
        if(ffStrEquals(version, "18,1") ||
           ffStrEquals(version, "18,2"))        return "MacBook Pro (16-inch, 2021)";
        if(ffStrEquals(version, "17,1"))        return "MacBook Pro (13-inch, M1, 2020)";
        if(ffStrEquals(version, "16,3"))        return "MacBook Pro (13-inch, 2020, Two Thunderbolt 3 ports)";
        if(ffStrEquals(version, "16,2"))        return "MacBook Pro (13-inch, 2020, Four Thunderbolt 3 ports)";
        if(ffStrEquals(version, "16,4") ||
           ffStrEquals(version, "16,1"))        return "MacBook Pro (16-inch, 2019)";
        if(ffStrEquals(version, "15,4"))        return "MacBook Pro (13-inch, 2019, Two Thunderbolt 3 ports)";
        if(ffStrEquals(version, "15,3"))        return "MacBook Pro (15-inch, 2019)";
        if(ffStrEquals(version, "15,2"))        return "MacBook Pro (13-inch, 2018/2019, Four Thunderbolt 3 ports)";
        if(ffStrEquals(version, "15,1"))        return "MacBook Pro (15-inch, 2018/2019)";
        if(ffStrEquals(version, "14,3"))        return "MacBook Pro (15-inch, 2017)";
        if(ffStrEquals(version, "14,2"))        return "MacBook Pro (13-inch, 2017, Four Thunderbolt 3 ports)";
        if(ffStrEquals(version, "14,1"))        return "MacBook Pro (13-inch, 2017, Two Thunderbolt 3 ports)";
        if(ffStrEquals(version, "13,3"))        return "MacBook Pro (15-inch, 2016)";
        if(ffStrEquals(version, "13,2"))        return "MacBook Pro (13-inch, 2016, Four Thunderbolt 3 ports)";
        if(ffStrEquals(version, "13,1"))        return "MacBook Pro (13-inch, 2016, Two Thunderbolt 3 ports)";
        if(ffStrEquals(version, "12,1"))        return "MacBook Pro (Retina, 13-inch, Early 2015)";
        if(ffStrEquals(version, "11,4") ||
           ffStrEquals(version, "11,5"))        return "MacBook Pro (Retina, 15-inch, Mid 2015)";
        if(ffStrEquals(version, "11,2") ||
           ffStrEquals(version, "11,3"))        return "MacBook Pro (Retina, 15-inch, Late 2013/Mid 2014)";
        if(ffStrEquals(version, "11,1"))        return "MacBook Pro (Retina, 13-inch, Late 2013/Mid 2014)";
        if(ffStrEquals(version, "10,2"))        return "MacBook Pro (Retina, 13-inch, Late 2012/Early 2013)";
        if(ffStrEquals(version, "10,1"))        return "MacBook Pro (Retina, 15-inch, Mid 2012/Early 2013)";
        if(ffStrEquals(version, "9,2"))         return "MacBook Pro (13-inch, Mid 2012)";
        if(ffStrEquals(version, "9,1"))         return "MacBook Pro (15-inch, Mid 2012)";
        if(ffStrEquals(version, "8,3"))         return "MacBook Pro (17-inch, 2011)";
        if(ffStrEquals(version, "8,2"))         return "MacBook Pro (15-inch, 2011)";
        if(ffStrEquals(version, "8,1"))         return "MacBook Pro (13-inch, 2011)";
        if(ffStrEquals(version, "7,1"))         return "MacBook Pro (13-inch, Mid 2010)";
        if(ffStrEquals(version, "6,2"))         return "MacBook Pro (15-inch, Mid 2010)";
        if(ffStrEquals(version, "6,1"))         return "MacBook Pro (17-inch, Mid 2010)";
        if(ffStrEquals(version, "5,5"))         return "MacBook Pro (13-inch, Mid 2009)";
        if(ffStrEquals(version, "5,3"))         return "MacBook Pro (15-inch, Mid 2009)";
        if(ffStrEquals(version, "5,2"))         return "MacBook Pro (17-inch, Mid/Early 2009)";
        if(ffStrEquals(version, "5,1"))         return "MacBook Pro (15-inch, Late 2008)";
        if(ffStrEquals(version, "4,1"))         return "MacBook Pro (17/15-inch, Early 2008)";
    }
    else if(ffStrbufStartsWithS(hwModel, "MacBookAir"))
    {
        const char* version = hwModel->chars + strlen("MacBookAir");
        if(ffStrEquals(version, "10,1"))        return "MacBook Air (M1, 2020)";
        if(ffStrEquals(version, "9,1"))         return "MacBook Air (Retina, 13-inch, 2020)";
        if(ffStrEquals(version, "8,2"))         return "MacBook Air (Retina, 13-inch, 2019)";
        if(ffStrEquals(version, "8,1"))         return "MacBook Air (Retina, 13-inch, 2018)";
        if(ffStrEquals(version, "7,2"))         return "MacBook Air (13-inch, Early 2015/2017)";
        if(ffStrEquals(version, "7,1"))         return "MacBook Air (11-inch, Early 2015)";
        if(ffStrEquals(version, "6,2"))         return "MacBook Air (13-inch, Mid 2013/Early 2014)";
        if(ffStrEquals(version, "6,1"))         return "MacBook Air (11-inch, Mid 2013/Early 2014)";
        if(ffStrEquals(version, "5,2"))         return "MacBook Air (13-inch, Mid 2012)";
        if(ffStrEquals(version, "5,1"))         return "MacBook Air (11-inch, Mid 2012)";
        if(ffStrEquals(version, "4,2"))         return "MacBook Air (13-inch, Mid 2011)";
        if(ffStrEquals(version, "4,1"))         return "MacBook Air (11-inch, Mid 2011)";
        if(ffStrEquals(version, "3,2"))         return "MacBook Air (13-inch, Late 2010)";
        if(ffStrEquals(version, "3,1"))         return "MacBook Air (11-inch, Late 2010)";
        if(ffStrEquals(version, "2,1"))         return "MacBook Air (Mid 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "Macmini"))
    {
        const char* version = hwModel->chars + strlen("Macmini");
        if(ffStrEquals(version, "9,1"))         return "Mac mini (M1, 2020)";
        if(ffStrEquals(version, "8,1"))         return "Mac mini (2018)";
        if(ffStrEquals(version, "7,1"))         return "Mac mini (Mid 2014)";
        if(ffStrEquals(version, "6,1") ||
           ffStrEquals(version, "6,2"))         return "Mac mini (Late 2012)";
        if(ffStrEquals(version, "5,1") ||
           ffStrEquals(version, "5,2"))         return "Mac mini (Mid 2011)";
        if(ffStrEquals(version, "4,1"))         return "Mac mini (Mid 2010)";
        if(ffStrEquals(version, "3,1"))         return "Mac mini (Early/Late 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "MacBook"))
    {
        const char* version = hwModel->chars + strlen("MacBook");
        if(ffStrEquals(version, "10,1"))        return "MacBook (Retina, 12-inch, 2017)";
        if(ffStrEquals(version, "9,1"))         return "MacBook (Retina, 12-inch, Early 2016)";
        if(ffStrEquals(version, "8,1"))         return "MacBook (Retina, 12-inch, Early 2015)";
        if(ffStrEquals(version, "7,1"))         return "MacBook (13-inch, Mid 2010)";
        if(ffStrEquals(version, "6,1"))         return "MacBook (13-inch, Late 2009)";
        if(ffStrEquals(version, "5,2"))         return "MacBook (13-inch, Early/Mid 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "MacPro"))
    {
        const char* version = hwModel->chars + strlen("MacPro");
        if(ffStrEquals(version, "7,1"))         return "Mac Pro (2019)";
        if(ffStrEquals(version, "6,1"))         return "Mac Pro (Late 2013)";
        if(ffStrEquals(version, "5,1"))         return "Mac Pro (Mid 2010 - Mid 2012)";
        if(ffStrEquals(version, "4,1"))         return "Mac Pro (Early 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "Mac"))
    {
        const char* version = hwModel->chars + strlen("Mac");
        if(ffStrEquals(version, "15,13"))       return "MacBook Air (15-inch, M3, 2024)";
        if(ffStrEquals(version, "15,2"))        return "MacBook Air (13-inch, M3, 2024)";
        if(ffStrEquals(version, "15,3"))        return "MacBook Pro (14-inch, Nov 2023, Two Thunderbolt / USB 4 ports)";
        if(ffStrEquals(version, "15,4"))        return "iMac (24-inch, 2023, Two Thunderbolt / USB 4 ports)";
        if(ffStrEquals(version, "15,5"))        return "iMac (24-inch, 2023, Two Thunderbolt / USB 4 ports, Two USB 3 ports)";
        if(ffStrEquals(version, "15,6") ||
           ffStrEquals(version, "15,8") ||
           ffStrEquals(version, "15,10"))       return "MacBook Pro (14-inch, Nov 2023, Three Thunderbolt 4 ports)";
        if(ffStrEquals(version, "15,7") ||
           ffStrEquals(version, "15,9") ||
           ffStrEquals(version, "15,11"))       return "MacBook Pro (16-inch, Nov 2023, Three Thunderbolt 4 ports)";
        if(ffStrEquals(version, "14,15"))       return "MacBook Air (15-inch, M2, 2023)";
        if(ffStrEquals(version, "14,14"))       return "Mac Studio (M2 Ultra, 2023, Two Thunderbolt 4 front ports)";
        if(ffStrEquals(version, "14,13"))       return "Mac Studio (M2 Max, 2023, Two USB-C front ports)";
        if(ffStrEquals(version, "14,8"))        return "Mac Pro (2023)";
        if(ffStrEquals(version, "14,6") ||
           ffStrEquals(version, "14,10"))       return "MacBook Pro (16-inch, 2023)";
        if(ffStrEquals(version, "14,5") ||
           ffStrEquals(version, "14,9"))        return "MacBook Pro (14-inch, 2023)";
        if(ffStrEquals(version, "14,3"))        return "Mac mini (M2, 2023, Two Thunderbolt 4 ports)";
        if(ffStrEquals(version, "14,12"))       return "Mac mini (M2, 2023, Four Thunderbolt 4 ports)";
        if(ffStrEquals(version, "14,7"))        return "MacBook Pro (13-inch, M2, 2022)";
        if(ffStrEquals(version, "14,2"))        return "MacBook Air (M2, 2022)";
        if(ffStrEquals(version, "13,1"))        return "Mac Studio (M1 Max, 2022, Two USB-C front ports)";
        if(ffStrEquals(version, "13,2"))        return "Mac Studio (M1 Ultra, 2022, Two Thunderbolt 4 front ports)";
    }
    else if(ffStrbufStartsWithS(hwModel, "iMac"))
    {
        const char* version = hwModel->chars + strlen("iMac");
        if(ffStrEquals(version, "21,1"))        return "iMac (24-inch, M1, 2021, Two Thunderbolt / USB 4 ports, Two USB 3 ports)";
        if(ffStrEquals(version, "21,2"))        return "iMac (24-inch, M1, 2021, Two Thunderbolt / USB 4 ports)";
        if(ffStrEquals(version, "20,1") ||
           ffStrEquals(version, "20,2"))        return "iMac (Retina 5K, 27-inch, 2020)";
        if(ffStrEquals(version, "19,1"))        return "iMac (Retina 5K, 27-inch, 2019)";
        if(ffStrEquals(version, "19,2"))        return "iMac (Retina 4K, 21.5-inch, 2019)";
        if(ffStrEquals(version, "Pro1,1"))      return "iMac Pro (2017)";
        if(ffStrEquals(version, "18,3"))        return "iMac (Retina 5K, 27-inch, 2017)";
        if(ffStrEquals(version, "18,2"))        return "iMac (Retina 4K, 21.5-inch, 2017)";
        if(ffStrEquals(version, "18,1"))        return "iMac (21.5-inch, 2017)";
        if(ffStrEquals(version, "17,1"))        return "iMac (Retina 5K, 27-inch, Late 2015)";
        if(ffStrEquals(version, "16,2"))        return "iMac (Retina 4K, 21.5-inch, Late 2015)";
        if(ffStrEquals(version, "16,1"))        return "iMac (21.5-inch, Late 2015)";
        if(ffStrEquals(version, "15,1"))        return "iMac (Retina 5K, 27-inch, Late 2014 - Mid 2015)";
        if(ffStrEquals(version, "14,4"))        return "iMac (21.5-inch, Mid 2014)";
        if(ffStrEquals(version, "14,2"))        return "iMac (27-inch, Late 2013)";
        if(ffStrEquals(version, "14,1"))        return "iMac (21.5-inch, Late 2013)";
        if(ffStrEquals(version, "13,2"))        return "iMac (27-inch, Late 2012)";
        if(ffStrEquals(version, "13,1"))        return "iMac (21.5-inch, Late 2012)";
        if(ffStrEquals(version, "12,2"))        return "iMac (27-inch, Mid 2011)";
        if(ffStrEquals(version, "12,1"))        return "iMac (21.5-inch, Mid 2011)";
        if(ffStrEquals(version, "11,3"))        return "iMac (27-inch, Mid 2010)";
        if(ffStrEquals(version, "11,2"))        return "iMac (21.5-inch, Mid 2010)";
        if(ffStrEquals(version, "10,1"))        return "iMac (27/21.5-inch, Late 2009)";
        if(ffStrEquals(version, "9,1"))         return "iMac (24/20-inch, Early 2009)";
    }
    return NULL;
}

const char* getProductNameWithIokit(FFstrbuf* result)
{
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t registryEntry = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceNameMatching("product"));
    if (!registryEntry)
        return "IOServiceGetMatchingService() failed";

    FF_CFTYPE_AUTO_RELEASE CFStringRef productName = IORegistryEntryCreateCFProperty(registryEntry, CFSTR("product-name"), kCFAllocatorDefault, kNilOptions);
    if (!productName)
        return "IORegistryEntryCreateCFProperty() failed";

    return ffCfStrGetString(productName, result);
}

const char* getOthersByIokit(FFHostResult* host)
{
    FF_IOOBJECT_AUTO_RELEASE io_registry_entry_t registryEntry = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceMatching("IOPlatformExpertDevice"));
    if (!registryEntry)
        return "IOServiceGetMatchingService() failed";

    FF_CFTYPE_AUTO_RELEASE CFStringRef serialNumber = IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, kNilOptions);
    if (serialNumber)
        ffCfStrGetString(serialNumber, &host->serial);

    FF_CFTYPE_AUTO_RELEASE CFStringRef uuid = IORegistryEntryCreateCFProperty(registryEntry, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, kNilOptions);
    if (uuid)
        ffCfStrGetString(uuid, &host->uuid);

    FF_CFTYPE_AUTO_RELEASE CFStringRef manufacturer = IORegistryEntryCreateCFProperty(registryEntry, CFSTR("manufacturer"), kCFAllocatorDefault, kNilOptions);
    if (manufacturer)
        ffCfStrGetString(manufacturer, &host->vendor);

    return NULL;
}

const char* ffDetectHost(FFHostResult* host)
{
    const char* error = ffSysctlGetString("hw.model", &host->family);
    if (error) return error;

    ffStrbufSetStatic(&host->name, getProductNameWithHwModel(&host->family));
    if (host->name.length == 0)
        getProductNameWithIokit(&host->name);
    if (host->name.length == 0)
        ffStrbufSet(&host->name, &host->family);
    getOthersByIokit(host);
    return NULL;
}
