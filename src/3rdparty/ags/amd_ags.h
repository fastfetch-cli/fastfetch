//
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

/// \file
/// \mainpage
/// AGS Library Overview
/// --------------------
/// This document provides an overview of the AGS (AMD GPU Services) library. The AGS library provides software developers with the ability to query
/// AMD GPU software and hardware state information that is not normally available through standard operating systems or graphic APIs.
///
/// The latest version of the API is publicly hosted here: https://github.com/GPUOpen-LibrariesAndSDKs/AGS_SDK/.
/// It is also worth checking http://gpuopen.com/gaming-product/amd-gpu-services-ags-library/ for any updates and articles on AGS.
/// \internal
/// Online documentation is publicly hosted here: http://gpuopen-librariesandsdks.github.io/ags/
/// \endinternal
///
/// ---------------------------------------
/// What's new in AGS 6.2 since version 6.1
/// ---------------------------------------
/// AGS 6.2 includes the following updates:
/// * Shader clock intrinsics
/// * Minor improvements and fixes
///
/// ---------------------------------------
/// What's new in AGS 6.1 since version 6.0
/// ---------------------------------------
/// AGS 6.1 includes the following updates:
/// * RDNA3 detection
///
/// ---------------------------------------
/// What's new in AGS 6.0 since version 5.4.2
/// ---------------------------------------
/// AGS 6.0 includes the following updates:
/// * DX12 ray tracing hit token for RDNA2 hardware.
/// * Shader intrinsic that exposes ReadLaneAt in DX12.
/// * Shader intrinsics that expose explicit float conversions in DX12.
/// * Refactored and revised API to minimize user error.
/// * Added agsGetVersionNumber.
/// * Detection for external GPUs.
/// * Detection of RDNA2 architecture.
/// * Grouped the more established intrinsics together into per year support.
/// * Function pointer typedefs for the API
///
/// ---------------------------------------
/// What's new in AGS 5.4.2 since version 5.4.1
/// ---------------------------------------
/// AGS 5.4.2 includes the following updates:
/// * sharedMemoryInBytes has been reinstated.
/// * Clock speed returned for APUs.
///
/// ---------------------------------------
/// What's new in AGS 5.4.1 since version 5.4.0
/// ---------------------------------------
/// AGS 5.4.1 includes the following updates:
/// * AsicFamily_Count to help with code maintenance.
/// * Visual Studio 2019 support.
/// * x86 support
/// * BaseInstance and BaseVertex intrinsics along with corresponding caps bits.
/// * GetWaveSize intrinsic along with corresponding caps bits.
///
/// ---------------------------------------
/// What's new in AGS 5.4 since version 5.3
/// ---------------------------------------
/// AGS 5.4 includes the following updates:
/// * A more detailed description of the GPU architecture, now including RDNA GPUs.
/// * Radeon 7 core and memory speeds returned.
/// * Draw index and Atomic U64 intrinsics for both DX11 and DX12.
///
/// ---------------------------------------
/// What's new in AGS 5.3 since version 5.2
/// ---------------------------------------
/// AGS 5.3 includes the following updates:
/// * DX11 deferred context support for Multi Draw Indirect and UAV Overlap extensions.
/// * A Radeon Software Version helper to determine whether the installed driver meets your game's minimum driver version requirements.
/// * Freesync HDR Gamma 2.2 mode which uses a 1010102 swapchain and can be considered as an alternative to using the 64 bit swapchain required for Freesync HDR scRGB.
///
/// Using the AGS library
/// ---------------------
/// It is recommended to take a look at the source code for the samples that come with the AGS SDK:
/// * AGSSample
/// * CrossfireSample
/// * EyefinitySample
/// The AGSSample application is the simplest of the three examples and demonstrates the code required to initialize AGS and use it to query the GPU and Eyefinity state.
/// The CrossfireSample application demonstrates the use of the new API to transfer resources on GPUs in Crossfire mode. Lastly, the EyefinitySample application provides a more
/// extensive example of Eyefinity setup than the basic example provided in AGSSample.
/// There are other samples on Github that demonstrate the DirectX shader extensions, such as the Barycentrics11 and Barycentrics12 samples.
///
/// To add AGS support to an existing project, follow these steps:
/// * Link your project against the correct import library. Choose from either the 32 bit or 64 bit version.
/// * Copy the AGS dll into the same directory as your game executable.
/// * Include the amd_ags.h header file from your source code.
/// * Include the AGS hlsl files if you are using the shader intrinsics.
/// * Declare a pointer to an AGSContext and make this available for all subsequent calls to AGS.
/// * On game initialization, call \ref agsInitialize passing in the address of the context. On success, this function will return a valid context pointer.
///
/// Don't forget to cleanup AGS by calling \ref agsDeInitialize when the app exits, after the device has been destroyed.

#ifndef AMD_AGS_H
#define AMD_AGS_H

#define AMD_AGS_VERSION_MAJOR 6             ///< AGS major version
#define AMD_AGS_VERSION_MINOR 2             ///< AGS minor version
#define AMD_AGS_VERSION_PATCH 0             ///< AGS patch version

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup Defines AGS defines
/// @{
#if defined (AGS_GCC)
#define AMD_AGS_API
#else
#define AMD_AGS_API __declspec(dllexport)   ///< AGS exported functions
#endif

#define AGS_MAKE_VERSION( major, minor, patch ) ( ( major << 22 ) | ( minor << 12 ) | patch ) ///< Macro to create the app and engine versions for the fields in \ref AGSDX12ExtensionParams and \ref AGSDX11ExtensionParams and the Radeon Software Version
#define AGS_UNSPECIFIED_VERSION 0xFFFFAD00                                                    ///< Use this to specify no version
#define AGS_CURRENT_VERSION AGS_MAKE_VERSION( AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH ) ///< Macro to return the current AGS version as defined by the AGS header file
/// @}

/// \defgroup enums General enumerations
/// @{

/// The return codes
typedef enum AGSReturnCode
{
    AGS_SUCCESS,                    ///< Successful function call
    AGS_FAILURE,                    ///< Failed to complete call for some unspecified reason
    AGS_INVALID_ARGS,               ///< Invalid arguments into the function
    AGS_OUT_OF_MEMORY,              ///< Out of memory when allocating space internally
    AGS_MISSING_D3D_DLL,            ///< Returned when a D3D dll fails to load
    AGS_LEGACY_DRIVER,              ///< Returned if a feature is not present in the installed driver
    AGS_NO_AMD_DRIVER_INSTALLED,    ///< Returned if the AMD GPU driver does not appear to be installed
    AGS_EXTENSION_NOT_SUPPORTED,    ///< Returned if the driver does not support the requested driver extension
    AGS_ADL_FAILURE,                ///< Failure in ADL (the AMD Display Library)
    AGS_DX_FAILURE,                 ///< Failure from DirectX runtime
    AGS_D3DDEVICE_NOT_CREATED       ///< Failure due to not creating the D3D device successfully via AGS.
} AGSReturnCode;

/// @}

typedef struct AGSContext AGSContext;  ///< All function calls in AGS require a pointer to a context. This is generated via \ref agsInitialize

/// The rectangle struct used by AGS.
typedef struct AGSRect
{
    int offsetX;    ///< Offset on X axis
    int offsetY;    ///< Offset on Y axis
    int width;      ///< Width of rectangle
    int height;     ///< Height of rectangle
} AGSRect;

/// The display info struct used to describe a display enumerated by AGS
typedef struct AGSDisplayInfo
{
    char                    name[ 256 ];                    ///< The name of the display
    char                    displayDeviceName[ 32 ];        ///< The display device name, i.e. DISPLAY_DEVICE::DeviceName

    unsigned int            isPrimaryDisplay : 1;           ///< Whether this display is marked as the primary display
    unsigned int            HDR10 : 1;                      ///< HDR10 is supported on this display
    unsigned int            dolbyVision : 1;                ///< Dolby Vision is supported on this display
    unsigned int            freesync : 1;                   ///< Freesync is supported on this display
    unsigned int            freesyncHDR : 1;                ///< Freesync HDR is supported on this display
    unsigned int            eyefinityInGroup : 1;           ///< The display is part of the Eyefinity group
    unsigned int            eyefinityPreferredDisplay : 1;  ///< The display is the preferred display in the Eyefinity group for displaying the UI
    unsigned int            eyefinityInPortraitMode : 1;    ///< The display is in the Eyefinity group but in portrait mode
    unsigned int            reservedPadding : 24;           ///< Reserved for future use

    int                     maxResolutionX;                 ///< The maximum supported resolution of the unrotated display
    int                     maxResolutionY;                 ///< The maximum supported resolution of the unrotated display
    float                   maxRefreshRate;                 ///< The maximum supported refresh rate of the display

    AGSRect                 currentResolution;              ///< The current resolution and position in the desktop, ignoring Eyefinity bezel compensation
    AGSRect                 visibleResolution;              ///< The visible resolution and position. When Eyefinity bezel compensation is enabled this will
                                                            ///< be the sub region in the Eyefinity single large surface (SLS)
    float                   currentRefreshRate;             ///< The current refresh rate

    int                     eyefinityGridCoordX;            ///< The X coordinate in the Eyefinity grid. -1 if not in an Eyefinity group
    int                     eyefinityGridCoordY;            ///< The Y coordinate in the Eyefinity grid. -1 if not in an Eyefinity group

    double                  chromaticityRedX;               ///< Red display primary X coord
    double                  chromaticityRedY;               ///< Red display primary Y coord

    double                  chromaticityGreenX;             ///< Green display primary X coord
    double                  chromaticityGreenY;             ///< Green display primary Y coord

    double                  chromaticityBlueX;              ///< Blue display primary X coord
    double                  chromaticityBlueY;              ///< Blue display primary Y coord

    double                  chromaticityWhitePointX;        ///< White point X coord
    double                  chromaticityWhitePointY;        ///< White point Y coord

    double                  screenDiffuseReflectance;       ///< Percentage expressed between 0 - 1
    double                  screenSpecularReflectance;      ///< Percentage expressed between 0 - 1

    double                  minLuminance;                   ///< The minimum luminance of the display in nits
    double                  maxLuminance;                   ///< The maximum luminance of the display in nits
    double                  avgLuminance;                   ///< The average luminance of the display in nits

    int                     logicalDisplayIndex;            ///< The internally used index of this display
    int                     adlAdapterIndex;                ///< The internally used ADL adapter index
    int                     reserved;                       ///< reserved field
} AGSDisplayInfo;

/// The ASIC family
typedef enum AsicFamily
{
    AsicFamily_Unknown,                                         ///< Unknown architecture, potentially from another IHV. Check \ref AGSDeviceInfo::vendorId
    AsicFamily_PreGCN,                                          ///< Pre GCN architecture.
    AsicFamily_GCN1,                                            ///< AMD GCN 1 architecture: Oland, Cape Verde, Pitcairn & Tahiti.
    AsicFamily_GCN2,                                            ///< AMD GCN 2 architecture: Hawaii & Bonaire.  This also includes APUs Kaveri and Carrizo.
    AsicFamily_GCN3,                                            ///< AMD GCN 3 architecture: Tonga & Fiji.
    AsicFamily_GCN4,                                            ///< AMD GCN 4 architecture: Polaris.
    AsicFamily_Vega,                                            ///< AMD Vega architecture, including Raven Ridge (ie AMD Ryzen CPU + AMD Vega GPU).
    AsicFamily_RDNA,                                            ///< AMD RDNA architecture
    AsicFamily_RDNA2,                                           ///< AMD RDNA2 architecture
    AsicFamily_RDNA3,                                           ///< AMD RDNA3 architecture

    AsicFamily_Count                                            ///< Number of enumerated ASIC families
} AsicFamily;

/// The device info struct used to describe a physical GPU enumerated by AGS
typedef struct AGSDeviceInfo
{
    const char*                     adapterString;                  ///< The adapter name string
    AsicFamily                      asicFamily;                     ///< Set to Unknown if not AMD hardware
    unsigned int                    isAPU : 1;                      ///< Whether this device is an APU
    unsigned int                    isPrimaryDevice : 1;            ///< Whether this device is marked as the primary device
    unsigned int                    isExternal :1;                  ///< Whether this device is a detachable, external device
    unsigned int                    reservedPadding : 29;           ///< Reserved for future use

    int                             vendorId;                       ///< The vendor id
    int                             deviceId;                       ///< The device id
    int                             revisionId;                     ///< The revision id

    int                             numCUs;                         ///< Number of compute units
    int                             numWGPs;                        ///< Number of RDNA Work Group Processors.  Only valid if ASIC is RDNA onwards.

    int                             numROPs;                        ///< Number of ROPs
    int                             coreClock;                      ///< Core clock speed at 100% power in MHz
    int                             memoryClock;                    ///< Memory clock speed at 100% power in MHz
    int                             memoryBandwidth;                ///< Memory bandwidth in MB/s
    float                           teraFlops;                      ///< Teraflops of GPU. Zero if not GCN onwards. Calculated from iCoreClock * iNumCUs * 64 Pixels/clk * 2 instructions/MAD

    unsigned long long              localMemoryInBytes;             ///< The size of local memory in bytes. 0 for non AMD hardware.
    unsigned long long              sharedMemoryInBytes;            ///< The size of system memory available to the GPU in bytes.  It is important to factor this into your VRAM budget for APUs
                                                                    ///< as the reported local memory will only be a small fraction of the total memory available to the GPU.

    int                             numDisplays;                    ///< The number of active displays found to be attached to this adapter.
    AGSDisplayInfo*                 displays;                       ///< List of displays allocated by AGS to be numDisplays in length.

    int                             eyefinityEnabled;               ///< Indicates if Eyefinity is active
    int                             eyefinityGridWidth;             ///< Contains width of the multi-monitor grid that makes up the Eyefinity Single Large Surface.
    int                             eyefinityGridHeight;            ///< Contains height of the multi-monitor grid that makes up the Eyefinity Single Large Surface.
    int                             eyefinityResolutionX;           ///< Contains width in pixels of the multi-monitor Single Large Surface.
    int                             eyefinityResolutionY;           ///< Contains height in pixels of the multi-monitor Single Large Surface.
    int                             eyefinityBezelCompensated;      ///< Indicates if bezel compensation is used for the current SLS display area. 1 if enabled, and 0 if disabled.

    int                             adlAdapterIndex;                ///< Internally used index into the ADL list of adapters
    int                             reserved;                       ///< reserved field
} AGSDeviceInfo;

/// \defgroup general General API functions
/// API for initialization, cleanup, HDR display modes and Crossfire GPU count
/// @{

typedef void* (__stdcall *AGS_ALLOC_CALLBACK)( size_t allocationSize );     ///< AGS user defined allocation prototype
typedef void (__stdcall *AGS_FREE_CALLBACK)( void* allocationPtr );         ///< AGS user defined free prototype

/// The configuration options that can be passed in to \ref agsInitialize
typedef struct AGSConfiguration
{
    AGS_ALLOC_CALLBACK      allocCallback;                  ///< Optional memory allocation callback. If not supplied, malloc() is used
    AGS_FREE_CALLBACK       freeCallback;                   ///< Optional memory freeing callback. If not supplied, free() is used
} AGSConfiguration;

/// The top level GPU information returned from \ref agsInitialize
typedef struct AGSGPUInfo
{
    const char*             driverVersion;                  ///< The AMD driver package version
    const char*             radeonSoftwareVersion;          ///< The Radeon Software Version

    int                     numDevices;                     ///< Number of GPUs in the system
    AGSDeviceInfo*          devices;                        ///< List of GPUs in the system
} AGSGPUInfo;

/// The struct to specify the display settings to the driver.
typedef struct AGSDisplaySettings AGSDisplaySettings;


/// The result returned from \ref agsCheckDriverVersion
typedef enum AGSDriverVersionResult
{
    AGS_SOFTWAREVERSIONCHECK_OK,                              ///< The reported Radeon Software Version is newer or the same as the required version
    AGS_SOFTWAREVERSIONCHECK_OLDER,                           ///< The reported Radeon Software Version is older than the required version
    AGS_SOFTWAREVERSIONCHECK_UNDEFINED                        ///< The check could not determine as result.  This could be because it is a private or custom driver or just invalid arguments.
} AGSDriverVersionResult;

///
/// Helper function to check the installed software version against the required software version.
///
/// \param [in] radeonSoftwareVersionReported       The Radeon Software Version returned from \ref AGSGPUInfo::radeonSoftwareVersion.
/// \param [in] radeonSoftwareVersionRequired       The Radeon Software Version to check against.  This is specificed using \ref AGS_MAKE_VERSION.
/// \return                                         The result of the check.
///
AMD_AGS_API AGSDriverVersionResult agsCheckDriverVersion( const char* radeonSoftwareVersionReported, unsigned int radeonSoftwareVersionRequired );

///
/// Function to return the AGS version number.
///
/// \return                                         The version number made using AGS_MAKE_VERSION( AMD_AGS_VERSION_MAJOR, AMD_AGS_VERSION_MINOR, AMD_AGS_VERSION_PATCH ).
///
AMD_AGS_API int agsGetVersionNumber();

///
/// Function used to initialize the AGS library.
/// agsVersion must be specified as AGS_CURRENT_VERSION or the call will return \ref AGS_INVALID_ARGS.
/// Must be called prior to any of the subsequent AGS API calls.
/// Must be called prior to ID3D11Device or ID3D12Device creation.
/// \note The caller of this function should handle the possibility of the call failing in the cases below. One option is to do a vendor id check and only call \ref agsInitialize if there is an AMD GPU present.
/// \note This function will fail with \ref AGS_NO_AMD_DRIVER_INSTALLED if there is no AMD driver found on the system.
/// \note This function will fail with \ref AGS_LEGACY_DRIVER in Catalyst versions before 12.20.
///
/// \param [in] agsVersion                          The API version specified using the \ref AGS_CURRENT_VERSION macro. If this does not match the version in the binary this initialization call will fail.
/// \param [in] config                              Optional pointer to a AGSConfiguration struct to override the default library configuration.
/// \param [out] context                            Address of a pointer to a context. This function allocates a context on the heap which is then required for all subsequent API calls.
/// \param [out] gpuInfo                            Optional pointer to a AGSGPUInfo struct which will get filled in for all the GPUs in the system.
///
AMD_AGS_API AGSReturnCode agsInitialize( int agsVersion, const AGSConfiguration* config, AGSContext** context, AGSGPUInfo* gpuInfo );

///
///   Function used to clean up the AGS library.
///
/// \param [in] context                             Pointer to a context. This function will deallocate the context from the heap.
///
AMD_AGS_API AGSReturnCode agsDeInitialize( AGSContext* context );

///
/// Function used to set a specific display into HDR mode
/// \note Setting all of the values apart from color space and transfer function to zero will cause the display to use defaults.
/// \note Call this function after each mode change (switch to fullscreen, any change in swapchain etc).
/// \note HDR10 PQ mode requires a 1010102 swapchain.
/// \note HDR10 scRGB mode requires an FP16 swapchain.
/// \note Freesync HDR scRGB mode requires an FP16 swapchain.
/// \note Freesync HDR Gamma 2.2 mode requires a 1010102 swapchain.
/// \note Dolby Vision requires a 8888 UNORM swapchain.
///
/// \param [in] context                             Pointer to a context. This is generated by \ref agsInitialize
/// \param [in] deviceIndex                         The index of the device listed in \ref AGSGPUInfo::devices.
/// \param [in] displayIndex                        The index of the display listed in \ref AGSDeviceInfo::displays.
/// \param [in] settings                            Pointer to the display settings to use.
///
AMD_AGS_API AGSReturnCode agsSetDisplayMode( AGSContext* context, int deviceIndex, int displayIndex, const AGSDisplaySettings* settings );

/// @}

/// @}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // AMD_AGS_H
