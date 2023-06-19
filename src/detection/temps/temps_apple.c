#include "fastfetch.h"
#include "temps_apple.h"
#include "util/stringUtils.h"

#include <stdint.h>
#include <math.h>
#include <IOKit/IOKitLib.h>

static const char kDataTypeFlt[] = "flt ";
static const char kDataTypeFp1f[] = "fp1f";
static const char kDataTypeFp4c[] = "fp4c";
static const char kDataTypeFp5b[] = "fp5b";
static const char kDataTypeFp6a[] = "fp6a";
static const char kDataTypeFp79[] = "fp79";
static const char kDataTypeFp88[] = "fp88";
static const char kDataTypeFpa6[] = "fpa6";
static const char kDataTypeFpc4[] = "fpc4";
static const char kDataTypeFpe2[] = "fpe2";
static const char kDataTypeSp1e[] = "sp1e";
static const char kDataTypeSp3c[] = "sp3c";
static const char kDataTypeSp4b[] = "sp4b";
static const char kDataTypeSp5a[] = "sp5a";
static const char kDataTypeSp69[] = "sp69";
static const char kDataTypeSp78[] = "sp78";
static const char kDataTypeSp87[] = "sp87";
static const char kDataTypeSp96[] = "sp96";
static const char kDataTypeSpb4[] = "spb4";
static const char kDataTypeSpf0[] = "spf0";
static const char kDataTypeUi8[] = "ui8 ";
static const char kDataTypeUi16[] = "ui16";
static const char kDataTypeUi32[] = "ui32";
static const char kDataTypeUi64[] = "ui64";
static const char kDataTypeSi8[] = "si8 ";
static const char kDataTypeSi16[] = "si16";
static const char kDataTypePwm[] = "{pwm";

static const char *kIOAppleSmcHiddenClassName = "AppleSMC";
static const char kSmcCmdReadBytes = 5;
static const char kSmcCmdReadKeyInfo = 9;
static const uint32_t kKernelIndexSmc = 2;

typedef struct
{
    char major;
    char minor;
    char build;
    char reserved[1];
    uint16_t release;
} SmcKeyData_vers_t;

typedef struct
{
    uint16_t version;
    uint16_t length;
    uint32_t cpuPLimit;
    uint32_t gpuPLimit;
    uint32_t memPLimit;
} SmcKeyData_pLimitData_t;

typedef struct
{
    uint32_t dataSize;
    uint32_t dataType;
    char dataAttributes;
} SmcKeyData_keyInfo_t;

typedef unsigned char SmcBytes_t[32];

typedef struct
{
    uint32_t key;
    SmcKeyData_vers_t vers;
    SmcKeyData_pLimitData_t pLimitData;
    SmcKeyData_keyInfo_t keyInfo;
    char result;
    char status;
    char data8;
    uint32_t data32;
    SmcBytes_t bytes;
} SmcKeyData_t;

typedef char UInt32Char_t[5];

typedef struct
{
    UInt32Char_t key;
    uint32_t dataSize;
    UInt32Char_t dataType;
    SmcBytes_t bytes;
} SmcVal_t;

static uint32_t smcStrtoul(const char *str, int size, int base)
{
    uint32_t total = 0;

    for (int i = 0; i < size; i++)
    {
        if (base == 16)
            total += (uint32_t)(str[i] << (size - 1 - i) * 8);
        else
            total += (uint32_t)((unsigned char)(str[i]) << (size - 1 - i) * 8);
    }
    return total;
}

static void smcUltostr(char *str, uint32_t val)
{
    str[0] = (char)(val >> 24);
    str[1] = (char)(val >> 16);
    str[2] = (char)(val >> 8);
    str[3] = (char)val;
    str[4] = '\0';
}

static const char *smcCall(io_connect_t conn, uint32_t selector, SmcKeyData_t *inputStructure, SmcKeyData_t *outputStructure)
{
    size_t structureInputSize;
    size_t structureOutputSize;
    structureInputSize = sizeof(SmcKeyData_t);
    structureOutputSize = sizeof(SmcKeyData_t);

    if (IOConnectCallStructMethod(conn, selector, inputStructure, structureInputSize, outputStructure, &structureOutputSize) != kIOReturnSuccess)
        return "IOConnectCallStructMethod(conn) failed";
    return NULL;
}

// Provides key info, using a cache to dramatically improve the energy impact of smcFanControl
static const char *smcGetKeyInfo(io_connect_t conn, const uint32_t key, SmcKeyData_keyInfo_t *key_info)
{
    SmcKeyData_t inputStructure = {0};
    SmcKeyData_t outputStructure = {0};

    inputStructure.key = key;
    inputStructure.data8 = kSmcCmdReadKeyInfo;

    const char *error = smcCall(conn, kKernelIndexSmc, &inputStructure, &outputStructure);
    if (error)
        return error;

    *key_info = outputStructure.keyInfo;
    return NULL;
}

static const char *smcReadSmcVal(io_connect_t conn, const UInt32Char_t key, SmcVal_t *val)
{
    SmcKeyData_t inputStructure = {0};
    SmcKeyData_t outputStructure = {0};

    inputStructure.key = smcStrtoul(key, 4, 16);
    strcpy(val->key, key);

    const char *error = smcGetKeyInfo(conn, inputStructure.key, &outputStructure.keyInfo);
    if (error)
        return error;

    val->dataSize = outputStructure.keyInfo.dataSize;
    smcUltostr(val->dataType, outputStructure.keyInfo.dataType);
    inputStructure.keyInfo.dataSize = val->dataSize;
    inputStructure.data8 = kSmcCmdReadBytes;

    error = smcCall(conn, kKernelIndexSmc, &inputStructure, &outputStructure);
    if (error)
        return error;

    memcpy(val->bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

    return NULL;
}

static const char *smcOpen(io_connect_t *conn)
{
    CFMutableDictionaryRef matchDict = IOServiceMatching(kIOAppleSmcHiddenClassName);
    if (matchDict == NULL)
        return "IOServiceMatching(\"AppleSmartBattery\") failed";

    io_iterator_t iterator;
    if (IOServiceGetMatchingServices(MACH_PORT_NULL, matchDict, &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_object_t device = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    if (device == 0)
        return "No SMC device found";

    kern_return_t result = IOServiceOpen(device, mach_task_self(), 0, conn);
    IOObjectRelease(device);
    if (result != kIOReturnSuccess)
        return "IOServiceOpen() failed";

    return NULL;
}

static const char *smcReadValue(io_connect_t conn, const UInt32Char_t key, double *value)
{
    SmcVal_t val = {0};
    const char* error = smcReadSmcVal(conn, key, &val);
    if (error != NULL)
        return error;
    if (val.dataSize == 0)
        return "Empty SMC result";

    if (ffStrEquals(val.dataType, kDataTypeUi8) ||
        ffStrEquals(val.dataType, kDataTypeUi16) ||
        ffStrEquals(val.dataType, kDataTypeUi32) ||
        ffStrEquals(val.dataType, kDataTypeUi64))
    {
        uint64_t tmp = 0;
        for (uint32_t i = 0; i < val.dataSize; i++)
            tmp += (uint64_t)((uint8_t)(val.bytes[i]) * pow(256, val.dataSize - 1 - i));
        *value = (double)tmp;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFlt))
    {
        *value = *(float *)(val.bytes);
    }
    else if (ffStrEquals(val.dataType, kDataTypeFp1f) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 32768.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFp4c) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 4096.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFp5b) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 2048.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFp6a) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 1024.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFp79) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 512.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFp88) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 256.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFpa6) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 64.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFpc4) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 16.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeFpe2) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 4.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp1e) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 16384.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp3c) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 4096.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp4b) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 2048.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp5a) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 1024.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp69) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 512.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp78) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 256.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp87) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 128.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSp96) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 64.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSpb4) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 16.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSpf0) && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 1.0;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSi8) && val.dataSize == 1)
    {
        signed char *bytes = (signed char *)val.bytes;
        int16_t temp = 0;
        temp += (int8_t)(bytes[0]);
        *value = temp;
    }
    else if (ffStrEquals(val.dataType, kDataTypeSi16) && val.dataSize == 2)
    {
        *value = ntohs(*(int16_t *)(val.bytes));
    }
    else if (ffStrEquals(val.dataType, kDataTypePwm) && val.dataSize == 2)
    {
        *value = (double)ntohs(*(uint16_t *)(val.bytes)) * 100 / 65536.0;
    }
    else
        return "Unknown SMC data type";
    return NULL;
}

static void detectTemp(io_connect_t conn, const char *sensor, const char *name, FFlist *list)
{
    double temp = 0;
    const char* error = smcReadValue(conn, sensor, &temp);
    if (error)
        return;

    FFTempValue* tempValue= (FFTempValue*)ffListAdd(list);
    tempValue->value = temp;

    assert(strlen(sensor) == 4);
    ffStrbufInitNS(&tempValue->deviceClass, 4, sensor);

    ffStrbufInitS(&tempValue->name, name);
}

const char *ffDetectCoreTemps(enum FFTempType type, FFlist *result)
{
    static io_connect_t conn;
    static dispatch_once_t once_control;
    dispatch_once_f(&once_control, &conn, (dispatch_function_t) smcOpen);
    if(!conn)
        return "smcOpen() failed";

    // https://github.com/exelban/stats/blob/master/Modules/Sensors/values.swift
    switch (type)
    {
    case FF_TEMP_CPU_X64:
        detectTemp(conn, "TC0D", "CPU diode", result);
        detectTemp(conn, "TC0E", "CPU diode virtual", result);
        detectTemp(conn, "TC0F", "CPU diode filtered", result);
        detectTemp(conn, "TC0P", "CPU proximity", result);
        break;

    case FF_TEMP_CPU_M1X:
        detectTemp(conn, "Tp09", "CPU efficient core 1", result);
        detectTemp(conn, "Tp0T", "CPU efficient core 2", result);

        detectTemp(conn, "Tp01", "CPU performance core 1", result);
        detectTemp(conn, "Tp05", "CPU performance core 2", result);
        detectTemp(conn, "Tp0D", "CPU performance core 3", result);
        detectTemp(conn, "Tp0H", "CPU performance core 4", result);
        detectTemp(conn, "Tp0L", "CPU performance core 5", result);
        detectTemp(conn, "Tp0P", "CPU performance core 6", result);
        detectTemp(conn, "Tp0X", "CPU performance core 7", result);
        detectTemp(conn, "Tp0b", "CPU performance core 8", result);
        break;

    case FF_TEMP_CPU_M2X:
        detectTemp(conn, "Tp05", "CPU efficient core 1", result);
        detectTemp(conn, "Tp0D", "CPU efficient core 2", result);
        detectTemp(conn, "Tp0j", "CPU efficient core 3", result);
        detectTemp(conn, "Tp0r", "CPU efficient core 4", result);

        detectTemp(conn, "Tp01", "CPU performance core 1", result);
        detectTemp(conn, "Tp09", "CPU performance core 2", result);
        detectTemp(conn, "Tp0f", "CPU performance core 3", result);
        detectTemp(conn, "Tp0n", "CPU performance core 4", result);
        break;

    case FF_TEMP_GPU_INTEL:
        detectTemp(conn, "TCGC", "GPU Intel Graphics", result);
        goto gpu_unknown;

    case FF_TEMP_GPU_AMD:
        detectTemp(conn, "TGDD", "GPU AMD Radeon", result);
        goto gpu_unknown;

    case FF_TEMP_GPU_UNKNOWN: // Nvidia?
    gpu_unknown:
        detectTemp(conn, "TG0D", "GPU diode", result);
        detectTemp(conn, "TG0P", "GPU proximity", result);
        break;

    case FF_TEMP_GPU_M1X:
        detectTemp(conn, "Tg05", "GPU 1", result);
        detectTemp(conn, "Tg0D", "GPU 2", result);
        detectTemp(conn, "Tg0L", "GPU 3", result);
        detectTemp(conn, "Tg0T", "GPU 4", result);
        break;

    case FF_TEMP_GPU_M2X:
        detectTemp(conn, "Tg0f", "GPU 1", result);
        detectTemp(conn, "Tg0n", "GPU 2", result);
        break;

    case FF_TEMP_BATTERY:
        detectTemp(conn, "TB1T", "Battery", result);
        break;

    case FF_TEMP_MEMORY:
        detectTemp(conn, "Tm02", "Memory", result);
        break;
    }

    return NULL;
}
