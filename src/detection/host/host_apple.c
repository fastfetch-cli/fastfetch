#include "host.h"
#include "common/sysctl.h"

static const char* getProductName(const FFstrbuf* hwModel)
{
    //https://github.com/hykilpikonna/hyfetch/blob/master/neofetch#L1386
    if(ffStrbufCompS(hwModel, "Mac14,7") == 0)               return "MacBook Pro (13-inch, M2, 2022)";
    if(ffStrbufCompS(hwModel, "MacBookPro18,3") == 0 ||
       ffStrbufCompS(hwModel, "MacBookPro18,4") == 0)        return "MacBook Pro (14-inch, 2021)";
    if(ffStrbufCompS(hwModel, "MacBookPro18,1") == 0 ||
       ffStrbufCompS(hwModel, "MacBookPro18,2") == 0)        return "MacBook Pro (16-inch, 2021)";
    if(ffStrbufCompS(hwModel, "MacBookPro17,1") == 0)        return "MacBook Pro (13-inch, M1, 2020)";
    if(ffStrbufCompS(hwModel, "MacBookPro16,4") == 0)        return "MacBook Pro (16-inch, 2019)";
    if(ffStrbufCompS(hwModel, "MacBookPro16,3") == 0)        return "MacBook Pro (13-inch, 2020, Two Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro16,2") == 0)        return "MacBook Pro (13-inch, 2020, Four Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro16,1") == 0)        return "MacBook Pro (16-inch, 2019)";
    if(ffStrbufCompS(hwModel, "MacBookPro15,4") == 0)        return "MacBook Pro (13-inch, 2019, Two Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro15,3") == 0)        return "MacBook Pro (15-inch, 2019)";
    if(ffStrbufCompS(hwModel, "MacBookPro15,2") == 0)        return "MacBook Pro (13-inch, 2018/2019, Four Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro15,1") == 0)        return "MacBook Pro (15-inch, 2018/2019)";
    if(ffStrbufCompS(hwModel, "MacBookPro14,3") == 0)        return "MacBook Pro (15-inch, 2017)";
    if(ffStrbufCompS(hwModel, "MacBookPro14,2") == 0)        return "MacBook Pro (13-inch, 2017, Four Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro14,1") == 0)        return "MacBook Pro (13-inch, 2017, Two Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro13,3") == 0)        return "MacBook Pro (15-inch, 2016)";
    if(ffStrbufCompS(hwModel, "MacBookPro13,2") == 0)        return "MacBook Pro (13-inch, 2016, Four Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro13,1") == 0)        return "MacBook Pro (13-inch, 2016, Two Thunderbolt 3 ports)";
    if(ffStrbufCompS(hwModel, "MacBookPro12,1") == 0)        return "MacBook Pro (Retina, 13-inch, Early 2015)";
    if(ffStrbufCompS(hwModel, "MacBookPro11,4") == 0 ||
       ffStrbufCompS(hwModel, "MacBookPro11,5") == 0)        return "MacBook Pro (Retina, 15-inch, Mid 2015)";
    if(ffStrbufCompS(hwModel, "MacBookPro11,2") == 0 ||
       ffStrbufCompS(hwModel, "MacBookPro11,3") == 0)        return "MacBook Pro (Retina, 15-inch, Late 2013/Mid 2014)";
    if(ffStrbufCompS(hwModel, "MacBookPro11,1") == 0)        return "MacBook Pro (Retina, 13-inch, Late 2013/Mid 2014)";
    if(ffStrbufCompS(hwModel, "MacBookPro10,2") == 0)        return "MacBook Pro (Retina, 13-inch, Late 2012/Early 2013)";
    if(ffStrbufCompS(hwModel, "MacBookPro10,1") == 0)        return "MacBook Pro (Retina, 15-inch, Mid 2012/Early 2013)";
    if(ffStrbufCompS(hwModel, "MacBookPro9,2") == 0)         return "MacBook Pro (13-inch, Mid 2012)";
    if(ffStrbufCompS(hwModel, "MacBookPro9,1") == 0)         return "MacBook Pro (15-inch, Mid 2012)";
    if(ffStrbufCompS(hwModel, "MacBookPro8,3") == 0)         return "MacBook Pro (17-inch, 2011)";
    if(ffStrbufCompS(hwModel, "MacBookPro8,2") == 0)         return "MacBook Pro (15-inch, 2011)";
    if(ffStrbufCompS(hwModel, "MacBookPro8,1") == 0)         return "MacBook Pro (13-inch, 2011)";
    if(ffStrbufCompS(hwModel, "MacBookPro7,1") == 0)         return "MacBook Pro (13-inch, Mid 2010)";
    if(ffStrbufCompS(hwModel, "MacBookPro6,2") == 0)         return "MacBook Pro (15-inch, Mid 2010)";
    if(ffStrbufCompS(hwModel, "MacBookPro6,1") == 0)         return "MacBook Pro (17-inch, Mid 2010)";
    if(ffStrbufCompS(hwModel, "MacBookPro5,5") == 0)         return "MacBook Pro (13-inch, Mid 2009)";
    if(ffStrbufCompS(hwModel, "MacBookPro5,3") == 0)         return "MacBook Pro (15-inch, Mid 2009)";
    if(ffStrbufCompS(hwModel, "MacBookPro5,2") == 0)         return "MacBook Pro (17-inch, Mid/Early 2009)";
    if(ffStrbufCompS(hwModel, "MacBookPro5,1") == 0)         return "MacBook Pro (15-inch, Late 2008)";
    if(ffStrbufCompS(hwModel, "MacBookPro4,1") == 0)         return "MacBook Pro (17/15-inch, Early 2008)";
    if(ffStrbufCompS(hwModel, "Mac14,2") == 0)               return "MacBook Air (M2, 2022)";
    if(ffStrbufCompS(hwModel, "MacBookAir10,1") == 0)        return "MacBook Air (M1, 2020)";
    if(ffStrbufCompS(hwModel, "MacBookAir9,1") == 0)         return "MacBook Air (Retina, 13-inch, 2020)";
    if(ffStrbufCompS(hwModel, "MacBookAir8,2") == 0)         return "MacBook Air (Retina, 13-inch, 2019)";
    if(ffStrbufCompS(hwModel, "MacBookAir8,1") == 0)         return "MacBook Air (Retina, 13-inch, 2018)";
    if(ffStrbufCompS(hwModel, "MacBookAir7,2") == 0)         return "MacBook Air (13-inch, Early 2015/2017)";
    if(ffStrbufCompS(hwModel, "MacBookAir7,1") == 0)         return "MacBook Air (11-inch, Early 2015)";
    if(ffStrbufCompS(hwModel, "MacBookAir6,2") == 0)         return "MacBook Air (13-inch, Mid 2013/Early 2014)";
    if(ffStrbufCompS(hwModel, "MacBookAir6,1") == 0)         return "MacBook Air (11-inch, Mid 2013/Early 2014)";
    if(ffStrbufCompS(hwModel, "MacBookAir5,2") == 0)         return "MacBook Air (13-inch, Mid 2012)";
    if(ffStrbufCompS(hwModel, "MacBookAir5,1") == 0)         return "MacBook Air (11-inch, Mid 2012)";
    if(ffStrbufCompS(hwModel, "MacBookAir4,2") == 0)         return "MacBook Air (13-inch, Mid 2011)";
    if(ffStrbufCompS(hwModel, "MacBookAir4,1") == 0)         return "MacBook Air (11-inch, Mid 2011)";
    if(ffStrbufCompS(hwModel, "MacBookAir3,2") == 0)         return "MacBook Air (13-inch, Late 2010)";
    if(ffStrbufCompS(hwModel, "MacBookAir3,1") == 0)         return "MacBook Air (11-inch, Late 2010)";
    if(ffStrbufCompS(hwModel, "MacBookAir2,1") == 0)         return "MacBook Air (Mid 2009)";
    if(ffStrbufCompS(hwModel, "MacBook10,1") == 0)           return "MacBook (Retina, 12-inch, 2017)";
    if(ffStrbufCompS(hwModel, "MacBook9,1") == 0)            return "MacBook (Retina, 12-inch, Early 2016)";
    if(ffStrbufCompS(hwModel, "MacBook8,1") == 0)            return "MacBook (Retina, 12-inch, Early 2015)";
    if(ffStrbufCompS(hwModel, "MacBook7,1") == 0)            return "MacBook (13-inch, Mid 2010)";
    if(ffStrbufCompS(hwModel, "MacBook6,1") == 0)            return "MacBook (13-inch, Late 2009)";
    if(ffStrbufCompS(hwModel, "MacBook5,2") == 0)            return "MacBook (13-inch, Early/Mid 2009)";
    if(ffStrbufCompS(hwModel, "Mac13,1") == 0)               return "Mac Studio (2022, Two USB-C front ports)";
    if(ffStrbufCompS(hwModel, "Mac13,2") == 0)               return "Mac Studio (2022, Two Thunderbolt 4 front ports)";
    if(ffStrbufCompS(hwModel, "Macmini9,1") == 0)            return "Mac mini (M1, 2020)";
    if(ffStrbufCompS(hwModel, "Macmini8,1") == 0)            return "Mac mini (2018)";
    if(ffStrbufCompS(hwModel, "Macmini7,1") == 0)            return "Mac mini (Mid 2014)";
    if(ffStrbufCompS(hwModel, "Macmini6,1") == 0 ||
       ffStrbufCompS(hwModel, "Macmini6,2") == 0)            return "Mac mini (Late 2012)";
    if(ffStrbufCompS(hwModel, "Macmini5,1") == 0 ||
       ffStrbufCompS(hwModel, "Macmini5,2") == 0)            return "Mac mini (Mid 2011)";
    if(ffStrbufCompS(hwModel, "Macmini4,1") == 0)            return "Mac mini (Mid 2010)";
    if(ffStrbufCompS(hwModel, "Macmini3,1") == 0)            return "Mac mini (Early/Late 2009)";
    if(ffStrbufCompS(hwModel, "MacPro7,1") == 0)             return "Mac Pro (2019)";
    if(ffStrbufCompS(hwModel, "MacPro6,1") == 0)             return "Mac Pro (Late 2013)";
    if(ffStrbufCompS(hwModel, "MacPro5,1") == 0)             return "Mac Pro (Mid 2010 - Mid 2012)";
    if(ffStrbufCompS(hwModel, "MacPro4,1") == 0)             return "Mac Pro (Early 2009)";
    if(ffStrbufCompS(hwModel, "iMac21,1") == 0 ||
       ffStrbufCompS(hwModel, "iMac21,2") == 0)              return "iMac (24-inch, M1, 2021)";
    if(ffStrbufCompS(hwModel, "iMac20,1") == 0 ||
       ffStrbufCompS(hwModel, "iMac20,2") == 0)              return "iMac (Retina 5K, 27-inch, 2020)";
    if(ffStrbufCompS(hwModel, "iMac19,1") == 0 ||
       ffStrbufCompS(hwModel, "iMac19,2") == 0)              return "iMac (Retina 4K, 21.5-inch, 2019)";
    if(ffStrbufCompS(hwModel, "iMacPro1,1") == 0)            return "iMac Pro (2017)";
    if(ffStrbufCompS(hwModel, "iMac18,3") == 0)              return "iMac (Retina 5K, 27-inch, 2017)";
    if(ffStrbufCompS(hwModel, "iMac18,2") == 0)              return "iMac (Retina 4K, 21.5-inch, 2017)";
    if(ffStrbufCompS(hwModel, "iMac18,1") == 0)              return "iMac (21.5-inch, 2017)";
    if(ffStrbufCompS(hwModel, "iMac17,1") == 0)              return "iMac (Retina 5K, 27-inch, Late 2015)";
    if(ffStrbufCompS(hwModel, "iMac16,2") == 0)              return "iMac (Retina 4K, 21.5-inch, Late 2015)";
    if(ffStrbufCompS(hwModel, "iMac16,1") == 0)              return "iMac (21.5-inch, Late 2015)";
    if(ffStrbufCompS(hwModel, "iMac15,1") == 0)              return "iMac (Retina 5K, 27-inch, Late 2014 - Mid 2015)";
    if(ffStrbufCompS(hwModel, "iMac14,4") == 0)              return "iMac (21.5-inch, Mid 2014)";
    if(ffStrbufCompS(hwModel, "iMac14,2") == 0)              return "iMac (27-inch, Late 2013)";
    if(ffStrbufCompS(hwModel, "iMac14,1") == 0)              return "iMac (21.5-inch, Late 2013)";
    if(ffStrbufCompS(hwModel, "iMac13,2") == 0)              return "iMac (27-inch, Late 2012)";
    if(ffStrbufCompS(hwModel, "iMac13,1") == 0)              return "iMac (21.5-inch, Late 2012)";
    if(ffStrbufCompS(hwModel, "iMac12,2") == 0)              return "iMac (27-inch, Mid 2011)";
    if(ffStrbufCompS(hwModel, "iMac12,1") == 0)              return "iMac (21.5-inch, Mid 2011)";
    if(ffStrbufCompS(hwModel, "iMac11,3") == 0)              return "iMac (27-inch, Mid 2010)";
    if(ffStrbufCompS(hwModel, "iMac11,2") == 0)              return "iMac (21.5-inch, Mid 2010)";
    if(ffStrbufCompS(hwModel, "iMac10,1") == 0)              return "iMac (27/21.5-inch, Late 2009)";
    if(ffStrbufCompS(hwModel, "iMac9,1") == 0)               return "iMac (24/20-inch, Early 2009)";
    return hwModel->chars;
}

void ffDetectHostImpl(FFHostResult* host)
{
    ffStrbufInit(&host->error);

    ffStrbufInit(&host->productName);
    ffStrbufInit(&host->productFamily);
    ffStrbufInit(&host->productVersion);
    ffStrbufInit(&host->productSku);

    ffStrbufInitA(&host->sysVendor, 0);
    ffStrbufInitA(&host->chassisType, 0);
    ffStrbufInitA(&host->chassisVendor, 0);
    ffStrbufInitA(&host->chassisVersion, 0);

    ffSysctlGetString("hw.model", &host->productFamily);
    ffStrbufAppendS(&host->productName, getProductName(&host->productFamily));
}
