#include "host.h"
#include "common/sysctl.h"

static inline bool strEqual(const char* a, const char* b)
{
    return strcmp(a, b) == 0;
}

static const char* getProductName(const FFstrbuf* hwModel)
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
        if(strEqual(version, "18,3") ||
           strEqual(version, "18,4"))        return "MacBook Pro (14-inch, 2021)";
        if(strEqual(version, "18,1") ||
           strEqual(version, "18,2"))        return "MacBook Pro (16-inch, 2021)";
        if(strEqual(version, "17,1"))        return "MacBook Pro (13-inch, M1, 2020)";
        if(strEqual(version, "16,4"))        return "MacBook Pro (16-inch, 2019)";
        if(strEqual(version, "16,3"))        return "MacBook Pro (13-inch, 2020, Two Thunderbolt 3 ports)";
        if(strEqual(version, "16,2"))        return "MacBook Pro (13-inch, 2020, Four Thunderbolt 3 ports)";
        if(strEqual(version, "16,1"))        return "MacBook Pro (16-inch, 2019)";
        if(strEqual(version, "15,4"))        return "MacBook Pro (13-inch, 2019, Two Thunderbolt 3 ports)";
        if(strEqual(version, "15,3"))        return "MacBook Pro (15-inch, 2019)";
        if(strEqual(version, "15,2"))        return "MacBook Pro (13-inch, 2018/2019, Four Thunderbolt 3 ports)";
        if(strEqual(version, "15,1"))        return "MacBook Pro (15-inch, 2018/2019)";
        if(strEqual(version, "14,3"))        return "MacBook Pro (15-inch, 2017)";
        if(strEqual(version, "14,2"))        return "MacBook Pro (13-inch, 2017, Four Thunderbolt 3 ports)";
        if(strEqual(version, "14,1"))        return "MacBook Pro (13-inch, 2017, Two Thunderbolt 3 ports)";
        if(strEqual(version, "13,3"))        return "MacBook Pro (15-inch, 2016)";
        if(strEqual(version, "13,2"))        return "MacBook Pro (13-inch, 2016, Four Thunderbolt 3 ports)";
        if(strEqual(version, "13,1"))        return "MacBook Pro (13-inch, 2016, Two Thunderbolt 3 ports)";
        if(strEqual(version, "12,1"))        return "MacBook Pro (Retina, 13-inch, Early 2015)";
        if(strEqual(version, "11,4") ||
           strEqual(version, "11,5"))        return "MacBook Pro (Retina, 15-inch, Mid 2015)";
        if(strEqual(version, "11,2") ||
           strEqual(version, "11,3"))        return "MacBook Pro (Retina, 15-inch, Late 2013/Mid 2014)";
        if(strEqual(version, "11,1"))        return "MacBook Pro (Retina, 13-inch, Late 2013/Mid 2014)";
        if(strEqual(version, "10,2"))        return "MacBook Pro (Retina, 13-inch, Late 2012/Early 2013)";
        if(strEqual(version, "10,1"))        return "MacBook Pro (Retina, 15-inch, Mid 2012/Early 2013)";
        if(strEqual(version, "9,2"))         return "MacBook Pro (13-inch, Mid 2012)";
        if(strEqual(version, "9,1"))         return "MacBook Pro (15-inch, Mid 2012)";
        if(strEqual(version, "8,3"))         return "MacBook Pro (17-inch, 2011)";
        if(strEqual(version, "8,2"))         return "MacBook Pro (15-inch, 2011)";
        if(strEqual(version, "8,1"))         return "MacBook Pro (13-inch, 2011)";
        if(strEqual(version, "7,1"))         return "MacBook Pro (13-inch, Mid 2010)";
        if(strEqual(version, "6,2"))         return "MacBook Pro (15-inch, Mid 2010)";
        if(strEqual(version, "6,1"))         return "MacBook Pro (17-inch, Mid 2010)";
        if(strEqual(version, "5,5"))         return "MacBook Pro (13-inch, Mid 2009)";
        if(strEqual(version, "5,3"))         return "MacBook Pro (15-inch, Mid 2009)";
        if(strEqual(version, "5,2"))         return "MacBook Pro (17-inch, Mid/Early 2009)";
        if(strEqual(version, "5,1"))         return "MacBook Pro (15-inch, Late 2008)";
        if(strEqual(version, "4,1"))         return "MacBook Pro (17/15-inch, Early 2008)";
    }
    else if(ffStrbufStartsWithS(hwModel, "MacBookAir"))
    {
        const char* version = hwModel->chars + strlen("MacBookAir");
        if(strEqual(version, "10,1"))        return "MacBook Air (M1, 2020)";
        if(strEqual(version, "9,1"))         return "MacBook Air (Retina, 13-inch, 2020)";
        if(strEqual(version, "8,2"))         return "MacBook Air (Retina, 13-inch, 2019)";
        if(strEqual(version, "8,1"))         return "MacBook Air (Retina, 13-inch, 2018)";
        if(strEqual(version, "7,2"))         return "MacBook Air (13-inch, Early 2015/2017)";
        if(strEqual(version, "7,1"))         return "MacBook Air (11-inch, Early 2015)";
        if(strEqual(version, "6,2"))         return "MacBook Air (13-inch, Mid 2013/Early 2014)";
        if(strEqual(version, "6,1"))         return "MacBook Air (11-inch, Mid 2013/Early 2014)";
        if(strEqual(version, "5,2"))         return "MacBook Air (13-inch, Mid 2012)";
        if(strEqual(version, "5,1"))         return "MacBook Air (11-inch, Mid 2012)";
        if(strEqual(version, "4,2"))         return "MacBook Air (13-inch, Mid 2011)";
        if(strEqual(version, "4,1"))         return "MacBook Air (11-inch, Mid 2011)";
        if(strEqual(version, "3,2"))         return "MacBook Air (13-inch, Late 2010)";
        if(strEqual(version, "3,1"))         return "MacBook Air (11-inch, Late 2010)";
        if(strEqual(version, "2,1"))         return "MacBook Air (Mid 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "Macmini"))
    {
        const char* version = hwModel->chars + strlen("Macmini");
        if(strEqual(version, "9,1"))         return "Mac mini (M1, 2020)";
        if(strEqual(version, "8,1"))         return "Mac mini (2018)";
        if(strEqual(version, "7,1"))         return "Mac mini (Mid 2014)";
        if(strEqual(version, "6,1") ||
           strEqual(version, "6,2"))         return "Mac mini (Late 2012)";
        if(strEqual(version, "5,1") ||
           strEqual(version, "5,2"))         return "Mac mini (Mid 2011)";
        if(strEqual(version, "4,1"))         return "Mac mini (Mid 2010)";
        if(strEqual(version, "3,1"))         return "Mac mini (Early/Late 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "MacBook"))
    {
        const char* version = hwModel->chars + strlen("MacBook");
        if(strEqual(version, "10,1"))        return "MacBook (Retina, 12-inch, 2017)";
        if(strEqual(version, "9,1"))         return "MacBook (Retina, 12-inch, Early 2016)";
        if(strEqual(version, "8,1"))         return "MacBook (Retina, 12-inch, Early 2015)";
        if(strEqual(version, "7,1"))         return "MacBook (13-inch, Mid 2010)";
        if(strEqual(version, "6,1"))         return "MacBook (13-inch, Late 2009)";
        if(strEqual(version, "5,2"))         return "MacBook (13-inch, Early/Mid 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "MacPro"))
    {
        const char* version = hwModel->chars + strlen("MacPro");
        if(strEqual(version, "7,1"))         return "Mac Pro (2019)";
        if(strEqual(version, "6,1"))         return "Mac Pro (Late 2013)";
        if(strEqual(version, "5,1"))         return "Mac Pro (Mid 2010 - Mid 2012)";
        if(strEqual(version, "4,1"))         return "Mac Pro (Early 2009)";
    }
    else if(ffStrbufStartsWithS(hwModel, "Mac"))
    {
        const char* version = hwModel->chars + strlen("Mac");
        if(strEqual(version, "14,6") ||
           strEqual(version, "14,10"))       return "MacBook Pro (16-inch, 2023)";
        if(strEqual(version, "14,5") ||
           strEqual(version, "14,9"))        return "MacBook Pro (14-inch, 2023)";
        if(strEqual(version, "14,3"))        return "Mac mini (M2, 2023, Two Thunderbolt 4 ports)";
        if(strEqual(version, "14,12"))       return "Mac mini (M2, 2023, Four Thunderbolt 4 ports)";
        if(strEqual(version, "14,7"))        return "MacBook Pro (13-inch, M2, 2022)";
        if(strEqual(version, "14,2"))        return "MacBook Air (M2, 2022)";
        if(strEqual(version, "13,1"))        return "Mac Studio (M1 Max, 2022, Two USB-C front ports)";
        if(strEqual(version, "13,2"))        return "Mac Studio (M1 Ultra, 2022, Two Thunderbolt 4 front ports)";
    }
    else if(ffStrbufStartsWithS(hwModel, "iMac"))
    {
        const char* version = hwModel->chars + strlen("iMac");
        if(strEqual(version, "21,1"))        return "iMac (24-inch, M1, 2021, Two Thunderbolt / USB 4 ports, Two USB 3 ports)";
        if(strEqual(version, "21,2"))        return "iMac (24-inch, M1, 2021, Two Thunderbolt / USB 4 ports)";
        if(strEqual(version, "20,1") ||
           strEqual(version, "20,2"))        return "iMac (Retina 5K, 27-inch, 2020)";
        if(strEqual(version, "19,1"))        return "iMac (Retina 5K, 27-inch, 2019)";
        if(strEqual(version, "19,2"))        return "iMac (Retina 4K, 21.5-inch, 2019)";
        if(strEqual(version, "Pro1,1"))      return "iMac Pro (2017)";
        if(strEqual(version, "18,3"))        return "iMac (Retina 5K, 27-inch, 2017)";
        if(strEqual(version, "18,2"))        return "iMac (Retina 4K, 21.5-inch, 2017)";
        if(strEqual(version, "18,1"))        return "iMac (21.5-inch, 2017)";
        if(strEqual(version, "17,1"))        return "iMac (Retina 5K, 27-inch, Late 2015)";
        if(strEqual(version, "16,2"))        return "iMac (Retina 4K, 21.5-inch, Late 2015)";
        if(strEqual(version, "16,1"))        return "iMac (21.5-inch, Late 2015)";
        if(strEqual(version, "15,1"))        return "iMac (Retina 5K, 27-inch, Late 2014 - Mid 2015)";
        if(strEqual(version, "14,4"))        return "iMac (21.5-inch, Mid 2014)";
        if(strEqual(version, "14,2"))        return "iMac (27-inch, Late 2013)";
        if(strEqual(version, "14,1"))        return "iMac (21.5-inch, Late 2013)";
        if(strEqual(version, "13,2"))        return "iMac (27-inch, Late 2012)";
        if(strEqual(version, "13,1"))        return "iMac (21.5-inch, Late 2012)";
        if(strEqual(version, "12,2"))        return "iMac (27-inch, Mid 2011)";
        if(strEqual(version, "12,1"))        return "iMac (21.5-inch, Mid 2011)";
        if(strEqual(version, "11,3"))        return "iMac (27-inch, Mid 2010)";
        if(strEqual(version, "11,2"))        return "iMac (21.5-inch, Mid 2010)";
        if(strEqual(version, "10,1"))        return "iMac (27/21.5-inch, Late 2009)";
        if(strEqual(version, "9,1"))         return "iMac (24/20-inch, Early 2009)";
    }
    return hwModel->chars;
}

void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);
    ffStrbufInitS(&host->sysVendor, "Apple");

    ffStrbufAppendS(&host->error, ffSysctlGetString("hw.model", &host->productFamily));
    if(host->error.length == 0)
        ffStrbufAppendS(&host->productName, getProductName(&host->productFamily));
}
