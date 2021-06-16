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

static bool getHostValueSpecific(FFstrbuf* basePath, const char* key, FFstrbuf* buffer)
{
    uint32_t basePathLength = basePath->length;

    ffStrbufAppendS(basePath, "product_");
    ffStrbufAppendS(basePath, key);
    ffGetFileContent(basePath->chars, buffer);
    ffStrbufSubstrBefore(basePath, basePathLength);

    return hostValueSet(buffer);
}

static bool getHostValue(FFstrbuf* devicesPath, FFstrbuf* classPath, const char* key, FFstrbuf* buffer)
{
    if(getHostValueSpecific(devicesPath, key, buffer))
        return true;

    if(getHostValueSpecific(classPath, key, buffer))
        return true;

    return false;
}

static void destroyStrbufs(FFstrbuf* devicesPath, FFstrbuf* classPath, FFstrbuf* family, FFstrbuf* name, FFstrbuf* version)
{
    ffStrbufDestroy(devicesPath);
    ffStrbufDestroy(classPath);
    ffStrbufDestroy(family);
    ffStrbufDestroy(name);
    ffStrbufDestroy(version);
}

void ffPrintHost(FFinstance* instance)
{
    if(ffPrintFromCache(instance, FF_HOST_MODULE_NAME, &instance->config.hostKey, &instance->config.hostFormat, FF_HOST_NUM_FORMAT_ARGS))
        return;

    FFstrbuf devicesPath;
    ffStrbufInitA(&devicesPath, 128);
    ffStrbufAppendS(&devicesPath, "/sys/devices/virtual/dmi/id/");

    FFstrbuf classPath;
    ffStrbufInitA(&classPath, 128);
    ffStrbufAppendS(&classPath, "/sys/class/dmi/id/");

    FFstrbuf family;
    ffStrbufInit(&family);
    bool familySet = getHostValue(&devicesPath, &classPath, "family", &family);

    FFstrbuf name;
    ffStrbufInit(&name);
    bool nameSet = getHostValue(&devicesPath, &classPath, "name", &name);

    if(name.length == 0)
    {
        ffGetFileContent("/sys/firmware/devicetree/base/model", &name);
        nameSet = hostValueSet(&name);
    }

    if(name.length == 0)
    {
        ffGetFileContent("/tmp/sysinfo/model", &name);
        nameSet = hostValueSet(&name);
    }

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
    bool versionSet = getHostValue(&devicesPath, &classPath, "version", &version);

    if(!familySet && !nameSet)
    {
        destroyStrbufs(&devicesPath, &classPath, &family, &name, &version);
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

    destroyStrbufs(&devicesPath, &classPath, &family, &name, &version);
}
