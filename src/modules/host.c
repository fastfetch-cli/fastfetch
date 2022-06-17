#include "fastfetch.h"

#include <ctype.h>

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 15

static bool hostValueSet(FFstrbuf* value)
{
    ffStrbufTrimRight(value, '\n');
    ffStrbufTrim(value, ' ');

    return
        value->length > 0 &&
        ffStrbufStartsWithIgnCaseS(value, "To be filled") != true &&
        ffStrbufStartsWithIgnCaseS(value, "To be set") != true &&
        ffStrbufStartsWithIgnCaseS(value, "OEM") != true &&
        ffStrbufStartsWithIgnCaseS(value, "O.E.M.") != true &&
        ffStrbufIgnCaseCompS(value, "None") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Product Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Name") != 0 &&
        ffStrbufIgnCaseCompS(value, "System Version") != 0 &&
        ffStrbufIgnCaseCompS(value, "Default string") != 0 &&
        ffStrbufIgnCaseCompS(value, "Undefined") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Specified") != 0 &&
        ffStrbufIgnCaseCompS(value, "Not Applicable") != 0 &&
        ffStrbufIgnCaseCompS(value, "INVALID") != 0 &&
        ffStrbufIgnCaseCompS(value, "Type1ProductConfigId") != 0 &&
        ffStrbufIgnCaseCompS(value, "All Series") != 0
    ;
}

#ifndef __ANDROID__
static void getHostValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffGetFileContent(devicesPath, buffer);

    if(buffer->length == 0)
        ffGetFileContent(classPath, buffer);
}
#endif

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_HOST_MODULE_NAME, &instance->config.host, FF_HOST_NUM_FORMAT_ARGS))
        return;

    FFstrbuf product_family;
    ffStrbufInit(&product_family);
    #ifndef __ANDROID__
        getHostValue("/sys/devices/virtual/dmi/id/product_family", "/sys/class/dmi/id/product_family", &product_family);
        if(!hostValueSet(&product_family))
            ffStrbufClear(&product_family);
    #else
        ffSettingsGetAndroidProperty("ro.product.device", &product_family);
    #endif

    FFstrbuf product_name;
    ffStrbufInit(&product_name);
    #ifndef __ANDROID__
        getHostValue("/sys/devices/virtual/dmi/id/product_name", "/sys/class/dmi/id/product_name", &product_name);

        if(product_name.length == 0)
            ffGetFileContent("/sys/firmware/devicetree/base/model", &product_name);

        if(product_name.length == 0)
            ffGetFileContent("/tmp/sysinfo/model", &product_name);

        if(ffStrbufStartsWithS(&product_name, "Standard PC"))
        {
            FFstrbuf copy;
            ffStrbufInitCopy(&copy, &product_name);
            ffStrbufSetS(&product_name, "KVM/QEMU ");
            ffStrbufAppend(&product_name, &copy);
            ffStrbufDestroy(&copy);
        }

        if(!hostValueSet(&product_name))
            ffStrbufClear(&product_name);

        //On WSL, the real host can't be detected. Instead use WSL as host.
        if(product_name.length == 0 && product_family.length == 0 && (
            getenv("WSLENV") != NULL ||
            getenv("WSL_DISTRO") != NULL ||
            getenv("WSL_INTEROP") != NULL
        )) ffStrbufAppendS(&product_name, "Windows Subsystem for Linux");
    #else
        ffSettingsGetAndroidProperty("ro.product.brand", &product_name);
        if(product_name.length > 0){
            product_name.chars[0] = (char) toupper(product_name.chars[0]);
            ffStrbufAppendC(&product_name, ' ');
        }

        if(!ffSettingsGetAndroidProperty("ro.product.model", &product_name))
            ffSettingsGetAndroidProperty("ro.product.name", &product_name);

        ffStrbufTrimRight(&product_name, ' ');
    #endif

    if(product_family.length == 0 && product_name.length == 0)
    {
        ffStrbufDestroy(&product_family);
        ffStrbufDestroy(&product_name);
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.host, "neither product_family nor product_name is set by O.E.M.");
        return;
    }

    #ifdef __ANDROID__
        #define FF_HOST_DATA(name) \
            FFstrbuf name; \
            ffStrbufInitA(&name, 0);
    #else
        #define FF_HOST_DATA(name) \
            FFstrbuf name; \
            ffStrbufInit(&name); \
            getHostValue("/sys/devices/virtual/dmi/id/"#name, "/sys/class/dmi/id/"#name, &name); \
            if(!hostValueSet(&name)) \
                ffStrbufClear(&name);
    #endif

    FF_HOST_DATA(product_version)
    FF_HOST_DATA(product_sku)
    FF_HOST_DATA(bios_date)
    FF_HOST_DATA(bios_release)
    FF_HOST_DATA(bios_vendor)
    FF_HOST_DATA(bios_version)
    FF_HOST_DATA(board_name)
    FF_HOST_DATA(board_vendor)
    FF_HOST_DATA(board_version)
    FF_HOST_DATA(chassis_type)
    FF_HOST_DATA(chassis_vendor)
    FF_HOST_DATA(chassis_version)
    FF_HOST_DATA(sys_vendor)

    FFstrbuf host;
    ffStrbufInit(&host);

    if(product_name.length > 0)
        ffStrbufAppend(&host, &product_name);
    else
        ffStrbufAppend(&host, &product_family);

    if(product_version.length > 0)
    {
        ffStrbufAppendC(&host, ' ');
        ffStrbufAppend(&host, &product_version);
    }

    ffPrintAndWriteToCache(instance, FF_HOST_MODULE_NAME, &instance->config.host, &host, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_STRBUF, &product_family},
        {FF_FORMAT_ARG_TYPE_STRBUF, &product_name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &product_version},
        {FF_FORMAT_ARG_TYPE_STRBUF, &product_sku},
        {FF_FORMAT_ARG_TYPE_STRBUF, &bios_date},
        {FF_FORMAT_ARG_TYPE_STRBUF, &bios_release},
        {FF_FORMAT_ARG_TYPE_STRBUF, &bios_vendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &bios_version},
        {FF_FORMAT_ARG_TYPE_STRBUF, &board_name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &board_vendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &board_version},
        {FF_FORMAT_ARG_TYPE_STRBUF, &chassis_type},
        {FF_FORMAT_ARG_TYPE_STRBUF, &chassis_vendor},
        {FF_FORMAT_ARG_TYPE_STRBUF, &chassis_version},
        {FF_FORMAT_ARG_TYPE_STRBUF, &sys_vendor}
    });

    ffStrbufDestroy(&product_family);
    ffStrbufDestroy(&product_name);

    //On Android, all of them get initialized without heap, so no need to destroy them.
    #ifndef __ANDROID__
        ffStrbufDestroy(&product_version);
        ffStrbufDestroy(&product_sku);
        ffStrbufDestroy(&bios_date);
        ffStrbufDestroy(&bios_release);
        ffStrbufDestroy(&bios_vendor);
        ffStrbufDestroy(&bios_version);
        ffStrbufDestroy(&board_name);
        ffStrbufDestroy(&board_vendor);
        ffStrbufDestroy(&board_version);
        ffStrbufDestroy(&chassis_type);
        ffStrbufDestroy(&chassis_vendor);
        ffStrbufDestroy(&chassis_version);
        ffStrbufDestroy(&sys_vendor);
    #endif
}
