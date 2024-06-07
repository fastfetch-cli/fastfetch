#pragma once

// DISCLAIMER:
// THIS FILE IS CREATED FROM SCRATCH, BY READING THE OFFICIAL IGCL API
// DOCUMENTATION REFERENCED BELOW, IN ORDER TO MAKE FASTFETCH MIT COMPLIANT.

#include <stdint.h>

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv412ctl_result_t
typedef enum ctl_result_t
{
    CTL_RESULT_SUCCESS = 0,
} ctl_result_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv420ctl_application_id_t
typedef struct ctl_application_id_t
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} ctl_application_id_t;

#define CTL_IMPL_VERSION (( 1 /*major*/ << 16 )|( 1 /*minor*/ & 0x0000ffff))

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv415ctl_init_flag_t
typedef enum ctl_init_flag_t
{
    CTL_INIT_FLAG_USE_LEVEL_ZERO = 1,
    CTL_INIT_FLAG_MAX
} ctl_init_flag_t;

typedef uint32_t ctl_version_info_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv415ctl_init_args_t
typedef struct ctl_init_args_t
{
    uint32_t Size;
    uint8_t Version;
    ctl_version_info_t AppVersion;
    ctl_init_flag_t flags;
    ctl_version_info_t SupportedVersion;
    ctl_application_id_t ApplicationUID;
} ctl_init_args_t;

typedef struct ctl_api_handle_t* ctl_api_handle_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv47ctlInitP15ctl_init_args_tP16ctl_api_handle_t
extern ctl_result_t ctlInit(ctl_init_args_t *pInitDesc, ctl_api_handle_t *phAPIHandle);
// https://intel.github.io/drivers.gpu.control-library/Control/api.html#ctlclose
extern ctl_result_t ctlClose(ctl_api_handle_t hAPIHandle);

typedef struct ctl_device_adapter_handle_t* ctl_device_adapter_handle_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv419ctlEnumerateDevices16ctl_api_handle_tP8uint32_tP27ctl_device_adapter_handle_t
extern ctl_result_t ctlEnumerateDevices(ctl_api_handle_t hAPIHandle, uint32_t *pCount, ctl_device_adapter_handle_t* phDevices);

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv417ctl_device_type_t
typedef enum ctl_device_type_t
{
    CTL_DEVICE_TYPE_GRAPHICS = 1,
    CTL_DEVICE_TYPE_SYSTEM = 2,
    CTL_DEVICE_TYPE_MAX
} ctl_device_type_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv422ctl_firmware_version_t
typedef struct ctl_firmware_version_t
{
    uint64_t major_version;
    uint64_t minor_version;
    uint64_t build_number;
} ctl_firmware_version_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv417ctl_adapter_bdf_t
typedef struct ctl_adapter_bdf_t
{
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} ctl_adapter_bdf_t;

#define IGCL_CTL_MAX_DEVICE_NAME_LEN 100
#define IGCL_CTL_MAX_RESERVED_SIZE 112

typedef enum ctl_adapter_properties_flag_t
{
    CTL_ADAPTER_PROPERTIES_FLAG_INTEGRATED = 1,
} ctl_adapter_properties_flag_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv431ctl_device_adapter_properties_t
typedef struct ctl_device_adapter_properties_t
{
    uint32_t Size;
    uint8_t Version;
    void* pDeviceID;
    uint32_t device_id_size;
    ctl_device_type_t device_type;
    uint32_t /*ctl_supported_functions_flags_t*/ supported_subfunction_flags;
    uint64_t driver_version;
    ctl_firmware_version_t firmware_version;
    uint32_t pci_vendor_id;
    uint32_t pci_device_id;
    uint32_t rev_id;
    uint32_t num_eus_per_sub_slice;
    uint32_t num_sub_slices_per_slice;
    uint32_t num_slices;
    char name[IGCL_CTL_MAX_DEVICE_NAME_LEN];
    ctl_adapter_properties_flag_t graphics_adapter_properties;
    uint32_t Frequency;
    uint16_t pci_subsys_id;
    uint16_t pci_subsys_vendor_id;
    ctl_adapter_bdf_t adapter_bdf;
    char reserved[IGCL_CTL_MAX_RESERVED_SIZE];
} ctl_device_adapter_properties_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv422ctlGetDeviceProperties27ctl_device_adapter_handle_tP31ctl_device_adapter_properties_t
extern ctl_result_t ctlGetDeviceProperties(ctl_device_adapter_handle_t hDAhandle, ctl_device_adapter_properties_t* pProperties);

typedef struct ctl_temp_handle_t* ctl_temp_handle_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#ctlenumtemperaturesensors
extern ctl_result_t ctlEnumTemperatureSensors(ctl_device_adapter_handle_t hDAhandle, uint32_t* pCount, ctl_temp_handle_t* phTemperature);
// https://intel.github.io/drivers.gpu.control-library/Control/api.html#ctltemperaturegetstate
extern ctl_result_t ctlTemperatureGetState(ctl_temp_handle_t hTemperature, double* pTemperature);
// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv420ctlEnumMemoryModules27ctl_device_adapter_handle_tP8uint32_tP16ctl_mem_handle_t

typedef struct ctl_mem_handle_t* ctl_mem_handle_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv420ctlEnumMemoryModules27ctl_device_adapter_handle_tP8uint32_tP16ctl_mem_handle_t
extern ctl_result_t ctlEnumMemoryModules(ctl_device_adapter_handle_t hDAhandle, uint32_t *pCount, ctl_mem_handle_t* phMemory);

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv415ctl_mem_state_t
typedef struct ctl_mem_state_t
{
    uint32_t Size;
    uint8_t Version;
    uint64_t free;
    uint64_t size;
} ctl_mem_state_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv417ctlMemoryGetState16ctl_mem_handle_tP15ctl_mem_state_t
extern ctl_result_t ctlMemoryGetState(ctl_mem_handle_t hMemory, ctl_mem_state_t *pState);

typedef struct ctl_freq_handle_t* ctl_freq_handle_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#ctlenumfrequencydomains
extern ctl_result_t ctlEnumFrequencyDomains(ctl_device_adapter_handle_t hDAhandle, uint32_t* pCount, ctl_freq_handle_t* phFrequency);

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv417ctl_freq_domain_t
typedef enum ctl_freq_domain_t
{
    CTL_FREQ_DOMAIN_GPU = 0,
    CTL_FREQ_DOMAIN_MEMORY = 1,
    CTL_FREQ_DOMAIN_MAX
} ctl_freq_domain_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv421ctl_freq_properties_t
typedef struct ctl_freq_properties_t
{
    uint32_t Size;
    uint8_t Version;
    ctl_freq_domain_t type;
    bool canControl;
    double min;
    double max;
} ctl_freq_properties_t;

// https://intel.github.io/drivers.gpu.control-library/Control/api.html#_CPPv425ctlFrequencyGetProperties17ctl_freq_handle_tP21ctl_freq_properties_t
extern ctl_result_t ctlFrequencyGetProperties(ctl_freq_handle_t hFrequency, ctl_freq_properties_t* pProperties);
