//===========================================================================
// Copyright (C) 2022-23 Intel Corporation
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this software
// or the related documents without Intel's prior written permission. This software
// and the related documents are provided as is, with no express or implied
// warranties, other than those that are expressly stated in the License.
//--------------------------------------------------------------------------

/** 
 *
 * @file ctl_api.h
 * @version v1-r1
 *
 */
#ifndef _CTL_API_H
#define _CTL_API_H
#if defined(__cplusplus)
#pragma once
#endif

// standard headers
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Intel 'ctlApi' common types
#if !defined(__GNUC__)
#pragma region common
#endif
///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MAKE_VERSION
/// @brief Generates generic ::'ctlApi' API versions
#define CTL_MAKE_VERSION( _major, _minor )  (( _major << 16 )|( _minor & 0x0000ffff))
#endif // CTL_MAKE_VERSION

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MAJOR_VERSION
/// @brief Extracts ::'ctlApi' API major version
#define CTL_MAJOR_VERSION( _ver )  ( _ver >> 16 )
#endif // CTL_MAJOR_VERSION

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MINOR_VERSION
/// @brief Extracts ::'ctlApi' API minor version
#define CTL_MINOR_VERSION( _ver )  ( _ver & 0x0000ffff )
#endif // CTL_MINOR_VERSION

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_IMPL_MAJOR_VERSION
/// @brief ::'ctlApi' API major version of this implementation
#define CTL_IMPL_MAJOR_VERSION  1
#endif // CTL_IMPL_MAJOR_VERSION

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_IMPL_MINOR_VERSION
/// @brief ::'ctlApi' API minor version of this implementation
#define CTL_IMPL_MINOR_VERSION  1
#endif // CTL_IMPL_MINOR_VERSION

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_IMPL_VERSION
/// @brief ::'ctlApi' API version of this implementation
#define CTL_IMPL_VERSION  CTL_MAKE_VERSION( CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION )
#endif // CTL_IMPL_VERSION

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_APICALL
#if defined(_WIN32)
/// @brief Calling convention for all API functions
#define CTL_APICALL  __cdecl
#else
#define CTL_APICALL  
#endif // defined(_WIN32)
#endif // CTL_APICALL

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_APIEXPORT
#if defined(_WIN32)
/// @brief Microsoft-specific dllexport storage-class attribute
#define CTL_APIEXPORT  __declspec(dllexport)
#else
#define CTL_APIEXPORT  
#endif // defined(_WIN32)
#endif // CTL_APIEXPORT

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_DLLEXPORT
#if defined(_WIN32)
/// @brief Microsoft-specific dllexport storage-class attribute
#define CTL_DLLEXPORT  __declspec(dllexport)
#endif // defined(_WIN32)
#endif // CTL_DLLEXPORT

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_DLLEXPORT
#if __GNUC__ >= 4
/// @brief GCC-specific dllexport storage-class attribute
#define CTL_DLLEXPORT  __attribute__ ((visibility ("default")))
#else
#define CTL_DLLEXPORT  
#endif // __GNUC__ >= 4
#endif // CTL_DLLEXPORT

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_BIT
/// @brief Generic macro for enumerator bit masks
#define CTL_BIT( _i )  ( 1 << _i )
#endif // CTL_BIT

///////////////////////////////////////////////////////////////////////////////
/// @brief Supported initialization flags 
typedef uint32_t ctl_init_flags_t;
typedef enum _ctl_init_flag_t
{
    CTL_INIT_FLAG_USE_LEVEL_ZERO = CTL_BIT(0),      ///< Use Level0 or not. This is usually required for telemetry,
                                                    ///< performance, frequency related APIs
    CTL_INIT_FLAG_MAX = 0x80000000

} ctl_init_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Version information
typedef uint32_t ctl_version_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a control API instance
typedef struct _ctl_api_handle_t *ctl_api_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a device adapter instance
typedef struct _ctl_device_adapter_handle_t *ctl_device_adapter_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a device temperature sensor
typedef struct _ctl_temp_handle_t *ctl_temp_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle for a device frequency domain
typedef struct _ctl_freq_handle_t *ctl_freq_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a power device.
typedef struct _ctl_pwr_handle_t *ctl_pwr_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a device fan
typedef struct _ctl_fan_handle_t *ctl_fan_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a device memory module
typedef struct _ctl_mem_handle_t *ctl_mem_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a device engine group
typedef struct _ctl_engine_handle_t *ctl_engine_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Base for all properties types
typedef struct _ctl_base_interface_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure

} ctl_base_interface_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Value type
typedef enum _ctl_property_value_type_t
{
    CTL_PROPERTY_VALUE_TYPE_BOOL = 0,               ///< Boolean
    CTL_PROPERTY_VALUE_TYPE_FLOAT = 1,              ///< Float
    CTL_PROPERTY_VALUE_TYPE_INT32 = 2,              ///< Int32
    CTL_PROPERTY_VALUE_TYPE_UINT32 = 3,             ///< Unsigned Int32
    CTL_PROPERTY_VALUE_TYPE_ENUM = 4,               ///< Enum
    CTL_PROPERTY_VALUE_TYPE_CUSTOM = 5,             ///< Custom argument
    CTL_PROPERTY_VALUE_TYPE_MAX

} ctl_property_value_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Property range details, a generic struct to hold min/max/step size
///        information of various feature properties
typedef struct _ctl_property_range_info_t
{
    float min_possible_value;                       ///< [out] Minimum possible value
    float max_possible_value;                       ///< [out] Maximum possible value
    float step_size;                                ///< [out] Step size possible
    float default_value;                            ///< [out] Default value

} ctl_property_range_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Property range details of integer type, a generic struct to hold
///        min/max/step size information of various feature properties
typedef struct _ctl_property_range_info_int_t
{
    int32_t min_possible_value;                     ///< [out] Minimum possible value
    int32_t max_possible_value;                     ///< [out] Maximum possible value
    int32_t step_size;                              ///< [out] Step size possible
    int32_t default_value;                          ///< [out] Default value

} ctl_property_range_info_int_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Property range details of unsigned integer type, a generic struct to
///        hold min/max/step size information of various feature properties
typedef struct _ctl_property_range_info_uint_t
{
    uint32_t min_possible_value;                    ///< [out] Minimum possible value
    uint32_t max_possible_value;                    ///< [out] Maximum possible value
    uint32_t step_size;                             ///< [out] Step size possible
    uint32_t default_value;                         ///< [out] Default value

} ctl_property_range_info_uint_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Bool feature details
typedef struct _ctl_property_info_boolean_t
{
    bool DefaultState;                              ///< [out] Default state

} ctl_property_info_boolean_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Bool feature for get/set
typedef struct _ctl_property_boolean_t
{
    bool Enable;                                    ///< [in,out] Enable

} ctl_property_boolean_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumeration feature details
typedef struct _ctl_property_info_enum_t
{
    uint64_t SupportedTypes;                        ///< [out] Supported possible values represented as a bitmask
    uint32_t DefaultType;                           ///< [out] Default type

} ctl_property_info_enum_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumeration feature for get/set
typedef struct _ctl_property_enum_t
{
    uint32_t EnableType;                            ///< [in,out] Enable with specific type

} ctl_property_enum_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Float feature details
typedef struct _ctl_property_info_float_t
{
    bool DefaultEnable;                             ///< [in,out] DefaultEnable
    ctl_property_range_info_t RangeInfo;            ///< [out] Min/max/default/step details

} ctl_property_info_float_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Float feature for get/set
typedef struct _ctl_property_float_t
{
    bool Enable;                                    ///< [in,out] Enable
    float Value;                                    ///< [in,out] Value

} ctl_property_float_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Int32 feature details
typedef struct _ctl_property_info_int_t
{
    bool DefaultEnable;                             ///< [in,out] DefaultEnable
    ctl_property_range_info_int_t RangeInfo;        ///< [out] Min/max/default/step details

} ctl_property_info_int_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Int32 feature for get/set
typedef struct _ctl_property_int_t
{
    bool Enable;                                    ///< [in,out] Enable
    int32_t Value;                                  ///< [in,out] Value

} ctl_property_int_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Int32 feature details
typedef struct _ctl_property_info_uint_t
{
    bool DefaultEnable;                             ///< [in,out] DefaultEnable
    ctl_property_range_info_uint_t RangeInfo;       ///< [out] Min/max/default/step details

} ctl_property_info_uint_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Int32 feature for get/set
typedef struct _ctl_property_uint_t
{
    bool Enable;                                    ///< [in,out] Enable
    uint32_t Value;                                 ///< [in,out] Value

} ctl_property_uint_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Feature element details, union of bool/float/enum property_info
///        structs. Used for feature specific capability check
typedef union _ctl_property_info_t
{
    ctl_property_info_boolean_t BoolType;           ///< [in,out] Boolean type fields
    ctl_property_info_float_t FloatType;            ///< [in,out] Float type fields
    ctl_property_info_int_t IntType;                ///< [in,out] Int type fields
    ctl_property_info_enum_t EnumType;              ///< [in,out] Enum type fields
    ctl_property_info_uint_t UIntType;              ///< [in,out] Unsigned Int type fields

} ctl_property_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Feature element details, union of bool/float/enum property structs.
///        Used for get/set calls
typedef union _ctl_property_t
{
    ctl_property_boolean_t BoolType;                ///< [in,out] Boolean type fields
    ctl_property_float_t FloatType;                 ///< [in,out] Float type fields
    ctl_property_int_t IntType;                     ///< [in,out] Int type fields
    ctl_property_enum_t EnumType;                   ///< [in,out] Enum type fields
    ctl_property_uint_t UIntType;                   ///< [in,out] Unsigned Int type fields

} ctl_property_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Defines Return/Error codes.
///        All generic error (bit30) codes are between 0x40000000-0x4000FFFF.
///        All 3D (bit 29) specific error codes are between 0x60000000-0x6000FFFF.
///        All media (bit 28) specific error codes are between 0x50000000-0x5000FFFF.
///        All display (bit 27) specific error codes are between 0x48000000-0x4800FFFF
///        All core (bit 26) specific error codes are between 0x44000000-0x4400FFFF
///        Success result code with additional info are between 0x00000001-0x0000FFFF.
typedef enum _ctl_result_t
{
    CTL_RESULT_SUCCESS = 0x00000000,                ///< success
    CTL_RESULT_SUCCESS_STILL_OPEN_BY_ANOTHER_CALLER = 0x00000001,   ///< success but still open by another caller
    CTL_RESULT_ERROR_SUCCESS_END = 0x0000FFFF,      ///< "Success group error code end value, not to be used
                                                    ///< "
    CTL_RESULT_ERROR_GENERIC_START = 0x40000000,    ///< Generic error code starting value, not to be used
    CTL_RESULT_ERROR_NOT_INITIALIZED = 0x40000001,  ///< Result not initialized
    CTL_RESULT_ERROR_ALREADY_INITIALIZED = 0x40000002,  ///< Already initialized
    CTL_RESULT_ERROR_DEVICE_LOST = 0x40000003,      ///< Device hung, reset, was removed, or driver update occurred
    CTL_RESULT_ERROR_OUT_OF_HOST_MEMORY = 0x40000004,   ///< Insufficient host memory to satisfy call
    CTL_RESULT_ERROR_OUT_OF_DEVICE_MEMORY = 0x40000005, ///< Insufficient device memory to satisfy call
    CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS = 0x40000006, ///< Access denied due to permission level
    CTL_RESULT_ERROR_NOT_AVAILABLE = 0x40000007,    ///< Resource was removed
    CTL_RESULT_ERROR_UNINITIALIZED = 0x40000008,    ///< Library not initialized
    CTL_RESULT_ERROR_UNSUPPORTED_VERSION = 0x40000009,  ///< Generic error code for unsupported versions
    CTL_RESULT_ERROR_UNSUPPORTED_FEATURE = 0x4000000a,  ///< Generic error code for unsupported features
    CTL_RESULT_ERROR_INVALID_ARGUMENT = 0x4000000b, ///< Generic error code for invalid arguments
    CTL_RESULT_ERROR_INVALID_API_HANDLE = 0x4000000c,   ///< API handle in invalid
    CTL_RESULT_ERROR_INVALID_NULL_HANDLE = 0x4000000d,  ///< Handle argument is not valid
    CTL_RESULT_ERROR_INVALID_NULL_POINTER = 0x4000000e, ///< Pointer argument may not be nullptr
    CTL_RESULT_ERROR_INVALID_SIZE = 0x4000000f,     ///< Size argument is invalid (e.g., must not be zero)
    CTL_RESULT_ERROR_UNSUPPORTED_SIZE = 0x40000010, ///< Size argument is not supported by the device (e.g., too large)
    CTL_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT = 0x40000011, ///< Image format is not supported by the device
    CTL_RESULT_ERROR_DATA_READ = 0x40000012,        ///< Data read error
    CTL_RESULT_ERROR_DATA_WRITE = 0x40000013,       ///< Data write error
    CTL_RESULT_ERROR_DATA_NOT_FOUND = 0x40000014,   ///< Data not found error
    CTL_RESULT_ERROR_NOT_IMPLEMENTED = 0x40000015,  ///< Function not implemented
    CTL_RESULT_ERROR_OS_CALL = 0x40000016,          ///< Operating system call failure
    CTL_RESULT_ERROR_KMD_CALL = 0x40000017,         ///< Kernel mode driver call failure
    CTL_RESULT_ERROR_UNLOAD = 0x40000018,           ///< Library unload failure
    CTL_RESULT_ERROR_ZE_LOADER = 0x40000019,        ///< Level0 loader not found
    CTL_RESULT_ERROR_INVALID_OPERATION_TYPE = 0x4000001a,   ///< Invalid operation type
    CTL_RESULT_ERROR_NULL_OS_INTERFACE = 0x4000001b,///< Null OS interface
    CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE = 0x4000001c,  ///< Null OS adapter handle
    CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE = 0x4000001d,///< Null display output handle
    CTL_RESULT_ERROR_WAIT_TIMEOUT = 0x4000001e,     ///< Timeout in Wait function
    CTL_RESULT_ERROR_PERSISTANCE_NOT_SUPPORTED = 0x4000001f,///< Persistance not supported
    CTL_RESULT_ERROR_PLATFORM_NOT_SUPPORTED = 0x40000020,   ///< Platform not supported
    CTL_RESULT_ERROR_UNKNOWN_APPLICATION_UID = 0x40000021,  ///< Unknown Appplicaion UID in Initialization call 
    CTL_RESULT_ERROR_INVALID_ENUMERATION = 0x40000022,  ///< The enum is not valid
    CTL_RESULT_ERROR_FILE_DELETE = 0x40000023,      ///< Error in file delete operation
    CTL_RESULT_ERROR_RESET_DEVICE_REQUIRED = 0x40000024,///< The device requires a reset.
    CTL_RESULT_ERROR_FULL_REBOOT_REQUIRED = 0x40000025, ///< The device requires a full reboot.
    CTL_RESULT_ERROR_LOAD = 0x40000026,             ///< Library load failure
    CTL_RESULT_ERROR_UNKNOWN = 0x4000FFFF,          ///< Unknown or internal error
    CTL_RESULT_ERROR_RETRY_OPERATION = 0x40010000,  ///< Operation failed, retry previous operation again
    CTL_RESULT_ERROR_GENERIC_END = 0x4000FFFF,      ///< "Generic error code end value, not to be used
                                                    ///< "
    CTL_RESULT_ERROR_CORE_START = 0x44000000,       ///< Core error code starting value, not to be used
    CTL_RESULT_ERROR_CORE_OVERCLOCK_NOT_SUPPORTED = 0x44000001, ///< The Overclock is not supported.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_VOLTAGE_OUTSIDE_RANGE = 0x44000002, ///< The Voltage exceeds the acceptable min/max.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_FREQUENCY_OUTSIDE_RANGE = 0x44000003,   ///< The Frequency exceeds the acceptable min/max.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_POWER_OUTSIDE_RANGE = 0x44000004,   ///< The Power exceeds the acceptable min/max.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_TEMPERATURE_OUTSIDE_RANGE = 0x44000005, ///< The Power exceeds the acceptable min/max.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_IN_VOLTAGE_LOCKED_MODE = 0x44000006,///< The Overclock is in voltage locked mode.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_RESET_REQUIRED = 0x44000007,///< It indicates that the requested change will not be applied until the
                                                    ///< device is reset.
    CTL_RESULT_ERROR_CORE_OVERCLOCK_WAIVER_NOT_SET = 0x44000008,///< The $OverclockWaiverSet function has not been called.
    CTL_RESULT_ERROR_CORE_END = 0x0440FFFF,         ///< "Core error code end value, not to be used
                                                    ///< "
    CTL_RESULT_ERROR_3D_START = 0x60000000,         ///< 3D error code starting value, not to be used
    CTL_RESULT_ERROR_3D_END = 0x6000FFFF,           ///< "3D error code end value, not to be used
                                                    ///< "
    CTL_RESULT_ERROR_MEDIA_START = 0x50000000,      ///< Media error code starting value, not to be used
    CTL_RESULT_ERROR_MEDIA_END = 0x5000FFFF,        ///< "Media error code end value, not to be used
                                                    ///< "
    CTL_RESULT_ERROR_DISPLAY_START = 0x48000000,    ///< Display error code starting value, not to be used
    CTL_RESULT_ERROR_INVALID_AUX_ACCESS_FLAG = 0x48000001,  ///< Invalid flag for Aux access
    CTL_RESULT_ERROR_INVALID_SHARPNESS_FILTER_FLAG = 0x48000002,///< Invalid flag for Sharpness
    CTL_RESULT_ERROR_DISPLAY_NOT_ATTACHED = 0x48000003, ///< Error for Display not attached
    CTL_RESULT_ERROR_DISPLAY_NOT_ACTIVE = 0x48000004,   ///< Error for display attached but not active
    CTL_RESULT_ERROR_INVALID_POWERFEATURE_OPTIMIZATION_FLAG = 0x48000005,   ///< Error for invalid power optimization flag
    CTL_RESULT_ERROR_INVALID_POWERSOURCE_TYPE_FOR_DPST = 0x48000006,///< DPST is supported only in DC Mode
    CTL_RESULT_ERROR_INVALID_PIXTX_GET_CONFIG_QUERY_TYPE = 0x48000007,  ///< Invalid query type for pixel transformation get configuration
    CTL_RESULT_ERROR_INVALID_PIXTX_SET_CONFIG_OPERATION_TYPE = 0x48000008,  ///< Invalid operation type for pixel transformation set configuration
    CTL_RESULT_ERROR_INVALID_SET_CONFIG_NUMBER_OF_SAMPLES = 0x48000009, ///< Invalid number of samples for pixel transformation set configuration
    CTL_RESULT_ERROR_INVALID_PIXTX_BLOCK_ID = 0x4800000a,   ///< Invalid block id for pixel transformation
    CTL_RESULT_ERROR_INVALID_PIXTX_BLOCK_TYPE = 0x4800000b, ///< Invalid block type for pixel transformation
    CTL_RESULT_ERROR_INVALID_PIXTX_BLOCK_NUMBER = 0x4800000c,   ///< Invalid block number for pixel transformation
    CTL_RESULT_ERROR_INSUFFICIENT_PIXTX_BLOCK_CONFIG_MEMORY = 0x4800000d,   ///< Insufficient memery allocated for BlockConfigs
    CTL_RESULT_ERROR_3DLUT_INVALID_PIPE = 0x4800000e,   ///< Invalid pipe for 3dlut
    CTL_RESULT_ERROR_3DLUT_INVALID_DATA = 0x4800000f,   ///< Invalid 3dlut data
    CTL_RESULT_ERROR_3DLUT_NOT_SUPPORTED_IN_HDR = 0x48000010,   ///< 3dlut not supported in HDR
    CTL_RESULT_ERROR_3DLUT_INVALID_OPERATION = 0x48000011,  ///< Invalid 3dlut operation
    CTL_RESULT_ERROR_3DLUT_UNSUCCESSFUL = 0x48000012,   ///< 3dlut call unsuccessful
    CTL_RESULT_ERROR_AUX_DEFER = 0x48000013,        ///< AUX defer failure
    CTL_RESULT_ERROR_AUX_TIMEOUT = 0x48000014,      ///< AUX timeout failure
    CTL_RESULT_ERROR_AUX_INCOMPLETE_WRITE = 0x48000015, ///< AUX incomplete write failure
    CTL_RESULT_ERROR_I2C_AUX_STATUS_UNKNOWN = 0x48000016,   ///< I2C/AUX unkonown failure
    CTL_RESULT_ERROR_I2C_AUX_UNSUCCESSFUL = 0x48000017, ///< I2C/AUX unsuccessful
    CTL_RESULT_ERROR_LACE_INVALID_DATA_ARGUMENT_PASSED = 0x48000018,///< Lace Incorrrect AggressivePercent data or LuxVsAggressive Map data
                                                    ///< passed by user
    CTL_RESULT_ERROR_EXTERNAL_DISPLAY_ATTACHED = 0x48000019,///< External Display is Attached hence fail the Display Switch
    CTL_RESULT_ERROR_CUSTOM_MODE_STANDARD_CUSTOM_MODE_EXISTS = 0x4800001a,  ///< Standard custom mode exists
    CTL_RESULT_ERROR_CUSTOM_MODE_NON_CUSTOM_MATCHING_MODE_EXISTS = 0x4800001b,  ///< Non custom matching mode exists
    CTL_RESULT_ERROR_CUSTOM_MODE_INSUFFICIENT_MEMORY = 0x4800001c,  ///< Custom mode insufficent memory
    CTL_RESULT_ERROR_ADAPTER_ALREADY_LINKED = 0x4800001d,   ///< Adapter is already linked
    CTL_RESULT_ERROR_ADAPTER_NOT_IDENTICAL = 0x4800001e,///< Adapter is not identical for linking
    CTL_RESULT_ERROR_ADAPTER_NOT_SUPPORTED_ON_LDA_SECONDARY = 0x4800001f,   ///< Adapter is LDA Secondary, so not supporting requested operation
    CTL_RESULT_ERROR_SET_FBC_FEATURE_NOT_SUPPORTED = 0x48000020,///< Set FBC Feature not supported
    CTL_RESULT_ERROR_DISPLAY_END = 0x4800FFFF,      ///< "Display error code end value, not to be used
                                                    ///< "
    CTL_RESULT_MAX

} ctl_result_t;

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MAX_DEVICE_NAME_LEN
/// @brief Maximum IPC handle size
#define CTL_MAX_DEVICE_NAME_LEN  100
#endif // CTL_MAX_DEVICE_NAME_LEN

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MAX_RESERVED_SIZE
/// @brief Maximum reserved size for future members.
#define CTL_MAX_RESERVED_SIZE  112
#endif // CTL_MAX_RESERVED_SIZE

///////////////////////////////////////////////////////////////////////////////
/// @brief General Physical Units.
typedef enum _ctl_units_t
{
    CTL_UNITS_FREQUENCY_MHZ = 0,                    ///< Type is Frequency with units in MHz.
    CTL_UNITS_OPERATIONS_GTS = 1,                   ///< Type is Frequency with units in GT/s (gigatransfers per second).
    CTL_UNITS_OPERATIONS_MTS = 2,                   ///< Type is Frequency with units in MT/s (megatransfers per second).
    CTL_UNITS_VOLTAGE_VOLTS = 3,                    ///< Type is Voltage with units in Volts.
    CTL_UNITS_POWER_WATTS = 4,                      ///< Type is Power with units in Watts.
    CTL_UNITS_TEMPERATURE_CELSIUS = 5,              ///< Type is Temperature with units in Celsius.
    CTL_UNITS_ENERGY_JOULES = 6,                    ///< Type is Energy with units in Joules.
    CTL_UNITS_TIME_SECONDS = 7,                     ///< Type is Time with units in Seconds.
    CTL_UNITS_MEMORY_BYTES = 8,                     ///< Type is Memory with units in Bytes.
    CTL_UNITS_ANGULAR_SPEED_RPM = 9,                ///< Type is Angular Speed with units in Revolutions per Minute.
    CTL_UNITS_POWER_MILLIWATTS = 10,                ///< Type is Power with units in MilliWatts.
    CTL_UNITS_PERCENT = 11,                         ///< Type is Percentage.
    CTL_UNITS_MEM_SPEED_GBPS = 12,                  ///< Type is Memory Speed in Gigabyte per Seconds (Gbps)
    CTL_UNITS_VOLTAGE_MILLIVOLTS = 13,              ///< Type is Voltage with units in milliVolts.
    CTL_UNITS_UNKNOWN = 0x4800FFFF,                 ///< Type of units unknown.
    CTL_UNITS_MAX

} ctl_units_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief General Data Types.
typedef enum _ctl_data_type_t
{
    CTL_DATA_TYPE_INT8 = 0,                         ///< The data type is 8 bit signed integer.
    CTL_DATA_TYPE_UINT8 = 1,                        ///< The data type is 8 bit unsigned integer.
    CTL_DATA_TYPE_INT16 = 2,                        ///< The data type is 16 bit signed integer.
    CTL_DATA_TYPE_UINT16 = 3,                       ///< The data type is 16 bit unsigned integer.
    CTL_DATA_TYPE_INT32 = 4,                        ///< The data type is 32 bit signed integer.
    CTL_DATA_TYPE_UINT32 = 5,                       ///< The data type is 32 bit unsigned integer.
    CTL_DATA_TYPE_INT64 = 6,                        ///< The data type is 64 bit signed integer.
    CTL_DATA_TYPE_UINT64 = 7,                       ///< The data type is 64 bit unsigned integer.
    CTL_DATA_TYPE_FLOAT = 8,                        ///< The data type is 32 bit floating point.
    CTL_DATA_TYPE_DOUBLE = 9,                       ///< The data type is 64 bit floating point.
    CTL_DATA_TYPE_STRING_ASCII = 10,                ///< The data type is an array of 8 bit unsigned integers.
    CTL_DATA_TYPE_STRING_UTF16 = 11,                ///< The data type is an array of 16 bit unsigned integers.
    CTL_DATA_TYPE_STRING_UTF132 = 12,               ///< The data type is an array of 32 bit unsigned integers.
    CTL_DATA_TYPE_UNKNOWN = 0x4800FFFF,             ///< The data type is unknown.
    CTL_DATA_TYPE_MAX

} ctl_data_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Union for Generic Data.
/// 
/// @details
///     - The telemetry data items could be of different types.
///     - Refer to ::ctl_data_type_t to find the current type.
typedef union _ctl_data_value_t
{
    int8_t data8;                                   ///< [out] The data type is 8 bit signed integer.
    uint8_t datau8;                                 ///< [out] The data type is 8 bit unsigned integer.
    int16_t data16;                                 ///< [out] The data type is 16 bit signed integer.
    uint16_t datau16;                               ///< [out] The data type is 16 bit unsigned integer.
    int32_t data32;                                 ///< [out] The data type is 32 bit signed integer.
    uint32_t datau32;                               ///< [out] The data type is 32 bit unsigned integer.
    int64_t data64;                                 ///< [out] The data type is 64 bit signed integer.
    uint64_t datau64;                               ///< [out] The data type is 64 bit unsigned integer.
    float datafloat;                                ///< [out] The data type is 32 bit floating point.
    double datadouble;                              ///< [out] The data type is 64 bit floating point.

} ctl_data_value_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Base for all properties types
typedef struct _ctl_base_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure

} ctl_base_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Application Unique ID
typedef struct _ctl_application_id_t
{
    uint32_t Data1;                                 ///< [in] Data1
    uint16_t Data2;                                 ///< [in] Data2
    uint16_t Data3;                                 ///< [in] Data3
    uint8_t Data4[8];                               ///< [in] Data4

} ctl_application_id_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Init arguments
typedef struct _ctl_init_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_version_info_t AppVersion;                  ///< [in][release] App's IGCL version
    ctl_init_flags_t flags;                         ///< [in][release] Caller version
    ctl_version_info_t SupportedVersion;            ///< [out][release] IGCL implementation version
    ctl_application_id_t ApplicationUID;            ///< [in] Application Provided Unique ID.Application can pass all 0's as
                                                    ///< the default ID

} ctl_init_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Reserved struct
typedef struct _ctl_reserved_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    void* pSpecialArg;                              ///< [in] Reserved struct
    uint32_t ArgSize;                               ///< [in] struct size

} ctl_reserved_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Reserved base struct
typedef struct _ctl_reserved_args_base_t
{
    ctl_application_id_t ReservedFuncID;            ///< [in] Unique ID for reserved/special function

} ctl_reserved_args_base_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Reserved - Unlock function capability
typedef struct _ctl_unlock_capability_t
{
    ctl_application_id_t ReservedFuncID;            ///< [in] Unique ID for reserved/special function
    ctl_application_id_t UnlockCapsID;              ///< [in] Unique ID to unlock a specific function

} ctl_unlock_capability_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Used by loader like modules to specify runtime implementation details
typedef struct _ctl_runtime_path_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_application_id_t UnlockID;                  ///< [in] Unique ID for reserved/special function
    wchar_t* pRuntimePath;                          ///< [in] Path to runtime DLL
    uint16_t DeviceID;                              ///< [in] Device ID of interest to caller. pRuntimePath should not be NULL.
    uint8_t RevID;                                  ///< [in] Revision ID of interest to caller. pRuntimePath should not be
                                                    ///< NULL.

} ctl_runtime_path_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Control Api Init
/// 
/// @details
///     - Control Api Init
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pInitDesc`
///         + `nullptr == phAPIHandle`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlInit(
    ctl_init_args_t* pInitDesc,                     ///< [in][out] App's control API version
    ctl_api_handle_t* phAPIHandle                   ///< [in][out][release] Control API handle
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Control Api Destroy
/// 
/// @details
///     - Control Api Close
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hAPIHandle`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlClose(
    ctl_api_handle_t hAPIHandle                     ///< [in][release] Control API implementation handle obtained during init
                                                    ///< call
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Runtime path
/// 
/// @details
///     - Control Api set runtime path. Optional call from a loader which allows
///       the loaded runtime to enumerate only the adapters which the specified
///       runtime is responsible for. This is done usually by a loader or by
///       callers who know how to get the specific runtime of interest. This
///       call right now is reserved for use by Intel components.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetRuntimePath(
    ctl_runtime_path_args_t* pArgs                  ///< [in] Runtime path
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Supported Functions
typedef uint32_t ctl_supported_functions_flags_t;
typedef enum _ctl_supported_functions_flag_t
{
    CTL_SUPPORTED_FUNCTIONS_FLAG_DISPLAY = CTL_BIT(0),  ///< [out] Is Display supported
    CTL_SUPPORTED_FUNCTIONS_FLAG_3D = CTL_BIT(1),   ///< [out] Is 3D supported
    CTL_SUPPORTED_FUNCTIONS_FLAG_MEDIA = CTL_BIT(2),///< [out] Is Media supported
    CTL_SUPPORTED_FUNCTIONS_FLAG_MAX = 0x80000000

} ctl_supported_functions_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Firmware version
typedef struct _ctl_firmware_version_t
{
    uint64_t major_version;                         ///< [out] Major version
    uint64_t minor_version;                         ///< [out] Minor version
    uint64_t build_number;                          ///< [out] Build number

} ctl_firmware_version_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief DeviceType
typedef enum _ctl_device_type_t
{
    CTL_DEVICE_TYPE_GRAPHICS = 1,                   ///< Graphics Device type
    CTL_DEVICE_TYPE_SYSTEM = 2,                     ///< System Device type
    CTL_DEVICE_TYPE_MAX

} ctl_device_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adapter Properties
typedef uint32_t ctl_adapter_properties_flags_t;
typedef enum _ctl_adapter_properties_flag_t
{
    CTL_ADAPTER_PROPERTIES_FLAG_INTEGRATED = CTL_BIT(0),///< [out] Is Integrated Graphics adapter
    CTL_ADAPTER_PROPERTIES_FLAG_LDA_PRIMARY = CTL_BIT(1),   ///< [out] Is Primary (Lead) adapter in a Linked Display Adapter (LDA)
                                                    ///< chain
    CTL_ADAPTER_PROPERTIES_FLAG_LDA_SECONDARY = CTL_BIT(2), ///< [out] Is Secondary (Linked) adapter in a Linked Display Adapter (LDA)
                                                    ///< chain
    CTL_ADAPTER_PROPERTIES_FLAG_MAX = 0x80000000

} ctl_adapter_properties_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adapter Pci Bus, Device, Function
typedef struct _ctl_adapter_bdf_t
{
    uint8_t bus;                                    ///< [out] PCI Bus Number
    uint8_t device;                                 ///< [out] PCI device number
    uint8_t function;                               ///< [out] PCI function

} ctl_adapter_bdf_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Device Adapter properties
typedef struct _ctl_device_adapter_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    void* pDeviceID;                                ///< [in,out] OS specific Device ID
    uint32_t device_id_size;                        ///< [in] size of the device ID
    ctl_device_type_t device_type;                  ///< [out] Device Type
    ctl_supported_functions_flags_t supported_subfunction_flags;///< [out] Supported functions
    uint64_t driver_version;                        ///< [out] Driver version
    ctl_firmware_version_t firmware_version;        ///< [out] Firmware version
    uint32_t pci_vendor_id;                         ///< [out] PCI Vendor ID
    uint32_t pci_device_id;                         ///< [out] PCI Device ID
    uint32_t rev_id;                                ///< [out] PCI Revision ID
    uint32_t num_eus_per_sub_slice;                 ///< [out] Number of EUs per sub-slice
    uint32_t num_sub_slices_per_slice;              ///< [out] Number of sub-slices per slice
    uint32_t num_slices;                            ///< [out] Number of slices
    char name[CTL_MAX_DEVICE_NAME_LEN];             ///< [out] Device name
    ctl_adapter_properties_flags_t graphics_adapter_properties; ///< [out] Graphics Adapter Properties
    uint32_t Frequency;                             ///< [out] Clock frequency for this device. Supported only for Version > 0
    uint16_t pci_subsys_id;                         ///< [out] PCI SubSys ID, Supported only for Version > 1
    uint16_t pci_subsys_vendor_id;                  ///< [out] PCI SubSys Vendor ID, Supported only for Version > 1
    ctl_adapter_bdf_t adapter_bdf;                  ///< [out] Pci Bus, Device, Function. Supported only for Version > 1
    char reserved[CTL_MAX_RESERVED_SIZE];           ///< [out] Reserved

} ctl_device_adapter_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief OperationType
typedef enum _ctl_operation_type_t
{
    CTL_OPERATION_TYPE_READ = 1,                    ///< Read operation
    CTL_OPERATION_TYPE_WRITE = 2,                   ///< Write operation
    CTL_OPERATION_TYPE_MAX

} ctl_operation_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Generic Structure for Void* datatypes
typedef struct _ctl_generic_void_datatype_t
{
    void* pData;                                    ///< [in,out]void pointer to memory
    uint32_t size;                                  ///< [in,out]size of the allocated memory

} ctl_generic_void_datatype_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Generic Structure for Revision datatypes
typedef struct _ctl_revision_datatype_t
{
    uint8_t major_version;                          ///< [in,out]Major Version
    uint8_t minor_version;                          ///< [in,out]Minor Version
    uint8_t revision_version;                       ///< [in,out]Revision Version

} ctl_revision_datatype_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Property Type flags
typedef uint32_t ctl_property_type_flags_t;
typedef enum _ctl_property_type_flag_t
{
    CTL_PROPERTY_TYPE_FLAG_DISPLAY = CTL_BIT(0),    ///< Display type. Supported scenarios: Sharpness/gamma/CSC
    CTL_PROPERTY_TYPE_FLAG_3D = CTL_BIT(1),         ///< 3D type. Supported scenarios: All set calls via IGCL's 3D APIs
    CTL_PROPERTY_TYPE_FLAG_MEDIA = CTL_BIT(2),      ///< Media type. Supported scenarios: All set calls via IGCL's media APIs
    CTL_PROPERTY_TYPE_FLAG_CORE = CTL_BIT(3),       ///< For future: Core graphic event types like clocking, frequency etc.
    CTL_PROPERTY_TYPE_FLAG_MAX = 0x80000000

} ctl_property_type_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Arguments related to wait for a property change function
typedef struct _ctl_wait_property_change_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_property_type_flags_t PropertyType;         ///< [in] Type of the property
    uint32_t TimeOutMilliSec;                       ///< [in][release] Time-out interval in milliseconds. Specify 0xFFFFFFFF if
                                                    ///< time-out is not desired
    uint32_t EventMiscFlags;                        ///< [in][release] Event flags for future use
    void* pReserved;                                ///< [in][release] Reserved for future use
    uint64_t ReservedOutFlags;                      ///< [out] Reserved out argument for future use

} ctl_wait_property_change_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Display orientation (rotation)
typedef enum _ctl_display_orientation_t
{
    CTL_DISPLAY_ORIENTATION_0 = 0,                  ///< 0 Degree
    CTL_DISPLAY_ORIENTATION_90 = 1,                 ///< 90 Degree
    CTL_DISPLAY_ORIENTATION_180 = 2,                ///< 180 Degree
    CTL_DISPLAY_ORIENTATION_270 = 3,                ///< 270 Degree
    CTL_DISPLAY_ORIENTATION_MAX

} ctl_display_orientation_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Rectangle
typedef struct _ctl_rect_t
{
    int32_t Left;                                   ///< [in,out] Left
    int32_t Top;                                    ///< [in,out] Top
    int32_t Right;                                  ///< [in,out] Right
    int32_t Bottom;                                 ///< [in,out] Bottom

} ctl_rect_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Wait for a property change. Note that this is a blocking call
/// 
/// @details
///     - Wait for a property change in display, 3d, media etc.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlWaitForPropertyChange(
    ctl_device_adapter_handle_t hDeviceAdapter,     ///< [in][release] handle to control device adapter
    ctl_wait_property_change_args_t* pArgs          ///< [in] Argument containing information about which property changes to
                                                    ///< listen for
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Reserved function
/// 
/// @details
///     - Reserved function
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlReservedCall(
    ctl_device_adapter_handle_t hDeviceAdapter,     ///< [in][release] handle to control device adapter
    ctl_reserved_args_t* pArgs                      ///< [in] Argument containing information
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_base_interface_t
typedef struct _ctl_base_interface_t ctl_base_interface_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_range_info_t
typedef struct _ctl_property_range_info_t ctl_property_range_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_range_info_int_t
typedef struct _ctl_property_range_info_int_t ctl_property_range_info_int_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_range_info_uint_t
typedef struct _ctl_property_range_info_uint_t ctl_property_range_info_uint_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_info_boolean_t
typedef struct _ctl_property_info_boolean_t ctl_property_info_boolean_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_boolean_t
typedef struct _ctl_property_boolean_t ctl_property_boolean_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_info_enum_t
typedef struct _ctl_property_info_enum_t ctl_property_info_enum_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_enum_t
typedef struct _ctl_property_enum_t ctl_property_enum_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_info_float_t
typedef struct _ctl_property_info_float_t ctl_property_info_float_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_float_t
typedef struct _ctl_property_float_t ctl_property_float_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_info_int_t
typedef struct _ctl_property_info_int_t ctl_property_info_int_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_int_t
typedef struct _ctl_property_int_t ctl_property_int_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_info_uint_t
typedef struct _ctl_property_info_uint_t ctl_property_info_uint_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_property_uint_t
typedef struct _ctl_property_uint_t ctl_property_uint_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_base_properties_t
typedef struct _ctl_base_properties_t ctl_base_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_application_id_t
typedef struct _ctl_application_id_t ctl_application_id_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_init_args_t
typedef struct _ctl_init_args_t ctl_init_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_reserved_args_t
typedef struct _ctl_reserved_args_t ctl_reserved_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_reserved_args_base_t
typedef struct _ctl_reserved_args_base_t ctl_reserved_args_base_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_unlock_capability_t
typedef struct _ctl_unlock_capability_t ctl_unlock_capability_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_runtime_path_args_t
typedef struct _ctl_runtime_path_args_t ctl_runtime_path_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_firmware_version_t
typedef struct _ctl_firmware_version_t ctl_firmware_version_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_adapter_bdf_t
typedef struct _ctl_adapter_bdf_t ctl_adapter_bdf_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_device_adapter_properties_t
typedef struct _ctl_device_adapter_properties_t ctl_device_adapter_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_generic_void_datatype_t
typedef struct _ctl_generic_void_datatype_t ctl_generic_void_datatype_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_revision_datatype_t
typedef struct _ctl_revision_datatype_t ctl_revision_datatype_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_wait_property_change_args_t
typedef struct _ctl_wait_property_change_args_t ctl_wait_property_change_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_rect_t
typedef struct _ctl_rect_t ctl_rect_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_endurance_gaming_caps_t
typedef struct _ctl_endurance_gaming_caps_t ctl_endurance_gaming_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_endurance_gaming_t
typedef struct _ctl_endurance_gaming_t ctl_endurance_gaming_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_endurance_gaming2_t
typedef struct _ctl_endurance_gaming2_t ctl_endurance_gaming2_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_adaptivesync_caps_t
typedef struct _ctl_adaptivesync_caps_t ctl_adaptivesync_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_adaptivesync_getset_t
typedef struct _ctl_adaptivesync_getset_t ctl_adaptivesync_getset_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_3d_app_profiles_caps_t
typedef struct _ctl_3d_app_profiles_caps_t ctl_3d_app_profiles_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_3d_app_profiles_t
typedef struct _ctl_3d_app_profiles_t ctl_3d_app_profiles_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_3d_tier_details_t
typedef struct _ctl_3d_tier_details_t ctl_3d_tier_details_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_3d_feature_details_t
typedef struct _ctl_3d_feature_details_t ctl_3d_feature_details_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_3d_feature_caps_t
typedef struct _ctl_3d_feature_caps_t ctl_3d_feature_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_3d_feature_getset_t
typedef struct _ctl_3d_feature_getset_t ctl_3d_feature_getset_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_kmd_load_features_t
typedef struct _ctl_kmd_load_features_t ctl_kmd_load_features_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_display_timing_t
typedef struct _ctl_display_timing_t ctl_display_timing_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_display_properties_t
typedef struct _ctl_display_properties_t ctl_display_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_adapter_display_encoder_properties_t
typedef struct _ctl_adapter_display_encoder_properties_t ctl_adapter_display_encoder_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_sharpness_filter_properties_t
typedef struct _ctl_sharpness_filter_properties_t ctl_sharpness_filter_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_sharpness_caps_t
typedef struct _ctl_sharpness_caps_t ctl_sharpness_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_sharpness_settings_t
typedef struct _ctl_sharpness_settings_t ctl_sharpness_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_i2c_access_args_t
typedef struct _ctl_i2c_access_args_t ctl_i2c_access_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_i2c_access_pinpair_args_t
typedef struct _ctl_i2c_access_pinpair_args_t ctl_i2c_access_pinpair_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_aux_access_args_t
typedef struct _ctl_aux_access_args_t ctl_aux_access_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_optimization_caps_t
typedef struct _ctl_power_optimization_caps_t ctl_power_optimization_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_optimization_lrr_t
typedef struct _ctl_power_optimization_lrr_t ctl_power_optimization_lrr_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_optimization_psr_t
typedef struct _ctl_power_optimization_psr_t ctl_power_optimization_psr_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_optimization_dpst_t
typedef struct _ctl_power_optimization_dpst_t ctl_power_optimization_dpst_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_optimization_settings_t
typedef struct _ctl_power_optimization_settings_t ctl_power_optimization_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_set_brightness_t
typedef struct _ctl_set_brightness_t ctl_set_brightness_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_get_brightness_t
typedef struct _ctl_get_brightness_t ctl_get_brightness_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_color_primaries_t
typedef struct _ctl_pixtx_color_primaries_t ctl_pixtx_color_primaries_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_pixel_format_t
typedef struct _ctl_pixtx_pixel_format_t ctl_pixtx_pixel_format_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_1dlut_config_t
typedef struct _ctl_pixtx_1dlut_config_t ctl_pixtx_1dlut_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_matrix_config_t
typedef struct _ctl_pixtx_matrix_config_t ctl_pixtx_matrix_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_3dlut_sample_t
typedef struct _ctl_pixtx_3dlut_sample_t ctl_pixtx_3dlut_sample_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_3dlut_config_t
typedef struct _ctl_pixtx_3dlut_config_t ctl_pixtx_3dlut_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_block_config_t
typedef struct _ctl_pixtx_block_config_t ctl_pixtx_block_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_pipe_get_config_t
typedef struct _ctl_pixtx_pipe_get_config_t ctl_pixtx_pipe_get_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pixtx_pipe_set_config_t
typedef struct _ctl_pixtx_pipe_set_config_t ctl_pixtx_pipe_set_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_panel_descriptor_access_args_t
typedef struct _ctl_panel_descriptor_access_args_t ctl_panel_descriptor_access_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_retro_scaling_settings_t
typedef struct _ctl_retro_scaling_settings_t ctl_retro_scaling_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_retro_scaling_caps_t
typedef struct _ctl_retro_scaling_caps_t ctl_retro_scaling_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_scaling_caps_t
typedef struct _ctl_scaling_caps_t ctl_scaling_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_scaling_settings_t
typedef struct _ctl_scaling_settings_t ctl_scaling_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_lace_lux_aggr_map_entry_t
typedef struct _ctl_lace_lux_aggr_map_entry_t ctl_lace_lux_aggr_map_entry_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_lace_lux_aggr_map_t
typedef struct _ctl_lace_lux_aggr_map_t ctl_lace_lux_aggr_map_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_lace_config_t
typedef struct _ctl_lace_config_t ctl_lace_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_sw_psr_settings_t
typedef struct _ctl_sw_psr_settings_t ctl_sw_psr_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_intel_arc_sync_monitor_params_t
typedef struct _ctl_intel_arc_sync_monitor_params_t ctl_intel_arc_sync_monitor_params_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_mux_properties_t
typedef struct _ctl_mux_properties_t ctl_mux_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_intel_arc_sync_profile_params_t
typedef struct _ctl_intel_arc_sync_profile_params_t ctl_intel_arc_sync_profile_params_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_edid_management_args_t
typedef struct _ctl_edid_management_args_t ctl_edid_management_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_get_set_custom_mode_args_t
typedef struct _ctl_get_set_custom_mode_args_t ctl_get_set_custom_mode_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_custom_src_mode_t
typedef struct _ctl_custom_src_mode_t ctl_custom_src_mode_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_child_display_target_mode_t
typedef struct _ctl_child_display_target_mode_t ctl_child_display_target_mode_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_combined_display_child_info_t
typedef struct _ctl_combined_display_child_info_t ctl_combined_display_child_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_combined_display_args_t
typedef struct _ctl_combined_display_args_t ctl_combined_display_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_genlock_display_info_t
typedef struct _ctl_genlock_display_info_t ctl_genlock_display_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_genlock_target_mode_list_t
typedef struct _ctl_genlock_target_mode_list_t ctl_genlock_target_mode_list_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_genlock_topology_t
typedef struct _ctl_genlock_topology_t ctl_genlock_topology_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_genlock_args_t
typedef struct _ctl_genlock_args_t ctl_genlock_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_vblank_ts_args_t
typedef struct _ctl_vblank_ts_args_t ctl_vblank_ts_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_lda_args_t
typedef struct _ctl_lda_args_t ctl_lda_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_dce_args_t
typedef struct _ctl_dce_args_t ctl_dce_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_engine_properties_t
typedef struct _ctl_engine_properties_t ctl_engine_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_engine_stats_t
typedef struct _ctl_engine_stats_t ctl_engine_stats_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_fan_speed_t
typedef struct _ctl_fan_speed_t ctl_fan_speed_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_fan_temp_speed_t
typedef struct _ctl_fan_temp_speed_t ctl_fan_temp_speed_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_fan_speed_table_t
typedef struct _ctl_fan_speed_table_t ctl_fan_speed_table_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_fan_properties_t
typedef struct _ctl_fan_properties_t ctl_fan_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_fan_config_t
typedef struct _ctl_fan_config_t ctl_fan_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_freq_properties_t
typedef struct _ctl_freq_properties_t ctl_freq_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_freq_range_t
typedef struct _ctl_freq_range_t ctl_freq_range_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_freq_state_t
typedef struct _ctl_freq_state_t ctl_freq_state_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_freq_throttle_time_t
typedef struct _ctl_freq_throttle_time_t ctl_freq_throttle_time_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_super_resolution_info_t
typedef struct _ctl_video_processing_super_resolution_info_t ctl_video_processing_super_resolution_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_super_resolution_t
typedef struct _ctl_video_processing_super_resolution_t ctl_video_processing_super_resolution_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_noise_reduction_info_t
typedef struct _ctl_video_processing_noise_reduction_info_t ctl_video_processing_noise_reduction_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_noise_reduction_t
typedef struct _ctl_video_processing_noise_reduction_t ctl_video_processing_noise_reduction_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_adaptive_contrast_enhancement_info_t
typedef struct _ctl_video_processing_adaptive_contrast_enhancement_info_t ctl_video_processing_adaptive_contrast_enhancement_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_adaptive_contrast_enhancement_t
typedef struct _ctl_video_processing_adaptive_contrast_enhancement_t ctl_video_processing_adaptive_contrast_enhancement_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_standard_color_correction_info_t
typedef struct _ctl_video_processing_standard_color_correction_info_t ctl_video_processing_standard_color_correction_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_standard_color_correction_t
typedef struct _ctl_video_processing_standard_color_correction_t ctl_video_processing_standard_color_correction_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_total_color_correction_info_t
typedef struct _ctl_video_processing_total_color_correction_info_t ctl_video_processing_total_color_correction_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_total_color_correction_t
typedef struct _ctl_video_processing_total_color_correction_t ctl_video_processing_total_color_correction_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_feature_details_t
typedef struct _ctl_video_processing_feature_details_t ctl_video_processing_feature_details_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_feature_caps_t
typedef struct _ctl_video_processing_feature_caps_t ctl_video_processing_feature_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_video_processing_feature_getset_t
typedef struct _ctl_video_processing_feature_getset_t ctl_video_processing_feature_getset_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_mem_properties_t
typedef struct _ctl_mem_properties_t ctl_mem_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_mem_state_t
typedef struct _ctl_mem_state_t ctl_mem_state_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_mem_bandwidth_t
typedef struct _ctl_mem_bandwidth_t ctl_mem_bandwidth_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_oc_telemetry_item_t
typedef struct _ctl_oc_telemetry_item_t ctl_oc_telemetry_item_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_oc_control_info_t
typedef struct _ctl_oc_control_info_t ctl_oc_control_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_oc_properties_t
typedef struct _ctl_oc_properties_t ctl_oc_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_oc_vf_pair_t
typedef struct _ctl_oc_vf_pair_t ctl_oc_vf_pair_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_psu_info_t
typedef struct _ctl_psu_info_t ctl_psu_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_telemetry_t
typedef struct _ctl_power_telemetry_t ctl_power_telemetry_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pci_address_t
typedef struct _ctl_pci_address_t ctl_pci_address_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pci_speed_t
typedef struct _ctl_pci_speed_t ctl_pci_speed_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pci_properties_t
typedef struct _ctl_pci_properties_t ctl_pci_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_pci_state_t
typedef struct _ctl_pci_state_t ctl_pci_state_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_properties_t
typedef struct _ctl_power_properties_t ctl_power_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_energy_counter_t
typedef struct _ctl_power_energy_counter_t ctl_power_energy_counter_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_sustained_limit_t
typedef struct _ctl_power_sustained_limit_t ctl_power_sustained_limit_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_burst_limit_t
typedef struct _ctl_power_burst_limit_t ctl_power_burst_limit_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_peak_limit_t
typedef struct _ctl_power_peak_limit_t ctl_power_peak_limit_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_power_limits_t
typedef struct _ctl_power_limits_t ctl_power_limits_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_energy_threshold_t
typedef struct _ctl_energy_threshold_t ctl_energy_threshold_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Forward-declare ctl_temp_properties_t
typedef struct _ctl_temp_properties_t ctl_temp_properties_t;



#if !defined(__GNUC__)
#pragma endregion // common
#endif
// Intel 'ctlApi' for Device Adapter
#if !defined(__GNUC__)
#pragma region _3D
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Feature type
typedef enum _ctl_3d_feature_t
{
    CTL_3D_FEATURE_FRAME_PACING = 0,                ///< Frame pacing. Contains generic enum type fields
    CTL_3D_FEATURE_ENDURANCE_GAMING = 1,            ///< Endurance gaming. Contains generic integer type fields. Value will be
                                                    ///< interpreted as the max FPS to be used when in DC mode globally or per
                                                    ///< application
    CTL_3D_FEATURE_FRAME_LIMIT = 2,                 ///< Frame limit for games. Contains generic integer type fields. Value
                                                    ///< will be interpreted as the max FPS to be used independent of system
                                                    ///< power state
    CTL_3D_FEATURE_ANISOTROPIC = 3,                 ///< ANISOTROPIC. Contains generic enum type fields
    CTL_3D_FEATURE_CMAA = 4,                        ///< CMAA. Contains generic enum type fields
    CTL_3D_FEATURE_TEXTURE_FILTERING_QUALITY = 5,   ///< Texture filtering quality. Contains generic enum type fields
    CTL_3D_FEATURE_ADAPTIVE_TESSELLATION = 6,       ///< Adaptive tessellation quality. Contains generic integer type fields
    CTL_3D_FEATURE_SHARPENING_FILTER = 7,           ///< Sharpening Filter. Contains generic integer type fields
    CTL_3D_FEATURE_MSAA = 8,                        ///< Msaa. Contains generic enum type fields
    CTL_3D_FEATURE_GAMING_FLIP_MODES = 9,           ///< Various Gaming flip modes like speed frame, smooth sync & force async
                                                    ///< flip. Contains generic enum type fields
    CTL_3D_FEATURE_ADAPTIVE_SYNC_PLUS = 10,         ///< Adaptive sync plus. Refer custom field ::ctl_adaptivesync_caps_t &
                                                    ///< ::ctl_adaptivesync_getset_t
    CTL_3D_FEATURE_APP_PROFILES = 11,               ///< Game Compatibility & Performance Profiles. Refer custom field
                                                    ///< ::ctl_3d_app_profiles_caps_t & ::ctl_3d_app_profiles_t
    CTL_3D_FEATURE_APP_PROFILE_DETAILS = 12,        ///< Game Profile Customization. Refer custom field ::ctl_3d_tier_details_t
    CTL_3D_FEATURE_EMULATED_TYPED_64BIT_ATOMICS = 13,   ///< Emulated Typed 64bit Atomics support in DG2
    CTL_3D_FEATURE_VRR_WINDOWED_BLT = 14,           ///< VRR windowed blt. Control VRR for windowed mode game
    CTL_3D_FEATURE_GLOBAL_OR_PER_APP = 15,          ///< Set global settings or per application settings
    CTL_3D_FEATURE_MAX

} ctl_3d_feature_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief 3D feature misc flags
typedef uint32_t ctl_3d_feature_misc_flags_t;
typedef enum _ctl_3d_feature_misc_flag_t
{
    CTL_3D_FEATURE_MISC_FLAG_DX11 = CTL_BIT(0),     ///< Feature supported on DX11
    CTL_3D_FEATURE_MISC_FLAG_DX12 = CTL_BIT(1),     ///< Feature supported on DX12
    CTL_3D_FEATURE_MISC_FLAG_VULKAN = CTL_BIT(2),   ///< Feature supported on VULKAN
    CTL_3D_FEATURE_MISC_FLAG_LIVE_CHANGE = CTL_BIT(3),  ///< User can change feature live without restarting the game
    CTL_3D_FEATURE_MISC_FLAG_MAX = 0x80000000

} ctl_3d_feature_misc_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Anisotropic values possible
typedef enum _ctl_3d_anisotropic_types_t
{
    CTL_3D_ANISOTROPIC_TYPES_APP_CHOICE = 0,        ///< Application choice
    CTL_3D_ANISOTROPIC_TYPES_2X = 2,                ///< 2X
    CTL_3D_ANISOTROPIC_TYPES_4X = 4,                ///< 4X
    CTL_3D_ANISOTROPIC_TYPES_8X = 8,                ///< 8X
    CTL_3D_ANISOTROPIC_TYPES_16X = 16,              ///< 16X
    CTL_3D_ANISOTROPIC_TYPES_MAX

} ctl_3d_anisotropic_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Texture filtering values possible
typedef enum _ctl_3d_texture_filtering_quality_types_t
{
    CTL_3D_TEXTURE_FILTERING_QUALITY_TYPES_PERFORMANCE = 0, ///< Performance
    CTL_3D_TEXTURE_FILTERING_QUALITY_TYPES_BALANCED = 1,///< Balanced
    CTL_3D_TEXTURE_FILTERING_QUALITY_TYPES_QUALITY = 2, ///< Quality
    CTL_3D_TEXTURE_FILTERING_QUALITY_TYPES_MAX

} ctl_3d_texture_filtering_quality_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Frame pacing values possible
typedef enum _ctl_3d_frame_pacing_types_t
{
    CTL_3D_FRAME_PACING_TYPES_DISABLE = 0,          ///< Disable
    CTL_3D_FRAME_PACING_TYPES_ENABLE_MODE_FRAME_NO_SMOOTHENING = 1, ///< Enable with scheduler without any frame smoothening
    CTL_3D_FRAME_PACING_TYPES_ENABLE_MODE_FRAME_MAX_SMOOTHENING = 2,///< Enable with scheduler with maximum smoothness
    CTL_3D_FRAME_PACING_TYPES_ENABLE_MODE_COMPETITIVE_GAMING = 3,   ///< Enable with scheduler in competitive gaming mode
    CTL_3D_FRAME_PACING_TYPES_MAX

} ctl_3d_frame_pacing_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Endurance Gaming control possible
typedef enum _ctl_3d_endurance_gaming_control_t
{
    CTL_3D_ENDURANCE_GAMING_CONTROL_TURN_OFF = 0,   ///< Endurance Gaming disable
    CTL_3D_ENDURANCE_GAMING_CONTROL_TURN_ON = 1,    ///< Endurance Gaming enable
    CTL_3D_ENDURANCE_GAMING_CONTROL_AUTO = 2,       ///< Endurance Gaming auto
    CTL_3D_ENDURANCE_GAMING_CONTROL_MAX

} ctl_3d_endurance_gaming_control_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Endurance Gaming modes possible
typedef enum _ctl_3d_endurance_gaming_mode_t
{
    CTL_3D_ENDURANCE_GAMING_MODE_BETTER_PERFORMANCE = 0,///< Endurance Gaming better performance mode
    CTL_3D_ENDURANCE_GAMING_MODE_BALANCED = 1,      ///< Endurance Gaming balanced mode
    CTL_3D_ENDURANCE_GAMING_MODE_MAXIMUM_BATTERY = 2,   ///< Endurance Gaming maximum battery mode
    CTL_3D_ENDURANCE_GAMING_MODE_MAX

} ctl_3d_endurance_gaming_mode_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Cmaa values possible
typedef enum _ctl_3d_cmaa_types_t
{
    CTL_3D_CMAA_TYPES_TURN_OFF = 0,                 ///< Turn off
    CTL_3D_CMAA_TYPES_OVERRIDE_MSAA = 1,            ///< Override MSAA
    CTL_3D_CMAA_TYPES_ENHANCE_APPLICATION = 2,      ///< Enhance Application
    CTL_3D_CMAA_TYPES_MAX

} ctl_3d_cmaa_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adaptive Tessellation
typedef enum _ctl_3d_adaptive_tessellation_types_t
{
    CTL_3D_ADAPTIVE_TESSELLATION_TYPES_TURN_OFF = 0,///< Turn off
    CTL_3D_ADAPTIVE_TESSELLATION_TYPES_TURN_ON = 1, ///< Turn on
    CTL_3D_ADAPTIVE_TESSELLATION_TYPES_MAX

} ctl_3d_adaptive_tessellation_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Sharpening filter values possible
typedef enum _ctl_3d_sharpening_filter_types_t
{
    CTL_3D_SHARPENING_FILTER_TYPES_TURN_OFF = 0,    ///< Turn off
    CTL_3D_SHARPENING_FILTER_TYPES_TURN_ON = 1,     ///< Turn on
    CTL_3D_SHARPENING_FILTER_TYPES_MAX

} ctl_3d_sharpening_filter_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief MSAA values possible
typedef enum _ctl_3d_msaa_types_t
{
    CTL_3D_MSAA_TYPES_APP_CHOICE = 0,               ///< Application choice
    CTL_3D_MSAA_TYPES_DISABLED = 1,                 ///< Disabled. MSAA count 1
    CTL_3D_MSAA_TYPES_2X = 2,                       ///< 2X
    CTL_3D_MSAA_TYPES_4X = 4,                       ///< 4X
    CTL_3D_MSAA_TYPES_8X = 8,                       ///< 8X
    CTL_3D_MSAA_TYPES_16X = 16,                     ///< 16X
    CTL_3D_MSAA_TYPES_MAX

} ctl_3d_msaa_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Gaming flip modes
typedef uint32_t ctl_gaming_flip_mode_flags_t;
typedef enum _ctl_gaming_flip_mode_flag_t
{
    CTL_GAMING_FLIP_MODE_FLAG_APPLICATION_DEFAULT = CTL_BIT(0), ///< Application Default
    CTL_GAMING_FLIP_MODE_FLAG_VSYNC_OFF = CTL_BIT(1),   ///< Convert all sync flips to async on the next possible scanline.
    CTL_GAMING_FLIP_MODE_FLAG_VSYNC_ON = CTL_BIT(2),///< Convert all async flips to sync flips.
    CTL_GAMING_FLIP_MODE_FLAG_SMOOTH_SYNC = CTL_BIT(3), ///< Reduce tearing effect with async flips
    CTL_GAMING_FLIP_MODE_FLAG_SPEED_FRAME = CTL_BIT(4), ///< Application unaware triple buffering
    CTL_GAMING_FLIP_MODE_FLAG_CAPPED_FPS = CTL_BIT(5),  ///< Limit the game FPS to panel RR
    CTL_GAMING_FLIP_MODE_FLAG_MAX = 0x80000000

} ctl_gaming_flip_mode_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Endurance Gaming caps
typedef struct _ctl_endurance_gaming_caps_t
{
    ctl_property_info_enum_t EGControlCaps;         ///< [out] Endurance Gaming control capability
    ctl_property_info_enum_t EGModeCaps;            ///< [out] Endurance Gaming mode capability

} ctl_endurance_gaming_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Endurance Gaming Get/Set
typedef struct _ctl_endurance_gaming_t
{
    ctl_3d_endurance_gaming_control_t EGControl;    ///< [in,out] Endurance Gaming control - Off/On/Auto
    ctl_3d_endurance_gaming_mode_t EGMode;          ///< [in,out] Endurance Gaming mode - Better Performance/Balanced/Maximum
                                                    ///< Battery

} ctl_endurance_gaming_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Endurance Gaming version2 Get/Set
typedef struct _ctl_endurance_gaming2_t
{
    ctl_3d_endurance_gaming_control_t EGControl;    ///< [in,out] Endurance Gaming control - Off/On/Auto
    ctl_3d_endurance_gaming_mode_t EGMode;          ///< [in,out] Endurance Gaming mode - Better Performance/Balanced/Maximum
                                                    ///< Battery
    bool IsFPRequired;                              ///< [out] Is frame pacing required, dynamic state
    double TargetFPS;                               ///< [out] Target FPS for frame pacing
    double RefreshRate;                             ///< [out] Refresh rate used to calculate target fps
    uint32_t Reserved[4];                           ///< [out] Reserved fields

} ctl_endurance_gaming2_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adaptive sync plus caps
typedef struct _ctl_adaptivesync_caps_t
{
    bool AdaptiveBalanceSupported;                  ///< [out] Adaptive balance supported
    ctl_property_info_float_t AdaptiveBalanceStrengthCaps;  ///< [out] Strength of adaptive balance algorithm - min/max/steps/default

} ctl_adaptivesync_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adaptive sync plus
typedef struct _ctl_adaptivesync_getset_t
{
    bool AdaptiveSync;                              ///< [in,out] Adaptive sync. Note that in Windows, OS controls state of
                                                    ///< adaptive sync and which game gets the feature using it's own policies
    bool AdaptiveBalance;                           ///< [in,out] Adaptive balance
    bool AllowAsyncForHighFPS;                      ///< [in,out] Allow async flips when FPS is higher than max refresh rate of
                                                    ///< the panel
    float AdaptiveBalanceStrength;                  ///< [in,out] Adaptive balance strength

} ctl_adaptivesync_getset_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Game tier types
typedef uint32_t ctl_3d_tier_type_flags_t;
typedef enum _ctl_3d_tier_type_flag_t
{
    CTL_3D_TIER_TYPE_FLAG_COMPATIBILITY = CTL_BIT(0),   ///< Compatibility Tier
    CTL_3D_TIER_TYPE_FLAG_PERFORMANCE = CTL_BIT(1), ///< Performance Tier
    CTL_3D_TIER_TYPE_FLAG_MAX = 0x80000000

} ctl_3d_tier_type_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Game tiers
typedef uint32_t ctl_3d_tier_profile_flags_t;
typedef enum _ctl_3d_tier_profile_flag_t
{
    CTL_3D_TIER_PROFILE_FLAG_TIER_1 = CTL_BIT(0),   ///< Tier 1 Profile
    CTL_3D_TIER_PROFILE_FLAG_TIER_2 = CTL_BIT(1),   ///< Tier 2 Profile
    CTL_3D_TIER_PROFILE_FLAG_TIER_RECOMMENDED = CTL_BIT(30),///< Recommended Tier Profile. If set other tier values shouldn't be set
    CTL_3D_TIER_PROFILE_FLAG_MAX = 0x80000000

} ctl_3d_tier_profile_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Game Profile Capabilities. Typically these remain the same across
///        games.
typedef struct _ctl_3d_app_profiles_caps_t
{
    ctl_3d_tier_type_flags_t SupportedTierTypes;    ///< [out] Tier of interest for capability check
    uint64_t Reserved;                              ///< [in,out] Reserved for future

} ctl_3d_app_profiles_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Game Profile tiers
typedef struct _ctl_3d_app_profiles_t
{
    ctl_3d_tier_type_flag_t TierType;               ///< [in] Tier type
    ctl_3d_tier_profile_flags_t SupportedTierProfiles;  ///< [out] Supported tier profiles bitmask
    ctl_3d_tier_profile_flags_t DefaultEnabledTierProfiles; ///< [out] Default tiers which driver will enable if there is no user
                                                    ///< specific setting for global or per application
    ctl_3d_tier_profile_flags_t CustomizationSupportedTierProfiles; ///< [out] Tiers supporting customization - reserved for future
    ctl_3d_tier_profile_flags_t EnabledTierProfiles;///< [in,out] Tier profile(s) to be enabled/disabled in the case of a set
                                                    ///< call. For a get call this will return the currently enabled tiers
    ctl_3d_tier_profile_flags_t CustomizationEnabledTierProfiles;   ///< [in,out] Tier profile(s) which are customized. Caller shall call
                                                    ///< ::ctl_3d_tier_details_t to get specifics if any.
    uint64_t Reserved;                              ///< [in,out] Reserved for future

} ctl_3d_app_profiles_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Game Profile tiers details
typedef struct _ctl_3d_tier_details_t
{
    ctl_3d_tier_type_flag_t TierType;               ///< [in] Tier type
    ctl_3d_tier_profile_flag_t TierProfile;         ///< [in] Tier profile(s) for get/set details
    uint64_t Reserved[4];                           ///< [in,out] Reserved for future

} ctl_3d_tier_details_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Emulated Typed 64bit Atomics
typedef enum _ctl_emulated_typed_64bit_atomics_types_t
{
    CTL_EMULATED_TYPED_64BIT_ATOMICS_TYPES_DEFAULT = 0, ///< Default settings is based on workload/driver decision.
    CTL_EMULATED_TYPED_64BIT_ATOMICS_TYPES_TURN_ON = 1, ///< Force Turn on
    CTL_EMULATED_TYPED_64BIT_ATOMICS_TYPES_TURN_OFF = 2,///< Force Turn off
    CTL_EMULATED_TYPED_64BIT_ATOMICS_TYPES_MAX

} ctl_emulated_typed_64bit_atomics_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief VRR windowed BLT control possible. Reserved functionality
typedef enum _ctl_3d_vrr_windowed_blt_reserved_t
{
    CTL_3D_VRR_WINDOWED_BLT_RESERVED_AUTO = 0,      ///< VRR windowed BLT auto
    CTL_3D_VRR_WINDOWED_BLT_RESERVED_TURN_ON = 1,   ///< VRR windowed BLT enable
    CTL_3D_VRR_WINDOWED_BLT_RESERVED_TURN_OFF = 2,  ///< VRR windowed BLT disable
    CTL_3D_VRR_WINDOWED_BLT_RESERVED_MAX

} ctl_3d_vrr_windowed_blt_reserved_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Global or per app values possible
typedef enum _ctl_3d_global_or_per_app_types_t
{
    CTL_3D_GLOBAL_OR_PER_APP_TYPES_NONE = 0,        ///< none
    CTL_3D_GLOBAL_OR_PER_APP_TYPES_PER_APP = 1,     ///< Opt for per app settings
    CTL_3D_GLOBAL_OR_PER_APP_TYPES_GLOBAL = 2,      ///< Opt for global settings
    CTL_3D_GLOBAL_OR_PER_APP_TYPES_MAX

} ctl_3d_global_or_per_app_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief 3D feature capability details which will have range/supported and
///        default values
typedef struct _ctl_3d_feature_details_t
{
    ctl_3d_feature_t FeatureType;                   ///< [out] 3D feature type
    ctl_property_value_type_t ValueType;            ///< [out] Type of value
    ctl_property_info_t Value;                      ///< [out] Union of various type of values for 3D features. For enum types
                                                    ///< this can be anisotropic/frame pacing etc. This member is valid iff
                                                    ///< ValueType is not CTL_PROPERTY_VALUE_TYPE_CUSTOM
    int32_t CustomValueSize;                        ///< [in] CustomValue buffer size. Typically for a feature requiring custom
                                                    ///< struct, caller will know of it upfront and can provide the right size
                                                    ///< info here
    void* pCustomValue;                             ///< [in,out] Pointer to a custom structure. Caller should allocate this
                                                    ///< buffer with known custom feature structure size. This member is valid
                                                    ///< iff ValueType is CTL_PROPERTY_VALUE_TYPE_CUSTOM
    bool PerAppSupport;                             ///< [out] Flag indicating whether the feature is supported per application
                                                    ///< or not
    int64_t ConflictingFeatures;                    ///< [out] Mask of ::ctl_3d_feature_t values which can't be enabled along
                                                    ///< with the mentioned FeatureType. If this is 0, it meant the feature
                                                    ///< doesn't have any conflicts with other features
    int16_t FeatureMiscSupport;                     ///< [out] 3D Feature Miscellaneous support flags. This will be based on
                                                    ///< ::ctl_3d_feature_misc_flags_t
    int16_t Reserved;                               ///< [out] Reserved
    int16_t Reserved1;                              ///< [out] Reserved
    int16_t Reserved2;                              ///< [out] Reserved

} ctl_3d_feature_details_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief 3D feature which are controllable
typedef struct _ctl_3d_feature_caps_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t NumSupportedFeatures;                  ///< [in,out] Number of elements in supported features array
    ctl_3d_feature_details_t* pFeatureDetails;      ///< [in,out] Array of feature details

} ctl_3d_feature_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief 3D feature for get/set
typedef struct _ctl_3d_feature_getset_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_3d_feature_t FeatureType;                   ///< [in] Features interested in
    char* ApplicationName;                          ///< [in] Application name for which the property type is applicable. If
                                                    ///< this is an empty string then this will get/set global settings for the
                                                    ///< given adapter. Note that this should contain only the name of the
                                                    ///< application and not the system specific path
    int8_t ApplicationNameLength;                   ///< [in] Length of ApplicationName string
    bool bSet;                                      ///< [in] Set this if it's a set call
    ctl_property_value_type_t ValueType;            ///< [in] Type of value. Caller has to ensure it provides the right value
                                                    ///< type which decides how one read the union structure below
    ctl_property_t Value;                           ///< [in,out] Union of various type of values for 3D features. For enum
                                                    ///< types this can be anisotropic/frame pacing etc. This member is valid
                                                    ///< iff ValueType is not CTL_PROPERTY_VALUE_TYPE_CUSTOM
    int32_t CustomValueSize;                        ///< [in] CustomValue buffer size. Typically for a feature requiring custom
                                                    ///< struct, caller will know of it upfront and cn provide the right size
                                                    ///< info here
    void* pCustomValue;                             ///< [in,out] Pointer to a custom structure. Caller should allocate this
                                                    ///< buffer with known custom feature structure size. This member is valid
                                                    ///< iff ValueType is CTL_PROPERTY_VALUE_TYPE_CUSTOM

} ctl_3d_feature_getset_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Load KMD gaming features. Restricted function
typedef struct _ctl_kmd_load_features_t
{
    ctl_application_id_t ReservedFuncID;            ///< [in] Unique ID for reserved/special function
    bool bLoad;                                     ///< [in] If set, will load known KMD features. If not set will reset known
                                                    ///< KMD features to default
    int64_t SubsetFeatureMask;                      ///< [in,out] Mask indicating the subset of KMD features within
                                                    ///< ::ctl_3d_feature_t values. Default of 0 indicate all KMD features
    char* ApplicationName;                          ///< [in] Application name for which the KMD properties are loaded for. If
                                                    ///< this is an empty string then this will load global settings for the
                                                    ///< given adapter. Note that this should contain only the name of the
                                                    ///< application and not the system specific path
    int8_t ApplicationNameLength;                   ///< [in] Length of ApplicationName string
    int8_t CallerComponent;                         ///< [in] Caller component
    int64_t Reserved[4];                            ///< [in] Reserved field

} ctl_kmd_load_features_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get 3D capabilities
/// 
/// @details
///     - The application gets 3D properties
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pFeatureCaps`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSupported3DCapabilities(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    ctl_3d_feature_caps_t* pFeatureCaps             ///< [in,out][release] 3D properties
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set 3D feature
/// 
/// @details
///     - 3D feature details
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pFeature`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSet3DFeature(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    ctl_3d_feature_getset_t* pFeature               ///< [in][release] 3D feature get/set parameter
    );


#if !defined(__GNUC__)
#pragma endregion // _3D
#endif
// Intel 'ctlApi' for Device Adapter
#if !defined(__GNUC__)
#pragma region display
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a display output instance
typedef struct _ctl_display_output_handle_t *ctl_display_output_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a i2c pin-pair instance
typedef struct _ctl_i2c_pin_pair_handle_t *ctl_i2c_pin_pair_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Check Driver version
/// 
/// @details
///     - The application checks driver version
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceAdapter`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlCheckDriverVersion(
    ctl_device_adapter_handle_t hDeviceAdapter,     ///< [in][release] handle to control device adapter
    ctl_version_info_t version_info                 ///< [in][release] Driver version info
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumerate devices
/// 
/// @details
///     - The application enumerates all device adapters in the system
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hAPIHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumerateDevices(
    ctl_api_handle_t hAPIHandle,                    ///< [in][release] Applications should pass the Control API handle returned
                                                    ///< by the CtlInit function 
    uint32_t* pCount,                               ///< [in,out][release] pointer to the number of device instances. If count
                                                    ///< is zero, then the api will update the value with the total
                                                    ///< number of drivers available. If count is non-zero, then the api will
                                                    ///< only retrieve the number of drivers.
                                                    ///< If count is larger than the number of drivers available, then the api
                                                    ///< will update the value with the correct number of drivers available.
    ctl_device_adapter_handle_t* phDevices          ///< [in,out][optional][release][range(0, *pCount)] array of driver
                                                    ///< instance handles
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumerate display outputs
/// 
/// @details
///     - Enumerates display output capabilities
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumerateDisplayOutputs(
    ctl_device_adapter_handle_t hDeviceAdapter,     ///< [in][release] handle to control device adapter
    uint32_t* pCount,                               ///< [in,out][release] pointer to the number of display output instances.
                                                    ///< If count is zero, then the api will update the value with the total
                                                    ///< number of outputs available. If count is non-zero, then the api will
                                                    ///< only retrieve the number of outputs.
                                                    ///< If count is larger than the number of drivers available, then the api
                                                    ///< will update the value with the correct number of drivers available.
    ctl_display_output_handle_t* phDisplayOutputs   ///< [in,out][optional][release][range(0, *pCount)] array of display output
                                                    ///< instance handles
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumerate I2C Pin Pairs
/// 
/// @details
///     - Returns available list of I2C Pin-Pairs on a requested adapter
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "The incoming pointer pCount is null"
///     - ::CTL_RESULT_ERROR_INVALID_SIZE - "The supplied Count is not equal to actual number of i2c pin-pair instances"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumerateI2CPinPairs(
    ctl_device_adapter_handle_t hDeviceAdapter,     ///< [in][release] handle to device adapter
    uint32_t* pCount,                               ///< [in,out][release] pointer to the number of i2c pin-pair instances. If
                                                    ///< count is zero, then the api will update the value with the total
                                                    ///< number of i2c pin-pair instances available. If count is non-zero and
                                                    ///< matches the avaialble number of pin-pairs, then the api will only
                                                    ///< return the avaialble number of i2c pin-pair instances in phI2cPinPairs.
    ctl_i2c_pin_pair_handle_t* phI2cPinPairs        ///< [out][optional][release][range(0, *pCount)] array of i2c pin pair
                                                    ///< instance handles. Need to be allocated by Caller when supplying the
                                                    ///< *pCount > 0. 
                                                    ///< If Count is not equal to actual number of i2c pin-pair instances, it
                                                    ///< will return CTL_RESULT_ERROR_INVALID_SIZE.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief OS specific Display identifiers
typedef union _ctl_os_display_encoder_identifier_t
{
    uint32_t WindowsDisplayEncoderID;               ///< [out] Windows OS Display encoder ID
    ctl_generic_void_datatype_t DisplayEncoderID;   ///< [out] Display encoder ID for non-windows OS

} ctl_os_display_encoder_identifier_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Various display types
typedef enum _ctl_display_output_types_t
{
    CTL_DISPLAY_OUTPUT_TYPES_INVALID = 0,           ///< Invalid
    CTL_DISPLAY_OUTPUT_TYPES_DISPLAYPORT = 1,       ///< DisplayPort
    CTL_DISPLAY_OUTPUT_TYPES_HDMI = 2,              ///< HDMI
    CTL_DISPLAY_OUTPUT_TYPES_DVI = 3,               ///< DVI
    CTL_DISPLAY_OUTPUT_TYPES_MIPI = 4,              ///< MIPI
    CTL_DISPLAY_OUTPUT_TYPES_CRT = 5,               ///< CRT
    CTL_DISPLAY_OUTPUT_TYPES_MAX

} ctl_display_output_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Supported output bits per color (bpc) bitmasks
typedef uint32_t ctl_output_bpc_flags_t;
typedef enum _ctl_output_bpc_flag_t
{
    CTL_OUTPUT_BPC_FLAG_6BPC = CTL_BIT(0),          ///< [out] Is 6bpc supported
    CTL_OUTPUT_BPC_FLAG_8BPC = CTL_BIT(1),          ///< [out] Is 8bpc supported
    CTL_OUTPUT_BPC_FLAG_10BPC = CTL_BIT(2),         ///< [out] Is 10bpc supported
    CTL_OUTPUT_BPC_FLAG_12BPC = CTL_BIT(3),         ///< [out] Is 12bpc supported
    CTL_OUTPUT_BPC_FLAG_MAX = 0x80000000

} ctl_output_bpc_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Display output features. This will indicate only the high level
///        capabilities
typedef uint32_t ctl_std_display_feature_flags_t;
typedef enum _ctl_std_display_feature_flag_t
{
    CTL_STD_DISPLAY_FEATURE_FLAG_HDCP = CTL_BIT(0), ///< [out] Is HDCP supported
    CTL_STD_DISPLAY_FEATURE_FLAG_HD_AUDIO = CTL_BIT(1), ///< [out] Is HD Audio supported
    CTL_STD_DISPLAY_FEATURE_FLAG_PSR = CTL_BIT(2),  ///< [out] Is VESA PSR supported
    CTL_STD_DISPLAY_FEATURE_FLAG_ADAPTIVESYNC_VRR = CTL_BIT(3), ///< [out] Is VESA Adaptive Sync or HDMI VRR supported
    CTL_STD_DISPLAY_FEATURE_FLAG_VESA_COMPRESSION = CTL_BIT(4), ///< [out] Is display compression (VESA DSC) supported
    CTL_STD_DISPLAY_FEATURE_FLAG_HDR = CTL_BIT(5),  ///< [out] Is HDR supported
    CTL_STD_DISPLAY_FEATURE_FLAG_HDMI_QMS = CTL_BIT(6), ///< [out] Is HDMI QMS supported
    CTL_STD_DISPLAY_FEATURE_FLAG_MAX = 0x80000000

} ctl_std_display_feature_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Advanced Graphics Features provided by Intel Graphics Adapter. This
///        will indicate only the high level capabilities
typedef uint32_t ctl_intel_display_feature_flags_t;
typedef enum _ctl_intel_display_feature_flag_t
{
    CTL_INTEL_DISPLAY_FEATURE_FLAG_DPST = CTL_BIT(0),   ///< [out] Is DPST supported
    CTL_INTEL_DISPLAY_FEATURE_FLAG_LACE = CTL_BIT(1),   ///< [out] Is LACE supported
    CTL_INTEL_DISPLAY_FEATURE_FLAG_DRRS = CTL_BIT(2),   ///< [out] Is DRRS supported
    CTL_INTEL_DISPLAY_FEATURE_FLAG_MAX = 0x80000000

} ctl_intel_display_feature_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Attached Display Mux Type
typedef enum _ctl_attached_display_mux_type_t
{
    CTL_ATTACHED_DISPLAY_MUX_TYPE_NATIVE = 0,       ///< [out] Native DP / HDMI 
    CTL_ATTACHED_DISPLAY_MUX_TYPE_THUNDERBOLT = 1,  ///< [out] Thunderbolt 
    CTL_ATTACHED_DISPLAY_MUX_TYPE_TYPE_C = 2,       ///< [out] USB Type C  
    CTL_ATTACHED_DISPLAY_MUX_TYPE_USB4 = 3,         ///< [out] USB4 
    CTL_ATTACHED_DISPLAY_MUX_TYPE_MAX

} ctl_attached_display_mux_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Signal Standard
typedef enum _ctl_signal_standard_type_t
{
    CTL_SIGNAL_STANDARD_TYPE_UNKNOWN = 0,           ///< [out] Unknown Signal Standard 
    CTL_SIGNAL_STANDARD_TYPE_CUSTOM = 1,            ///< [out] Custom added timing 
    CTL_SIGNAL_STANDARD_TYPE_DMT = 2,               ///< [out] DMT timing  
    CTL_SIGNAL_STANDARD_TYPE_GTF = 3,               ///< [out] GTF Timing 
    CTL_SIGNAL_STANDARD_TYPE_CVT = 4,               ///< [out] CVT Timing 
    CTL_SIGNAL_STANDARD_TYPE_CTA = 5,               ///< [out] CTA Timing 
    CTL_SIGNAL_STANDARD_TYPE_MAX

} ctl_signal_standard_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Protocol Converter Location
typedef uint32_t ctl_protocol_converter_location_flags_t;
typedef enum _ctl_protocol_converter_location_flag_t
{
    CTL_PROTOCOL_CONVERTER_LOCATION_FLAG_ONBOARD = CTL_BIT(0),  ///< [out] OnBoard Protocol Converter
    CTL_PROTOCOL_CONVERTER_LOCATION_FLAG_EXTERNAL = CTL_BIT(1), ///< [out] External Dongle
    CTL_PROTOCOL_CONVERTER_LOCATION_FLAG_MAX = 0x80000000

} ctl_protocol_converter_location_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief [out] Display Output configuration related flags which indicate how
///        the output pixel stream drive the panel
typedef uint32_t ctl_display_config_flags_t;
typedef enum _ctl_display_config_flag_t
{
    CTL_DISPLAY_CONFIG_FLAG_DISPLAY_ACTIVE = CTL_BIT(0),///< [out] DisplayActive 0: InActive 1: Active 
    CTL_DISPLAY_CONFIG_FLAG_DISPLAY_ATTACHED = CTL_BIT(1),  ///< [out] DisplayAttached.This Bit indicates if any dongle/display/hub is
                                                    ///< attached to the encoder. 0: Not Attached 1: Attached 
    CTL_DISPLAY_CONFIG_FLAG_IS_DONGLE_CONNECTED_TO_ENCODER = CTL_BIT(2),///< [out] This BIT will be set if a dongle/hub/onboard protocol converter
                                                    ///< , is attached to the encoder
    CTL_DISPLAY_CONFIG_FLAG_DITHERING_ENABLED = CTL_BIT(3), ///< [out] This BIT will be set if dithering is enabled on the encoder
    CTL_DISPLAY_CONFIG_FLAG_MAX = 0x80000000

} ctl_display_config_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief [out] Encoder configuration related flags which indicate how the
///        output pixel stream drive the panel 
typedef uint32_t ctl_encoder_config_flags_t;
typedef enum _ctl_encoder_config_flag_t
{
    CTL_ENCODER_CONFIG_FLAG_INTERNAL_DISPLAY = CTL_BIT(0),  ///< [out] Internal connection or not
    CTL_ENCODER_CONFIG_FLAG_VESA_TILED_DISPLAY = CTL_BIT(1),///< [out] VESA DisplayID based tiled display which is driven by either
                                                    ///< multiple physical connections (DisplayPort SST) or virtual streams
                                                    ///< (DisplayPort MST)
    CTL_ENCODER_CONFIG_FLAG_TYPEC_CAPABLE = CTL_BIT(2), ///< [out] This is set if encoder supports type c display 
    CTL_ENCODER_CONFIG_FLAG_TBT_CAPABLE = CTL_BIT(3),   ///< [out] This is set if encoder supports Thunderbolt display 
    CTL_ENCODER_CONFIG_FLAG_DITHERING_SUPPORTED = CTL_BIT(4),   ///< [out] This BIT will be set if encoder supports dithering
    CTL_ENCODER_CONFIG_FLAG_VIRTUAL_DISPLAY = CTL_BIT(5),   ///< [out] This BIT will be set if this is a virtual display.Hardware based
                                                    ///< features will not be applicable to this display.For collage display
                                                    ///< this will be set for the virtual output created by driver. For split
                                                    ///< display this will be set for the virtual split displays created out of
                                                    ///< one single physical display
    CTL_ENCODER_CONFIG_FLAG_HIDDEN_DISPLAY = CTL_BIT(6),///< [out] This BIT will be set if display is hidden from OS
    CTL_ENCODER_CONFIG_FLAG_COLLAGE_DISPLAY = CTL_BIT(7),   ///< [out] This BIT will be set if this is a collage display
    CTL_ENCODER_CONFIG_FLAG_SPLIT_DISPLAY = CTL_BIT(8), ///< [out] This BIT will be set if this is a split display
    CTL_ENCODER_CONFIG_FLAG_COMPANION_DISPLAY = CTL_BIT(9), ///< [out] This BIT will be set if this is a companion display
    CTL_ENCODER_CONFIG_FLAG_MGPU_COLLAGE_DISPLAY = CTL_BIT(10), ///< [out] This BIT will be set if this is a Multi GPU collage display
    CTL_ENCODER_CONFIG_FLAG_MAX = 0x80000000

} ctl_encoder_config_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Display Timing
typedef struct _ctl_display_timing_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint64_t PixelClock;                            ///< [out] Pixel Clock in Hz
    uint32_t HActive;                               ///< [out] Horizontal Active
    uint32_t VActive;                               ///< [out] Vertical Active
    uint32_t HTotal;                                ///< [out] Horizontal Total
    uint32_t VTotal;                                ///< [out] Vertical Total
    uint32_t HBlank;                                ///< [out] Horizontal Blank
    uint32_t VBlank;                                ///< [out] Vertical Blank
    uint32_t HSync;                                 ///< [out] Horizontal Blank
    uint32_t VSync;                                 ///< [out] Vertical Blank
    float RefreshRate;                              ///< [out] Refresh Rate
    ctl_signal_standard_type_t SignalStandard;      ///< [out] Signal Standard
    uint8_t VicId;                                  ///< [out] VIC ID for CTA timings

} ctl_display_timing_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief This structure will contain the properties of the display currently
///        attached to the encoder.
typedef struct _ctl_display_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_os_display_encoder_identifier_t Os_display_encoder_handle;  ///< [out] OS specific Display ID
    ctl_display_output_types_t Type;                ///< [out] Device Type from display HW stand point. If a DisplayPort
                                                    ///< protocol converter is involved, this will indicate it's DisplayPort.
                                                    ///< The protocol converter's output will be available from
                                                    ///< ProtocolConverterOutput field
    ctl_attached_display_mux_type_t AttachedDisplayMuxType; ///< [out] Attached Display Mux Type
    ctl_display_output_types_t ProtocolConverterOutput; ///< [out] Protocol output type which can be used if config flags indicate
                                                    ///< it's a protocol converter. If it's not a protocol converter this will
                                                    ///< be set to CTL_DISPLAY_OUTPUT_TYPES_INVALID
    ctl_revision_datatype_t SupportedSpec;          ///< [out] Supported industry spec version.
    ctl_output_bpc_flags_t SupportedOutputBPCFlags; ///< [out] Supported output bits per color. Refer ::ctl_output_bpc_flag_t.
                                                    ///< This is independent of RGB or YCbCr output.This is the max BPC
                                                    ///< supported.BPC will vary per mode based on restrictions like bandwidth
                                                    ///< and monitor support
    ctl_protocol_converter_location_flags_t ProtocolConverterType;  ///< [out] Currently Active Protocol Converter. Refer
                                                    ///< ::ctl_protocol_converter_location_flag_t
    ctl_display_config_flags_t DisplayConfigFlags;  ///< [out] Output configuration related flags which indicate how the output
                                                    ///< pixel stream drive the panel. Refer ::ctl_display_config_flag_t
    ctl_std_display_feature_flags_t FeatureEnabledFlags;///< [out] Enabled Display features.Refer ::ctl_std_display_feature_flag_t.
    ctl_std_display_feature_flags_t FeatureSupportedFlags;  ///< [out] Display Supported feature.Refer ::ctl_std_display_feature_flag_t
    ctl_intel_display_feature_flags_t AdvancedFeatureEnabledFlags;  ///< [out] Enabled advanced feature.Refer
                                                    ///< ::ctl_intel_display_feature_flag_t.
    ctl_intel_display_feature_flags_t AdvancedFeatureSupportedFlags;///< [out] Supported advanced feature.Refer
                                                    ///< ::ctl_intel_display_feature_flag_t.
    ctl_display_timing_t Display_Timing_Info;       ///< [out] Applied Timing on the Display
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_display_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adapter's display encoder properties
typedef struct _ctl_adapter_display_encoder_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_os_display_encoder_identifier_t Os_display_encoder_handle;  ///< [out] OS specific Display ID
    ctl_display_output_types_t Type;                ///< [out] Device Type from display HW stand point. If a DisplayPort
                                                    ///< protocol converter is involved, this will indicate it's DisplayPort.
                                                    ///< The protocol converter's output will be available from
                                                    ///< ProtocolConverterOutput field
    bool IsOnBoardProtocolConverterOutputPresent;   ///< [out] Protocol output type which can be used if it's a protocol
                                                    ///< converter. If it's not a protocol converter this will be set to
                                                    ///< CTL_DISPLAY_OUTPUT_TYPES_INVALID
    ctl_revision_datatype_t SupportedSpec;          ///< [out] Supported industry spec version
    ctl_output_bpc_flags_t SupportedOutputBPCFlags; ///< [out] Supported output bits per color. Refer ::ctl_output_bpc_flag_t.
                                                    ///< This is independent of RGB or YCbCr output.This is the max BPC
                                                    ///< supported.BPC will vary per mode based on restrictions like bandwidth
                                                    ///< and monitor support
    ctl_encoder_config_flags_t EncoderConfigFlags;  ///< [out] Output configuration related flags which indicate how the output
                                                    ///< pixel stream drive the panel. Refer ::ctl_encoder_config_flag_t  
                                                    ///< Note:  
                                                    ///<    Virtual = 1: This indicates that its a software display. Hardware
                                                    ///< based features will not be applicable to this display. 
                                                    ///<    Collage=1,Virtual=1: Indicates the fake display output created by
                                                    ///< driver which has the combined resolution of multiple physical displays
                                                    ///< involved in collage configuration  
                                                    ///<    Collage=1,Virtual=0: Indicates the child physical displays involved
                                                    ///< in a collage configuration. These are real physical outputs  
                                                    ///<    Split=1,Virtual=1  : Indicates the fake display output created by
                                                    ///< driver which occupies a portion of a real physical display  
                                                    ///<    Split=1,Virtual=0  : Indicates the physical display which got split
                                                    ///< to form multiple split displays  
                                                    ///<    Split=1,Collage=1  : Invalid combination    
                                                    ///<    MgpuCollage=1,Collage=1,Virtual=1: Indicates the fake display
                                                    ///< output created by driver which has the combined resolution of multiple
                                                    ///< physical displays spread across multiple GPUs involved in Multi-GPU
                                                    ///< collage configuration
                                                    ///<    MgpuCollage=1,Collage=1,Virtual=0: Indicates the child physical
                                                    ///< displays involved in a Multi-GPU collage configuration. These are real
                                                    ///< physical outputs 
    ctl_std_display_feature_flags_t FeatureSupportedFlags;  ///< [out] Adapter Supported feature flags. Refer
                                                    ///< ::ctl_std_display_feature_flag_t
    ctl_intel_display_feature_flags_t AdvancedFeatureSupportedFlags;///< [out] Advanced Features Supported by the Adapter. Refer
                                                    ///< ::ctl_intel_display_feature_flag_t
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 60 bytes

} ctl_adapter_display_encoder_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Device Properties
/// 
/// @details
///     - The application gets device properties
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetDeviceProperties(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to control device adapter
    ctl_device_adapter_properties_t* pProperties    ///< [in,out][release] Query result for device properties
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Display  Properties
/// 
/// @details
///     - The application gets display  properties
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetDisplayProperties(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_display_properties_t* pProperties           ///< [in,out][release] Query result for display  properties
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Adapter Display encoder  Properties
/// 
/// @details
///     - The application gets the graphic adapters display encoder properties
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetAdaperDisplayEncoderProperties(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_adapter_display_encoder_properties_t* pProperties   ///< [in,out][release] Query result for adapter display encoder properties
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Level0 Device handle
/// 
/// @details
///     - The application gets OneAPI Level0 Device handles
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pZeDevice`
///         + `nullptr == hInstance`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetZeDevice(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    void* pZeDevice,                                ///< [out][release] ze_device handle
    void** hInstance                                ///< [out][release] Module instance which caller can use to get export
                                                    ///< functions directly
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Various sharpness filter types
typedef uint32_t ctl_sharpness_filter_type_flags_t;
typedef enum _ctl_sharpness_filter_type_flag_t
{
    CTL_SHARPNESS_FILTER_TYPE_FLAG_NON_ADAPTIVE = CTL_BIT(0),   ///< Non-adaptive sharpness
    CTL_SHARPNESS_FILTER_TYPE_FLAG_ADAPTIVE = CTL_BIT(1),   ///< Adaptive sharpness
    CTL_SHARPNESS_FILTER_TYPE_FLAG_MAX = 0x80000000

} ctl_sharpness_filter_type_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Sharpness filter properties
typedef struct _ctl_sharpness_filter_properties_t
{
    ctl_sharpness_filter_type_flags_t FilterType;   ///< [out] Filter type. Refer ::ctl_sharpness_filter_type_flag_t
    ctl_property_range_info_t FilterDetails;        ///< [out] Min, max & step size information

} ctl_sharpness_filter_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Various sharpness filter types
typedef struct _ctl_sharpness_caps_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_sharpness_filter_type_flags_t SupportedFilterFlags; ///< [out] Supported sharpness filters for a given display output. Refer
                                                    ///< ::ctl_sharpness_filter_type_flag_t
    uint8_t NumFilterTypes;                         ///< [out] Number of elements in filter properties array
    ctl_sharpness_filter_properties_t* pFilterProperty; ///< [in,out] Array of filter properties structure describing supported
                                                    ///< filter capabilities. Caller should provide a pre-allocated memory for
                                                    ///< this.

} ctl_sharpness_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Current sharpness setting
typedef struct _ctl_sharpness_settings_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool Enable;                                    ///< [in,out] Current or new state of sharpness setting
    ctl_sharpness_filter_type_flags_t FilterType;   ///< [in,out] Current or new filter to be set. Refer
                                                    ///< ::ctl_sharpness_filter_type_flag_t
    float Intensity;                                ///< [in,out] Setting intensity to be applied

} ctl_sharpness_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Sharpness capability
/// 
/// @details
///     - Returns sharpness capability
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSharpnessCaps`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSharpnessCaps(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_sharpness_caps_t* pSharpnessCaps            ///< [in,out][release] Query result for sharpness capability
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Sharpness setting
/// 
/// @details
///     - Returns current sharpness settings
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSharpnessSettings`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetCurrentSharpness(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_sharpness_settings_t* pSharpnessSettings    ///< [in,out][release] Query result for sharpness current settings
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set Sharpness setting
/// 
/// @details
///     - Set current sharpness settings
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSharpnessSettings`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetCurrentSharpness(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_sharpness_settings_t* pSharpnessSettings    ///< [in][release] Set sharpness current settings
    );

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_I2C_MAX_DATA_SIZE
/// @brief I2C Maximum data size
#define CTL_I2C_MAX_DATA_SIZE  0x0080
#endif // CTL_I2C_MAX_DATA_SIZE

///////////////////////////////////////////////////////////////////////////////
/// @brief I2C Access Args input Flags bitmasks
typedef uint32_t ctl_i2c_flags_t;
typedef enum _ctl_i2c_flag_t
{
    CTL_I2C_FLAG_ATOMICI2C = CTL_BIT(0),            ///< Force Atomic I2C.
    CTL_I2C_FLAG_1BYTE_INDEX = CTL_BIT(1),          ///< 1-byte Indexed operation. If no Index Size flag set, decided based on
                                                    ///< Offset Value.
    CTL_I2C_FLAG_2BYTE_INDEX = CTL_BIT(2),          ///< 2-byte Indexed operation. If no Index Size flag set, decided based on
                                                    ///< Offset Value.
    CTL_I2C_FLAG_4BYTE_INDEX = CTL_BIT(3),          ///< 4-byte Indexed operation. If no Index Size flag set, decided based on
                                                    ///< Offset Value.
    CTL_I2C_FLAG_SPEED_SLOW = CTL_BIT(4),           ///< If no Speed Flag is set, defaults to Best Option possible.
    CTL_I2C_FLAG_SPEED_FAST = CTL_BIT(5),           ///< If no Speed Flag is set, defaults to Best Option possible.
    CTL_I2C_FLAG_SPEED_BIT_BASH = CTL_BIT(6),       ///< Uses Slower access using SW bit bashing method. If no Speed Flag is
                                                    ///< set, defaults to Best Option possible.
    CTL_I2C_FLAG_MAX = 0x80000000

} ctl_i2c_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief I2C access arguments
typedef struct _ctl_i2c_access_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t DataSize;                              ///< [in,out] Valid data size
    uint32_t Address;                               ///< [in] Address to read or write
    ctl_operation_type_t OpType;                    ///< [in] Operation type, 1 for Read, 2 for Write, for Write operation, App
                                                    ///< needs to run with admin privileges
    uint32_t Offset;                                ///< [in] Offset
    ctl_i2c_flags_t Flags;                          ///< [in] I2C Flags. Refer ::ctl_i2c_flag_t
    uint64_t RAD;                                   ///< [in] RAD, For Future use, to be used for branch devices, Interface
                                                    ///< will be provided to get RAD
    uint8_t Data[CTL_I2C_MAX_DATA_SIZE];            ///< [in,out] Data array

} ctl_i2c_access_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief I2C Access
/// 
/// @details
///     - Interface to access I2C using display handle as identifier.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pI2cAccessArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_SIZE - "Invalid I2C data size"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlI2CAccess(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_i2c_access_args_t* pI2cAccessArgs           ///< [in,out] I2c access arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief I2C Access on PinPair Args input Flags bitmasks
typedef uint32_t ctl_i2c_pinpair_flags_t;
typedef enum _ctl_i2c_pinpair_flag_t
{
    CTL_I2C_PINPAIR_FLAG_ATOMICI2C = CTL_BIT(0),    ///< Force Atomic I2C.
    CTL_I2C_PINPAIR_FLAG_1BYTE_INDEX = CTL_BIT(1),  ///< 1-byte Indexed operation. If no Index Size flag set, decided based on
                                                    ///< Offset Value.
    CTL_I2C_PINPAIR_FLAG_2BYTE_INDEX = CTL_BIT(2),  ///< 2-byte Indexed operation. If no Index Size flag set, decided based on
                                                    ///< Offset Value.
    CTL_I2C_PINPAIR_FLAG_4BYTE_INDEX = CTL_BIT(3),  ///< 4-byte Indexed operation. If no Index Size flag set, decided based on
                                                    ///< Offset Value.
    CTL_I2C_PINPAIR_FLAG_SPEED_SLOW = CTL_BIT(4),   ///< If no Speed Flag is set, defaults to Best Option possible.
    CTL_I2C_PINPAIR_FLAG_SPEED_FAST = CTL_BIT(5),   ///< If no Speed Flag is set, defaults to Best Option possible.
    CTL_I2C_PINPAIR_FLAG_SPEED_BIT_BASH = CTL_BIT(6),   ///< Uses Slower access using SW bit bashing method. If no Speed Flag is
                                                    ///< set, defaults to Best Option possible.
    CTL_I2C_PINPAIR_FLAG_MAX = 0x80000000

} ctl_i2c_pinpair_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief I2C access on Pin Pair arguments
typedef struct _ctl_i2c_access_pinpair_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t DataSize;                              ///< [in,out] Valid data size
    uint32_t Address;                               ///< [in] Address to read or write
    ctl_operation_type_t OpType;                    ///< [in] Operation type, 1 for Read, 2 for Write, for Write operation, App
                                                    ///< needs to run with admin privileges
    uint32_t Offset;                                ///< [in] Offset
    ctl_i2c_pinpair_flags_t Flags;                  ///< [in] I2C Flags. Refer ::ctl_i2c_pinpair_flag_t
    uint8_t Data[CTL_I2C_MAX_DATA_SIZE];            ///< [in,out] Data array
    uint32_t ReservedFields[4];                     ///< [in] Reserved for future use, must be set to Zero.

} ctl_i2c_access_pinpair_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief I2C Access On Pin Pair
/// 
/// @details
///     - Interface to access I2C using pin-pair handle as identifier.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hI2cPinPair`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pI2cAccessArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_SIZE - "Invalid I2C data size"
///     - ::CTL_RESULT_ERROR_INVALID_ARGUMENT - "Invalid Args passed"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_HANDLE - "Invalid or Null handle passed"
///     - ::CTL_RESULT_ERROR_EXTERNAL_DISPLAY_ATTACHED - "Write to Address not allowed when Display is connected"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlI2CAccessOnPinPair(
    ctl_i2c_pin_pair_handle_t hI2cPinPair,          ///< [in] Handle to I2C pin pair.
    ctl_i2c_access_pinpair_args_t* pI2cAccessArgs   ///< [in,out] I2c access arguments.
    );

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_AUX_MAX_DATA_SIZE
/// @brief Aux Maximum data size
#define CTL_AUX_MAX_DATA_SIZE  132
#endif // CTL_AUX_MAX_DATA_SIZE

///////////////////////////////////////////////////////////////////////////////
/// @brief AUX Flags bitmasks
typedef uint32_t ctl_aux_flags_t;
typedef enum _ctl_aux_flag_t
{
    CTL_AUX_FLAG_NATIVE_AUX = CTL_BIT(0),           ///< For Native AUX operation
    CTL_AUX_FLAG_I2C_AUX = CTL_BIT(1),              ///< For I2C AUX operation
    CTL_AUX_FLAG_I2C_AUX_MOT = CTL_BIT(2),          ///< For I2C AUX MOT operation
    CTL_AUX_FLAG_MAX = 0x80000000

} ctl_aux_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief AUX access arguments
typedef struct _ctl_aux_access_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_operation_type_t OpType;                    ///< [in] Operation type, 1 for Read, 2 for Write, for Write operation, App
                                                    ///< needs to run with admin privileges
    ctl_aux_flags_t Flags;                          ///< [in] Aux Flags. Refer ::ctl_aux_flag_t
    uint32_t Address;                               ///< [in] Address to read or write
    uint64_t RAD;                                   ///< [in] RAD, For Future use, to be used for branch devices, Interface
                                                    ///< will be provided to get RAD
    uint32_t PortID;                                ///< [in] Port ID, For Future use, to be used for SST tiled devices
    uint32_t DataSize;                              ///< [in,out] Valid data size
    uint8_t Data[CTL_AUX_MAX_DATA_SIZE];            ///< [in,out] Data array

} ctl_aux_access_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Aux Access
/// 
/// @details
///     - The application does Aux access, PSR needs to be disabled for AUX
///       call.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pAuxAccessArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_SIZE - "Invalid AUX data size"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_INVALID_AUX_ACCESS_FLAG - "Invalid flag for AUX access"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlAUXAccess(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_aux_access_args_t* pAuxAccessArgs           ///< [in,out] Aux access arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Power saving features (Each individual feature's set & get call can be
///        called only once at a time)
typedef uint32_t ctl_power_optimization_flags_t;
typedef enum _ctl_power_optimization_flag_t
{
    CTL_POWER_OPTIMIZATION_FLAG_FBC = CTL_BIT(0),   ///< Frame buffer compression
    CTL_POWER_OPTIMIZATION_FLAG_PSR = CTL_BIT(1),   ///< Panel self refresh
    CTL_POWER_OPTIMIZATION_FLAG_DPST = CTL_BIT(2),  ///< Display power saving technology (Panel technology dependent)
    CTL_POWER_OPTIMIZATION_FLAG_LRR = CTL_BIT(3),   ///< Low refresh rate (LRR/ALRR/UBRR), UBRR is supported only for IGCC and
                                                    ///< NDA clients. UBZRR and UBLRR both can not be enabled at the same time,
                                                    ///< only one can be enabled at a given time
    CTL_POWER_OPTIMIZATION_FLAG_LACE = CTL_BIT(4),  ///< Lighting Aware Contrast Enhancement
    CTL_POWER_OPTIMIZATION_FLAG_MAX = 0x80000000

} ctl_power_optimization_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief GPU/Panel/TCON dependent power optimization technology
typedef uint32_t ctl_power_optimization_dpst_flags_t;
typedef enum _ctl_power_optimization_dpst_flag_t
{
    CTL_POWER_OPTIMIZATION_DPST_FLAG_BKLT = CTL_BIT(0), ///< Intel DPST with Backlight control
    CTL_POWER_OPTIMIZATION_DPST_FLAG_PANEL_CABC = CTL_BIT(1),   ///< Panel TCON specific Content Adaptive Control mechanism
    CTL_POWER_OPTIMIZATION_DPST_FLAG_OPST = CTL_BIT(2), ///< Intel OLED Power Saving Technology
    CTL_POWER_OPTIMIZATION_DPST_FLAG_ELP = CTL_BIT(3),  ///< TCON based Edge Luminance Profile
    CTL_POWER_OPTIMIZATION_DPST_FLAG_EPSM = CTL_BIT(4), ///< Extra power saving mode
    CTL_POWER_OPTIMIZATION_DPST_FLAG_APD = CTL_BIT(5),  ///< Adaptive Pixel Dimming
    CTL_POWER_OPTIMIZATION_DPST_FLAG_PIXOPTIX = CTL_BIT(6), ///< TCON+ based DPST like solution
    CTL_POWER_OPTIMIZATION_DPST_FLAG_MAX = 0x80000000

} ctl_power_optimization_dpst_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Power Source
typedef enum _ctl_power_source_t
{
    CTL_POWER_SOURCE_AC = 0,                        ///< Power Source AC
    CTL_POWER_SOURCE_DC = 1,                        ///< Power Source DC
    CTL_POWER_SOURCE_MAX

} ctl_power_source_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Power Optimization Plan
typedef enum _ctl_power_optimization_plan_t
{
    CTL_POWER_OPTIMIZATION_PLAN_BALANCED = 0,       ///< Balanced mode
    CTL_POWER_OPTIMIZATION_PLAN_HIGH_PERFORMANCE = 1,   ///< High Performance Mode
    CTL_POWER_OPTIMIZATION_PLAN_POWER_SAVER = 2,    ///< Power Saver Mode
    CTL_POWER_OPTIMIZATION_PLAN_MAX

} ctl_power_optimization_plan_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Type of low refresh rate feature
typedef uint32_t ctl_power_optimization_lrr_flags_t;
typedef enum _ctl_power_optimization_lrr_flag_t
{
    CTL_POWER_OPTIMIZATION_LRR_FLAG_LRR10 = CTL_BIT(0), ///< LRR 1.0
    CTL_POWER_OPTIMIZATION_LRR_FLAG_LRR20 = CTL_BIT(1), ///< LRR 2.0
    CTL_POWER_OPTIMIZATION_LRR_FLAG_LRR25 = CTL_BIT(2), ///< LRR 2.5
    CTL_POWER_OPTIMIZATION_LRR_FLAG_ALRR = CTL_BIT(3),  ///< Autonomous LRR
    CTL_POWER_OPTIMIZATION_LRR_FLAG_UBLRR = CTL_BIT(4), ///< User based low refresh rate
    CTL_POWER_OPTIMIZATION_LRR_FLAG_UBZRR = CTL_BIT(5), ///< User based zero refresh rate
    CTL_POWER_OPTIMIZATION_LRR_FLAG_MAX = 0x80000000

} ctl_power_optimization_lrr_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Power optimization caps
typedef struct _ctl_power_optimization_caps_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_power_optimization_flags_t SupportedFeatures;   ///< [out] Supported power optimization features. Refer
                                                    ///< ::ctl_power_optimization_flag_t

} ctl_power_optimization_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Power optimization features
/// 
/// @details
///     - Returns power optimization capabilities
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pPowerOptimizationCaps`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetPowerOptimizationCaps(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_power_optimization_caps_t* pPowerOptimizationCaps   ///< [in,out][release] Query result for power optimization features
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief LRR detailed settings
typedef struct _ctl_power_optimization_lrr_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_power_optimization_lrr_flags_t SupportedLRRTypes;   ///< [out] LRR type(s). Refer ::ctl_power_optimization_lrr_flag_t
    ctl_power_optimization_lrr_flags_t CurrentLRRTypes; ///< [in,out] Current enabled LRR type(s) or the LRR type(s) to set to.
                                                    ///< Refer ::ctl_power_optimization_lrr_flag_t
    bool bRequirePSRDisable;                        ///< [out] Require PSR disable for any change in the selected LRR feature.
                                                    ///< Caller can re-enable PSR once the respective LRR feature is
                                                    ///< enable/disabled. E.g. for UBRR based on platform this flag may not be
                                                    ///< set in which case caller doesn't need to do an explicit PSR disable
    uint16_t LowRR;                                 ///< [out] Lowest RR used for LRR functionality if known to source

} ctl_power_optimization_lrr_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief PSR detailed settings
typedef struct _ctl_power_optimization_psr_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint8_t PSRVersion;                             ///< [in,out] A value of 1 means PSR1, 2 means PSR2
    bool FullFetchUpdate;                           ///< [in,out] Full fetch and update

} ctl_power_optimization_psr_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief DPST detailed settings
typedef struct _ctl_power_optimization_dpst_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint8_t MinLevel;                               ///< [out] Minimum supported aggressiveness level
    uint8_t MaxLevel;                               ///< [out] Maximum supported aggressiveness level
    uint8_t Level;                                  ///< [in,out] Current aggressiveness level to be set
    ctl_power_optimization_dpst_flags_t SupportedFeatures;  ///< [out] Supported features
    ctl_power_optimization_dpst_flags_t EnabledFeatures;///< [in,out] Features enabled or to be enabled. Fill only one feature for
                                                    ///< SET call

} ctl_power_optimization_dpst_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Feature specific power optimization data
typedef union _ctl_power_optimization_feature_specific_info_t
{
    ctl_power_optimization_lrr_t LRRInfo;           ///< [out] LRR info
    ctl_power_optimization_psr_t PSRInfo;           ///< [in,out] PSR info
    ctl_power_optimization_dpst_t DPSTInfo;         ///< [in,out] DPST info

} ctl_power_optimization_feature_specific_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Power optimization settings
typedef struct _ctl_power_optimization_settings_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_power_optimization_plan_t PowerOptimizationPlan;///< [in] Power optimization power plan (max power/max perf/balanced)
    ctl_power_optimization_flags_t PowerOptimizationFeature;///< [in] Power optimization feature interested in. Refer
                                                    ///< ::ctl_power_optimization_flag_t
    bool Enable;                                    ///< [in,out] Enable state
    ctl_power_optimization_feature_specific_info_t FeatureSpecificData; ///< [in,out] Data specific to the feature caller is interested in
    ctl_power_source_t PowerSource;                 ///< [in] AC/DC

} ctl_power_optimization_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Brightness settings for SET call
typedef struct _ctl_set_brightness_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t TargetBrightness;                      ///< [in] The brightness level that the display need to transitioning to in
                                                    ///< milli-percentage. Range is 0-100000 (100%)
    uint32_t SmoothTransitionTimeInMs;              ///< [in] Transition Time for brightness to take effect in milli-seconds.
                                                    ///< If its 0 then it will be an immediate change. Maximum possible value
                                                    ///< is 1000ms.
    uint32_t ReservedFields[4];                     ///< [in] Reserved for future use

} ctl_set_brightness_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Brightness settings for GET call
typedef struct _ctl_get_brightness_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t TargetBrightness;                      ///< [out] The brightness level that the display is currently transitioning
                                                    ///< to in milli-percentage. If not in a transition, this should equal the
                                                    ///< current brightness. Range is 0-100000 (100%)
    uint32_t CurrentBrightness;                     ///< [out] The current brightness level of the display in milli-percentage
    uint32_t ReservedFields[4];                     ///< [out] Reserved for future use

} ctl_get_brightness_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Power optimization setting
/// 
/// @details
///     - Returns power optimization setting for a specific feature
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pPowerOptimizationSettings`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_POWERFEATURE_OPTIMIZATION_FLAG - "Unsupported PowerOptimizationFeature"
///     - ::CTL_RESULT_ERROR_INVALID_POWERSOURCE_TYPE_FOR_DPST - "DPST is supported only in DC Mode"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetPowerOptimizationSetting(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_power_optimization_settings_t* pPowerOptimizationSettings   ///< [in,out][release] Power optimization data to be fetched
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set Power optimization setting
/// 
/// @details
///     - Set power optimization setting for a specific feature
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pPowerOptimizationSettings`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_POWERFEATURE_OPTIMIZATION_FLAG - "Unsupported PowerOptimizationFeature"
///     - ::CTL_RESULT_ERROR_INVALID_POWERSOURCE_TYPE_FOR_DPST - "DPST is supported only in DC Mode"
///     - ::CTL_RESULT_ERROR_SET_FBC_FEATURE_NOT_SUPPORTED - "Set FBC Feature not supported"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetPowerOptimizationSetting(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_power_optimization_settings_t* pPowerOptimizationSettings   ///< [in][release] Power optimization data to be applied
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set Brightness on companion display
/// 
/// @details
///     - Set Brightness for a target display. Currently support is only for
///       companion display.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSetBrightnessSetting`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_ARGUMENT - "Invalid Brightness data passed as argument"
///     - ::CTL_RESULT_ERROR_DISPLAY_NOT_ACTIVE - "Display not active"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Currently Brightness API is supported only on companion display"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetBrightnessSetting(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_set_brightness_t* pSetBrightnessSetting     ///< [in][release] Brightness settings to be applied
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Brightness setting
/// 
/// @details
///     - Get Brightness for a target display. Currently support is only for
///       companion display.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pGetBrightnessSetting`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_DISPLAY_NOT_ACTIVE - "Display not active"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Currently Brightness API is supported only on companion display"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetBrightnessSetting(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_get_brightness_t* pGetBrightnessSetting     ///< [out][release] Brightness settings data to be fetched
    );

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MAX_NUM_SAMPLES_PER_CHANNEL_1D_LUT
/// @brief Maximum number of samples per channel 1D LUT
#define CTL_MAX_NUM_SAMPLES_PER_CHANNEL_1D_LUT  8192
#endif // CTL_MAX_NUM_SAMPLES_PER_CHANNEL_1D_LUT

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixtx pipe set configuration flags bitmasks
typedef uint32_t ctl_pixtx_pipe_set_config_flags_t;
typedef enum _ctl_pixtx_pipe_set_config_flag_t
{
    CTL_PIXTX_PIPE_SET_CONFIG_FLAG_PERSIST_ACROSS_POWER_EVENTS = CTL_BIT(0),///< For maintaining persistance across power events
    CTL_PIXTX_PIPE_SET_CONFIG_FLAG_MAX = 0x80000000

} ctl_pixtx_pipe_set_config_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation block types
typedef enum _ctl_pixtx_block_type_t
{
    CTL_PIXTX_BLOCK_TYPE_1D_LUT = 1,                ///< Block type 1D LUT
    CTL_PIXTX_BLOCK_TYPE_3D_LUT = 2,                ///< Block type 3D LUT
    CTL_PIXTX_BLOCK_TYPE_3X3_MATRIX = 3,            ///< Block type 3x3 matrix
    CTL_PIXTX_BLOCK_TYPE_3X3_MATRIX_AND_OFFSETS = 4,///< Block type 3x3 matrix and offsets
    CTL_PIXTX_BLOCK_TYPE_MAX

} ctl_pixtx_block_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation LUT sampling types
typedef enum _ctl_pixtx_lut_sampling_type_t
{
    CTL_PIXTX_LUT_SAMPLING_TYPE_UNIFORM = 0,        ///< Uniform LUT sampling
    CTL_PIXTX_LUT_SAMPLING_TYPE_NONUNIFORM = 1,     ///< Non uniform LUT sampling, Required mainly in HDR mode
    CTL_PIXTX_LUT_SAMPLING_TYPE_MAX

} ctl_pixtx_lut_sampling_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Configuration query types
typedef enum _ctl_pixtx_config_query_type_t
{
    CTL_PIXTX_CONFIG_QUERY_TYPE_CAPABILITY = 0,     ///< Get complete pixel processing pipeline capability
    CTL_PIXTX_CONFIG_QUERY_TYPE_CURRENT = 1,        ///< Get the configuration set through last set call
    CTL_PIXTX_CONFIG_QUERY_TYPE_MAX

} ctl_pixtx_config_query_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Configuration operation types
typedef enum _ctl_pixtx_config_opertaion_type_t
{
    CTL_PIXTX_CONFIG_OPERTAION_TYPE_RESTORE_DEFAULT = 1,///< Restore block by block or entire pipe line. Use NumBlocks = 0 to
                                                    ///< restore all.
    CTL_PIXTX_CONFIG_OPERTAION_TYPE_SET_CUSTOM = 2, ///< Custom LUT or matrix can be set thorugh this option.
    CTL_PIXTX_CONFIG_OPERTAION_TYPE_MAX

} ctl_pixtx_config_opertaion_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation gamma encoding types
typedef enum _ctl_pixtx_gamma_encoding_type_t
{
    CTL_PIXTX_GAMMA_ENCODING_TYPE_SRGB = 0,         ///< Gamma encoding SRGB
    CTL_PIXTX_GAMMA_ENCODING_TYPE_REC709 = 1,       ///< Gamma encoding REC709, Applicable for REC2020 as well
    CTL_PIXTX_GAMMA_ENCODING_TYPE_ST2084 = 2,       ///< Gamma encoding ST2084
    CTL_PIXTX_GAMMA_ENCODING_TYPE_HLG = 3,          ///< Gamma encoding HLG
    CTL_PIXTX_GAMMA_ENCODING_TYPE_LINEAR = 4,       ///< Gamma encoding linear
    CTL_PIXTX_GAMMA_ENCODING_TYPE_MAX

} ctl_pixtx_gamma_encoding_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation color space types
typedef enum _ctl_pixtx_color_space_t
{
    CTL_PIXTX_COLOR_SPACE_REC709 = 0,               ///< Color space REC709
    CTL_PIXTX_COLOR_SPACE_REC2020 = 1,              ///< Color space REC2020
    CTL_PIXTX_COLOR_SPACE_ADOBE_RGB = 2,            ///< Color space AdobeRGB
    CTL_PIXTX_COLOR_SPACE_P3_D65 = 3,               ///< Color space P3_D65
    CTL_PIXTX_COLOR_SPACE_P3_DCI = 4,               ///< Color space P3_DCI
    CTL_PIXTX_COLOR_SPACE_P3_D60 = 5,               ///< Color space P3_D60
    CTL_PIXTX_COLOR_SPACE_CUSTOM = 0xFFFF,          ///< Color space custom, Refer ::ctl_pixtx_color_primaries_t for color
                                                    ///< primary details
    CTL_PIXTX_COLOR_SPACE_MAX

} ctl_pixtx_color_space_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation color model types
typedef enum _ctl_pixtx_color_model_t
{
    CTL_PIXTX_COLOR_MODEL_RGB_FR = 0,               ///< Color model RGB full range
    CTL_PIXTX_COLOR_MODEL_RGB_LR = 1,               ///< Color model RGB limited range
    CTL_PIXTX_COLOR_MODEL_YCBCR_422_FR = 2,         ///< Color model YCBCR 422 full range
    CTL_PIXTX_COLOR_MODEL_YCBCR_422_LR = 3,         ///< Color model YCBCR 422 limited range
    CTL_PIXTX_COLOR_MODEL_YCBCR_420_FR = 4,         ///< Color model YCBCR 420 full range
    CTL_PIXTX_COLOR_MODEL_YCBCR_420_LR = 5,         ///< Color model YCBCR 420 limited range
    CTL_PIXTX_COLOR_MODEL_YCBCR_444_FR = 6,         ///< Color model YCBCR 444 full range
    CTL_PIXTX_COLOR_MODEL_YCBCR_444_LR = 7,         ///< Color model YCBCR 444 limited range
    CTL_PIXTX_COLOR_MODEL_MAX

} ctl_pixtx_color_model_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation color primaries
typedef struct _ctl_pixtx_color_primaries_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    double xR;                                      ///< [out] CIE1931 x value with maximum red pixel value
    double yR;                                      ///< [out] CIE1931 y value with maximum red pixel value
    double xG;                                      ///< [out] CIE1931 x value with maximum green pixel value
    double yG;                                      ///< [out] CIE1931 y value with maximum green pixel value
    double xB;                                      ///< [out] CIE1931 x value with maximum blue pixel value
    double yB;                                      ///< [out] CIE1931 y value with maximum blue pixel value
    double xW;                                      ///< [out] CIE1931 x value with maximum white pixel value
    double yW;                                      ///< [out] CIE1931 y value with maximum white pixel value

} ctl_pixtx_color_primaries_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation pixel format
typedef struct _ctl_pixtx_pixel_format_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t BitsPerColor;                          ///< [out] Bits per color, It Will be 16 for FP16 case
    bool IsFloat;                                   ///< [out] Will be set for FP16 or other floating point encoding schemes
    ctl_pixtx_gamma_encoding_type_t EncodingType;   ///< [out] Encoding type
    ctl_pixtx_color_space_t ColorSpace;             ///< [out] Color space
    ctl_pixtx_color_model_t ColorModel;             ///< [out] Color model
    ctl_pixtx_color_primaries_t ColorPrimaries;     ///< [out] Color primaries, Used mainly for custom color space
    double MaxBrightness;                           ///< [out] Maximum brightness of pixel values. If no input is given,
                                                    ///< default will be set to sRGB during set call. If panel capability is
                                                    ///< not known get call will default to sRGB.
    double MinBrightness;                           ///< [out] Minimum brightness of pixel values

} ctl_pixtx_pixel_format_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation 1D LUT configuration
typedef struct _ctl_pixtx_1dlut_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_pixtx_lut_sampling_type_t SamplingType;     ///< [in,out] Blocks with non-uniform sampling capability support unifrom
                                                    ///< sampling also but not vice versa.
    uint32_t NumSamplesPerChannel;                  ///< [in,out] Number of samples per channel. Resampled internally based on
                                                    ///< HW capability for uniformly sampled LUT.Maximum supported value is
                                                    ///< ::CTL_MAX_NUM_SAMPLES_PER_CHANNEL_1D_LUT Caller needs to use exact
                                                    ///< sampling position given in pSamplePositions for non-uniformly sampled
                                                    ///< LUTs.
    uint32_t NumChannels;                           ///< [in,out] Number of channels, 1 for Grey scale LUT, 3 for RGB LUT
    double* pSampleValues;                          ///< [in,out] Pointer to sample values, R array followed by G and B arrays
                                                    ///< in case of multi-channel LUT. Allocation size for pSampleValues should
                                                    ///< be NumSamplesPerChannel * NumChannels * sizeof(double)
    double* pSamplePositions;                       ///< [out] LUT (same for all channels) to represent sampling positions for
                                                    ///< non-uniformly sampled LUTs.Can be NULL in case uniformly sampled LUTs

} ctl_pixtx_1dlut_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation matrix configuration
typedef struct _ctl_pixtx_matrix_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    double PreOffsets[3];                           ///< [in,out] Pre offsets
    double PostOffsets[3];                          ///< [in,out] Post offsets
    double Matrix[3][3];                            ///< [in,out] 3x3 Matrix

} ctl_pixtx_matrix_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation 3D LUT sample. Samples are converted to integer
///        based on underlying HW capabilities. Hence slight precision loss will
///        be observed while getting sample values.
typedef struct _ctl_pixtx_3dlut_sample_t
{
    double Red;                                     ///< [in,out] Red output value
    double Green;                                   ///< [in,out] Green output value
    double Blue;                                    ///< [in,out] Blue output value

} ctl_pixtx_3dlut_sample_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation 3D LUT configuration
typedef struct _ctl_pixtx_3dlut_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t NumSamplesPerChannel;                  ///< [in,out] Number of samples per channel
    ctl_pixtx_3dlut_sample_t* pSampleValues;        ///< [in,out] Pointer to sample values, R in outer most loop followed by G
                                                    ///< and B

} ctl_pixtx_3dlut_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation configuration
typedef union _ctl_pixtx_config_t
{
    ctl_pixtx_1dlut_config_t OneDLutConfig;         ///< [in,out] 1D LUT configuration
    ctl_pixtx_3dlut_config_t ThreeDLutConfig;       ///< [in,out] 3D LUT configuration
    ctl_pixtx_matrix_config_t MatrixConfig;         ///< [in,out] Matrix configuration

} ctl_pixtx_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation block configuration
typedef struct _ctl_pixtx_block_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t BlockId;                               ///< [in,out] Unique ID for each pixel processing block. Id for a block is
                                                    ///< fixed for a platform.
    ctl_pixtx_block_type_t BlockType;               ///< [in,out] Block type
    ctl_pixtx_config_t Config;                      ///< [in,out] Configuration

} ctl_pixtx_block_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation pipe get configuration
typedef struct _ctl_pixtx_pipe_get_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_pixtx_config_query_type_t QueryType;        ///< [in] Query operation type
    ctl_pixtx_pixel_format_t InputPixelFormat;      ///< [out] Input pixel format
    ctl_pixtx_pixel_format_t OutputPixelFormat;     ///< [out] Output pixel format
    uint32_t NumBlocks;                             ///< [out] Number of blocks
    ctl_pixtx_block_config_t* pBlockConfigs;        ///< [out] Pointer to specific configs

} ctl_pixtx_pipe_get_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation pipe set configuration
typedef struct _ctl_pixtx_pipe_set_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_pixtx_config_opertaion_type_t OpertaionType;///< [in] Set operation type
    ctl_pixtx_pipe_set_config_flags_t Flags;        ///< [in] Config flags. Refer ::ctl_pixtx_pipe_set_config_flag_t
    uint32_t NumBlocks;                             ///< [in] Number of blocks
    ctl_pixtx_block_config_t* pBlockConfigs;        ///< [in,out] Array of block specific configs

} ctl_pixtx_pipe_set_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation get pipe configuration
/// 
/// @details
///     - The application does pixel transformation get pipe configuration
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pPixTxGetConfigArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
///     - ::CTL_RESULT_ERROR_INVALID_PIXTX_GET_CONFIG_QUERY_TYPE - "Invalid query type"
///     - ::CTL_RESULT_ERROR_INVALID_PIXTX_BLOCK_ID - "Invalid block id"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PIXTX_BLOCK_CONFIG_MEMORY - "Insufficient memery allocated for BlockConfigs"
///     - ::CTL_RESULT_ERROR_3DLUT_INVALID_PIPE - "Invalid pipe for 3dlut"
///     - ::CTL_RESULT_ERROR_3DLUT_INVALID_DATA - "Invalid 3dlut data"
///     - ::CTL_RESULT_ERROR_3DLUT_NOT_SUPPORTED_IN_HDR - "3dlut not supported in HDR"
///     - ::CTL_RESULT_ERROR_3DLUT_INVALID_OPERATION - "Invalid 3dlut operation"
///     - ::CTL_RESULT_ERROR_3DLUT_UNSUCCESSFUL - "3dlut call unsuccessful"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPixelTransformationGetConfig(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_pixtx_pipe_get_config_t* pPixTxGetConfigArgs///< [in,out] Pixel transformation get pipe configiguration arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Pixel transformation set pipe configuration
/// 
/// @details
///     - The application does pixel transformation set pipe configuration
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pPixTxSetConfigArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
///     - ::CTL_RESULT_ERROR_INVALID_PIXTX_SET_CONFIG_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_SET_CONFIG_NUMBER_OF_SAMPLES - "Invalid number of samples"
///     - ::CTL_RESULT_ERROR_INVALID_PIXTX_BLOCK_ID - "Invalid block id"
///     - ::CTL_RESULT_ERROR_PERSISTANCE_NOT_SUPPORTED - "Persistance not supported"
///     - ::CTL_RESULT_ERROR_3DLUT_INVALID_PIPE - "Invalid pipe for 3dlut"
///     - ::CTL_RESULT_ERROR_3DLUT_INVALID_DATA - "Invalid 3dlut data"
///     - ::CTL_RESULT_ERROR_3DLUT_NOT_SUPPORTED_IN_HDR - "3dlut not supported in HDR"
///     - ::CTL_RESULT_ERROR_3DLUT_INVALID_OPERATION - "Invalid 3dlut operation"
///     - ::CTL_RESULT_ERROR_3DLUT_UNSUCCESSFUL - "3dlut call unsuccessful"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPixelTransformationSetConfig(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_pixtx_pipe_set_config_t* pPixTxSetConfigArgs///< [in,out] Pixel transformation set pipe configiguration arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Panel descriptor access arguments
typedef struct _ctl_panel_descriptor_access_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_operation_type_t OpType;                    ///< [in] Operation type, 1 for Read, 2 for Write. App needs to run with
                                                    ///< admin privileges for Write operation, Currently only Read operation is
                                                    ///< supported
    uint32_t BlockNumber;                           ///< [in] Block number, Need to provide only if acccessing EDID
    uint32_t DescriptorDataSize;                    ///< [in] Descriptor data size, Should be 0 for querying the size and
                                                    ///< should be DescriptorDataSize derived from query call otherwise
    uint8_t* pDescriptorData;                       ///< [in,out] Panel descriptor data

} ctl_panel_descriptor_access_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Panel Descriptor Access
/// 
/// @details
///     - The application does EDID or Display ID access
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pPanelDescriptorAccessArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPanelDescriptorAccess(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_panel_descriptor_access_args_t* pPanelDescriptorAccessArgs  ///< [in,out] Panel descriptor access arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief  Retro Scaling Types
typedef uint32_t ctl_retro_scaling_type_flags_t;
typedef enum _ctl_retro_scaling_type_flag_t
{
    CTL_RETRO_SCALING_TYPE_FLAG_INTEGER = CTL_BIT(0),   ///< Integer Scaling
    CTL_RETRO_SCALING_TYPE_FLAG_NEAREST_NEIGHBOUR = CTL_BIT(1), ///< Nearest Neighbour Scaling
    CTL_RETRO_SCALING_TYPE_FLAG_MAX = 0x80000000

} ctl_retro_scaling_type_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Set/Get Retro Scaling Type
typedef struct _ctl_retro_scaling_settings_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool Get;                                       ///< [in][release] Set to true to get current scaling . Set to False to Set
                                                    ///< the scaling
    bool Enable;                                    ///< [in,out] State of the scaler
    ctl_retro_scaling_type_flags_t RetroScalingType;///< [out] Requested retro scaling types. Refer
                                                    ///< ::ctl_retro_scaling_type_flag_t

} ctl_retro_scaling_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Retro Scaling caps
typedef struct _ctl_retro_scaling_caps_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_retro_scaling_type_flags_t SupportedRetroScaling;   ///< [out] Supported retro scaling types

} ctl_retro_scaling_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Supported Retro Scaling Types
/// 
/// @details
///     - Returns supported retro scaling capabilities
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pRetroScalingCaps`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSupportedRetroScalingCapability(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to adapter
    ctl_retro_scaling_caps_t* pRetroScalingCaps     ///< [in,out][release] Query result for supported retro scaling types
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Retro Scaling
/// 
/// @details
///     - Get or Set the status of retro scaling.This Api will do a physical
///       modeset resulting in flash on the screen
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pGetSetRetroScalingType`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSetRetroScaling(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to adapter
    ctl_retro_scaling_settings_t* pGetSetRetroScalingType   ///< [in,out][release] Get or Set the retro scaling type
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Scaling Types
typedef uint32_t ctl_scaling_type_flags_t;
typedef enum _ctl_scaling_type_flag_t
{
    CTL_SCALING_TYPE_FLAG_IDENTITY = CTL_BIT(0),    ///< No scaling is applied and display manages scaling itself when possible
    CTL_SCALING_TYPE_FLAG_CENTERED = CTL_BIT(1),    ///< Source is not scaled but place in the center of the target display
    CTL_SCALING_TYPE_FLAG_STRETCHED = CTL_BIT(2),   ///< Source is stretched to fit the target size
    CTL_SCALING_TYPE_FLAG_ASPECT_RATIO_CENTERED_MAX = CTL_BIT(3),   ///< The aspect ratio is maintained with the source centered
    CTL_SCALING_TYPE_FLAG_CUSTOM = CTL_BIT(4),      ///< None of the standard types match this .Additional parameters are
                                                    ///< required which should be set via a private driver interface
    CTL_SCALING_TYPE_FLAG_MAX = 0x80000000

} ctl_scaling_type_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Scaling caps
typedef struct _ctl_scaling_caps_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_scaling_type_flags_t SupportedScaling;      ///< [out] Supported scaling types. Refer ::ctl_scaling_type_flag_t

} ctl_scaling_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Set/Get Scaling type
typedef struct _ctl_scaling_settings_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool Enable;                                    ///< [in,out] State of the scaler
    ctl_scaling_type_flags_t ScalingType;           ///< [in,out] Requested scaling types. Refer ::ctl_scaling_type_flag_t
    uint32_t CustomScalingX;                        ///< [in,out] Custom Scaling X resolution
    uint32_t CustomScalingY;                        ///< [in,out] Custom Scaling Y resolution
    bool HardwareModeSet;                           ///< [in] Flag to indicate hardware modeset should be done to apply the
                                                    ///< scaling.Setting this to true would result in a flash on the screen. If
                                                    ///< this flag is set to false , API will request the OS to do a virtual
                                                    ///< modeset , but the OS can ignore this request and do a hardware modeset
                                                    ///< in some instances

} ctl_scaling_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Supported Scaling Types
/// 
/// @details
///     - Returns supported scaling capabilities
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pScalingCaps`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSupportedScalingCapability(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_scaling_caps_t* pScalingCaps                ///< [in,out][release] Query result for supported scaling types
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Current Scaling
/// 
/// @details
///     - Returns current active scaling
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pGetCurrentScalingType`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetCurrentScaling(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_scaling_settings_t* pGetCurrentScalingType  ///< [in,out][release] Query result for active scaling types
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set Scaling Type
/// 
/// @details
///     - Returns current active scaling
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSetScalingType`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetCurrentScaling(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_scaling_settings_t* pSetScalingType         ///< [in,out][release] Set scaling types
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Ambient light based enhancement table entry
typedef struct _ctl_lace_lux_aggr_map_entry_t
{
    uint32_t Lux;                                   ///< [in,out] Ambient lux
    uint8_t AggressivenessPercent;                  ///< [in,out] Pixel boost agressiveness

} ctl_lace_lux_aggr_map_entry_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Ambient light based enhancement table
typedef struct _ctl_lace_lux_aggr_map_t
{
    uint32_t MaxNumEntries;                         ///< [out] Max Number of entries in mapping table supported
    uint32_t NumEntries;                            ///< [in,out] Number of entries in the given mapping table
    ctl_lace_lux_aggr_map_entry_t* pLuxToAggrMappingTable;  ///< [in] Max number of Entries which can be passed in
                                                    ///< LuxToAggrMappingTable

} ctl_lace_lux_aggr_map_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Data specific to the mode caller is interested in
typedef union _ctl_lace_aggr_config_t
{
    uint8_t FixedAggressivenessLevelPercent;        ///< [in,out] Fixed aggressiveness level, applicable for
                                                    ///< CTL_LACE_MODE_FIXED_AGGR_LEVEL
    ctl_lace_lux_aggr_map_t AggrLevelMap;           ///< [in,out] Lux to enhancement mapping table, applicable for
                                                    ///< CTL_LACE_MODE_AMBIENT_ADAPTIVE

} ctl_lace_aggr_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Operations used for additional settings
typedef uint32_t ctl_get_operation_flags_t;
typedef enum _ctl_get_operation_flag_t
{
    CTL_GET_OPERATION_FLAG_CURRENT = CTL_BIT(0),    ///< Get the details set through last set call
    CTL_GET_OPERATION_FLAG_DEFAULT = CTL_BIT(1),    ///< Get the driver default values
    CTL_GET_OPERATION_FLAG_CAPABILITY = CTL_BIT(2), ///< Get capability
    CTL_GET_OPERATION_FLAG_MAX = 0x80000000

} ctl_get_operation_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Set Operations used for additional settings
typedef enum _ctl_set_operation_t
{
    CTL_SET_OPERATION_RESTORE_DEFAULT = 0,          ///< Restore default values
    CTL_SET_OPERATION_CUSTOM = 1,                   ///< Set custom values
    CTL_SET_OPERATION_MAX

} ctl_set_operation_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief  Lace Trigger Modes
typedef uint32_t ctl_lace_trigger_flags_t;
typedef enum _ctl_lace_trigger_flag_t
{
    CTL_LACE_TRIGGER_FLAG_AMBIENT_LIGHT = CTL_BIT(0),   ///< LACE enhancement depends on Ambient light
    CTL_LACE_TRIGGER_FLAG_FIXED_AGGRESSIVENESS = CTL_BIT(1),///< LACE enhancement is as per given fixed aggressiveness level
    CTL_LACE_TRIGGER_FLAG_MAX = 0x80000000

} ctl_lace_trigger_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Set/Get LACE Config
typedef struct _ctl_lace_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool Enabled;                                   ///< [in,out] Enable or disable LACE feature
    ctl_get_operation_flags_t OpTypeGet;            ///< [in] Get Operations used for additional settings
    ctl_set_operation_t OpTypeSet;                  ///< [in] Set Operations used for additional settings
    ctl_lace_trigger_flags_t Trigger;               ///< [in,out] LACE operating mode to be Triggerd
    ctl_lace_aggr_config_t LaceConfig;              ///< [in,out] Data specific to the mode, caller is interested in

} ctl_lace_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get LACE Config
/// 
/// @details
///     - Returns current LACE Config
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pLaceConfig`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_LACE_INVALID_DATA_ARGUMENT_PASSED - "Lace Incorrrect AggressivePercent data or LuxVsAggressive Map data passed by user"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetLACEConfig(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_lace_config_t* pLaceConfig                  ///< [out]Lace configuration
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets LACE Config
/// 
/// @details
///     - Sets LACE Config
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pLaceConfig`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_LACE_INVALID_DATA_ARGUMENT_PASSED - "Lace Incorrrect AggressivePercent data or LuxVsAggressive Map data passed by user"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetLACEConfig(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in]Handle to display output
    ctl_lace_config_t* pLaceConfig                  ///< [in]Lace configuration
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Software PSR status/Set Software PSR settings
typedef struct _ctl_sw_psr_settings_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool Set;                                       ///< [in][release] Set to False to Get Software PSR status. Set to True to
                                                    ///< Enable/Disable Software PSR
    bool Supported;                                 ///< [out] When Get is True, returns if SW PSR is supported
    bool Enable;                                    ///< [in,out] When Get is True, returns current state of  Software PSR.
                                                    ///< When Get is False, Enables/Diasbles Software PSR

} ctl_sw_psr_settings_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Software PSR caps/Set software PSR State
/// 
/// @details
///     - Returns Software PSR status or Sets Software PSR capabilities. This is
///       a reserved capability. By default, software PSR is not supported/will
///       not be enabled, need application to activate it, please contact Intel
///       for activation.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSoftwarePsrSetting`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSoftwarePSR(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_sw_psr_settings_t* pSoftwarePsrSetting      ///< [in,out][release] Get Software PSR caps/state or Set Software PSR
                                                    ///< state
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Intel Arc Sync Monitor Params
typedef struct _ctl_intel_arc_sync_monitor_params_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool IsIntelArcSyncSupported;                   ///< [out] Intel Arc Sync support for the monitor
    float MinimumRefreshRateInHz;                   ///< [out] Minimum Intel Arc Sync refresh rate supported by the monitor
    float MaximumRefreshRateInHz;                   ///< [out] Maximum Intel Arc Sync refresh rate supported by the monitor
    uint32_t MaxFrameTimeIncreaseInUs;              ///< [out] Max frame time increase in micro seconds from DID2.1 Adaptive
                                                    ///< Sync block
    uint32_t MaxFrameTimeDecreaseInUs;              ///< [out] Max frame time decrease in micro seconds from DID2.1 Adaptive
                                                    ///< Sync block

} ctl_intel_arc_sync_monitor_params_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Intel Arc Sync information for monitor
/// 
/// @details
///     - Returns Intel Arc Sync information for selected monitor
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pIntelArcSyncMonitorParams`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetIntelArcSyncInfoForMonitor(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_intel_arc_sync_monitor_params_t* pIntelArcSyncMonitorParams ///< [in,out][release] Intel Arc Sync params for monitor
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Handle of a MUX output instance
typedef struct _ctl_mux_output_handle_t *ctl_mux_output_handle_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Enumerate Display MUX Devices on this system across adapters
/// 
/// @details
///     - The application enumerates all MUX devices in the system
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hAPIHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
///         + `nullptr == phMuxDevices`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumerateMuxDevices(
    ctl_api_handle_t hAPIHandle,                    ///< [in][release] Applications should pass the Control API handle returned
                                                    ///< by the CtlInit function 
    uint32_t* pCount,                               ///< [in,out][release] pointer to the number of MUX device instances. If
                                                    ///< input count is zero, then the api will update the value with the total
                                                    ///< number of MUX devices available and return the Count value. If input
                                                    ///< count is non-zero, then the api will only retrieve the number of MUX Devices.
                                                    ///< If count is larger than the number of MUX devices available, then the
                                                    ///< api will update the value with the correct number of MUX devices available.
    ctl_mux_output_handle_t* phMuxDevices           ///< [out][range(0, *pCount)] array of MUX device instance handles
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Display MUX device properties
typedef struct _ctl_mux_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint8_t MuxId;                                  ///< [out] MUX ID of this MUX device enumerated
    uint32_t Count;                                 ///< [in,out] Pointer to the number of display output instances this MUX
                                                    ///< object can drive. If count is zero, then the api will update the value
                                                    ///< with the total
                                                    ///< number of outputs available. If count is non-zero, then the api will
                                                    ///< only retrieve the number of outputs.
                                                    ///< If count is larger than the number of display outputs MUX can drive,
                                                    ///< then the api will update the value with the correct number of display
                                                    ///< outputs MUX can driver.
    ctl_display_output_handle_t* phDisplayOutputs;  ///< [in,out][range(0, *pCount)] Array of display output instance handles
                                                    ///< this MUX device can drive
    uint8_t IndexOfDisplayOutputOwningMux;          ///< [out] [range(0, (Count-1))] This is the index into the
                                                    ///< phDisplayOutputs list to the display output which currently owns the
                                                    ///< MUX output. This doesn't mean display is active

} ctl_mux_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Display Mux properties
/// 
/// @details
///     - Get the propeties of the Mux device
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hMuxDevice`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pMuxProperties`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetMuxProperties(
    ctl_mux_output_handle_t hMuxDevice,             ///< [in] MUX device instance handle
    ctl_mux_properties_t* pMuxProperties            ///< [in,out] MUX device properties
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Switch Mux output
/// 
/// @details
///     - Switches the MUX output
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hMuxDevice`
///         + `nullptr == hInactiveDisplayOutput`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSwitchMux(
    ctl_mux_output_handle_t hMuxDevice,             ///< [in] MUX device instance handle
    ctl_display_output_handle_t hInactiveDisplayOutput  ///< [out] Input selection for this MUX, which if active will drive the
                                                    ///< output of this MUX device. This should be one of the display output
                                                    ///< handles reported under this MUX device's properties.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Intel Arc Sync profile
typedef enum _ctl_intel_arc_sync_profile_t
{
    CTL_INTEL_ARC_SYNC_PROFILE_INVALID = 0,         ///< Invalid profile
    CTL_INTEL_ARC_SYNC_PROFILE_RECOMMENDED = 1,     ///< Default. Selects appropriate profile based on the monitor. COMPATIBLE
                                                    ///< profile is applied if profile is not available for the monitor
    CTL_INTEL_ARC_SYNC_PROFILE_EXCELLENT = 2,       ///< Unconstrained. Full VRR range of the monitor can be used
    CTL_INTEL_ARC_SYNC_PROFILE_GOOD = 3,            ///< Some minor range constraints, unlikely to effect user experience but
                                                    ///< can reduce flicker on some monitors
    CTL_INTEL_ARC_SYNC_PROFILE_COMPATIBLE = 4,      ///< Significant constraints that will reduce flicker considerably but are
                                                    ///< likely to cause some level of judder onscreen especially when refresh
                                                    ///< rates are changing rapidly
    CTL_INTEL_ARC_SYNC_PROFILE_OFF = 5,             ///< Disable Intel Arc Sync on this monitor. This disables variable rate
                                                    ///< flips on this monitor. All sync flips will occur at the OS requested
                                                    ///< refresh rate
    CTL_INTEL_ARC_SYNC_PROFILE_VESA = 6,            ///< Applies vesa specified constraints if the monitor has provided them,
                                                    ///< COMPATIBLE profile if not
    CTL_INTEL_ARC_SYNC_PROFILE_CUSTOM = 7,          ///< Unlocks controls to set a custom Intel Arc Sync profile
    CTL_INTEL_ARC_SYNC_PROFILE_MAX

} ctl_intel_arc_sync_profile_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Intel Arc Sync Profile Params
typedef struct _ctl_intel_arc_sync_profile_params_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_intel_arc_sync_profile_t IntelArcSyncProfile;   ///< [in,out] Intel Arc Sync profile used by driver. Refer
                                                    ///< ::ctl_intel_arc_sync_profile_t
    float MaxRefreshRateInHz;                       ///< [in,out] Maximum refresh rate utilized by the driver
    float MinRefreshRateInHz;                       ///< [in,out] Minimum refresh rate utilized by the driver
    uint32_t MaxFrameTimeIncreaseInUs;              ///< [in,out] Maximum frame time increase (in micro seconds) imposed by the
                                                    ///< driver
    uint32_t MaxFrameTimeDecreaseInUs;              ///< [in,out] Maximum frame time decrease (in micro seconds) imposed by the
                                                    ///< driver

} ctl_intel_arc_sync_profile_params_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Intel Arc Sync profile
/// 
/// @details
///     - Returns Intel Arc Sync profile for selected monitor
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pIntelArcSyncProfileParams`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetIntelArcSyncProfile(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_intel_arc_sync_profile_params_t* pIntelArcSyncProfileParams ///< [in,out][release] Intel Arc Sync params for monitor
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set Intel Arc Sync profile
/// 
/// @details
///     - Sets Intel Arc Sync profile for selected monitor. In a mux situation,
///       this API should be called for all display IDs associated with a
///       physical display.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pIntelArcSyncProfileParams`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlSetIntelArcSyncProfile(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in][release] Handle to display output
    ctl_intel_arc_sync_profile_params_t* pIntelArcSyncProfileParams ///< [in][release] Intel Arc Sync params for monitor
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief EDID Management operation type
typedef enum _ctl_edid_management_optype_t
{
    CTL_EDID_MANAGEMENT_OPTYPE_READ_EDID = 1,       ///< This operation type is to read an output's EDID. Set edid_type input
                                                    ///< arg to read MONITOR EDID or previously OVERRIDDEN EDID or CURRENT
                                                    ///< active EDID. Read EDID is a 2 pass call. First call with size = 0,
                                                    ///< pEdidBuf = nullptr to get the size, then call with allocated buffer to
                                                    ///< get the EDID data. READ operation is applicable for any normal, edid
                                                    ///< locked or edid overridden display output device.
    CTL_EDID_MANAGEMENT_OPTYPE_LOCK_EDID = 2,       ///< To make an output always connected with OVERRIDE or MONITOR EDID
                                                    ///< across reboots. When output isn't connected call with OVERRIDE EDID;
                                                    ///< when connected, either set OVERRIDE and provide pEdidBuf or set
                                                    ///< MONITOR and driver will use monitor's EDID. There is no change to EDID
                                                    ///< stored in Monitor. Cannot be called when override is active. Any OS
                                                    ///< EDID override will take precedence over IGCL override.
    CTL_EDID_MANAGEMENT_OPTYPE_UNLOCK_EDID = 3,     ///< To undo lock EDID operation, i.e. it makes output as detached in
                                                    ///< response to unplug. This operation removes past supplied EDID; output
                                                    ///< status is reported to OS as it is; output restores back to monitor's
                                                    ///< EDID when it is connected
    CTL_EDID_MANAGEMENT_OPTYPE_OVERRIDE_EDID = 4,   ///< To replace an output's EDID with supplied one (pEdidBuf) only when
                                                    ///< physical display is connected. There is no change to EDID stored in
                                                    ///< Monitor. Cannot apply this operation on locked output. When no output
                                                    ///< device attached, the supplied EDID will be persisted in driver for
                                                    ///< future use. Any OS EDID override will take precedence over IGCL
                                                    ///< override.
    CTL_EDID_MANAGEMENT_OPTYPE_UNDO_OVERRIDE_EDID = 5,  ///< To undo override EDID operation, that is remove previously overridden
                                                    ///< EDID on an output. Output restores back to monitor's EDID when it is
                                                    ///< connected
    CTL_EDID_MANAGEMENT_OPTYPE_MAX

} ctl_edid_management_optype_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief EDID type. Used in LOCK_EDID and READ_EDID calls.
typedef enum _ctl_edid_type_t
{
    CTL_EDID_TYPE_CURRENT = 1,                      ///< [in] Used to return currently active EDID in READ_EDID call.
    CTL_EDID_TYPE_OVERRIDE = 2,                     ///< [in] Is it user supplied EDID. Used in LOCK_EDID call with Supplied
                                                    ///< EDID or in READ_EDID to get Supplied EDID.
    CTL_EDID_TYPE_MONITOR = 3,                      ///< [in] Is it Monitor's EDID. Used in LOCK_EDID and READ_EDID calls.
    CTL_EDID_TYPE_MAX

} ctl_edid_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Edid management operation Out Flags
typedef uint32_t ctl_edid_management_out_flags_t;
typedef enum _ctl_edid_management_out_flag_t
{
    CTL_EDID_MANAGEMENT_OUT_FLAG_OS_CONN_NOTIFICATION = CTL_BIT(0), ///< [out] If OS was notified about a connection change. App will need to
                                                    ///< wait for the OS action to complete.
    CTL_EDID_MANAGEMENT_OUT_FLAG_SUPPLIED_EDID = CTL_BIT(1),///< [out] Is it previously supplied EDID, set for READ_EDID(CURRENT).
    CTL_EDID_MANAGEMENT_OUT_FLAG_MONITOR_EDID = CTL_BIT(2), ///< [out] Is it Monitor's EDID, set for READ_EDID(CURRENT).
    CTL_EDID_MANAGEMENT_OUT_FLAG_DISPLAY_CONNECTED = CTL_BIT(3),///< [out] Is Monitor physically connected
    CTL_EDID_MANAGEMENT_OUT_FLAG_MAX = 0x80000000

} ctl_edid_management_out_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief EDID management
typedef struct _ctl_edid_management_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_edid_management_optype_t OpType;            ///< [in] EDID managmeent operation type
    ctl_edid_type_t EdidType;                       ///< [in] EDID Type, Monitor or Supplied
    uint32_t EdidSize;                              ///< [in,out] EDID Size, should be 0 for querying the size of EDID, should
                                                    ///< be previously returned size to read EDID. if buffer isn't big enough
                                                    ///< to fit EDID, returns size of EDID bytes.
    uint8_t* pEdidBuf;                              ///< [in,out] buffer holding EDID data
    ctl_edid_management_out_flags_t OutFlags;       ///< [out] Output flags to inform about status of EDID management
                                                    ///< operations

} ctl_edid_management_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief EDID Management allows managing an output's EDID or Plugged Status.
/// 
/// @details
///     - To manage output's EDID or Display ID. Supports native DP SST and HDMI
///       Display types.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pEdidManagementArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
///     - ::CTL_RESULT_ERROR_INVALID_ARGUMENT - "Invalid combination of parameters"
///     - ::CTL_RESULT_ERROR_DISPLAY_NOT_ATTACHED - "Error for Output Device not attached"
///     - ::CTL_RESULT_ERROR_OUT_OF_DEVICE_MEMORY - "Insufficient device memory to satisfy call"
///     - ::CTL_RESULT_ERROR_DATA_NOT_FOUND - "Requested EDID data not present."
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEdidManagement(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_edid_management_args_t* pEdidManagementArgs ///< [in,out] EDID management arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Custom mode operation types
typedef enum _ctl_custom_mode_operation_types_t
{
    CTL_CUSTOM_MODE_OPERATION_TYPES_GET_CUSTOM_SOURCE_MODES = 0,///< Get details of all previous applied custom modes if any.
    CTL_CUSTOM_MODE_OPERATION_TYPES_ADD_CUSTOM_SOURCE_MODE = 1, ///< Add a new mode. Allows only single mode adition at a time.
    CTL_CUSTOM_MODE_OPERATION_TYPES_REMOVE_CUSTOM_SOURCE_MODES = 2, ///< Remove previously added custom mode. Allows single or multiple mode
                                                    ///< removal at a time.
    CTL_CUSTOM_MODE_OPERATION_TYPES_MAX

} ctl_custom_mode_operation_types_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Custom Mode
typedef struct _ctl_get_set_custom_mode_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_custom_mode_operation_types_t CustomModeOpType; ///< [in] Custom mode operation type
    uint32_t NumOfModes;                            ///< [in,out] Number of Custom Src Modes to be added/removed/Read.
    ctl_custom_src_mode_t* pCustomSrcModeList;      ///< [in,out] Custom mode source list which holds source modes to be
                                                    ///< added/removed/Read.

} ctl_get_set_custom_mode_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Custom Mode
typedef struct _ctl_custom_src_mode_t
{
    uint32_t SourceX;                               ///< [in,out] CustomMode Source X Size
    uint32_t SourceY;                               ///< [in,out] CustomMode Source Y Size 

} ctl_custom_src_mode_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Custom mode.
/// 
/// @details
///     - To get or set custom mode.
///     - Add custom source mode operation supports only single mode additon at
///       a time.
///     - Remove custom source mode operation supports single or multiple mode
///       removal at a time.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCustomModeArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernal mode driver call failure"
///     - ::CTL_RESULT_ERROR_INVALID_ARGUMENT - "Invalid combination of parameters"
///     - ::CTL_RESULT_ERROR_CUSTOM_MODE_STANDARD_CUSTOM_MODE_EXISTS - "Standard custom mode exists"
///     - ::CTL_RESULT_ERROR_CUSTOM_MODE_NON_CUSTOM_MATCHING_MODE_EXISTS - "Non custom matching mode exists"
///     - ::CTL_RESULT_ERROR_CUSTOM_MODE_INSUFFICIENT_MEMORY - "Custom mode insufficent memory"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSetCustomMode(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_get_set_custom_mode_args_t* pCustomModeArgs ///< [in,out] Custom mode arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Combined Display operation type
typedef enum _ctl_combined_display_optype_t
{
    CTL_COMBINED_DISPLAY_OPTYPE_IS_SUPPORTED_CONFIG = 1,///< To check whether given outputs can form a combined display, no changes
                                                    ///< are applied
    CTL_COMBINED_DISPLAY_OPTYPE_ENABLE = 2,         ///< To setup and enable a combined display
    CTL_COMBINED_DISPLAY_OPTYPE_DISABLE = 3,        ///< To disable combined display
    CTL_COMBINED_DISPLAY_OPTYPE_QUERY_CONFIG = 4,   ///< To query combined display configuration
    CTL_COMBINED_DISPLAY_OPTYPE_MAX

} ctl_combined_display_optype_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Combined Display's child display target mode
typedef struct _ctl_child_display_target_mode_t
{
    uint32_t Width;                                 ///< [in,out] Width
    uint32_t Height;                                ///< [in,out] Height
    float RefreshRate;                              ///< [in,out] Refresh Rate
    uint32_t ReservedFields[4];                     ///< [out] Reserved field of 16 bytes

} ctl_child_display_target_mode_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Combined Display's child display information
typedef struct _ctl_combined_display_child_info_t
{
    ctl_display_output_handle_t hDisplayOutput;     ///< [in,out] Display output handle under combined display configuration
    ctl_rect_t FbSrc;                               ///< [in,out] FrameBuffer source's RECT within Combined Display respective
    ctl_rect_t FbPos;                               ///< [in,out] FrameBuffer target's RECT within output size
    ctl_display_orientation_t DisplayOrientation;   ///< [in,out] 0/180 Degree Display orientation (rotation)
    ctl_child_display_target_mode_t TargetMode;     ///< [in,out] Desired target mode (width, height, refresh)

} ctl_combined_display_child_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Combined Display arguments
typedef struct _ctl_combined_display_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_combined_display_optype_t OpType;           ///< [in] Combined display operation type
    bool IsSupported;                               ///< [out] Returns yes/no in response to IS_SUPPORTED_CONFIG command
    uint8_t NumOutputs;                             ///< [in,out] Number of outputs part of desired combined display
                                                    ///< configuration
    uint32_t CombinedDesktopWidth;                  ///< [in,out] Width of desired combined display configuration
    uint32_t CombinedDesktopHeight;                 ///< [in,out] Height of desired combined display configuration
    ctl_combined_display_child_info_t* pChildInfo;  ///< [in,out] List of child display information respective to each output.
                                                    ///< Up to 16 displays are supported with up to 4 displays per GPU.
    ctl_display_output_handle_t hCombinedDisplayOutput; ///< [in,out] Handle to combined display output

} ctl_combined_display_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Combined Display
/// 
/// @details
///     - To get or set combined display with given Child Targets on a Single
///       GPU or across identical GPUs. Multi-GPU(MGPU) combined display is
///       reserved i.e. it is not public and requires special application GUID.
///       MGPU Combined Display will get activated or deactivated in next boot.
///       MGPU scenario will internally link the associated adapters via Linked
///       Display Adapter Call, with supplied hDeviceAdapter being the LDA
///       Primary. If Genlock and enabled in Driver registry and supported by
///       given Display Config, MGPU Combined Display will enable MGPU Genlock
///       with supplied hDeviceAdapter being the Genlock Primary Adapter and the
///       First Child Display being the Primary Display.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCombinedDisplayArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
///     - ::CTL_RESULT_ERROR_FEATURE_NOT_SUPPORTED - "Combined Display feature is not supported in this platform"
///     - ::CTL_RESULT_ERROR_ADAPTER_NOT_SUPPORTED_ON_LDA_SECONDARY - "Unsupported (secondary) adapter handle passed"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSetCombinedDisplay(
    ctl_device_adapter_handle_t hDeviceAdapter,     ///< [in][release] Handle to control device adapter
    ctl_combined_display_args_t* pCombinedDisplayArgs   ///< [in,out] Setup and get combined display arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Display Genlock Operations
typedef enum _ctl_genlock_operation_t
{
    CTL_GENLOCK_OPERATION_GET_TIMING_DETAILS = 0,   ///< Get details of GENLOCK support and timing information
    CTL_GENLOCK_OPERATION_VALIDATE = 1,             ///< Driver to verify that the topology is Genlock capable
    CTL_GENLOCK_OPERATION_ENABLE = 2,               ///< Enable GENLOCK
    CTL_GENLOCK_OPERATION_DISABLE = 3,              ///< Disable GENLOCK
    CTL_GENLOCK_OPERATION_GET_TOPOLOGY = 4,         ///< Get details of the current Genlock topology that is applied
    CTL_GENLOCK_OPERATION_MAX

} ctl_genlock_operation_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Display Genlock Info
typedef struct _ctl_genlock_display_info_t
{
    ctl_display_output_handle_t hDisplayOutput;     ///< [in,out] Display output handle under Genlock topology
    bool IsPrimary;                                 ///< [in,out] Genlock Primary

} ctl_genlock_display_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Genlock Target Mode List
typedef struct _ctl_genlock_target_mode_list_t
{
    ctl_display_output_handle_t hDisplayOutput;     ///< [in] Display output handle for whom target mode list is required
    uint32_t NumModes;                              ///< [in,out] Number of supported Modes that is returned from a driver
    ctl_display_timing_t* pTargetModes;             ///< [out] Display Genlock operation and information

} ctl_genlock_target_mode_list_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Genlock Topology
typedef struct _ctl_genlock_topology_t
{
    uint8_t NumGenlockDisplays;                     ///< [in,out] Number of Genlock displays
    bool IsPrimaryGenlockSystem;                    ///< [in,out] Primary Genlock system
    ctl_display_timing_t CommonTargetMode;          ///< [in] Common target mode
    ctl_genlock_display_info_t* pGenlockDisplayInfo;///< [in,out] List of Genlock display info
    ctl_genlock_target_mode_list_t* pGenlockModeList;   ///< [out] List of Genlock target modes

} ctl_genlock_topology_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Display Genlock Arg type
typedef struct _ctl_genlock_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_genlock_operation_t Operation;              ///< [in] Display Genlock Operation
    ctl_genlock_topology_t GenlockTopology;         ///< [in,out] Display Genlock array of topology structures
    bool IsGenlockEnabled;                          ///< [out] Whether the feature is currently enabled or not
    bool IsGenlockPossible;                         ///< [out] Indicates if Genlock can be enabled/disabled with the given
                                                    ///< topology

} ctl_genlock_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Display Genlock
/// 
/// @details
///     - To get or set Display Genlock.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == hDeviceAdapter`
///         + `nullptr == pGenlockArgs`
///         + `nullptr == hFailureDeviceAdapter`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_INVALID_SIZE - "Invalid topology structure size"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSetDisplayGenlock(
    ctl_device_adapter_handle_t* hDeviceAdapter,    ///< [in][release] Handle to control device adapter
    ctl_genlock_args_t* pGenlockArgs,               ///< [in,out] Display Genlock operation and information
    uint32_t AdapterCount,                          ///< [in] Number of device adapters
    ctl_device_adapter_handle_t* hFailureDeviceAdapter  ///< [out] Handle to address the failure device adapter in an error case
    );

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_MAX_DISPLAYS_FOR_MGPU_COLLAGE
/// @brief Maximum number of displays for Single Large Screen
#define CTL_MAX_DISPLAYS_FOR_MGPU_COLLAGE  16
#endif // CTL_MAX_DISPLAYS_FOR_MGPU_COLLAGE

///////////////////////////////////////////////////////////////////////////////
/// @brief Vblank timestamp arguments
typedef struct _ctl_vblank_ts_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint8_t NumOfTargets;                           ///< [out] Number of child targets
    uint64_t VblankTS[CTL_MAX_DISPLAYS_FOR_MGPU_COLLAGE];   ///< [out] List of vblank timestamps in microseconds per child target

} ctl_vblank_ts_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Vblank Timestamp
/// 
/// @details
///     - To get a list of vblank timestamps in microseconds for each child
///       target of a display.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pVblankTSArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS - "Insufficient permissions"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetVblankTimestamp(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_vblank_ts_args_t* pVblankTSArgs             ///< [out] Get vblank timestamp arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Link Display Adapters Arguments
typedef struct _ctl_lda_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint8_t NumAdapters;                            ///< [in,out] Numbers of adapters to be linked. Up to 4 adapters are
                                                    ///< supported
    ctl_device_adapter_handle_t* hLinkedAdapters;   ///< [in,out][release] List of Control device adapter handles to be linked,
                                                    ///< first one being Primary Adapter
    uint64_t Reserved[4];                           ///< [out] Reserved fields. Set to zero.

} ctl_lda_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Link Display Adapters
/// 
/// @details
///     - To Link Display Adapters.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPrimaryAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pLdaArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
///     - ::CTL_RESULT_ERROR_ADAPTER_ALREADY_LINKED - "Adapter is already linked"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlLinkDisplayAdapters(
    ctl_device_adapter_handle_t hPrimaryAdapter,    ///< [in][release] Handle to Primary adapter in LDA chain
    ctl_lda_args_t* pLdaArgs                        ///< [in] Link Display Adapters Arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Unlink Display Adapters
/// 
/// @details
///     - To Unlink Display Adapters
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPrimaryAdapter`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
///     - ::CTL_RESULT_ERROR_ADAPTER_NOT_SUPPORTED_ON_LDA_SECONDARY - "Unsupported (secondary) adapter handle passed"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlUnlinkDisplayAdapters(
    ctl_device_adapter_handle_t hPrimaryAdapter     ///< [in][release] Handle to Primary adapter in LDA chain
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Linked Display Adapters
/// 
/// @details
///     - To return list of Linked Display Adapters.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPrimaryAdapter`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pLdaArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
///     - ::CTL_RESULT_ERROR_ADAPTER_NOT_SUPPORTED_ON_LDA_SECONDARY - "Unsupported (secondary) adapter handle passed"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetLinkedDisplayAdapters(
    ctl_device_adapter_handle_t hPrimaryAdapter,    ///< [in][release] Handle to Primary adapter in LDA chain
    ctl_lda_args_t* pLdaArgs                        ///< [out] Link Display Adapters Arguments
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Dynamic Contrast Enhancement arguments
typedef struct _ctl_dce_args_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool Set;                                       ///< [in] Flag to indicate Set or Get operation
    uint32_t TargetBrightnessPercent;               ///< [in] Target brightness percent
    double PhaseinSpeedMultiplier;                  ///< [in] Phase-in speed multiplier for brightness to take effect
    uint32_t NumBins;                               ///< [in,out] Number of histogram bins
    bool Enable;                                    ///< [in,out] For get calls, this represents current state & for set this
                                                    ///< represents future state
    bool IsSupported;                               ///< [out] is DCE feature supported
    uint32_t* pHistogram;                           ///< [out] Bin wise histogram data of size NumBins * sizeof(uint32_t) for
                                                    ///< current frame

} ctl_dce_args_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Dynamic Contrast Enhancement
/// 
/// @details
///     - To get the DCE feature status and, if feature is enabled, returns the
///       current histogram, or to set the brightness at the phase-in speed
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDisplayOutput`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pDceArgs`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
///     - ::CTL_RESULT_ERROR_NULL_OS_DISPLAY_OUTPUT_HANDLE - "Null OS display output handle"
///     - ::CTL_RESULT_ERROR_NULL_OS_INTERFACE - "Null OS interface"
///     - ::CTL_RESULT_ERROR_NULL_OS_ADAPATER_HANDLE - "Null OS adapter handle"
///     - ::CTL_RESULT_ERROR_KMD_CALL - "Kernel mode driver call failure"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_HANDLE - "Invalid or Null handle passed"
///     - ::CTL_RESULT_ERROR_INVALID_NULL_POINTER - "Invalid null pointer"
///     - ::CTL_RESULT_ERROR_INVALID_OPERATION_TYPE - "Invalid operation type"
///     - ::CTL_RESULT_ERROR_INVALID_ARGUMENT - "Invalid combination of parameters"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSetDynamicContrastEnhancement(
    ctl_display_output_handle_t hDisplayOutput,     ///< [in] Handle to display output
    ctl_dce_args_t* pDceArgs                        ///< [in,out] Dynamic Contrast Enhancement arguments
    );


#if !defined(__GNUC__)
#pragma endregion // display
#endif
// Intel 'ctlApi' for Device Adapter - Engine groups
#if !defined(__GNUC__)
#pragma region engine
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Accelerator engine groups
typedef enum _ctl_engine_group_t
{
    CTL_ENGINE_GROUP_GT = 0,                        ///< Access information about all engines combined.
    CTL_ENGINE_GROUP_RENDER = 1,                    ///< Access information about all render and compute engines combined.
    CTL_ENGINE_GROUP_MEDIA = 2,                     ///< Access information about all media engines combined.
    CTL_ENGINE_GROUP_MAX

} ctl_engine_group_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Engine group properties
typedef struct _ctl_engine_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_engine_group_t type;                        ///< [out] The engine group

} ctl_engine_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Engine activity counters
/// 
/// @details
///     - Percent utilization is calculated by taking two snapshots (s1, s2) and
///       using the equation: %util = (s2.activeTime - s1.activeTime) /
///       (s2.timestamp - s1.timestamp)
typedef struct _ctl_engine_stats_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint64_t activeTime;                            ///< [out] Monotonic counter for time in microseconds that this resource is
                                                    ///< actively running workloads.
    uint64_t timestamp;                             ///< [out] Monotonic timestamp counter in microseconds when activeTime
                                                    ///< counter was sampled.
                                                    ///< This timestamp should only be used to calculate delta time between
                                                    ///< snapshots of this structure.
                                                    ///< Never take the delta of this timestamp with the timestamp from a
                                                    ///< different structure since they are not guaranteed to have the same base.
                                                    ///< The absolute value of the timestamp is only valid during within the
                                                    ///< application and may be different on the next execution.

} ctl_engine_stats_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of engine groups
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumEngineGroups(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to adapter
    uint32_t* pCount,                               ///< [in,out] pointer to the number of components of this type.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of components of this type that are available.
                                                    ///< if count is greater than the number of components of this type that
                                                    ///< are available, then the driver shall update the value with the correct
                                                    ///< number of components.
    ctl_engine_handle_t* phEngine                   ///< [in,out][optional][range(0, *pCount)] array of handle of components of
                                                    ///< this type.
                                                    ///< if count is less than the number of components of this type that are
                                                    ///< available, then the driver shall only retrieve that number of
                                                    ///< component handles.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get engine group properties
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hEngine`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEngineGetProperties(
    ctl_engine_handle_t hEngine,                    ///< [in] Handle for the component.
    ctl_engine_properties_t* pProperties            ///< [in,out] The properties for the specified engine group.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the activity stats for an engine group
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hEngine`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pStats`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEngineGetActivity(
    ctl_engine_handle_t hEngine,                    ///< [in] Handle for the component.
    ctl_engine_stats_t* pStats                      ///< [in,out] Will contain a snapshot of the engine group activity
                                                    ///< counters.
    );


#if !defined(__GNUC__)
#pragma endregion // engine
#endif
// Intel 'ctlApi' for Device Adapter- Fan management
#if !defined(__GNUC__)
#pragma region fan
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Fan resource speed mode
typedef enum _ctl_fan_speed_mode_t
{
    CTL_FAN_SPEED_MODE_DEFAULT = 0,                 ///< The fan speed is operating using the hardware default settings
    CTL_FAN_SPEED_MODE_FIXED = 1,                   ///< The fan speed is currently set to a fixed value
    CTL_FAN_SPEED_MODE_TABLE = 2,                   ///< The fan speed is currently controlled dynamically by hardware based on
                                                    ///< a temp/speed table
    CTL_FAN_SPEED_MODE_MAX

} ctl_fan_speed_mode_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Fan speed units
typedef enum _ctl_fan_speed_units_t
{
    CTL_FAN_SPEED_UNITS_RPM = 0,                    ///< The fan speed is in units of revolutions per minute (rpm)
    CTL_FAN_SPEED_UNITS_PERCENT = 1,                ///< The fan speed is a percentage of the maximum speed of the fan
    CTL_FAN_SPEED_UNITS_MAX

} ctl_fan_speed_units_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Fan speed
typedef struct _ctl_fan_speed_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    int32_t speed;                                  ///< [in,out] The speed of the fan. On output, a value of -1 indicates that
                                                    ///< there is no fixed fan speed setting.
    ctl_fan_speed_units_t units;                    ///< [in,out] The units that the fan speed is expressed in. On output, if
                                                    ///< fan speed is -1 then units should be ignored.

} ctl_fan_speed_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Fan temperature/speed pair
typedef struct _ctl_fan_temp_speed_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t temperature;                           ///< [in,out] Temperature in degrees Celsius.
    ctl_fan_speed_t speed;                          ///< [in,out] The speed of the fan

} ctl_fan_temp_speed_t;

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_FAN_TEMP_SPEED_PAIR_COUNT
/// @brief Maximum number of fan temperature/speed pairs in the fan speed table.
#define CTL_FAN_TEMP_SPEED_PAIR_COUNT  32
#endif // CTL_FAN_TEMP_SPEED_PAIR_COUNT

///////////////////////////////////////////////////////////////////////////////
/// @brief Fan speed table
typedef struct _ctl_fan_speed_table_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    int32_t numPoints;                              ///< [in,out] The number of valid points in the fan speed table. 0 means
                                                    ///< that there is no fan speed table configured. -1 means that a fan speed
                                                    ///< table is not supported by the hardware.
    ctl_fan_temp_speed_t table[CTL_FAN_TEMP_SPEED_PAIR_COUNT];  ///< [in,out] Array of temperature/fan speed pairs. The table is ordered
                                                    ///< based on temperature from lowest to highest.

} ctl_fan_speed_table_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Fan properties
typedef struct _ctl_fan_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool canControl;                                ///< [out] Indicates if software can control the fan speed assuming the
                                                    ///< user has permissions
    uint32_t supportedModes;                        ///< [out] Bitfield of supported fan configuration modes
                                                    ///< (1<<::ctl_fan_speed_mode_t)
    uint32_t supportedUnits;                        ///< [out] Bitfield of supported fan speed units
                                                    ///< (1<<::ctl_fan_speed_units_t)
    int32_t maxRPM;                                 ///< [out] The maximum RPM of the fan. A value of -1 means that this
                                                    ///< property is unknown. 
    int32_t maxPoints;                              ///< [out] The maximum number of points in the fan temp/speed table. A
                                                    ///< value of -1 means that this fan doesn't support providing a temp/speed
                                                    ///< table.

} ctl_fan_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Fan configuration
typedef struct _ctl_fan_config_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_fan_speed_mode_t mode;                      ///< [in,out] The fan speed mode (fixed, temp-speed table)
    ctl_fan_speed_t speedFixed;                     ///< [in,out] The current fixed fan speed setting
    ctl_fan_speed_table_t speedTable;               ///< [out] A table containing temperature/speed pairs

} ctl_fan_config_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of fans
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumFans(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to the adapter
    uint32_t* pCount,                               ///< [in,out] pointer to the number of components of this type.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of components of this type that are available.
                                                    ///< if count is greater than the number of components of this type that
                                                    ///< are available, then the driver shall update the value with the correct
                                                    ///< number of components.
    ctl_fan_handle_t* phFan                         ///< [in,out][optional][range(0, *pCount)] array of handle of components of
                                                    ///< this type.
                                                    ///< if count is less than the number of components of this type that are
                                                    ///< available, then the driver shall only retrieve that number of
                                                    ///< component handles.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get fan properties
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFan`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFanGetProperties(
    ctl_fan_handle_t hFan,                          ///< [in] Handle for the component.
    ctl_fan_properties_t* pProperties               ///< [in,out] Will contain the properties of the fan.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get fan configurations and the current fan speed mode (default, fixed,
///        temp-speed table)
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFan`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pConfig`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFanGetConfig(
    ctl_fan_handle_t hFan,                          ///< [in] Handle for the component.
    ctl_fan_config_t* pConfig                       ///< [in,out] Will contain the current configuration of the fan.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Configure the fan to run with hardware factory settings (set mode to
///        ::CTL_FAN_SPEED_MODE_DEFAULT)
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFan`
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
///         + User does not have permissions to make these modifications.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFanSetDefaultMode(
    ctl_fan_handle_t hFan                           ///< [in] Handle for the component.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Configure the fan to rotate at a fixed speed (set mode to
///        ::CTL_FAN_SPEED_MODE_FIXED)
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFan`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == speed`
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
///         + User does not have permissions to make these modifications.
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_FEATURE
///         + Fixing the fan speed not supported by the hardware or the fan speed units are not supported. See ::ctl_fan_properties_t.supportedModes and ::ctl_fan_properties_t.supportedUnits.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFanSetFixedSpeedMode(
    ctl_fan_handle_t hFan,                          ///< [in] Handle for the component.
    const ctl_fan_speed_t* speed                    ///< [in] The fixed fan speed setting
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Configure the fan to adjust speed based on a temperature/speed table
///        (set mode to ::CTL_FAN_SPEED_MODE_TABLE)
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFan`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == speedTable`
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
///         + User does not have permissions to make these modifications.
///     - ::CTL_RESULT_ERROR_INVALID_ARGUMENT
///         + The temperature/speed pairs in the array are not sorted on temperature from lowest to highest.
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_FEATURE
///         + Fan speed table not supported by the hardware or the fan speed units are not supported. See ::ctl_fan_properties_t.supportedModes and ::ctl_fan_properties_t.supportedUnits.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFanSetSpeedTableMode(
    ctl_fan_handle_t hFan,                          ///< [in] Handle for the component.
    const ctl_fan_speed_table_t* speedTable         ///< [in] A table containing temperature/speed pairs.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get current state of a fan - current mode and speed
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFan`
///     - CTL_RESULT_ERROR_INVALID_ENUMERATION
///         + `::CTL_FAN_SPEED_UNITS_PERCENT < units`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSpeed`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_FEATURE
///         + The requested fan speed units are not supported. See ::ctl_fan_properties_t.supportedUnits.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFanGetState(
    ctl_fan_handle_t hFan,                          ///< [in] Handle for the component.
    ctl_fan_speed_units_t units,                    ///< [in] The units in which the fan speed should be returned.
    int32_t* pSpeed                                 ///< [in,out] Will contain the current speed of the fan in the units
                                                    ///< requested. A value of -1 indicates that the fan speed cannot be
                                                    ///< measured.
    );


#if !defined(__GNUC__)
#pragma endregion // fan
#endif
// Intel 'ctlApi' for Device Adapter - Frequency domains
#if !defined(__GNUC__)
#pragma region frequency
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Frequency domains.
typedef enum _ctl_freq_domain_t
{
    CTL_FREQ_DOMAIN_GPU = 0,                        ///< GPU Core Domain.
    CTL_FREQ_DOMAIN_MEMORY = 1,                     ///< Local Memory Domain.
    CTL_FREQ_DOMAIN_MAX

} ctl_freq_domain_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Frequency properties
typedef struct _ctl_freq_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_freq_domain_t type;                         ///< [out] The hardware block that this frequency domain controls (GPU,
                                                    ///< memory, ...)
    bool canControl;                                ///< [out] Indicates if software can control the frequency of this domain
                                                    ///< assuming the user has permissions
    double min;                                     ///< [out] The minimum hardware clock frequency in units of MHz.
    double max;                                     ///< [out] The maximum non-overclock hardware clock frequency in units of
                                                    ///< MHz.

} ctl_freq_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Frequency range between which the hardware can operate. The limits can
///        be above or below the hardware limits - the hardware will clamp
///        appropriately.
typedef struct _ctl_freq_range_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    double min;                                     ///< [in,out] The min frequency in MHz below which hardware frequency
                                                    ///< management will not request frequencies. On input, setting to 0 will
                                                    ///< permit the frequency to go down to the hardware minimum. On output, a
                                                    ///< negative value indicates that no external minimum frequency limit is
                                                    ///< in effect.
    double max;                                     ///< [in,out] The max frequency in MHz above which hardware frequency
                                                    ///< management will not request frequencies. On input, setting to 0 or a
                                                    ///< very big number will permit the frequency to go all the way up to the
                                                    ///< hardware maximum. On output, a negative number indicates that no
                                                    ///< external maximum frequency limit is in effect.

} ctl_freq_range_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Frequency throttle reasons
typedef uint32_t ctl_freq_throttle_reason_flags_t;
typedef enum _ctl_freq_throttle_reason_flag_t
{
    CTL_FREQ_THROTTLE_REASON_FLAG_AVE_PWR_CAP = CTL_BIT(0), ///< frequency throttled due to average power excursion (PL1)
    CTL_FREQ_THROTTLE_REASON_FLAG_BURST_PWR_CAP = CTL_BIT(1),   ///< frequency throttled due to burst power excursion (PL2)
    CTL_FREQ_THROTTLE_REASON_FLAG_CURRENT_LIMIT = CTL_BIT(2),   ///< frequency throttled due to current excursion (PL4)
    CTL_FREQ_THROTTLE_REASON_FLAG_THERMAL_LIMIT = CTL_BIT(3),   ///< frequency throttled due to thermal excursion (T > TjMax)
    CTL_FREQ_THROTTLE_REASON_FLAG_PSU_ALERT = CTL_BIT(4),   ///< frequency throttled due to power supply assertion
    CTL_FREQ_THROTTLE_REASON_FLAG_SW_RANGE = CTL_BIT(5),///< frequency throttled due to software supplied frequency range
    CTL_FREQ_THROTTLE_REASON_FLAG_HW_RANGE = CTL_BIT(6),///< frequency throttled due to a sub block that has a lower frequency
                                                    ///< range when it receives clocks
    CTL_FREQ_THROTTLE_REASON_FLAG_MAX = 0x80000000

} ctl_freq_throttle_reason_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Frequency state
typedef struct _ctl_freq_state_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    double currentVoltage;                          ///< [out] Current voltage in Volts. A negative value indicates that this
                                                    ///< property is not known.
    double request;                                 ///< [out] The current frequency request in MHz. A negative value indicates
                                                    ///< that this property is not known.
    double tdp;                                     ///< [out] The maximum frequency in MHz supported under the current TDP
                                                    ///< conditions. This fluctuates dynamically based on the power and thermal
                                                    ///< limits of the part. A negative value indicates that this property is
                                                    ///< not known.
    double efficient;                               ///< [out] The efficient minimum frequency in MHz. A negative value
                                                    ///< indicates that this property is not known.
    double actual;                                  ///< [out] The resolved frequency in MHz. A negative value indicates that
                                                    ///< this property is not known.
    ctl_freq_throttle_reason_flags_t throttleReasons;   ///< [out] The reasons that the frequency is being limited by the hardware.
                                                    ///< Returns 0 (frequency not throttled) or a combination of ::ctl_freq_throttle_reason_flag_t.

} ctl_freq_state_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Frequency throttle time snapshot
/// 
/// @details
///     - Percent time throttled is calculated by taking two snapshots (s1, s2)
///       and using the equation: %throttled = (s2.throttleTime -
///       s1.throttleTime) / (s2.timestamp - s1.timestamp)
typedef struct _ctl_freq_throttle_time_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint64_t throttleTime;                          ///< [out] The monotonic counter of time in microseconds that the frequency
                                                    ///< has been limited by the hardware.
    uint64_t timestamp;                             ///< [out] Microsecond timestamp when throttleTime was captured.
                                                    ///< This timestamp should only be used to calculate delta time between
                                                    ///< snapshots of this structure.
                                                    ///< Never take the delta of this timestamp with the timestamp from a
                                                    ///< different structure since they are not guaranteed to have the same base.
                                                    ///< The absolute value of the timestamp is only valid during within the
                                                    ///< application and may be different on the next execution.

} ctl_freq_throttle_time_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of frequency domains
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumFrequencyDomains(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    uint32_t* pCount,                               ///< [in,out] pointer to the number of components of this type.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of components of this type that are available.
                                                    ///< if count is greater than the number of components of this type that
                                                    ///< are available, then the driver shall update the value with the correct
                                                    ///< number of components.
    ctl_freq_handle_t* phFrequency                  ///< [in,out][optional][range(0, *pCount)] array of handle of components of
                                                    ///< this type.
                                                    ///< if count is less than the number of components of this type that are
                                                    ///< available, then the driver shall only retrieve that number of
                                                    ///< component handles.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get frequency properties - available frequencies
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFrequency`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFrequencyGetProperties(
    ctl_freq_handle_t hFrequency,                   ///< [in] Handle for the component.
    ctl_freq_properties_t* pProperties              ///< [in,out] The frequency properties for the specified domain.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get available non-overclocked hardware clock frequencies for the
///        frequency domain
/// 
/// @details
///     - The list of available frequencies is returned in order of slowest to
///       fastest.
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFrequency`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFrequencyGetAvailableClocks(
    ctl_freq_handle_t hFrequency,                   ///< [in] Device handle of the device.
    uint32_t* pCount,                               ///< [in,out] pointer to the number of frequencies.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of frequencies that are available.
                                                    ///< if count is greater than the number of frequencies that are available,
                                                    ///< then the driver shall update the value with the correct number of frequencies.
    double* phFrequency                             ///< [in,out][optional][range(0, *pCount)] array of frequencies in units of
                                                    ///< MHz and sorted from slowest to fastest.
                                                    ///< if count is less than the number of frequencies that are available,
                                                    ///< then the driver shall only retrieve that number of frequencies.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get current frequency limits
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFrequency`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pLimits`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFrequencyGetRange(
    ctl_freq_handle_t hFrequency,                   ///< [in] Handle for the component.
    ctl_freq_range_t* pLimits                       ///< [in,out] The range between which the hardware can operate for the
                                                    ///< specified domain.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set frequency range between which the hardware can operate.
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFrequency`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pLimits`
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
///         + User does not have permissions to make these modifications.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFrequencySetRange(
    ctl_freq_handle_t hFrequency,                   ///< [in] Handle for the component.
    const ctl_freq_range_t* pLimits                 ///< [in] The limits between which the hardware can operate for the
                                                    ///< specified domain.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get current frequency state - frequency request, actual frequency, TDP
///        limits
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFrequency`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pState`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFrequencyGetState(
    ctl_freq_handle_t hFrequency,                   ///< [in] Handle for the component.
    ctl_freq_state_t* pState                        ///< [in,out] Frequency state for the specified domain.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get frequency throttle time
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hFrequency`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pThrottleTime`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlFrequencyGetThrottleTime(
    ctl_freq_handle_t hFrequency,                   ///< [in] Handle for the component.
    ctl_freq_throttle_time_t* pThrottleTime         ///< [in,out] Will contain a snapshot of the throttle time counters for the
                                                    ///< specified domain.
    );


#if !defined(__GNUC__)
#pragma endregion // frequency
#endif
// Intel 'ctlApi' for Device Adapter
#if !defined(__GNUC__)
#pragma region media
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Feature type
typedef enum _ctl_video_processing_feature_t
{
    CTL_VIDEO_PROCESSING_FEATURE_FILM_MODE_DETECTION = 0,   ///< Film mode detection.  Contains CTL_PROPERTY_VALUE_TYPE_BOOL ValueType.
    CTL_VIDEO_PROCESSING_FEATURE_NOISE_REDUCTION = 1,   ///< Noise reduction.  Contains CTL_PROPERTY_VALUE_TYPE_CUSTOM type field
                                                    ///< using struct ::ctl_video_processing_noise_reduction_t.
    CTL_VIDEO_PROCESSING_FEATURE_SHARPNESS = 2,     ///< Sharpness.  Contains CTL_PROPERTY_VALUE_TYPE_UINT32 ValueType.
    CTL_VIDEO_PROCESSING_FEATURE_ADAPTIVE_CONTRAST_ENHANCEMENT = 3, ///< Adaptive contrast enhancement.  Contains
                                                    ///< CTL_PROPERTY_VALUE_TYPE_CUSTOM type field using struct
                                                    ///< ::ctl_video_processing_adaptive_contrast_enhancement_t.
    CTL_VIDEO_PROCESSING_FEATURE_SUPER_RESOLUTION = 4,  ///< Super resolution.  Contains CTL_PROPERTY_VALUE_TYPE_CUSTOM ValueType
                                                    ///< using ::ctl_video_processing_super_resolution_t. By defaut, Super
                                                    ///< resolution is not active, need application to activate it, please
                                                    ///< contact Intel for super resolution activation.
    CTL_VIDEO_PROCESSING_FEATURE_STANDARD_COLOR_CORRECTION = 5, ///< Standard color correction.  Controls Hue, Saturation, Contrast,
                                                    ///< Brightness.  Contains CTL_PROPERTY_VALUE_TYPE_CUSTOM type field using
                                                    ///< struct ::ctl_video_processing_standard_color_correction_t.
    CTL_VIDEO_PROCESSING_FEATURE_TOTAL_COLOR_CORRECTION = 6,///< Total color correction.  Controls Red, Green, Blue, Yellow, Cyan,
                                                    ///< Magenta.  Contains CTL_PROPERTY_VALUE_TYPE_CUSTOM type field using
                                                    ///< struct ::ctl_video_processing_total_color_correction_t.
    CTL_VIDEO_PROCESSING_FEATURE_SKIN_TONE_ENHANCEMENT = 7, ///< Skin tone enhancement.  Contains CTL_PROPERTY_VALUE_TYPE_UINT32
                                                    ///< ValueType.
    CTL_VIDEO_PROCESSING_FEATURE_MAX

} ctl_video_processing_feature_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Super resolution values possible
typedef uint32_t ctl_video_processing_super_resolution_flags_t;
typedef enum _ctl_video_processing_super_resolution_flag_t
{
    CTL_VIDEO_PROCESSING_SUPER_RESOLUTION_FLAG_DISABLE = CTL_BIT(0),///< Disable
    CTL_VIDEO_PROCESSING_SUPER_RESOLUTION_FLAG_ENABLE_DEFAULT_SCENARIO_MODE = CTL_BIT(1),   ///< Enable with default super resolution mode
    CTL_VIDEO_PROCESSING_SUPER_RESOLUTION_FLAG_ENABLE_CONFERENCE_SCENARIO_MODE = CTL_BIT(2),///< Super resolution mode targeted at video conference content
    CTL_VIDEO_PROCESSING_SUPER_RESOLUTION_FLAG_ENABLE_CAMERA_SCENARIO_MODE = CTL_BIT(3),///< Super resolution mode targeted at camera capture content (e.g.
                                                    ///< security camera)
    CTL_VIDEO_PROCESSING_SUPER_RESOLUTION_FLAG_MAX = 0x80000000

} ctl_video_processing_super_resolution_flag_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Super Resolution feature details structure to be used with
///        SUPER_RESOLUTION
typedef struct _ctl_video_processing_super_resolution_info_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_video_processing_super_resolution_flags_t super_resolution_flag;///< [in,out] SUPER_RESOLUTION flag
    ctl_property_info_uint_t super_resolution_range_in_width;   ///< [in,out] The range of input width information(min, max, default and
                                                    ///< step size)which super resolution is capable of supporting.
    ctl_property_info_uint_t super_resolution_range_in_height;  ///< [in,out] The range of input height information(min, max, default and
                                                    ///< step size)which super resolution is capable of supporting.
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_super_resolution_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Super Resolution Get/Set structure to be used with SUPER_RESOLUTION
typedef struct _ctl_video_processing_super_resolution_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_video_processing_super_resolution_flags_t super_resolution_flag;///< [in,out] SUPER_RESOLUTION flag
    bool super_resolution_max_in_enabled;           ///< [in,out] The enabling of maximum input width and height limition. If
                                                    ///< enabled, super resolution will always take effect if the input
                                                    ///< resolution is smaller than the below specified max resolution;
                                                    ///< otherwise, super_resolution_max_in_width and
                                                    ///< super_resolution_max_in_height will be ignored
    uint32_t super_resolution_max_in_width;         ///< [in,out] The maximum input width limition value setting which super
                                                    ///< resolution will be allowed to enabled.
    uint32_t super_resolution_max_in_height;        ///< [in,out] The maximum input height limiation value setting which super
                                                    ///< resolution will be allowed to enabled.
    bool super_resolution_reboot_reset;             ///< [in,out] Resetting of super resolution after rebooting.
    uint32_t ReservedFields[15];                    ///< [out] Reserved field of 60 bytes
    char ReservedBytes[3];                          ///< [out] Reserved field of 3 bytes

} ctl_video_processing_super_resolution_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Noise Reduction feature details structure to be used with
///        NOISE_REDUCTION
typedef struct _ctl_video_processing_noise_reduction_info_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_property_info_uint_t noise_reduction;       ///< [in,out] Noise reduction min, max, default and step size information
    bool noise_reduction_auto_detect_supported;     ///< [in,out] Noise reduction Auto Detect is supported; only valid if
                                                    ///< NOISE_REDUCTION is enabled.  If enabled, noise reduction level is
                                                    ///< automatically determined and set value is not used.
    ctl_property_info_boolean_t noise_reduction_auto_detect;///< [in,out] Noise reduction auto detect default information
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_noise_reduction_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Noise Reduction Get/Set structure to be used with NOISE_REDUCTION
typedef struct _ctl_video_processing_noise_reduction_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_property_uint_t noise_reduction;            ///< [in,out] Noise reduction enable and value setting
    ctl_property_boolean_t noise_reduction_auto_detect; ///< [in,out] Noise reduction auto detect setting
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_noise_reduction_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adaptive Contrast Enhancement feature details structure to be used
///        with ADAPTIVE_CONTRAST_ENHANCEMENT
typedef struct _ctl_video_processing_adaptive_contrast_enhancement_info_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_property_info_uint_t adaptive_contrast_enhancement; ///< [in,out] Adaptive Contrast Enhancement min, max, default and step size
                                                    ///< information
    bool adaptive_contrast_enhancement_coexistence_supported;   ///< [in,out] Adaptive contrast enhancement coexistance is supported; only
                                                    ///< valid if ADAPTIVE_CONTRAST_ENHANCEMENT is enabled.  If enabled, Video
                                                    ///< adaptive contrast ehancement will be allowed to be enabled and coexist
                                                    ///< with Display adaptive contrast ehancement feature.
    ctl_property_info_boolean_t adaptive_contrast_enhancement_coexistence;  ///< [in,out] Adaptive contrast enhancement coexistence default information
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_adaptive_contrast_enhancement_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Adaptive Contrast Enhancement Get/Set structure to be used with
///        ADAPTIVE_CONTRAST_ENHANCEMENT
typedef struct _ctl_video_processing_adaptive_contrast_enhancement_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_property_uint_t adaptive_contrast_enhancement;  ///< [in,out] Adaptive Contrast Enhancement enable and value setting
    ctl_property_boolean_t adaptive_contrast_enhancement_coexistence;   ///< [in,out] Adaptive contrast enhancement coexistance setting
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_adaptive_contrast_enhancement_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Standard Color Correction feature details structure to be used with
///        STANDARD_COLOR_CORRECTION
typedef struct _ctl_video_processing_standard_color_correction_info_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool standard_color_correction_default_enable;  ///< [in,out] STANDARD_COLOR_CORRECTION default enable setting.  This
                                                    ///< global settings controls all of Hue, Saturation, Contrast, Brightness
                                                    ///< enabling.  Individual Enable controls shall be ignored.
    ctl_property_info_float_t brightness;           ///< [in,out] Brightness min, max, default and step size information
    ctl_property_info_float_t contrast;             ///< [in,out] Contrast min, max, default and step size information
    ctl_property_info_float_t hue;                  ///< [in,out] Hue min, max, default and step size information
    ctl_property_info_float_t saturation;           ///< [in,out] Saturation min, max, default and step size information
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_standard_color_correction_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Standard Color Correction Get/Set structure to be used with
///        STANDARD_COLOR_CORRECTION
typedef struct _ctl_video_processing_standard_color_correction_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool standard_color_correction_enable;          ///< [in,out] STANDARD_COLOR_CORRECTION enable setting.  This global
                                                    ///< setting controls all of Hue, Saturation, Contrast, Brightness
                                                    ///< enabling.
    float brightness;                               ///< [in,out] Brightness value
    float contrast;                                 ///< [in,out] Contrast value
    float hue;                                      ///< [in,out] Hue value
    float saturation;                               ///< [in,out] Saturation value
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_standard_color_correction_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Total Color Correction Get/Set structure to be used with
///        TOTAL_COLOR_CORRECTION
typedef struct _ctl_video_processing_total_color_correction_info_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool total_color_correction_default_enable;     ///< [in,out] TOTAL_COLOR_CORRECTION enable setting.  This global setting
                                                    ///< controls all of Red, Green, Blue, Yellow, Cyan, Magenta enabling. 
                                                    ///< Individual Enable controls shall be ignored.
    ctl_property_info_uint_t red;                   ///< [in,out] Red min, max, default and step size information
    ctl_property_info_uint_t green;                 ///< [in,out] Green min, max, default and step size information
    ctl_property_info_uint_t blue;                  ///< [in,out] Blue min, max, default and step size information
    ctl_property_info_uint_t yellow;                ///< [in,out] Yellow min, max, default and step size information
    ctl_property_info_uint_t cyan;                  ///< [in,out] Cyan min, max, default and step size information
    ctl_property_info_uint_t magenta;               ///< [in,out] Magenta min, max, default and step size information
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_total_color_correction_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Total Color Correction Get/Set structure to be used with
///        TOTAL_COLOR_CORRECTION
typedef struct _ctl_video_processing_total_color_correction_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool total_color_correction_enable;             ///< [in,out] TOTAL_COLOR_CORRECTION enable setting.  This global setting
                                                    ///< controls all of Red, Green, Blue, Yellow, Cyan, Magenta enabling.
    uint32_t red;                                   ///< [in,out] Red value
    uint32_t green;                                 ///< [in,out] Green value
    uint32_t blue;                                  ///< [in,out] Blue value
    uint32_t yellow;                                ///< [in,out] Yellow value
    uint32_t cyan;                                  ///< [in,out] Cyan value
    uint32_t magenta;                               ///< [in,out] Magenta value
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_total_color_correction_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Video Processing feature details which will have range supported and
///        default values
typedef struct _ctl_video_processing_feature_details_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_video_processing_feature_t FeatureType;     ///< [out] Video processing feature type
    ctl_property_value_type_t ValueType;            ///< [out] Type of value
    ctl_property_info_t Value;                      ///< [out] Union of various type of values for Video Processing features.
                                                    ///< For enum types this can be noise reduction, color control etc. This
                                                    ///< member is valid iff ValueType is not CTL_PROPERTY_VALUE_TYPE_CUSTOM
    int32_t CustomValueSize;                        ///< [in] CustomValue buffer size
    void* pCustomValue;                             ///< [in,out] Pointer to a custom structure. Features that use CustomType,
                                                    ///< after the first query for all of the supported features the user needs
                                                    ///< to allocate this buffer and then query again just this specific
                                                    ///< feature for the structure to be filled in. Caller should allocate this
                                                    ///< buffer with known custom feature structure size. This member is valid
                                                    ///< iff ValueType is CTL_PROPERTY_VALUE_TYPE_CUSTOM.
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_feature_details_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Video Processing features which are controllable
typedef struct _ctl_video_processing_feature_caps_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t NumSupportedFeatures;                  ///< [in,out] Number of elements in supported features array
    ctl_video_processing_feature_details_t* pFeatureDetails;///< [in,out] Array of supported features and their details
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_feature_caps_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Video Processing feature for get/set
typedef struct _ctl_video_processing_feature_getset_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_video_processing_feature_t FeatureType;     ///< [in] Features interested in
    char* ApplicationName;                          ///< [in] Application name for which the property type is applicable. If
                                                    ///< this is an empty string then this will get/set global settings for the
                                                    ///< given adapter. Note that this should contain only the name of the
                                                    ///< application and not the system specific path.  [This is not currently
                                                    ///< supported and should be an empty string.]
    int8_t ApplicationNameLength;                   ///< [in] Length of ApplicationName string
    bool bSet;                                      ///< [in] Set this if it's a set call
    ctl_property_value_type_t ValueType;            ///< [in] Type of value. Caller has to ensure it provides the right value
                                                    ///< type which decides how one read the union structure below
    ctl_property_t Value;                           ///< [in,out] Union of various type of values for Video Processing
                                                    ///< features. For enum types this can be noise reduction, color control
                                                    ///< etc. This member is valid iff ValueType is not
                                                    ///< CTL_PROPERTY_VALUE_TYPE_CUSTOM
    int32_t CustomValueSize;                        ///< [in] CustomValue buffer size.  For a feature requiring custom struct,
                                                    ///< caller will know of it upfront the struct to use based on the feautre
                                                    ///< and can provide the right size info here
    void* pCustomValue;                             ///< [in,out] Pointer to a custom structure. Caller should allocate this
                                                    ///< buffer with known custom feature structure size. This member is valid
                                                    ///< iff ValueType is CTL_PROPERTY_VALUE_TYPE_CUSTOM
    uint32_t ReservedFields[16];                    ///< [out] Reserved field of 64 bytes

} ctl_video_processing_feature_getset_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Video Processing capabilities
/// 
/// @details
///     - The application gets Video Processing properties
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pFeatureCaps`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSupportedVideoProcessingCapabilities(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    ctl_video_processing_feature_caps_t* pFeatureCaps   ///< [in,out][release] Video Processing properties
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get/Set Video Processing feature details
/// 
/// @details
///     - Video Processing feature details
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pFeature`
///     - ::CTL_RESULT_ERROR_UNSUPPORTED_VERSION - "Unsupported version"
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlGetSetVideoProcessingFeature(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    ctl_video_processing_feature_getset_t* pFeature ///< [in][release] Video Processing feature get/set parameter
    );


#if !defined(__GNUC__)
#pragma endregion // media
#endif
// Intel 'ctlApi' for Device Adapter - Memory management
#if !defined(__GNUC__)
#pragma region memory
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Memory module types
typedef enum _ctl_mem_type_t
{
    CTL_MEM_TYPE_HBM = 0,                           ///< HBM memory
    CTL_MEM_TYPE_DDR = 1,                           ///< DDR memory
    CTL_MEM_TYPE_DDR3 = 2,                          ///< DDR3 memory
    CTL_MEM_TYPE_DDR4 = 3,                          ///< DDR4 memory
    CTL_MEM_TYPE_DDR5 = 4,                          ///< DDR5 memory
    CTL_MEM_TYPE_LPDDR = 5,                         ///< LPDDR memory
    CTL_MEM_TYPE_LPDDR3 = 6,                        ///< LPDDR3 memory
    CTL_MEM_TYPE_LPDDR4 = 7,                        ///< LPDDR4 memory
    CTL_MEM_TYPE_LPDDR5 = 8,                        ///< LPDDR5 memory
    CTL_MEM_TYPE_GDDR4 = 9,                         ///< GDDR4 memory
    CTL_MEM_TYPE_GDDR5 = 10,                        ///< GDDR5 memory
    CTL_MEM_TYPE_GDDR5X = 11,                       ///< GDDR5X memory
    CTL_MEM_TYPE_GDDR6 = 12,                        ///< GDDR6 memory
    CTL_MEM_TYPE_GDDR6X = 13,                       ///< GDDR6X memory
    CTL_MEM_TYPE_GDDR7 = 14,                        ///< GDDR7 memory
    CTL_MEM_TYPE_UNKNOWN = 15,                      ///< UNKNOWN memory
    CTL_MEM_TYPE_MAX

} ctl_mem_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Memory module location
typedef enum _ctl_mem_loc_t
{
    CTL_MEM_LOC_SYSTEM = 0,                         ///< System memory
    CTL_MEM_LOC_DEVICE = 1,                         ///< On board local device memory
    CTL_MEM_LOC_MAX

} ctl_mem_loc_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Memory properties
typedef struct _ctl_mem_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_mem_type_t type;                            ///< [out] The memory type
    ctl_mem_loc_t location;                         ///< [out] Location of this memory (system, device)
    uint64_t physicalSize;                          ///< [out] Physical memory size in bytes. A value of 0 indicates that this
                                                    ///< property is not known. However, a call to ::ctlMemoryGetState() will
                                                    ///< correctly return the total size of usable memory.
    int32_t busWidth;                               ///< [out] Width of the memory bus. A value of -1 means that this property
                                                    ///< is unknown.
    int32_t numChannels;                            ///< [out] The number of memory channels. A value of -1 means that this
                                                    ///< property is unknown.

} ctl_mem_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Memory state - health, allocated
/// 
/// @details
///     - Percent allocation is given by 100 * (size - free / size.
///     - Percent free is given by 100 * free / size.
typedef struct _ctl_mem_state_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint64_t free;                                  ///< [out] The free memory in bytes
    uint64_t size;                                  ///< [out] The total allocatable memory in bytes (can be less than
                                                    ///< ::ctl_mem_properties_t.physicalSize)

} ctl_mem_state_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Memory bandwidth
/// 
/// @details
///     - Percent bandwidth is calculated by taking two snapshots (s1, s2) and
///       using the equation: %bw = 10^6 * ((s2.readCounter - s1.readCounter) +
///       (s2.writeCounter - s1.writeCounter)) / (s2.maxBandwidth *
///       (s2.timestamp - s1.timestamp))
typedef struct _ctl_mem_bandwidth_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint64_t maxBandwidth;                          ///< [out] Current maximum bandwidth in units of bytes/sec
    uint64_t timestamp;                             ///< [out] The timestamp (in microseconds) when these measurements were sampled.
                                                    ///< This timestamp should only be used to calculate delta time between
                                                    ///< snapshots of this structure.
                                                    ///< Never take the delta of this timestamp with the timestamp from a
                                                    ///< different structure since they are not guaranteed to have the same base.
                                                    ///< The absolute value of the timestamp is only valid during within the
                                                    ///< application and may be different on the next execution.
    uint64_t readCounter;                           ///< [out] Total bytes read from memory. Supported only for Version > 0
    uint64_t writeCounter;                          ///< [out] Total bytes written to memory. Supported only for Version > 0

} ctl_mem_bandwidth_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of memory modules
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumMemoryModules(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    uint32_t* pCount,                               ///< [in,out] pointer to the number of components of this type.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of components of this type that are available.
                                                    ///< if count is greater than the number of components of this type that
                                                    ///< are available, then the driver shall update the value with the correct
                                                    ///< number of components.
    ctl_mem_handle_t* phMemory                      ///< [in,out][optional][range(0, *pCount)] array of handle of components of
                                                    ///< this type.
                                                    ///< if count is less than the number of components of this type that are
                                                    ///< available, then the driver shall only retrieve that number of
                                                    ///< component handles.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get memory properties
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hMemory`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlMemoryGetProperties(
    ctl_mem_handle_t hMemory,                       ///< [in] Handle for the component.
    ctl_mem_properties_t* pProperties               ///< [in,out] Will contain memory properties.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get memory state - health, allocated
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hMemory`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pState`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlMemoryGetState(
    ctl_mem_handle_t hMemory,                       ///< [in] Handle for the component.
    ctl_mem_state_t* pState                         ///< [in,out] Will contain the current health and allocated memory.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get memory bandwidth
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hMemory`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pBandwidth`
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
///         + User does not have permissions to query this telemetry.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlMemoryGetBandwidth(
    ctl_mem_handle_t hMemory,                       ///< [in] Handle for the component.
    ctl_mem_bandwidth_t* pBandwidth                 ///< [in,out] Will contain the current health, free memory, total memory
                                                    ///< size.
    );


#if !defined(__GNUC__)
#pragma endregion // memory
#endif
// Intel 'ctlApi' for Device Adapter - Overclock
#if !defined(__GNUC__)
#pragma region overclock
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Telemetry Item for each telemetry property
/// 
/// @details
///     - If the supported field is true, then the entire structure has valid
///       information.
///     - The ::ctl_data_value_t is of type ::ctl_data_type_t and units
///       ::ctl_units_t
typedef struct _ctl_oc_telemetry_item_t
{
    bool bSupported;                                ///< [out] Indicates if the value is supported.
    ctl_units_t units;                              ///< [out] Indicates the units of the value.
    ctl_data_type_t type;                           ///< [out] Indicates the data type.
    ctl_data_value_t value;                         ///< [out] The value of type ::ctl_data_type_t and units ::ctl_units_t.

} ctl_oc_telemetry_item_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Overclocking Control Information
/// 
/// @details
///     - Whether the device supports overclocking.
///     - The
///       bSupported/bRelative/bReference/units/min/max/step/default/reference
///       values for the available overclock controls
///     - The idea is to facilitate the way the applications present overclock
///       settings to the user. If bSupported is false, the corresponding
///       overclock control is not supported
///     - The setting units will be an enum that enables the application to know
///       the units for the control setting e.g. MHz. The min and max settings
///       give the limits for the control.
///     - The step setting gives the minimum change in the control value (plus
///       or minus) - if a control is not changed by at least this amount, the
///       hardware may round up or down.
///     - The default values gives the manufacturing setting for the control.
///       Some controls such as frequency offset and voltage offset are
///       relative; in this case, bRelative will be true, otherwise the control
///       settings are absolute values.
///     - For relative controls and if bReference is true, the reference value
///       gives the absolute value at the default setting.
///     - If bReference is false, the absolute value of the default setting is
///       no not known and it is probably better to display the setting to users
///       as percentage offsets.
typedef struct _ctl_oc_control_info_t
{
    bool bSupported;                                ///< [out] Indicates if the values are known.
    bool bRelative;                                 ///< [out] Indicates if the values are meant to be taken as relative values
                                                    ///< instead of absolut values.
    bool bReference;                                ///< [out] For relative values, this indicates if a reference is available.
    ctl_units_t units;                              ///< [out] Units for the values.
    double min;                                     ///< [out] Minimum Value.
    double max;                                     ///< [out] Maximum Value.
    double step;                                    ///< [out] Step Value.
    double Default;                                 ///< [out] Default Value.
    double reference;                               ///< [out] Reference Value if the bReference is true.

} ctl_oc_control_info_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Overclock properties
typedef struct _ctl_oc_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool bSupported;                                ///< [out] Indicates if the adapter supports overclocking.
    ctl_oc_control_info_t gpuFrequencyOffset;       ///< [out] related to function ::ctlOverclockGpuFrequencyOffsetSet
    ctl_oc_control_info_t gpuVoltageOffset;         ///< [out] related to function ::ctlOverclockGpuVoltageOffsetSet
    ctl_oc_control_info_t vramFrequencyOffset;      ///< [out] Property Field Deprecated / No Longer Supported
    ctl_oc_control_info_t vramVoltageOffset;        ///< [out] Property Field Deprecated / No Longer Supported
    ctl_oc_control_info_t powerLimit;               ///< [out] related to function ::ctlOverclockPowerLimitSet
    ctl_oc_control_info_t temperatureLimit;         ///< [out] related to function ::ctlOverclockTemperatureLimitSet

} ctl_oc_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Overclock Voltage Frequency Pair
typedef struct _ctl_oc_vf_pair_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    double Voltage;                                 ///< [in,out] Voltage component of the pair in mV.
    double Frequency;                               ///< [in,out] Frequency component of the pair in MHz.

} ctl_oc_vf_pair_t;

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_PSU_COUNT
/// @brief Maximum number power supply units.
#define CTL_PSU_COUNT  5
#endif // CTL_PSU_COUNT

///////////////////////////////////////////////////////////////////////////////
/// @brief PSU Type.
typedef enum _ctl_psu_type_t
{
    CTL_PSU_TYPE_PSU_NONE = 0,                      ///< Type of the PSU is unknown.
    CTL_PSU_TYPE_PSU_PCIE = 1,                      ///< Type of the PSU is PCIe
    CTL_PSU_TYPE_PSU_6PIN = 2,                      ///< Type of the PSU is 6 PIN
    CTL_PSU_TYPE_PSU_8PIN = 3,                      ///< Type of the PSU is 8 PIN
    CTL_PSU_TYPE_MAX

} ctl_psu_type_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief PSU Info
typedef struct _ctl_psu_info_t
{
    bool bSupported;                                ///< [out] Indicates if this PSU entry is supported.
    ctl_psu_type_t psuType;                         ///< [out] Type of the PSU.
    ctl_oc_telemetry_item_t energyCounter;          ///< [out] Snapshot of the monotonic energy counter maintained by hardware.
                                                    ///< It measures the total energy consumed this power source. By taking the
                                                    ///< delta between two snapshots and dividing by the delta time in seconds,
                                                    ///< an application can compute the average power.
    ctl_oc_telemetry_item_t voltage;                ///< [out] Instantaneous snapshot of the voltage of this power source.

} ctl_psu_info_t;

///////////////////////////////////////////////////////////////////////////////
#ifndef CTL_FAN_COUNT
/// @brief Maximum number of Fans
#define CTL_FAN_COUNT  5
#endif // CTL_FAN_COUNT

///////////////////////////////////////////////////////////////////////////////
/// @brief Power Telemetry
typedef struct _ctl_power_telemetry_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_oc_telemetry_item_t timeStamp;              ///< [out] Snapshot of the timestamp counter that measures the total time
                                                    ///< since Jan 1, 1970 UTC. It is a decimal value in seconds with a minimum
                                                    ///< accuracy of 1 millisecond.
    ctl_oc_telemetry_item_t gpuEnergyCounter;       ///< [out] Snapshot of the monotonic energy counter maintained by hardware.
                                                    ///< It measures the total energy consumed by the GPU chip. By taking the
                                                    ///< delta between two snapshots and dividing by the delta time in seconds,
                                                    ///< an application can compute the average power.
    ctl_oc_telemetry_item_t gpuVoltage;             ///< [out] Instantaneous snapshot of the voltage feeding the GPU chip. It
                                                    ///< is measured at the power supply output - chip input will be lower.
    ctl_oc_telemetry_item_t gpuCurrentClockFrequency;   ///< [out] Instantaneous snapshot of the GPU chip frequency.
    ctl_oc_telemetry_item_t gpuCurrentTemperature;  ///< [out] Instantaneous snapshot of the GPU chip temperature, read from
                                                    ///< the sensor reporting the highest value.
    ctl_oc_telemetry_item_t globalActivityCounter;  ///< [out] Snapshot of the monotonic global activity counter. It measures
                                                    ///< the time in seconds (accurate down to 1 millisecond) that any GPU
                                                    ///< engine is busy. By taking the delta between two snapshots and dividing
                                                    ///< by the delta time in seconds, an application can compute the average
                                                    ///< percentage utilization of the GPU..
    ctl_oc_telemetry_item_t renderComputeActivityCounter;   ///< [out] Snapshot of the monotonic 3D/compute activity counter. It
                                                    ///< measures the time in seconds (accurate down to 1 millisecond) that any
                                                    ///< 3D render/compute engine is busy. By taking the delta between two
                                                    ///< snapshots and dividing by the delta time in seconds, an application
                                                    ///< can compute the average percentage utilization of all 3D
                                                    ///< render/compute blocks in the GPU.
    ctl_oc_telemetry_item_t mediaActivityCounter;   ///< [out] Snapshot of the monotonic media activity counter. It measures
                                                    ///< the time in seconds (accurate down to 1 millisecond) that any media
                                                    ///< engine is busy. By taking the delta between two snapshots and dividing
                                                    ///< by the delta time in seconds, an application can compute the average
                                                    ///< percentage utilization of all media blocks in the GPU.
    bool gpuPowerLimited;                           ///< [out] Instantaneous indication that the desired GPU frequency is being
                                                    ///< throttled because the GPU chip is exceeding the maximum power limits.
                                                    ///< Increasing the power limits using ::ctlOverclockPowerLimitSet() is one
                                                    ///< way to remove this limitation.
    bool gpuTemperatureLimited;                     ///< [out] Instantaneous indication that the desired GPU frequency is being
                                                    ///< throttled because the GPU chip is exceeding the temperature limits.
                                                    ///< Increasing the temperature limits using
                                                    ///< ::ctlOverclockTemperatureLimitSet() is one way to reduce this
                                                    ///< limitation. Improving the cooling solution is another way.
    bool gpuCurrentLimited;                         ///< [out] Instantaneous indication that the desired GPU frequency is being
                                                    ///< throttled because the GPU chip has exceeded the power supply current
                                                    ///< limits. A better power supply is required to reduce this limitation.
    bool gpuVoltageLimited;                         ///< [out] Instantaneous indication that the GPU frequency cannot be
                                                    ///< increased because the voltage limits have been reached. Increase the
                                                    ///< voltage offset using ::ctlOverclockGpuVoltageOffsetSet() is one way to
                                                    ///< reduce this limitation.
    bool gpuUtilizationLimited;                     ///< [out] Instantaneous indication that due to lower GPU utilization, the
                                                    ///< hardware has lowered the GPU frequency.
    ctl_oc_telemetry_item_t vramEnergyCounter;      ///< [out] Snapshot of the monotonic energy counter maintained by hardware.
                                                    ///< It measures the total energy consumed by the local memory modules. By
                                                    ///< taking the delta between two snapshots and dividing by the delta time
                                                    ///< in seconds, an application can compute the average power.
    ctl_oc_telemetry_item_t vramVoltage;            ///< [out] Instantaneous snapshot of the voltage feeding the memory
                                                    ///< modules.
    ctl_oc_telemetry_item_t vramCurrentClockFrequency;  ///< [out] Instantaneous snapshot of the raw clock frequency driving the
                                                    ///< memory modules.
    ctl_oc_telemetry_item_t vramCurrentEffectiveFrequency;  ///< [out] Instantaneous snapshot of the effective data transfer rate that
                                                    ///< the memory modules can sustain based on the current clock frequency..
    ctl_oc_telemetry_item_t vramReadBandwidthCounter;   ///< [out] Instantaneous snapshot of the monotonic counter that measures
                                                    ///< the read traffic from the memory modules. By taking the delta between
                                                    ///< two snapshots and dividing by the delta time in seconds, an
                                                    ///< application can compute the average read bandwidth.
    ctl_oc_telemetry_item_t vramWriteBandwidthCounter;  ///< [out] Instantaneous snapshot of the monotonic counter that measures
                                                    ///< the write traffic to the memory modules. By taking the delta between
                                                    ///< two snapshots and dividing by the delta time in seconds, an
                                                    ///< application can compute the average write bandwidth.
    ctl_oc_telemetry_item_t vramCurrentTemperature; ///< [out] Instantaneous snapshot of the GPU chip temperature, read from
                                                    ///< the sensor reporting the highest value.
    bool vramPowerLimited;                          ///< [out] Instantaneous indication that the memory frequency is being
                                                    ///< throttled because the memory modules are exceeding the maximum power
                                                    ///< limits.
    bool vramTemperatureLimited;                    ///< [out] Instantaneous indication that the memory frequency is being
                                                    ///< throttled because the memory modules are exceeding the temperature
                                                    ///< limits.
    bool vramCurrentLimited;                        ///< [out] Instantaneous indication that the memory frequency is being
                                                    ///< throttled because the memory modules have exceeded the power supply
                                                    ///< current limits.
    bool vramVoltageLimited;                        ///< [out] Instantaneous indication that the memory frequency cannot be
                                                    ///< increased because the voltage limits have been reached.
    bool vramUtilizationLimited;                    ///< [out] Instantaneous indication that due to lower memory traffic, the
                                                    ///< hardware has lowered the memory frequency.
    ctl_oc_telemetry_item_t totalCardEnergyCounter; ///< [out] Total Card Energy Counter.
    ctl_psu_info_t psu[CTL_PSU_COUNT];              ///< [out] PSU voltage and power.
    ctl_oc_telemetry_item_t fanSpeed[CTL_FAN_COUNT];///< [out] Fan speed.

} ctl_power_telemetry_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get overclock properties - available properties.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pOcProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGetProperties(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    ctl_oc_properties_t* pOcProperties              ///< [in,out] The overclocking properties for the specified domain.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Overclock Waiver - Warranty Waiver.
/// 
/// @details
///     - Most of the overclock functions will return an error if the waiver is
///       not set. This is because most overclock settings will increase the
///       electric/thermal stress on the part and thus reduce its lifetime.
///     - By setting the waiver, the user is indicate that they are accepting a
///       reduction in the lifetime of the part.
///     - It is the responsibility of overclock applications to notify each user
///       at least once with a popup of the dangers and requiring acceptance.
///     - Only once the user has accepted should this function be called by the
///       application.
///     - It is acceptable for the application to cache the user choice and call
///       this function on future executions without issuing the popup.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockWaiverSet(
    ctl_device_adapter_handle_t hDeviceHandle       ///< [in][release] Handle to display adapter
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the Overclock Frequency Offset for the GPU in MHz.
/// 
/// @details
///     - Determine the current frequency offset in effect (refer to
///       ::ctlOverclockGpuFrequencyOffsetSet() for details).
///     - The value returned may be different from the value that was previously
///       set by the application depending on hardware limitations or if the
///       function ::ctlOverclockGpuFrequencyOffsetSet() has been called or
///       another application that has changed the value.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pOcFrequencyOffset`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGpuFrequencyOffsetGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double* pOcFrequencyOffset                      ///< [in,out] The Turbo Overclocking Frequency Desired in MHz.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the Overclock Frequency Offset for the GPU in MHZ.
/// 
/// @details
///     - The purpose of this function is to increase/decrease the frequency at
///       which typical workloads will run within the same thermal budget.
///     - The frequency offset is expressed in units of ±1MHz.
///     - The actual operating frequency for each workload is not guaranteed to
///       change exactly by the specified offset.
///     - For positive frequency offsets, the factory maximum frequency may
///       increase by up to the specified amount.
///     - For negative frequency offsets, the overclock waiver must have been
///       set since this can result in running the part at voltages beyond the
///       part warrantee limits. An error is returned if the waiver has not been
///       set.
///     - Specifying large values for the frequency offset can lead to
///       instability. It is recommended that changes are made in small
///       increments and stability/performance measured running intense GPU
///       workloads before increasing further.
///     - This setting is not persistent through system reboots or driver
///       resets/hangs. It is up to the overclock application to reapply the
///       settings in those cases.
///     - This setting can cause system/device instability. It is up to the
///       overclock application to detect if the system has rebooted
///       unexpectedly or the device was restarted. When this occurs, the
///       application should not reapply the overclock settings automatically
///       but instead return to previously known good settings or notify the
///       user that the settings are not being applied.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGpuFrequencyOffsetSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double ocFrequencyOffset                        ///< [in] The Turbo Overclocking Frequency Desired in MHz.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the Overclock Gpu Voltage Offset in mV.
/// 
/// @details
///     - Determine the current voltage offset in effect on the hardware (refer
///       to ::ctlOverclockGpuVoltageOffsetSet for details).
///     - The value returned may be different from the value that was previously
///       set by the application depending on hardware limitations or if the
///       function ::ctlOverclockGpuVoltageOffsetSet has been called or another
///       application that has changed the value.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pOcVoltageOffset`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGpuVoltageOffsetGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double* pOcVoltageOffset                        ///< [in,out] The Turbo Overclocking Frequency Desired in mV.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the Overclock Gpu Voltage Offset in mV.
/// 
/// @details
///     - The purpose of this function is to attempt to run the GPU up to higher
///       voltages beyond the part warrantee limits. This can permit running at
///       even higher frequencies than can be obtained using the frequency
///       offset setting, but at the risk of reducing the lifetime of the part.
///     - The voltage offset is expressed in units of ±millivolts with values
///       permitted down to a resolution of 1 millivolt.
///     - The overclock waiver must be set before calling this function
///       otherwise and error will be returned.
///     - There is no guarantee that a workload can operate at the higher
///       frequencies permitted by this setting. Significantly more heat will be
///       generated at these high frequencies/voltages which will necessitate a
///       good cooling solution.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGpuVoltageOffsetSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double ocVoltageOffset                          ///< [in] The Turbo Overclocking Frequency Desired in mV.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets the Locked GPU Voltage for Overclocking in mV.
/// 
/// @details
///     - The purpose of this function is to determine if the current values of
///       the frequency/voltage lock.
///     - If the lock is not currently active, will return 0 for frequency and
///       voltage.
///     - Note that the operating frequency/voltage may be lower than these
///       settings if power/thermal limits are exceeded.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pVfPair`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGpuLockGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    ctl_oc_vf_pair_t* pVfPair                       ///< [out] The current locked voltage and frequency.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Locks the GPU voltage for Overclocking in mV.
/// 
/// @details
///     - The purpose of this function is to provide an interface for scanners
///       to lock the frequency and voltage to fixed values.
///     - The frequency is expressed in units of MHz with a resolution of 1MHz.
///     - The voltage is expressed in units of ±millivolts with values
///       permitted down to a resolution of 1 millivolt.
///     - The overclock waiver must be set since fixing the voltage at a high
///       value puts unnecessary stress on the part.
///     - The actual frequency may reduce depending on power/thermal
///       limitations.
///     - Requesting a frequency and/or voltage of 0 will return the hardware to
///       dynamic frequency/voltage management with any previous frequency
///       offset or voltage offset settings reapplied.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockGpuLockSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    ctl_oc_vf_pair_t vFPair                         ///< [in] The current locked voltage and frequency.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the current Vram Frequency Offset in GT/s.
/// 
/// @details
///     - The purpose of this function is to return the current VRAM frequency
///       offset in units of GT/s.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pOcFrequencyOffset`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockVramFrequencyOffsetGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double* pOcFrequencyOffset                      ///< [in,out] The current Memory Frequency in GT/s.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the desired Vram frquency Offset in GT/s
/// 
/// @details
///     - The purpose of this function is to increase/decrease the frequency of
///       VRAM.
///     - The frequency offset is expressed in units of GT/s with a minimum step
///       size given by ::ctlOverclockGetProperties.
///     - The actual operating frequency for each workload is not guaranteed to
///       change exactly by the specified offset.
///     - The waiver must be set using clibOverclockWaiverSet() before this
///       function can be called.
///     - This setting is not persistent through system reboots or driver
///       resets/hangs. It is up to the overclock application to reapply the
///       settings in those cases.
///     - This setting can cause system/device instability. It is up to the
///       overclock application to detect if the system has rebooted
///       unexpectedly or the device was restarted. When this occurs, the
///       application should not reapply the overclock settings automatically
///       but instead return to previously known good settings or notify the
///       user that the settings are not being applied.
///     - If the memory controller doesn't support changes to frequency on the
///       fly, one of the following return codes will be given:
///     - ::CTL_RESULT_ERROR_RESET_DEVICE_REQUIRED: The requested memory
///       overclock will be applied when the device is reset or the system is
///       rebooted. In this case, the overclock software should check if the
///       overclock request was applied after the reset/reboot. If it was and
///       when the overclock application shuts down gracefully and if the
///       overclock application wants the setting to be persistent, the
///       application should request the same overclock settings again so that
///       they will be applied on the next reset/reboot. If this is not done,
///       then every time the device is reset and overclock is requested, the
///       device needs to be reset a second time.
///     - ::CTL_RESULT_ERROR_FULL_REBOOT_REQUIRED: The requested memory
///       overclock will be applied when the system is rebooted. In this case,
///       the overclock software should check if the overclock request was
///       applied after the reboot. If it was and when the overclock application
///       shuts down gracefully and if the overclock application wants the
///       setting to be persistent, the application should request the same
///       overclock settings again so that they will be applied on the next
///       reset/reboot. If this is not done and the overclock setting is
///       requested after the reboot has occurred, a second reboot will be
///       required.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockVramFrequencyOffsetSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double ocFrequencyOffset                        ///< [in] The desired Memory Frequency in GT/s.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the Overclock Vram Voltage Offset in mV.
/// 
/// @details
///     - The purpose of this function is to increase/decrease the voltage of
///       VRAM.
///     - The voltage offset is expressed in units of millivolts with a minimum
///       step size given by ::ctlOverclockGetProperties.
///     - The waiver must be set using ::ctlOverclockWaiverSet before this
///       function can be called.
///     - This setting is not persistent through system reboots or driver
///       resets/hangs. It is up to the overclock application to reapply the
///       settings in those cases.
///     - This setting can cause system/device instability. It is up to the
///       overclock application to detect if the system has rebooted
///       unexpectedly or the device was restarted. When this occurs, the
///       application should not reapply the overclock settings automatically
///       but instead return to previously known good settings or notify the
///       user that the settings are not being applied.
///     - If the memory controller doesn't support changes to voltage on the
///       fly, one of the following return codes will be given:
///     - ::CTL_RESULT_ERROR_RESET_DEVICE_REQUIRED: The requested memory
///       overclock will be applied when the device is reset or the system is
///       rebooted. In this case, the overclock software should check if the
///       overclock request was applied after the reset/reboot. If it was and
///       when the overclock application shuts down gracefully and if the
///       overclock application wants the setting to be persistent, the
///       application should request the same overclock settings again so that
///       they will be applied on the next reset/reboot. If this is not done,
///       then every time the device is reset and overclock is requested, the
///       device needs to be reset a second time.
///     - ::CTL_RESULT_ERROR_FULL_REBOOT_REQUIRED: The requested memory
///       overclock will be applied when the system is rebooted. In this case,
///       the overclock software should check if the overclock request was
///       applied after the reboot. If it was and when the overclock application
///       shuts down gracefully and if the overclock application wants the
///       setting to be persistent, the application should request the same
///       overclock settings again so that they will be applied on the next
///       reset/reboot. If this is not done and the overclock setting is
///       requested after the reboot has occurred, a second reboot will be
///       required.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pVoltage`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockVramVoltageOffsetGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double* pVoltage                                ///< [out] The current locked voltage in mV.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the Overclock Vram Voltage Offset in mV.
/// 
/// @details
///     - The purpose of this function is to set the maximum sustained power
///       limit. If the average GPU power averaged over a few seconds exceeds
///       this value, the frequency of the GPU will be throttled.
///     - Set a value of 0 to disable this power limit. In this case, the GPU
///       frequency will not throttle due to average power but may hit other
///       limits.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockVramVoltageOffsetSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double voltage                                  ///< [in] The voltage to be locked in mV.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the sustained power limit in mW.
/// 
/// @details
///     - The purpose of this function is to read the current sustained power
///       limit.
///     - A value of 0 means that the limit is disabled - the GPU frequency can
///       run as high as possible until other limits are hit.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pSustainedPowerLimit`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockPowerLimitGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double* pSustainedPowerLimit                    ///< [in,out] The current sustained power limit in mW.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the sustained power limit in mW.
/// 
/// @details
///     - The purpose of this function is to set the maximum sustained power
///       limit. If the average GPU power averaged over a few seconds exceeds
///       this value, the frequency of the GPU will be throttled.
///     - Set a value of 0 to disable this power limit. In this case, the GPU
///       frequency will not throttle due to average power but may hit other
///       limits.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockPowerLimitSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double sustainedPowerLimit                      ///< [in] The desired sustained power limit in mW.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the current temperature limit in Celsius.
/// 
/// @details
///     - The purpose of this function is to read the current thermal limit.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pTemperatureLimit`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockTemperatureLimitGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double* pTemperatureLimit                       ///< [in,out] The current temperature limit in Celsius.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set the temperature limit in Celsius.
/// 
/// @details
///     - The purpose of this function is to change the maximum thermal limit.
///       When the GPU temperature exceeds this value, the GPU frequency will be
///       throttled.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockTemperatureLimitSet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    double temperatureLimit                         ///< [in] The desired temperature limit in Celsius.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get Power Telemetry.
/// 
/// @details
///     - Limited rate of 50 ms, any call under 50 ms will return the same
///       information.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pTelemetryInfo`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPowerTelemetryGet(
    ctl_device_adapter_handle_t hDeviceHandle,      ///< [in][release] Handle to display adapter
    ctl_power_telemetry_t* pTelemetryInfo           ///< [out] The overclocking properties for the specified domain.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Reset all Overclock Settings to stock
/// 
/// @details
///     - Reset all Overclock setting to default using single API call
///     - This request resets any changes made to GpuFrequencyOffset,
///       GpuVoltageOffset, PowerLimit, TemperatureLimit, GpuLock
///     - This Doesn't reset any Fan Curve Changes. It can be reset using
///       ctlFanSetDefaultMode
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDeviceHandle`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlOverclockResetToDefault(
    ctl_device_adapter_handle_t hDeviceHandle       ///< [in][release] Handle to display adapter
    );


#if !defined(__GNUC__)
#pragma endregion // overclock
#endif
// Intel 'ctlApi' for Device Adapter - PCI Information
#if !defined(__GNUC__)
#pragma region pci
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief PCI address
typedef struct _ctl_pci_address_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint32_t domain;                                ///< [out] BDF domain
    uint32_t bus;                                   ///< [out] BDF bus
    uint32_t device;                                ///< [out] BDF device
    uint32_t function;                              ///< [out] BDF function

} ctl_pci_address_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief PCI speed
typedef struct _ctl_pci_speed_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    int32_t gen;                                    ///< [out] The link generation. A value of -1 means that this property is
                                                    ///< unknown.
    int32_t width;                                  ///< [out] The number of lanes. A value of -1 means that this property is
                                                    ///< unknown.
    int64_t maxBandwidth;                           ///< [out] The maximum bandwidth in bytes/sec (sum of all lanes). A value
                                                    ///< of -1 means that this property is unknown.

} ctl_pci_speed_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Static PCI properties
typedef struct _ctl_pci_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_pci_address_t address;                      ///< [out] The BDF address
    ctl_pci_speed_t maxSpeed;                       ///< [out] Fastest port configuration supported by the device (sum of all
                                                    ///< lanes)
    bool resizable_bar_supported;                   ///< [out] Support for Resizable Bar on this device.
    bool resizable_bar_enabled;                     ///< [out] Resizable Bar enabled on this device

} ctl_pci_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Dynamic PCI state
typedef struct _ctl_pci_state_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_pci_speed_t speed;                          ///< [out] The current port configure speed

} ctl_pci_state_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get PCI properties - address, max speed
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPciGetProperties(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    ctl_pci_properties_t* pProperties               ///< [in,out] Will contain the PCI properties.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get current PCI state - current speed
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pState`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPciGetState(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    ctl_pci_state_t* pState                         ///< [in,out] Will contain the PCI properties.
    );


#if !defined(__GNUC__)
#pragma endregion // pci
#endif
// Intel 'ctlApi' for Device Adapter - Power management
#if !defined(__GNUC__)
#pragma region power
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Properties related to device power settings
typedef struct _ctl_power_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool canControl;                                ///< [out] Software can change the power limits of this domain assuming the
                                                    ///< user has permissions.
    int32_t defaultLimit;                           ///< [out] The factory default TDP power limit of the part in milliwatts. A
                                                    ///< value of -1 means that this is not known.
    int32_t minLimit;                               ///< [out] The minimum power limit in milliwatts that can be requested.
    int32_t maxLimit;                               ///< [out] The maximum power limit in milliwatts that can be requested.

} ctl_power_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Energy counter snapshot
/// 
/// @details
///     - Average power is calculated by taking two snapshots (s1, s2) and using
///       the equation: PowerWatts = (s2.energy - s1.energy) / (s2.timestamp -
///       s1.timestamp)
typedef struct _ctl_power_energy_counter_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    uint64_t energy;                                ///< [out] The monotonic energy counter in microjoules.
    uint64_t timestamp;                             ///< [out] Microsecond timestamp when energy was captured.
                                                    ///< This timestamp should only be used to calculate delta time between
                                                    ///< snapshots of this structure.
                                                    ///< Never take the delta of this timestamp with the timestamp from a
                                                    ///< different structure since they are not guaranteed to have the same base.
                                                    ///< The absolute value of the timestamp is only valid during within the
                                                    ///< application and may be different on the next execution.

} ctl_power_energy_counter_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Sustained power limits
/// 
/// @details
///     - The power controller (Punit) will throttle the operating frequency if
///       the power averaged over a window (typically seconds) exceeds this
///       limit.
typedef struct _ctl_power_sustained_limit_t
{
    bool enabled;                                   ///< [in,out] indicates if the limit is enabled (true) or ignored (false)
    int32_t power;                                  ///< [in,out] power limit in milliwatts
    int32_t interval;                               ///< [in,out] power averaging window (Tau) in milliseconds

} ctl_power_sustained_limit_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Burst power limit
/// 
/// @details
///     - The power controller (Punit) will throttle the operating frequency of
///       the device if the power averaged over a few milliseconds exceeds a
///       limit known as PL2. Typically PL2 > PL1 so that it permits the
///       frequency to burst higher for short periods than would be otherwise
///       permitted by PL1.
typedef struct _ctl_power_burst_limit_t
{
    bool enabled;                                   ///< [in,out] indicates if the limit is enabled (true) or ignored (false)
    int32_t power;                                  ///< [in,out] power limit in milliwatts

} ctl_power_burst_limit_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Peak power limit
/// 
/// @details
///     - The power controller (Punit) will preemptively throttle the operating
///       frequency of the device when the instantaneous power exceeds this
///       limit. The limit is known as PL4. It expresses the maximum power that
///       can be drawn from the power supply.
///     - If this power limit is removed or set too high, the power supply will
///       generate an interrupt when it detects an overcurrent condition and the
///       power controller will throttle the device frequencies down to min. It
///       is thus better to tune the PL4 value in order to avoid such
///       excursions.
typedef struct _ctl_power_peak_limit_t
{
    int32_t powerAC;                                ///< [in,out] power limit in milliwatts for the AC power source.
    int32_t powerDC;                                ///< [in,out] power limit in milliwatts for the DC power source. On input,
                                                    ///< this is ignored if the product does not have a battery. On output,
                                                    ///< this will be -1 if the product does not have a battery.

} ctl_power_peak_limit_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Power limits
typedef struct _ctl_power_limits_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_power_sustained_limit_t sustainedPowerLimit;///< [in,out] sustained power limit.
    ctl_power_burst_limit_t burstPowerLimit;        ///< [in,out] burst power limit.
    ctl_power_peak_limit_t peakPowerLimits;         ///< [in,out] peak power limit.

} ctl_power_limits_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Energy threshold
/// 
/// @details
///     - .
typedef struct _ctl_energy_threshold_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    bool enable;                                    ///< [in,out] Indicates if the energy threshold is enabled.
    double threshold;                               ///< [in,out] The energy threshold in Joules. Will be 0.0 if no threshold
                                                    ///< has been set.
    uint32_t processId;                             ///< [in,out] The host process ID that set the energy threshold. Will be
                                                    ///< 0xFFFFFFFF if no threshold has been set.

} ctl_energy_threshold_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of power domains
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumPowerDomains(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    uint32_t* pCount,                               ///< [in,out] pointer to the number of components of this type.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of components of this type that are available.
                                                    ///< if count is greater than the number of components of this type that
                                                    ///< are available, then the driver shall update the value with the correct
                                                    ///< number of components.
    ctl_pwr_handle_t* phPower                       ///< [in,out][optional][range(0, *pCount)] array of handle of components of
                                                    ///< this type.
                                                    ///< if count is less than the number of components of this type that are
                                                    ///< available, then the driver shall only retrieve that number of
                                                    ///< component handles.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get properties related to a power domain
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPower`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPowerGetProperties(
    ctl_pwr_handle_t hPower,                        ///< [in] Handle for the component.
    ctl_power_properties_t* pProperties             ///< [in,out] Structure that will contain property data.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get energy counter
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPower`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pEnergy`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPowerGetEnergyCounter(
    ctl_pwr_handle_t hPower,                        ///< [in] Handle for the component.
    ctl_power_energy_counter_t* pEnergy             ///< [in,out] Will contain the latest snapshot of the energy counter and
                                                    ///< timestamp when the last counter value was measured.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get power limits
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPower`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPowerGetLimits(
    ctl_pwr_handle_t hPower,                        ///< [in] Handle for the component.
    ctl_power_limits_t* pPowerLimits                ///< [in,out][optional] Structure that will contain the power limits.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Set power limits
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hPower`
///     - ::CTL_RESULT_ERROR_INSUFFICIENT_PERMISSIONS
///         + User does not have permissions to make these modifications.
///     - ::CTL_RESULT_ERROR_NOT_AVAILABLE
///         + The device is in use, meaning that the GPU is under Over clocking, applying power limits under overclocking is not supported.
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlPowerSetLimits(
    ctl_pwr_handle_t hPower,                        ///< [in] Handle for the component.
    const ctl_power_limits_t* pPowerLimits          ///< [in][optional] Structure that will contain the power limits.
    );


#if !defined(__GNUC__)
#pragma endregion // power
#endif
// Intel 'ctlApi' for Device Adapter - Temperature Sensors
#if !defined(__GNUC__)
#pragma region temperature
#endif
///////////////////////////////////////////////////////////////////////////////
/// @brief Temperature sensors
typedef enum _ctl_temp_sensors_t
{
    CTL_TEMP_SENSORS_GLOBAL = 0,                    ///< The maximum temperature across all device sensors
    CTL_TEMP_SENSORS_GPU = 1,                       ///< The maximum temperature across all sensors in the GPU
    CTL_TEMP_SENSORS_MEMORY = 2,                    ///< The maximum temperature across all sensors in the local memory
    CTL_TEMP_SENSORS_GLOBAL_MIN = 3,                ///< The minimum temperature across all device sensors
    CTL_TEMP_SENSORS_GPU_MIN = 4,                   ///< The minimum temperature across all sensors in the GPU
    CTL_TEMP_SENSORS_MEMORY_MIN = 5,                ///< The minimum temperature across all sensors in the local device memory
    CTL_TEMP_SENSORS_MAX

} ctl_temp_sensors_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Temperature sensor properties
typedef struct _ctl_temp_properties_t
{
    uint32_t Size;                                  ///< [in] size of this structure
    uint8_t Version;                                ///< [in] version of this structure
    ctl_temp_sensors_t type;                        ///< [out] Which part of the device the temperature sensor measures
    double maxTemperature;                          ///< [out] Will contain the maximum temperature for the specific device in
                                                    ///< degrees Celsius.

} ctl_temp_properties_t;

///////////////////////////////////////////////////////////////////////////////
/// @brief Get handle of temperature sensors
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hDAhandle`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pCount`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlEnumTemperatureSensors(
    ctl_device_adapter_handle_t hDAhandle,          ///< [in][release] Handle to display adapter
    uint32_t* pCount,                               ///< [in,out] pointer to the number of components of this type.
                                                    ///< if count is zero, then the driver shall update the value with the
                                                    ///< total number of components of this type that are available.
                                                    ///< if count is greater than the number of components of this type that
                                                    ///< are available, then the driver shall update the value with the correct
                                                    ///< number of components.
    ctl_temp_handle_t* phTemperature                ///< [in,out][optional][range(0, *pCount)] array of handle of components of
                                                    ///< this type.
                                                    ///< if count is less than the number of components of this type that are
                                                    ///< available, then the driver shall only retrieve that number of
                                                    ///< component handles.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get temperature sensor properties
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hTemperature`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pProperties`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlTemperatureGetProperties(
    ctl_temp_handle_t hTemperature,                 ///< [in] Handle for the component.
    ctl_temp_properties_t* pProperties              ///< [in,out] Will contain the temperature sensor properties.
    );

///////////////////////////////////////////////////////////////////////////////
/// @brief Get the temperature from a specified sensor
/// 
/// @details
///     - The application may call this function from simultaneous threads.
///     - The implementation of this function should be lock-free.
/// 
/// @returns
///     - CTL_RESULT_SUCCESS
///     - CTL_RESULT_ERROR_UNINITIALIZED
///     - CTL_RESULT_ERROR_DEVICE_LOST
///     - CTL_RESULT_ERROR_INVALID_NULL_HANDLE
///         + `nullptr == hTemperature`
///     - CTL_RESULT_ERROR_INVALID_NULL_POINTER
///         + `nullptr == pTemperature`
CTL_APIEXPORT ctl_result_t CTL_APICALL
ctlTemperatureGetState(
    ctl_temp_handle_t hTemperature,                 ///< [in] Handle for the component.
    double* pTemperature                            ///< [in,out] Will contain the temperature read from the specified sensor
                                                    ///< in degrees Celsius.
    );


#if !defined(__GNUC__)
#pragma endregion // temperature
#endif


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlInit 
typedef ctl_result_t (CTL_APICALL *ctl_pfnInit_t)(
    ctl_init_args_t*,
    ctl_api_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlClose 
typedef ctl_result_t (CTL_APICALL *ctl_pfnClose_t)(
    ctl_api_handle_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetRuntimePath 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetRuntimePath_t)(
    ctl_runtime_path_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlWaitForPropertyChange 
typedef ctl_result_t (CTL_APICALL *ctl_pfnWaitForPropertyChange_t)(
    ctl_device_adapter_handle_t,
    ctl_wait_property_change_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlReservedCall 
typedef ctl_result_t (CTL_APICALL *ctl_pfnReservedCall_t)(
    ctl_device_adapter_handle_t,
    ctl_reserved_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSupported3DCapabilities 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSupported3DCapabilities_t)(
    ctl_device_adapter_handle_t,
    ctl_3d_feature_caps_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSet3DFeature 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSet3DFeature_t)(
    ctl_device_adapter_handle_t,
    ctl_3d_feature_getset_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlCheckDriverVersion 
typedef ctl_result_t (CTL_APICALL *ctl_pfnCheckDriverVersion_t)(
    ctl_device_adapter_handle_t,
    ctl_version_info_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumerateDevices 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumerateDevices_t)(
    ctl_api_handle_t,
    uint32_t*,
    ctl_device_adapter_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumerateDisplayOutputs 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumerateDisplayOutputs_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_display_output_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumerateI2CPinPairs 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumerateI2CPinPairs_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_i2c_pin_pair_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetDeviceProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetDeviceProperties_t)(
    ctl_device_adapter_handle_t,
    ctl_device_adapter_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetDisplayProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetDisplayProperties_t)(
    ctl_display_output_handle_t,
    ctl_display_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetAdaperDisplayEncoderProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetAdaperDisplayEncoderProperties_t)(
    ctl_display_output_handle_t,
    ctl_adapter_display_encoder_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetZeDevice 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetZeDevice_t)(
    ctl_device_adapter_handle_t,
    void*,
    void**
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSharpnessCaps 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSharpnessCaps_t)(
    ctl_display_output_handle_t,
    ctl_sharpness_caps_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetCurrentSharpness 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetCurrentSharpness_t)(
    ctl_display_output_handle_t,
    ctl_sharpness_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetCurrentSharpness 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetCurrentSharpness_t)(
    ctl_display_output_handle_t,
    ctl_sharpness_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlI2CAccess 
typedef ctl_result_t (CTL_APICALL *ctl_pfnI2CAccess_t)(
    ctl_display_output_handle_t,
    ctl_i2c_access_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlI2CAccessOnPinPair 
typedef ctl_result_t (CTL_APICALL *ctl_pfnI2CAccessOnPinPair_t)(
    ctl_i2c_pin_pair_handle_t,
    ctl_i2c_access_pinpair_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlAUXAccess 
typedef ctl_result_t (CTL_APICALL *ctl_pfnAUXAccess_t)(
    ctl_display_output_handle_t,
    ctl_aux_access_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetPowerOptimizationCaps 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetPowerOptimizationCaps_t)(
    ctl_display_output_handle_t,
    ctl_power_optimization_caps_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetPowerOptimizationSetting 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetPowerOptimizationSetting_t)(
    ctl_display_output_handle_t,
    ctl_power_optimization_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetPowerOptimizationSetting 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetPowerOptimizationSetting_t)(
    ctl_display_output_handle_t,
    ctl_power_optimization_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetBrightnessSetting 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetBrightnessSetting_t)(
    ctl_display_output_handle_t,
    ctl_set_brightness_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetBrightnessSetting 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetBrightnessSetting_t)(
    ctl_display_output_handle_t,
    ctl_get_brightness_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPixelTransformationGetConfig 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPixelTransformationGetConfig_t)(
    ctl_display_output_handle_t,
    ctl_pixtx_pipe_get_config_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPixelTransformationSetConfig 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPixelTransformationSetConfig_t)(
    ctl_display_output_handle_t,
    ctl_pixtx_pipe_set_config_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPanelDescriptorAccess 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPanelDescriptorAccess_t)(
    ctl_display_output_handle_t,
    ctl_panel_descriptor_access_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSupportedRetroScalingCapability 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSupportedRetroScalingCapability_t)(
    ctl_device_adapter_handle_t,
    ctl_retro_scaling_caps_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSetRetroScaling 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSetRetroScaling_t)(
    ctl_device_adapter_handle_t,
    ctl_retro_scaling_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSupportedScalingCapability 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSupportedScalingCapability_t)(
    ctl_display_output_handle_t,
    ctl_scaling_caps_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetCurrentScaling 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetCurrentScaling_t)(
    ctl_display_output_handle_t,
    ctl_scaling_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetCurrentScaling 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetCurrentScaling_t)(
    ctl_display_output_handle_t,
    ctl_scaling_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetLACEConfig 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetLACEConfig_t)(
    ctl_display_output_handle_t,
    ctl_lace_config_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetLACEConfig 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetLACEConfig_t)(
    ctl_display_output_handle_t,
    ctl_lace_config_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSoftwarePSR 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSoftwarePSR_t)(
    ctl_display_output_handle_t,
    ctl_sw_psr_settings_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetIntelArcSyncInfoForMonitor 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetIntelArcSyncInfoForMonitor_t)(
    ctl_display_output_handle_t,
    ctl_intel_arc_sync_monitor_params_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumerateMuxDevices 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumerateMuxDevices_t)(
    ctl_api_handle_t,
    uint32_t*,
    ctl_mux_output_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetMuxProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetMuxProperties_t)(
    ctl_mux_output_handle_t,
    ctl_mux_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSwitchMux 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSwitchMux_t)(
    ctl_mux_output_handle_t,
    ctl_display_output_handle_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetIntelArcSyncProfile 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetIntelArcSyncProfile_t)(
    ctl_display_output_handle_t,
    ctl_intel_arc_sync_profile_params_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlSetIntelArcSyncProfile 
typedef ctl_result_t (CTL_APICALL *ctl_pfnSetIntelArcSyncProfile_t)(
    ctl_display_output_handle_t,
    ctl_intel_arc_sync_profile_params_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEdidManagement 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEdidManagement_t)(
    ctl_display_output_handle_t,
    ctl_edid_management_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSetCustomMode 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSetCustomMode_t)(
    ctl_display_output_handle_t,
    ctl_get_set_custom_mode_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSetCombinedDisplay 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSetCombinedDisplay_t)(
    ctl_device_adapter_handle_t,
    ctl_combined_display_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSetDisplayGenlock 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSetDisplayGenlock_t)(
    ctl_device_adapter_handle_t*,
    ctl_genlock_args_t*,
    uint32_t,
    ctl_device_adapter_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetVblankTimestamp 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetVblankTimestamp_t)(
    ctl_display_output_handle_t,
    ctl_vblank_ts_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlLinkDisplayAdapters 
typedef ctl_result_t (CTL_APICALL *ctl_pfnLinkDisplayAdapters_t)(
    ctl_device_adapter_handle_t,
    ctl_lda_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlUnlinkDisplayAdapters 
typedef ctl_result_t (CTL_APICALL *ctl_pfnUnlinkDisplayAdapters_t)(
    ctl_device_adapter_handle_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetLinkedDisplayAdapters 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetLinkedDisplayAdapters_t)(
    ctl_device_adapter_handle_t,
    ctl_lda_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSetDynamicContrastEnhancement 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSetDynamicContrastEnhancement_t)(
    ctl_display_output_handle_t,
    ctl_dce_args_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumEngineGroups 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumEngineGroups_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_engine_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEngineGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEngineGetProperties_t)(
    ctl_engine_handle_t,
    ctl_engine_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEngineGetActivity 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEngineGetActivity_t)(
    ctl_engine_handle_t,
    ctl_engine_stats_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumFans 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumFans_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_fan_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFanGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFanGetProperties_t)(
    ctl_fan_handle_t,
    ctl_fan_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFanGetConfig 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFanGetConfig_t)(
    ctl_fan_handle_t,
    ctl_fan_config_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFanSetDefaultMode 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFanSetDefaultMode_t)(
    ctl_fan_handle_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFanSetFixedSpeedMode 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFanSetFixedSpeedMode_t)(
    ctl_fan_handle_t,
    const ctl_fan_speed_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFanSetSpeedTableMode 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFanSetSpeedTableMode_t)(
    ctl_fan_handle_t,
    const ctl_fan_speed_table_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFanGetState 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFanGetState_t)(
    ctl_fan_handle_t,
    ctl_fan_speed_units_t,
    int32_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumFrequencyDomains 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumFrequencyDomains_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_freq_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFrequencyGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFrequencyGetProperties_t)(
    ctl_freq_handle_t,
    ctl_freq_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFrequencyGetAvailableClocks 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFrequencyGetAvailableClocks_t)(
    ctl_freq_handle_t,
    uint32_t*,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFrequencyGetRange 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFrequencyGetRange_t)(
    ctl_freq_handle_t,
    ctl_freq_range_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFrequencySetRange 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFrequencySetRange_t)(
    ctl_freq_handle_t,
    const ctl_freq_range_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFrequencyGetState 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFrequencyGetState_t)(
    ctl_freq_handle_t,
    ctl_freq_state_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlFrequencyGetThrottleTime 
typedef ctl_result_t (CTL_APICALL *ctl_pfnFrequencyGetThrottleTime_t)(
    ctl_freq_handle_t,
    ctl_freq_throttle_time_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSupportedVideoProcessingCapabilities 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSupportedVideoProcessingCapabilities_t)(
    ctl_device_adapter_handle_t,
    ctl_video_processing_feature_caps_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlGetSetVideoProcessingFeature 
typedef ctl_result_t (CTL_APICALL *ctl_pfnGetSetVideoProcessingFeature_t)(
    ctl_device_adapter_handle_t,
    ctl_video_processing_feature_getset_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumMemoryModules 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumMemoryModules_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_mem_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlMemoryGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnMemoryGetProperties_t)(
    ctl_mem_handle_t,
    ctl_mem_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlMemoryGetState 
typedef ctl_result_t (CTL_APICALL *ctl_pfnMemoryGetState_t)(
    ctl_mem_handle_t,
    ctl_mem_state_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlMemoryGetBandwidth 
typedef ctl_result_t (CTL_APICALL *ctl_pfnMemoryGetBandwidth_t)(
    ctl_mem_handle_t,
    ctl_mem_bandwidth_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGetProperties_t)(
    ctl_device_adapter_handle_t,
    ctl_oc_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockWaiverSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockWaiverSet_t)(
    ctl_device_adapter_handle_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGpuFrequencyOffsetGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGpuFrequencyOffsetGet_t)(
    ctl_device_adapter_handle_t,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGpuFrequencyOffsetSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGpuFrequencyOffsetSet_t)(
    ctl_device_adapter_handle_t,
    double
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGpuVoltageOffsetGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGpuVoltageOffsetGet_t)(
    ctl_device_adapter_handle_t,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGpuVoltageOffsetSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGpuVoltageOffsetSet_t)(
    ctl_device_adapter_handle_t,
    double
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGpuLockGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGpuLockGet_t)(
    ctl_device_adapter_handle_t,
    ctl_oc_vf_pair_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockGpuLockSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockGpuLockSet_t)(
    ctl_device_adapter_handle_t,
    ctl_oc_vf_pair_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockVramFrequencyOffsetGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockVramFrequencyOffsetGet_t)(
    ctl_device_adapter_handle_t,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockVramFrequencyOffsetSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockVramFrequencyOffsetSet_t)(
    ctl_device_adapter_handle_t,
    double
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockVramVoltageOffsetGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockVramVoltageOffsetGet_t)(
    ctl_device_adapter_handle_t,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockVramVoltageOffsetSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockVramVoltageOffsetSet_t)(
    ctl_device_adapter_handle_t,
    double
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockPowerLimitGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockPowerLimitGet_t)(
    ctl_device_adapter_handle_t,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockPowerLimitSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockPowerLimitSet_t)(
    ctl_device_adapter_handle_t,
    double
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockTemperatureLimitGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockTemperatureLimitGet_t)(
    ctl_device_adapter_handle_t,
    double*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockTemperatureLimitSet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockTemperatureLimitSet_t)(
    ctl_device_adapter_handle_t,
    double
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPowerTelemetryGet 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPowerTelemetryGet_t)(
    ctl_device_adapter_handle_t,
    ctl_power_telemetry_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlOverclockResetToDefault 
typedef ctl_result_t (CTL_APICALL *ctl_pfnOverclockResetToDefault_t)(
    ctl_device_adapter_handle_t
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPciGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPciGetProperties_t)(
    ctl_device_adapter_handle_t,
    ctl_pci_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPciGetState 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPciGetState_t)(
    ctl_device_adapter_handle_t,
    ctl_pci_state_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumPowerDomains 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumPowerDomains_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_pwr_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPowerGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPowerGetProperties_t)(
    ctl_pwr_handle_t,
    ctl_power_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPowerGetEnergyCounter 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPowerGetEnergyCounter_t)(
    ctl_pwr_handle_t,
    ctl_power_energy_counter_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPowerGetLimits 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPowerGetLimits_t)(
    ctl_pwr_handle_t,
    ctl_power_limits_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlPowerSetLimits 
typedef ctl_result_t (CTL_APICALL *ctl_pfnPowerSetLimits_t)(
    ctl_pwr_handle_t,
    const ctl_power_limits_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlEnumTemperatureSensors 
typedef ctl_result_t (CTL_APICALL *ctl_pfnEnumTemperatureSensors_t)(
    ctl_device_adapter_handle_t,
    uint32_t*,
    ctl_temp_handle_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlTemperatureGetProperties 
typedef ctl_result_t (CTL_APICALL *ctl_pfnTemperatureGetProperties_t)(
    ctl_temp_handle_t,
    ctl_temp_properties_t*
    );


///////////////////////////////////////////////////////////////////////////////
/// @brief Function-pointer for ctlTemperatureGetState 
typedef ctl_result_t (CTL_APICALL *ctl_pfnTemperatureGetState_t)(
    ctl_temp_handle_t,
    double*
    );


#if defined(__cplusplus)
} // extern "C"
#endif

#endif // _CTL_API_H