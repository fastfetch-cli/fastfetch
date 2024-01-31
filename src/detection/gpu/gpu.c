#include "gpu.h"
#include "detection/vulkan/vulkan.h"
#include "detection/opengl/opengl.h"

const char* FF_GPU_VENDOR_NAME_APPLE = "Apple";
const char* FF_GPU_VENDOR_NAME_AMD = "AMD";
const char* FF_GPU_VENDOR_NAME_INTEL = "Intel";
const char* FF_GPU_VENDOR_NAME_NVIDIA = "NVIDIA";
const char* FF_GPU_VENDOR_NAME_QUALCOMM = "Qualcomm";
const char* FF_GPU_VENDOR_NAME_MTK = "MTK";
const char* FF_GPU_VENDOR_NAME_VMWARE = "VMware";
const char* FF_GPU_VENDOR_NAME_PARALLEL = "Parallel";
const char* FF_GPU_VENDOR_NAME_MICROSOFT = "Microsoft";
const char* FF_GPU_VENDOR_NAME_REDHAT = "RedHat";
const char* FF_GPU_VENDOR_NAME_ORACLE = "Oracle";

static inline bool arrayContains(const unsigned arr[], unsigned vendorId, unsigned length)
{
    for (unsigned i = 0; i < length; ++i)
    {
        if (arr[i] == vendorId)
            return true;
    }
    return false;
}

const char* ffGetGPUVendorString(unsigned vendorId)
{
    // https://devicehunt.com/all-pci-vendors
    if(vendorId == 0x106b)
        return FF_GPU_VENDOR_NAME_APPLE;
    if(arrayContains((const unsigned[]) {0x1002, 0x1022}, vendorId, 2))
        return FF_GPU_VENDOR_NAME_AMD;
    else if(arrayContains((const unsigned[]) {0x03e7, 0x8086, 0x8087}, vendorId, 3))
        return FF_GPU_VENDOR_NAME_INTEL;
    else if(arrayContains((const unsigned[]) {0x0955, 0x10de, 0x12d2}, vendorId, 3))
        return FF_GPU_VENDOR_NAME_NVIDIA;
    else if(arrayContains((const unsigned[]) {0x5143}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_QUALCOMM;
    else if(arrayContains((const unsigned[]) {0x14c3}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_MTK;
    else if(arrayContains((const unsigned[]) {0x15ad}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_VMWARE;
    else if(arrayContains((const unsigned[]) {0x1af4}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_REDHAT;
    else if(arrayContains((const unsigned[]) {0x1ab8}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_PARALLEL;
    else if(arrayContains((const unsigned[]) {0x1414}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_MICROSOFT;
    else if(arrayContains((const unsigned[]) {0x108e}, vendorId, 1))
        return FF_GPU_VENDOR_NAME_ORACLE;
    return NULL;
}

const char* detectByOpenGL(FFlist* gpus)
{
    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);
    const char* error = ffDetectOpenGL(&instance.config.modules.openGL, &result);
    if (!error)
    {
        FFGPUResult* gpu = (FFGPUResult*) ffListAdd(gpus);
        gpu->type = FF_GPU_TYPE_UNKNOWN;
        ffStrbufInitMove(&gpu->vendor, &result.vendor);
        ffStrbufInitMove(&gpu->name, &result.renderer);
        ffStrbufInit(&gpu->driver);
        ffStrbufInitF(&gpu->platformApi, "OpenGL %s", result.version.chars);
        gpu->temperature = FF_GPU_TEMP_UNSET;
        gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;
        gpu->frequency = FF_GPU_FREQUENCY_UNSET;
        gpu->dedicated = gpu->shared = (FFGPUMemory){0, 0};
        gpu->deviceId = 0;

        if (ffStrbufIgnCaseEqualS(&gpu->vendor, "Mesa"))
            ffStrbufClear(&gpu->vendor);

        if (!gpu->vendor.length)
        {
            if (ffStrbufContainS(&gpu->name, "Apple"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE);
            else if (ffStrbufContainS(&gpu->name, "Intel"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_INTEL);
            else if (ffStrbufContainS(&gpu->name, "AMD") || ffStrbufContainS(&gpu->name, "ATI"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_AMD);
            else if (ffStrbufContainS(&gpu->name, "NVIDIA"))
                ffStrbufSetStatic(&gpu->vendor, FF_GPU_VENDOR_NAME_NVIDIA);
        }
        if (ffStrbufEqualS(&gpu->vendor, FF_GPU_VENDOR_NAME_APPLE))
            gpu->type = FF_GPU_TYPE_INTEGRATED;
    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.renderer);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.slv);
    return error;
}

const char* ffDetectGPU(const FFGPUOptions* options, FFlist* result)
{
    if (!options->forceVulkan)
    {
        const char* error = ffDetectGPUImpl(options, result);
        if (!error && result->length > 0) return NULL;
    }
    FFVulkanResult* vulkan = ffDetectVulkan();
    if (!vulkan->error && vulkan->gpus.length > 0)
    {
        ffListDestroy(result);
        ffListInitMove(result, &vulkan->gpus);
        return NULL;
    }
    if (detectByOpenGL(result) == NULL)
        return NULL;

    return "GPU detection failed";
}

#ifndef _WIN32
void ffGPUParsePciIds(FFstrbuf* content, uint8_t subclass, uint16_t vendor, uint16_t device, FFGPUResult* gpu)
{
    if (content->length)
    {
        char buffer[32];
        uint32_t len = (uint32_t) snprintf(buffer, sizeof(buffer), "\n%04x  ", vendor);
        char* start = (char*) memmem(content->chars, content->length, buffer, len);
        char* end = content->chars + content->length;
        if (start)
        {
            start += len;
            end = memchr(start, '\n', (uint32_t) (end - start));
            if (!end)
                end = content->chars + content->length;
            if (!gpu->vendor.length)
                ffStrbufSetNS(&gpu->vendor, (uint32_t) (end - start), start);

            start = end; // point to '\n' of vendor
            end = start + 1; // point to start of devices
            // find the start of next vendor
            while (end[0] == '\t' || end[0] == '#')
            {
                end = strchr(end, '\n');
                if (!end)
                {
                    end = content->chars + content->length;
                    break;
                }
                else
                    end++;
            }

            len = (uint32_t) snprintf(buffer, sizeof(buffer), "\n\t%04x  ", device);
            start = memmem(start, (size_t) (end - start), buffer, len);
            if (start)
            {
                start += len;
                end = memchr(start, '\n', (uint32_t) (end - start));
                if (!end)
                    end = content->chars + content->length;

                char* openingBracket = memchr(start, '[', (uint32_t) (end - start));
                if (openingBracket)
                {
                    openingBracket++;
                    char* closingBracket = memchr(openingBracket, ']', (uint32_t) (end - openingBracket));
                    if (closingBracket)
                        ffStrbufSetNS(&gpu->name, (uint32_t) (closingBracket - openingBracket), openingBracket);
                }
                if (!gpu->name.length)
                    ffStrbufSetNS(&gpu->name, (uint32_t) (end - start), start);
            }
        }
    }

    if (!gpu->name.length)
    {
        const char* subclassStr;
        switch (subclass)
        {
        case 0 /*PCI_CLASS_DISPLAY_VGA*/: subclassStr = " (VGA compatible)"; break;
        case 1 /*PCI_CLASS_DISPLAY_XGA*/: subclassStr = " (XGA compatible)"; break;
        case 2 /*PCI_CLASS_DISPLAY_3D*/: subclassStr = " (3D)"; break;
        default: subclassStr = ""; break;
        }

        ffStrbufSetF(&gpu->name, "%s Device %04X%s", gpu->vendor.length ? gpu->vendor.chars : "Unknown", device, subclassStr);
    }
}
#endif
