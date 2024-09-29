#include "fastfetch.h"
#include "smc_temps.h"
#include "util/stringUtils.h"

#include <stdint.h>
#include <math.h>
#include <IOKit/IOKitLib.h>

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
    size_t size = sizeof(SmcKeyData_t);

    if (IOConnectCallStructMethod(conn, selector, inputStructure, size, outputStructure, &size) != kIOReturnSuccess)
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
    io_object_t device = IOServiceGetMatchingService(MACH_PORT_NULL, IOServiceMatching("AppleSMC"));
    if (!device)
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

    if (ffStrEquals(val.dataType, "ui8 ") ||
        ffStrEquals(val.dataType, "ui16") ||
        ffStrEquals(val.dataType, "ui32") ||
        ffStrEquals(val.dataType, "ui64"))
    {
        uint64_t tmp = 0;
        for (uint32_t i = 0; i < val.dataSize; i++)
            tmp += (uint64_t)((uint8_t)(val.bytes[i]) * pow(256, val.dataSize - 1 - i));
        *value = (double)tmp;
    }
    else if (ffStrEquals(val.dataType, "flt "))
    {
        *value = *(float *)(val.bytes);
    }
    else if (ffStrEquals(val.dataType, "fp1f") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 32768.0;
    }
    else if (ffStrEquals(val.dataType, "fp4c") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 4096.0;
    }
    else if (ffStrEquals(val.dataType, "fp5b") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 2048.0;
    }
    else if (ffStrEquals(val.dataType, "fp6a") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 1024.0;
    }
    else if (ffStrEquals(val.dataType, "fp79") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 512.0;
    }
    else if (ffStrEquals(val.dataType, "fp88") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 256.0;
    }
    else if (ffStrEquals(val.dataType, "fpa6") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 64.0;
    }
    else if (ffStrEquals(val.dataType, "fpc4") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 16.0;
    }
    else if (ffStrEquals(val.dataType, "fpe2") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 4.0;
    }
    else if (ffStrEquals(val.dataType, "sp1e") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 16384.0;
    }
    else if (ffStrEquals(val.dataType, "sp3c") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 4096.0;
    }
    else if (ffStrEquals(val.dataType, "sp4b") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 2048.0;
    }
    else if (ffStrEquals(val.dataType, "sp5a") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 1024.0;
    }
    else if (ffStrEquals(val.dataType, "sp69") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 512.0;
    }
    else if (ffStrEquals(val.dataType, "sp78") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 256.0;
    }
    else if (ffStrEquals(val.dataType, "sp87") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 128.0;
    }
    else if (ffStrEquals(val.dataType, "sp96") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 64.0;
    }
    else if (ffStrEquals(val.dataType, "spb4") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 16.0;
    }
    else if (ffStrEquals(val.dataType, "spf0") && val.dataSize == 2)
    {
        *value = ntohs(*(uint16_t *)(val.bytes)) / 1.0;
    }
    else if (ffStrEquals(val.dataType, "si8 ") && val.dataSize == 1)
    {
        signed char *bytes = (signed char *)val.bytes;
        int16_t temp = 0;
        temp += (int8_t)(bytes[0]);
        *value = temp;
    }
    else if (ffStrEquals(val.dataType, "si16") && val.dataSize == 2)
    {
        *value = ntohs(*(int16_t *)(val.bytes));
    }
    else if (ffStrEquals(val.dataType, "{pwm") && val.dataSize == 2)
    {
        *value = (double)ntohs(*(uint16_t *)(val.bytes)) * 100 / 65536.0;
    }
    else
        return "Unknown SMC data type";
    return NULL;
}

static bool detectTemp(io_connect_t conn, const char *sensor, double* sum)
{
    double temp = 0;
    const char* error = smcReadValue(conn, sensor, &temp);
    if (error) return false;
    // https://github.com/exelban/stats/blob/14e29c4d60229c363cca9c9d25c30c87b7870830/Modules/Sensors/readers.swift#L124
    if (temp < 10 || temp > 120) return false;
    *sum += temp;
    return true;
}

const char *ffDetectSmcTemps(enum FFTempType type, double *result)
{
    static io_connect_t conn;
    if (!conn)
    {
        if (smcOpen(&conn) != NULL)
            conn = (io_connect_t) -1;
    }
    else if (conn == (io_connect_t) -1)
        return "Could not open SMC connection";

    uint32_t count = 0;
    *result = 0;

    // https://github.com/exelban/stats/blob/master/Modules/Sensors/values.swift
    switch (type)
    {
    case FF_TEMP_CPU_X64:
        count += detectTemp(conn, "TC0D", result); // CPU diode
        count += detectTemp(conn, "TC0E", result); // CPU diode virtual
        count += detectTemp(conn, "TC0F", result); // CPU diode filtered
        count += detectTemp(conn, "TC0P", result); // CPU proximity
        break;

    case FF_TEMP_CPU_M1X:
        count += detectTemp(conn, "Tp09", result); // CPU efficient core 1
        count += detectTemp(conn, "Tp0T", result); // CPU efficient core 2

        count += detectTemp(conn, "Tp01", result); // CPU performance core 1
        count += detectTemp(conn, "Tp05", result); // CPU performance core 2
        count += detectTemp(conn, "Tp0D", result); // CPU performance core 3
        count += detectTemp(conn, "Tp0H", result); // CPU performance core 4
        count += detectTemp(conn, "Tp0L", result); // CPU performance core 5
        count += detectTemp(conn, "Tp0P", result); // CPU performance core 6
        count += detectTemp(conn, "Tp0X", result); // CPU performance core 7
        count += detectTemp(conn, "Tp0b", result); // CPU performance core 8
        break;

    case FF_TEMP_CPU_M2X:
        count += detectTemp(conn, "Tp1h", result); // CPU efficiency core 1
        count += detectTemp(conn, "Tp1t", result); // CPU efficiency core 2
        count += detectTemp(conn, "Tp1p", result); // CPU efficiency core 3
        count += detectTemp(conn, "Tp1l", result); // CPU efficiency core 4

        count += detectTemp(conn, "Tp01", result); // CPU performance core 1
        count += detectTemp(conn, "Tp05", result); // CPU performance core 2
        count += detectTemp(conn, "Tp09", result); // CPU performance core 3
        count += detectTemp(conn, "Tp0D", result); // CPU performance core 4
        count += detectTemp(conn, "Tp0X", result); // CPU performance core 5
        count += detectTemp(conn, "Tp0b", result); // CPU performance core 6
        count += detectTemp(conn, "Tp0f", result); // CPU performance core 7
        count += detectTemp(conn, "Tp0j", result); // CPU performance core 8
        break;

    case FF_TEMP_CPU_M3X:
        count += detectTemp(conn, "Te05", result); // CPU efficiency core 1
        count += detectTemp(conn, "Te0L", result); // CPU efficiency core 2
        count += detectTemp(conn, "Te0P", result); // CPU efficiency core 3
        count += detectTemp(conn, "Te0S", result); // CPU efficiency core 4
        count += detectTemp(conn, "Tf04", result); // CPU performance core 1
        count += detectTemp(conn, "Tf09", result); // CPU performance core 2
        count += detectTemp(conn, "Tf0A", result); // CPU performance core 3
        count += detectTemp(conn, "Tf0B", result); // CPU performance core 4
        count += detectTemp(conn, "Tf0D", result); // CPU performance core 5
        count += detectTemp(conn, "Tf0E", result); // CPU performance core 6
        count += detectTemp(conn, "Tf44", result); // CPU performance core 7
        count += detectTemp(conn, "Tf49", result); // CPU performance core 8
        count += detectTemp(conn, "Tf4A", result); // CPU performance core 9
        count += detectTemp(conn, "Tf4B", result); // CPU performance core 10
        count += detectTemp(conn, "Tf4D", result); // CPU performance core 11
        count += detectTemp(conn, "Tf4E", result); // CPU performance core 12
        break;

    case FF_TEMP_GPU_INTEL:
        count += detectTemp(conn, "TCGC", result); // GPU Intel Graphics
        goto gpu_unknown;

    case FF_TEMP_GPU_AMD:
        count += detectTemp(conn, "TGDD", result); // GPU AMD Radeon
        goto gpu_unknown;

    case FF_TEMP_GPU_UNKNOWN: // Nvidia?
    gpu_unknown:
        count += detectTemp(conn, "TG0D", result); // GPU diode
        count += detectTemp(conn, "TG0P", result); // GPU proximity
        break;

    case FF_TEMP_GPU_M1X:
        count += detectTemp(conn, "Tg05", result); // GPU 1
        count += detectTemp(conn, "Tg0D", result); // GPU 2
        count += detectTemp(conn, "Tg0L", result); // GPU 3
        count += detectTemp(conn, "Tg0T", result); // GPU 4
        break;

    case FF_TEMP_GPU_M2X:
        count += detectTemp(conn, "Tg0f", result); // GPU 1
        count += detectTemp(conn, "Tg0j", result); // GPU 2
        break;

    case FF_TEMP_GPU_M3X:
        count += detectTemp(conn, "Tf14", result); // GPU 1
        count += detectTemp(conn, "Tf18", result); // GPU 2
        count += detectTemp(conn, "Tf19", result); // GPU 3
        count += detectTemp(conn, "Tf1A", result); // GPU 4
        count += detectTemp(conn, "Tf24", result); // GPU 5
        count += detectTemp(conn, "Tf28", result); // GPU 6
        count += detectTemp(conn, "Tf29", result); // GPU 7
        count += detectTemp(conn, "Tf2A", result); // GPU 8
        break;

    case FF_TEMP_BATTERY:
        count += detectTemp(conn, "TB1T", result); // Battery
        count += detectTemp(conn, "TB2T", result); // Battery
        break;

    case FF_TEMP_MEMORY:
        count += detectTemp(conn, "Tm02", result); // Memory 1
        count += detectTemp(conn, "Tm06", result); // Memory 2
        count += detectTemp(conn, "Tm08", result); // Memory 3
        count += detectTemp(conn, "Tm09", result); // Memory 4
        break;
    }

    if (count == 0)
        return "No temperatures detected";

    *result /= count;

    return NULL;
}
