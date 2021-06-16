#include "fastfetch.h"

#define FF_HOST_MODULE_NAME "Host"
#define FF_HOST_NUM_FORMAT_ARGS 3

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

static inline void getHostValue(const char* devicesPath, const char* classPath, FFstrbuf* buffer)
{
    ffGetFileContent(devicesPath, buffer);

    if(buffer->length == 0)
        ffGetFileContent(classPath, buffer);
}

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_HOST_MODULE_NAME, &instance->config.hostKey, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS))
        return;

    FFstrbuf family;
    ffStrbufInit(&family);
    getHostValue("/sys/devices/virtual/dmi/id/product_family", "/sys/class/dmi/id/product_family", &family);
    bool familySet = hostValueSet(&family);

    FFstrbuf name;
    ffStrbufInit(&name);
    getHostValue("/sys/devices/virtual/dmi/id/product_name", "/sys/class/dmi/id/product_name", &name);

    if(name.length == 0)
        ffGetFileContent("/sys/firmware/devicetree/base/model", &name);

    if(name.length == 0)
        ffGetFileContent("/tmp/sysinfo/model", &name);

    bool nameSet = hostValueSet(&name);

    if(ffStrbufStartsWithS(&name, "Standard PC"))
    {
        FFstrbuf copy;
        ffStrbufInitCopy(&copy, &name);
        ffStrbufSetS(&name, "KVM/QEMU ");
        ffStrbufAppend(&name, &copy);
        ffStrbufDestroy(&copy);
    }

    FFstrbuf version;
    ffStrbufInit(&version);
    getHostValue("/sys/devices/virtual/dmi/id/product_version", "/sys/class/dmi/id/product_version", &version);
    bool versionSet = hostValueSet(&version);

    if(!familySet && !nameSet)
    {
        ffStrbufDestroy(&family);
        ffStrbufDestroy(&name);
        ffStrbufDestroy(&version);
        ffPrintError(instance, FF_HOST_MODULE_NAME, 0, &instance->config.hostKey, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS, "neither family nor name is set by O.E.M.");
        return;
    }

    FFstrbuf host;
    ffStrbufInit(&host);

    if(nameSet)
        ffStrbufAppend(&host, &name);
    else
        ffStrbufAppend(&host, &family);

    if(versionSet)
    {
        ffStrbufAppendC(&host, ' ');
        ffStrbufAppend(&host, &version);
    }

    ffPrintAndSaveToCache(instance, FF_HOST_MODULE_NAME, &instance->config.hostKey, &host, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS, (FFformatarg[]) {
        {FF_FORMAT_ARG_TYPE_STRBUF, &family},
        {FF_FORMAT_ARG_TYPE_STRBUF, &name},
        {FF_FORMAT_ARG_TYPE_STRBUF, &version}
    });

    ffStrbufDestroy(&family);
    ffStrbufDestroy(&name);
    ffStrbufDestroy(&version);
}
