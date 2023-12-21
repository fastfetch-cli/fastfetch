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

#if !defined (AGS_DIRECTX_TYPES_INCLUDED)
// Forward declaration of D3D and DXGI types
struct IDXGIAdapter;
struct IDXGISwapChain;
struct DXGI_SWAP_CHAIN_DESC;
enum D3D_DRIVER_TYPE;
enum D3D_FEATURE_LEVEL;
enum D3D_PRIMITIVE_TOPOLOGY;

// Forward declaration of D3D11 types
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Resource;
struct ID3D11Buffer;
struct ID3D11Texture1D;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct D3D11_BUFFER_DESC;
struct D3D11_TEXTURE1D_DESC;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_TEXTURE3D_DESC;
struct D3D11_SUBRESOURCE_DATA;
struct tagRECT;
typedef tagRECT D3D11_RECT;             ///< typedef this ourselves so we don't have to drag d3d11.h in

// Forward declaration of D3D12 types
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
#endif

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

struct AGSContext;  ///< All function calls in AGS require a pointer to a context. This is generated via \ref agsInitialize

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

/// The device info struct used to describe a physical GPU enumerated by AGS
typedef struct AGSDeviceInfo
{
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
typedef struct AGSDisplaySettings
{
    /// The display mode
    typedef enum Mode
    {
        Mode_SDR,                                           ///< SDR mode
        Mode_HDR10_PQ,                                      ///< HDR10 PQ encoding, requiring a 1010102 UNORM swapchain and PQ encoding in the output shader.
        Mode_HDR10_scRGB,                                   ///< HDR10 scRGB, requiring an FP16 swapchain. Values of 1.0 == 80 nits, 125.0 == 10000 nits.
        Mode_FreesyncHDR_scRGB,                             ///< Freesync HDR scRGB, requiring an FP16 swapchain. A value of 1.0 == 80 nits.
        Mode_FreesyncHDR_Gamma22,                           ///< Freesync HDR Gamma 2.2, requiring a 1010102 UNORM swapchain.  The output needs to be encoded to gamma 2.2.
        Mode_DolbyVision,                                   ///< Dolby Vision, requiring an 8888 UNORM swapchain

        Mode_Count                                          ///< Number of enumerated display modes
    } Mode;

    Mode                    mode;                           ///< The display mode to set the display into

    double                  chromaticityRedX;               ///< Red display primary X coord
    double                  chromaticityRedY;               ///< Red display primary Y coord

    double                  chromaticityGreenX;             ///< Green display primary X coord
    double                  chromaticityGreenY;             ///< Green display primary Y coord

    double                  chromaticityBlueX;              ///< Blue display primary X coord
    double                  chromaticityBlueY;              ///< Blue display primary Y coord

    double                  chromaticityWhitePointX;        ///< White point X coord
    double                  chromaticityWhitePointY;        ///< White point Y coord

    double                  minLuminance;                   ///< The minimum scene luminance in nits
    double                  maxLuminance;                   ///< The maximum scene luminance in nits

    double                  maxContentLightLevel;           ///< The maximum content light level in nits (MaxCLL)
    double                  maxFrameAverageLightLevel;      ///< The maximum frame average light level in nits (MaxFALL)

    unsigned int            disableLocalDimming : 1;        ///< Disables local dimming if possible
    unsigned int            reservedPadding : 31;           ///< Reserved
} AGSDisplaySettings;


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


/// \defgroup dxappreg App Registration
/// @{
/// This extension allows an application to voluntarily register itself with the driver, providing a more robust app detection solution and avoid the issue of the driver relying on exe names to match the app to a driver profile.
/// It is available when creating the device for both DirectX11 and DirectX12 via \ref agsDriverExtensionsDX11_CreateDevice and \ref agsDriverExtensionsDX12_CreateDevice respectively.
/// This feature is supported in Radeon Software Version 17.9.2 onwards.
/// Rules:
/// * AppName or EngineName must be set, but both are not required. Engine profiles will be used only if app specific profiles do not exist.
/// * In an engine, the EngineName should be set, so a default profile can be built. If an app modifies the engine, the AppName should be set, to allow a profile for the specific app.
/// * Version number is not mandatory, but recommended. The use of which can prevent the use of profiles for incompatible versions (for instance engine versions that introduce or change features), and can help prevent older profiles from being used (and introducing new bugs) before the profile is tested with new app builds.
/// * If Version numbers are used and a new version is introduced, a new profile will not be enabled until an AMD engineer has been able to update a previous profile, or make a new one.
///
/// The cases for profile selection are as follows:
///
/// |Case|Profile Applied|
/// |----|---------------|
/// | App or Engine Version has profile | The profile is used. |
/// | App or Engine Version num < profile version num | The closest profile > the version number is used. |
/// | App or Engine Version num > profile version num | No profile selected/The previous method is used. |
/// | App and Engine Version have profile | The App's profile is used. |
/// | App and Engine Version num < profile version | The closest App profile > the version number is used. |
/// | App and Engine Version, no App profile found | The Engine profile will be used. |
/// | App/Engine name but no Version, has profile | The latest profile is used. |
/// | No name or version, or no profile | The previous app detection method is used. |
///
/// As shown above, if an App name is given, and a profile is found for that app, that will be prioritized. The Engine name and profile will be used only if no app name is given, or no viable profile is found for the app name.
/// In the case that App nor Engine have a profile, the previous app detection methods will be used. If given a version number that is larger than any profile version number, no profile will be selected.
/// This is specifically to prevent cases where an update to an engine or app will cause catastrophic breaks in the profile, allowing an engineer to test the profile before clearing it for public use with the new engine/app update.
///
/// @}

/// \defgroup dx12 DirectX12 Extensions
/// DirectX12 driver extensions
/// @{

/// \defgroup dx12init Device and device object creation and cleanup
/// It is now mandatory to call \ref agsDriverExtensionsDX12_CreateDevice when creating a device if the user wants to access any future DX12 AMD extensions.
/// The corresponding \ref agsDriverExtensionsDX12_DestroyDevice call must be called to release the device and free up the internal resources allocated by the create call.
/// @{

/// The struct to specify the DX12 device creation parameters
typedef struct AGSDX12DeviceCreationParams
{
    IDXGIAdapter*               pAdapter;                   ///< Pointer to the adapter to use when creating the device.  This may be null.
    IID                         iid;                        ///< The interface ID for the type of device to be created.
    D3D_FEATURE_LEVEL           FeatureLevel;               ///< The minimum feature level to create the device with.
} AGSDX12DeviceCreationParams;

/// The struct to specify DX12 additional device creation parameters
typedef struct AGSDX12ExtensionParams
{
    const WCHAR*    pAppName;               ///< Application name
    const WCHAR*    pEngineName;            ///< Engine name
    unsigned int    appVersion;             ///< Application version
    unsigned int    engineVersion;          ///< Engine version
    unsigned int    uavSlot;                ///< The UAV slot reserved for intrinsic support.  Refer to the \ref agsDriverExtensionsDX12_CreateDevice documentation for more details.
} AGSDX12ExtensionParams;

/// The struct to hold all the returned parameters from the device creation call
typedef struct AGSDX12ReturnedParams
{
    ID3D12Device*           pDevice;                            ///< The newly created device
    typedef struct ExtensionsSupported                          /// Extensions for DX12
    {
        unsigned int        intrinsics16 : 1;                   ///< Supported in Radeon Software Version 16.9.2 onwards. ReadFirstLane, ReadLane, LaneID, Swizzle, Ballot, MBCount, Med3, Barycentrics
        unsigned int        intrinsics17 : 1;                   ///< Supported in Radeon Software Version 17.9.1 onwards. WaveReduce, WaveScan
        unsigned int        userMarkers : 1;                    ///< Supported in Radeon Software Version 17.9.1 onwards.
        unsigned int        appRegistration : 1;                ///< Supported in Radeon Software Version 17.9.1 onwards.
        unsigned int        UAVBindSlot : 1;                    ///< Supported in Radeon Software Version 19.5.1 onwards.
        unsigned int        intrinsics19 : 1;                   ///< Supported in Radeon Software Version 19.12.2 onwards. DrawIndex, AtomicU64
        unsigned int        baseVertex : 1;                     ///< Supported in Radeon Software Version 20.2.1 onwards.
        unsigned int        baseInstance : 1;                   ///< Supported in Radeon Software Version 20.2.1 onwards.
        unsigned int        getWaveSize : 1;                    ///< Supported in Radeon Software Version 20.5.1 onwards.
        unsigned int        floatConversion : 1;                ///< Supported in Radeon Software Version 20.5.1 onwards.
        unsigned int        readLaneAt : 1;                     ///< Supported in Radeon Software Version 20.11.2 onwards.
        unsigned int        rayHitToken : 1;                    ///< Supported in Radeon Software Version 20.11.2 onwards.
        unsigned int        shaderClock : 1;                    ///< Supported in Radeon Software Version 23.1.1 onwards.
        unsigned int        padding : 19;                       ///< Reserved
    } ExtensionsSupported;
    ExtensionsSupported     extensionsSupported;                ///< List of supported extensions
} AGSDX12ReturnedParams;

/// The space id for DirectX12 intrinsic support
const unsigned int AGS_DX12_SHADER_INTRINSICS_SPACE_ID = 0x7FFF0ADE; // 2147420894

///
/// Function used to create a D3D12 device with additional AMD-specific initialization parameters.
///
/// When using the HLSL shader extensions please note:
/// * The shader compiler should not use the D3DCOMPILE_SKIP_OPTIMIZATION (/Od) option or /O0, otherwise it will not work.
/// * The shader compiler needs D3DCOMPILE_ENABLE_STRICTNESS (/Ges) enabled.
/// * The intrinsic instructions require a 5.1 shader model.
/// * The Root Signature will need to reserve an extra UAV resource slot. This is not a real resource that requires allocating, it is just used to encode the intrinsic instructions.
///
/// The easiest way to set up the reserved UAV slot is to specify it at u0.  The register space id will automatically be assumed to be \ref AGS_DX12_SHADER_INTRINSICS_SPACE_ID.
/// The HLSL expects this as default and the set up code would look similar to this:
/// \code{.cpp}
/// CD3DX12_DESCRIPTOR_RANGE range[];
/// ...
/// range[ 0 ].Init( D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, AGS_DX12_SHADER_INTRINSICS_SPACE_ID ); // u0 at driver-reserved space id
/// \endcode
///
/// Newer drivers also support a user-specified slot in which case the register space id is assumed to be 0.  It is important that the \ref AGSDX12ReturnedParams::ExtensionsSupported::UAVBindSlot bit is set.
/// to ensure the driver can support this.  If not, then u0 and \ref AGS_DX12_SHADER_INTRINSICS_SPACE_ID must be used.
/// If the driver does support this feature and a non zero slot is required, then the HLSL must also define AMD_EXT_SHADER_INTRINSIC_UAV_OVERRIDE as the matching slot value.
///
/// \param [in] context                             Pointer to a context. This is generated by \ref agsInitialize
/// \param [in] creationParams                      Pointer to the struct to specify the existing DX12 device creation parameters.
/// \param [in] extensionParams                     Optional pointer to the struct to specify DX12 additional device creation parameters.
/// \param [out] returnedParams                     Pointer to struct to hold all the returned parameters from the call.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_CreateDevice( AGSContext* context, const AGSDX12DeviceCreationParams* creationParams, const AGSDX12ExtensionParams* extensionParams, AGSDX12ReturnedParams* returnedParams );

///
/// Function to destroy the D3D12 device.
/// This call will also cleanup any AMD-specific driver extensions for D3D12.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] device                              Pointer to the D3D12 device.
/// \param [out] deviceReferences                   Optional pointer to an unsigned int that will be set to the value returned from device->Release().
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_DestroyDevice( AGSContext* context, ID3D12Device* device, unsigned int* deviceReferences );

/// @}

/// \defgroup dx12usermarkers User Markers
/// @{

///
/// Function used to push an AMD user marker onto the command list.
/// This is only has an effect if \ref AGSDX12ReturnedParams::ExtensionsSupported::userMarkers is present.
/// Supported in Radeon Software Version 17.9.1 onwards.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] commandList                         Pointer to the command list.
/// \param [in] data                                The UTF-8 marker string.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_PushMarker( AGSContext* context, ID3D12GraphicsCommandList* commandList, const char* data );

///
/// Function used to pop an AMD user marker on the command list.
/// Supported in Radeon Software Version 17.9.1 onwards.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] commandList                         Pointer to the command list.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_PopMarker( AGSContext* context, ID3D12GraphicsCommandList* commandList );

///
/// Function used to insert an single event AMD user marker onto the command list.
/// Supported in Radeon Software Version 17.9.1 onwards.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] commandList                         Pointer to the command list.
/// \param [in] data                                The UTF-8 marker string.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_SetMarker( AGSContext* context, ID3D12GraphicsCommandList* commandList, const char* data );

/// @}

/// @}

/// \defgroup dx11 DirectX11 Extensions
/// DirectX11 driver extensions
/// @{

/// \defgroup dx11init Device creation and cleanup
/// It is now mandatory to call \ref agsDriverExtensionsDX11_CreateDevice when creating a device if the user wants to access any DX11 AMD extensions.
/// The corresponding \ref agsDriverExtensionsDX11_DestroyDevice call must be called to release the device and free up the internal resources allocated by the create call.
/// @{

/// The different modes to control Crossfire behavior.
typedef enum AGSCrossfireMode
{
    AGS_CROSSFIRE_MODE_DRIVER_AFR = 0,                      ///< Use the default driver-based AFR rendering.  If this mode is specified, do NOT use the agsDriverExtensionsDX11_Create*() APIs to create resources
    AGS_CROSSFIRE_MODE_EXPLICIT_AFR,                        ///< Use the AGS Crossfire API functions to perform explicit AFR rendering without requiring a CF driver profile
    AGS_CROSSFIRE_MODE_DISABLE                              ///< Completely disable AFR rendering
} AGSCrossfireMode;

/// The struct to specify the existing DX11 device creation parameters
typedef struct AGSDX11DeviceCreationParams
{
    IDXGIAdapter*               pAdapter;                   ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    D3D_DRIVER_TYPE             DriverType;                 ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    HMODULE                     Software;                   ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    UINT                        Flags;                      ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    const D3D_FEATURE_LEVEL*    pFeatureLevels;             ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    UINT                        FeatureLevels;              ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    UINT                        SDKVersion;                 ///< Consult the DX documentation on D3D11CreateDevice for this parameter
    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc;             ///< Optional swapchain description. Specify this to invoke D3D11CreateDeviceAndSwapChain instead of D3D11CreateDevice.
} AGSDX11DeviceCreationParams;

/// The struct to specify DX11 additional device creation parameters
typedef struct AGSDX11ExtensionParams
{
    const WCHAR*                pAppName;                   ///< Application name
    const WCHAR*                pEngineName;                ///< Engine name
    unsigned int                appVersion;                 ///< Application version
    unsigned int                engineVersion;              ///< Engine version
    unsigned int                numBreadcrumbMarkers;       ///< The number of breadcrumb markers to allocate. Each marker is a uint64 (ie 8 bytes). If 0, the system is disabled.
    unsigned int                uavSlot;                    ///< The UAV slot reserved for intrinsic support. This must match the slot defined in the HLSL, i.e. "#define AmdDxExtShaderIntrinsicsUAVSlot".
                                                            /// The default slot is 7, but the caller is free to use an alternative slot.
                                                            /// If 0 is specified, then the default of 7 will be used.
    AGSCrossfireMode            crossfireMode;              ///< Desired Crossfire mode
} AGSDX11ExtensionParams;

/// The struct to hold all the returned parameters from the device creation call
typedef struct AGSDX11ReturnedParams
{
    ID3D11Device*           pDevice;                            ///< The newly created device
    ID3D11DeviceContext*    pImmediateContext;                  ///< The newly created immediate device context
    IDXGISwapChain*         pSwapChain;                         ///< The newly created swap chain. This is only created if a valid pSwapChainDesc is supplied in AGSDX11DeviceCreationParams.
    D3D_FEATURE_LEVEL       featureLevel;                       ///< The feature level supported by the newly created device
    typedef struct ExtensionsSupported                          /// Extensions for DX11
    {
        unsigned int        quadList : 1;                       ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        screenRectList : 1;                 ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        uavOverlap : 1;                     ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        depthBoundsTest : 1;                ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        multiDrawIndirect : 1;              ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        multiDrawIndirectCountIndirect : 1; ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        crossfireAPI : 1;                   ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        createShaderControls : 1;           ///< Supported in Radeon Software Version 16.9.2 onwards.
        unsigned int        intrinsics16 : 1;                   ///< Supported in Radeon Software Version 16.9.2 onwards. ReadFirstLane, ReadLane, LaneID, Swizzle, Ballot, MBCount, Med3, Barycentrics
        unsigned int        multiView : 1;                      ///< Supported in Radeon Software Version 16.12.1 onwards.
        unsigned int        intrinsics17 : 1;                   ///< Supported in Radeon Software Version 17.9.1 onwards. WaveReduce, WaveScan
        unsigned int        appRegistration : 1;                ///< Supported in Radeon Software Version 17.9.1 onwards.
        unsigned int        breadcrumbMarkers : 1;              ///< Supported in Radeon Software Version 17.11.1 onwards.
        unsigned int        MDIDeferredContexts : 1;            ///< Supported in Radeon Software Version 18.8.1 onwards.
        unsigned int        UAVOverlapDeferredContexts : 1;     ///< Supported in Radeon Software Version 18.8.1 onwards.
        unsigned int        depthBoundsDeferredContexts : 1;    ///< Supported in Radeon Software Version 18.8.1 onwards.
        unsigned int        intrinsics19 : 1;                   ///< Supported in Radeon Software Version 19.12.2 onwards. DrawIndex, AtomicU64
        unsigned int        getWaveSize : 1;                    ///< Supported in Radeon Software Version 20.2.1 onwards.
        unsigned int        baseVertex : 1;                     ///< Supported in Radeon Software Version 20.2.1 onwards.
        unsigned int        baseInstance : 1;                   ///< Supported in Radeon Software Version 20.2.1 onwards.
        unsigned int        padding : 12;                       ///< Reserved
    } ExtensionsSupported;
    ExtensionsSupported     extensionsSupported;                ///< List of supported extensions
    unsigned int            crossfireGPUCount;                  ///< The number of GPUs that are active for this app
    void*                   breadcrumbBuffer;                   ///< The CPU buffer returned if the initialization of the breadcrumb was successful
} AGSDX11ReturnedParams;

///
/// Function used to create a D3D11 device with additional AMD-specific initialization parameters.
///
/// When using the HLSL shader extensions please note:
/// * The shader compiler should not use the D3DCOMPILE_SKIP_OPTIMIZATION (/Od) option, otherwise it will not work.
/// * The shader compiler needs D3DCOMPILE_ENABLE_STRICTNESS (/Ges) enabled.
///
/// \param [in] context                             Pointer to a context. This is generated by \ref agsInitialize
/// \param [in] creationParams                      Pointer to the struct to specify the existing DX11 device creation parameters.
/// \param [in] extensionParams                     Optional pointer to the struct to specify DX11 additional device creation parameters.
/// \param [out] returnedParams                     Pointer to struct to hold all the returned parameters from the call.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateDevice( AGSContext* context, const AGSDX11DeviceCreationParams* creationParams, const AGSDX11ExtensionParams* extensionParams, AGSDX11ReturnedParams* returnedParams );

///
/// Function to destroy the D3D11 device and its immediate context.
/// This call will also cleanup any AMD-specific driver extensions for D3D11.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] device                              Pointer to the D3D11 device.
/// \param [out] deviceReferences                   Optional pointer to an unsigned int that will be set to the value returned from device->Release().
/// \param [in] immediateContext                    Pointer to the D3D11 immediate device context.
/// \param [out] immediateContextReferences         Optional pointer to an unsigned int that will be set to the value returned from immediateContext->Release().
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_DestroyDevice( AGSContext* context, ID3D11Device* device, unsigned int* deviceReferences, ID3D11DeviceContext* immediateContext, unsigned int* immediateContextReferences );

/// @}

/// \defgroup breadcrumbs Breadcrumb API
/// API for writing top-of-pipe and bottom-of-pipe markers to help track down GPU hangs.
///
/// The API is available if the \ref AGSDX11ReturnedParams::ExtensionsSupported::breadcrumbMarkers is present.
///
/// To use the API, a non zero value needs to be specificed in \ref AGSDX11ExtensionParams::numBreadcrumbMarkers.  This enables the API (if available) and allocates a system memory buffer
/// which is returned to the user in \ref AGSDX11ReturnedParams::breadcrumbBuffer.
///
/// The user can now write markers before and after draw calls using \ref agsDriverExtensionsDX11_WriteBreadcrumb.
///
/// \section background Background
///
/// A top-of-pipe (TOP) command is scheduled for execution as soon as the command processor (CP) reaches the command.
/// A bottom-of-pipe (BOP) command is scheduled for execution once the previous rendering commands (draw and dispatch) finish execution.
/// TOP and BOP commands do not block CP. i.e. the CP schedules the command for execution then proceeds to the next command without waiting.
/// To effectively use TOP and BOP commands, it is important to understand how they interact with rendering commands:
///
/// When the CP encounters a rendering command it queues it for execution and moves to the next command.  The queued rendering commands are issued in order.
/// There can be multiple rendering commands running in parallel.  When a rendering command is issued we say it is at the top of the pipe.  When a rendering command
/// finishes execution we say it has reached the bottom of the pipe.
///
/// A BOP command remains in a waiting queue and is executed once prior rendering commands finish.  The queue of BOP commands is limited to 64 entries in GCN generation 1, 2, 3, 4 and 5.
/// If the 64 limit is reached the CP will stop queueing BOP commands and also rendering commands.  Developers should limit the number of BOP commands that write markers to avoid contention.
/// In general, developers should limit both TOP and BOP commands to avoid stalling the CP.
///
/// \subsection eg1 Example 1:
///
/// \code{.cpp}
/// // Start of a command buffer
/// WriteMarker(TopOfPipe, 1)
/// WriteMarker(BottomOfPipe, 2)
/// WriteMarker(BottomOfPipe, 3)
/// DrawX
/// WriteMarker(BottomOfPipe, 4)
/// WriteMarker(BottomOfPipe, 5)
/// WriteMarker(TopOfPipe, 6)
/// // End of command buffer
/// \endcode
///
/// In the above example, the CP writes markers 1, 2 and 3 without waiting:
/// Marker 1 is TOP so it's independent from other commands
/// There's no wait for marker 2 and 3 because there are no draws preceding the BOP commands
/// Marker 4 is only written once DrawX finishes execution
/// Marker 5 doesn't wait for additional draws so it is written right after marker 4
/// Marker 6 can be written as soon as the CP reaches the command. For instance, it is very possible that CP writes marker 6 while DrawX 
/// is running and therefore marker 6 gets written before markers 4 and 5
///
/// \subsection eg2 Example 2:
///
/// \code{.cpp}
/// WriteMarker(TopOfPipe, 1)
/// DrawX
/// WriteMarker(BottomOfPipe, 2)
/// WriteMarker(TopOfPipe, 3)
/// DrawY
/// WriteMarker(BottomOfPipe, 4)
/// \endcode
///
/// In this example marker 1 is written before the start of DrawX
/// Marker 2 is written once DrawX finishes execution
/// Similarly marker 3 is written before the start of DrawY
/// Marker 4 is written once DrawY finishes execution
/// In case of a GPU hang, if markers 1 and 3 are written but markers 2 and 4 are missing we can conclude that:
/// The CP has reached both DrawX and DrawY commands since marker 1 and 3 are present
/// The fact that marker 2 and 4 are missing means that either DrawX is hanging while DrawY is at the top of the pipe or both DrawX and DrawY
/// started and both are simultaneously hanging
///
/// \subsection eg3 Example 3:
///
/// \code{.cpp}
/// // Start of a command buffer
/// WriteMarker(BottomOfPipe, 1)
/// DrawX
/// WriteMarker(BottomOfPipe, 2)
/// DrawY
/// WriteMarker(BottomOfPipe, 3)
/// DrawZ
/// WriteMarker(BottomOfPipe, 4)
/// // End of command buffer
/// \endcode
///
/// In this example marker 1 is written before the start of DrawX
/// Marker 2 is written once DrawX finishes
/// Marker 3 is written once DrawY finishes
/// Marker 4 is written once DrawZ finishes 
/// If the GPU hangs and only marker 1 is written we can conclude that the hang is happening in either DrawX, DrawY or DrawZ
/// If the GPU hangs and only marker 1 and 2 are written we can conclude that the hang is happening in DrawY or DrawZ
/// If the GPU hangs and only marker 4 is missing we can conclude that the hang is happening in DrawZ
///
/// \subsection eg4 Example 4:
///
/// \code{.cpp}
/// Start of a command buffer
/// WriteMarker(TopOfPipe, 1)
/// DrawX
/// WriteMarker(TopOfPipe, 2)
/// DrawY
/// WriteMarker(TopOfPipe, 3)
/// DrawZ
/// // End of command buffer
/// \endcode
///
/// In this example, in case the GPU hangs and only marker 1 is written we can conclude that the hang is happening in DrawX
/// In case the GPU hangs and only marker 1 and 2 are written we can conclude that the hang is happening in DrawX or DrawY
/// In case the GPU hangs and all 3 markers are written we can conclude that the hang is happening in any of DrawX, DrawY or DrawZ
///
/// \subsection eg5 Example 5:
///
/// \code{.cpp}
/// DrawX
/// WriteMarker(TopOfPipe, 1)
/// WriteMarker(BottomOfPipe, 2)
/// DrawY
/// WriteMarker(TopOfPipe, 3)
/// WriteMarker(BottomOfPipe, 4)
/// \endcode
///
/// Marker 1 is written right after DrawX is queued for execution.
/// Marker 2 is only written once DrawX finishes execution.
/// Marker 3 is written right after DrawY is queued for execution.
/// Marker 4 is only written once DrawY finishes execution
/// If marker 1 is written we would know that the CP has reached the command DrawX (DrawX at the top of the pipe).
/// If marker 2 is written we can say that DrawX has finished execution (DrawX at the bottom of the pipe). 
/// In case the GPU hangs and only marker 1 and 3 are written we can conclude that the hang is happening in DrawX or DrawY
/// In case the GPU hangs and only marker 1 is written we can conclude that the hang is happening in DrawX
/// In case the GPU hangs and only marker 4 is missing we can conclude that the hang is happening in DrawY
///
/// \section data Retrieving GPU Data
///
/// In the event of a GPU hang, the user can inspect the system memory buffer to determine which draw has caused the hang.
/// For example:
/// \code{.cpp}
///     // Force the work to be flushed to prevent CPU ahead of GPU
///     g_pImmediateContext->Flush();
///     
///     // Present the information rendered to the back buffer to the front buffer (the screen)
///     HRESULT hr = g_pSwapChain->Present( 0, 0 );
///     
///     // Read the marker data buffer once detect device lost
///     if ( hr != S_OK )
///     {
///         for (UINT i = 0; i < g_NumMarkerWritten; i++)
///         {
///             UINT64* pTempData;
///             pTempData = static_cast<UINT64*>(pMarkerBuffer);
/// 
///             // Write the marker data to file
///             ofs << i << "\r\n";
///             ofs << std::hex << *(pTempData + i * 2) << "\r\n";
///             ofs << std::hex << *(pTempData + (i * 2 + 1)) << "\r\n";
/// 
///             WCHAR s1[256];
///             setlocale(LC_NUMERIC, "en_US.iso88591");
/// 
///             // Output the marker data to console
///             swprintf(s1, 256, L" The Draw count is %d; The Top maker is % 016llX and the Bottom marker is % 016llX \r\n", i, *(pTempData + i * 2), *(pTempData + (i * 2 + 1)));
/// 
///             OutputDebugStringW(s1);
///         }
///     }
/// \endcode
///
/// The console output would resemble something like:
/// \code{.cpp}
/// D3D11: Removing Device. 
/// D3D11 ERROR: ID3D11Device::RemoveDevice: Device removal has been triggered for the following reason (DXGI_ERROR_DEVICE_HUNG: The Device took an unreasonable amount of time to execute its commands, or the hardware crashed/hung. As a result, the TDR (Timeout Detection and Recovery) mechanism has been triggered. The current Device Context was executing commands when the hang occurred. The application may want to respawn and fallback to less aggressive use of the display hardware). [ EXECUTION ERROR #378: DEVICE_REMOVAL_PROCESS_AT_FAULT]
///  The Draw count is 0; The Top maker is 00000000DEADCAFE and the Bottom marker is 00000000DEADBEEF 
///  The Draw count is 1; The Top maker is 00000000DEADCAFE and the Bottom marker is 00000000DEADBEEF 
///  The Draw count is 2; The Top maker is 00000000DEADCAFE and the Bottom marker is 00000000DEADBEEF 
///  The Draw count is 3; The Top maker is 00000000DEADCAFE and the Bottom marker is 00000000DEADBEEF 
///  The Draw count is 4; The Top maker is 00000000DEADCAFE and the Bottom marker is 00000000DEADBEEF 
///  The Draw count is 5; The Top maker is CDCDCDCDCDCDCDCD and the Bottom marker is CDCDCDCDCDCDCDCD 
///  The Draw count is 6; The Top maker is CDCDCDCDCDCDCDCD and the Bottom marker is CDCDCDCDCDCDCDCD 
///  The Draw count is 7; The Top maker is CDCDCDCDCDCDCDCD and the Bottom marker is CDCDCDCDCDCDCDCD 
/// \endcode
/// 
/// @{

/// The breadcrumb marker struct used by \ref agsDriverExtensionsDX11_WriteBreadcrumb
typedef struct AGSBreadcrumbMarker
{
    /// The marker type
    typedef enum Type
    {
        TopOfPipe       = 0,    ///< Top-of-pipe marker
        BottomOfPipe    = 1     ///< Bottom-of-pipe marker
    } Type;

    unsigned long long  markerData; ///< The user data to write.
    Type                type;       ///< Whether this marker is top or bottom of pipe.
    unsigned int        index;      ///< The index of the marker. This should be less than the value specified in \ref AGSDX11ExtensionParams::numBreadcrumbMarkers
} AGSBreadcrumbMarker;

///
/// Function to write a breadcrumb marker.
///
/// This method inserts a write marker operation in the GPU command stream. In the case where the GPU is hanging the write
/// command will never be reached and the marker will never get written to memory.
///
/// In order to use this function, \ref AGSDX11ExtensionParams::numBreadcrumbMarkers must be set to a non zero value.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] marker                              Pointer to a marker.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_WriteBreadcrumb( AGSContext* context, const AGSBreadcrumbMarker* marker );

/// @}

/// \defgroup dx11Topology Extended Topology
/// API for primitive topologies
/// @{

/// Additional topologies supported via extensions
typedef enum AGSPrimitiveTopologyDX11
{
    AGS_PRIMITIVE_TOPOLOGY_QUADLIST                         = 7,    ///< Quad list
    AGS_PRIMITIVE_TOPOLOGY_SCREENRECTLIST                   = 9     ///< Screen rect list
} AGSPrimitiveTopologyDX11;

///
/// Function used to set the primitive topology. If you are using any of the extended topology types, then this function should
/// be called to set ALL topology types.
///
/// The Quad List extension is a convenient way to submit quads without using an index buffer. Note that this still submits two triangles at the driver level. 
/// In order to use this function, AGS must already be initialized and agsDriverExtensionsDX11_Init must have been called successfully.
///
/// The Screen Rect extension, which is only available on GCN hardware, allows the user to pass in three of the four corners of a rectangle. 
/// The hardware then uses the bounding box of the vertices to rasterize the rectangle primitive (i.e. as a rectangle rather than two triangles). 
/// \note Note that this will not return valid interpolated values, only valid SV_Position values.
/// \note If either the Quad List or Screen Rect extension are used, then agsDriverExtensionsDX11_IASetPrimitiveTopology should be called in place of the native DirectX11 equivalent all the time.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] topology                            The topology to set on the D3D11 device. This can be either an AGS-defined topology such as AGS_PRIMITIVE_TOPOLOGY_QUADLIST
///                                                 or a standard D3D-defined topology such as D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP.
///                                                 NB. the AGS-defined types will require casting to a D3D_PRIMITIVE_TOPOLOGY type.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_IASetPrimitiveTopology( AGSContext* context, D3D_PRIMITIVE_TOPOLOGY topology );

/// @}

/// \defgroup dx11UAVOverlap UAV Overlap
/// API for enabling overlapping UAV writes
///
/// The AMD DX11 driver will automatically track resource usage and insert barriers as necessary to clear read-after-write (RAW) and write-after-write (WAW)
/// hazards. The UAV overlap extension allows applications to indicate to the driver it can skip inserting barriers for UAV resources used in
/// dispatches and draws within the \ref agsDriverExtensionsDX11_BeginUAVOverlap/ \ref agsDriverExtensionsDX11_EndUAVOverlap calls. This can be useful for applications to allow
/// multiple back-to-back dispatches or draws in flight even if they are accessing the same UAV resource but the data written or read does not overlap within the resource.
///
/// Usage would be as follows:
/// \code{.cpp}
///     m_device->Dispatch( ... );  // First call that writes to the UAV
///
///     // Disable automatic WAW syncs
///     agsDriverExtensionsDX11_BeginUAVOverlap( m_agsContext );
///
///     // Submit other dispatches that write to the same UAV concurrently
///     m_device->Dispatch( ... );
///     m_device->Dispatch( ... );
///     m_device->Dispatch( ... );
///
///     // Reenable automatic WAW syncs
///     agsDriverExtensionsDX11_EndUAVOverlap( m_agsContext );
/// \endcode
/// @{

///
/// Function used indicate to the driver the start of the overlap scope.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
///                                                 with the AGS_DX11_EXTENSION_DEFERRED_CONTEXTS bit.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_BeginUAVOverlap( AGSContext* context, ID3D11DeviceContext* dxContext );

///
/// Function used indicate to the driver the end of the overlap scope.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
///                                                 with the AGS_DX11_EXTENSION_DEFERRED_CONTEXTS bit.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_EndUAVOverlap( AGSContext* context, ID3D11DeviceContext* dxContext );

/// @}

/// \defgroup dx11DepthBoundsTest Depth Bounds Test
/// API for enabling depth bounds testing
/// @{

///
/// Function used to set the depth bounds test extension
///
/// \param [in] context                             Pointer to a context
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
/// \param [in] enabled                             Whether to enable or disable the depth bounds testing. If disabled, the next two args are ignored.
/// \param [in] minDepth                            The near depth range to clip against.
/// \param [in] maxDepth                            The far depth range to clip against.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_SetDepthBounds( AGSContext* context, ID3D11DeviceContext* dxContext, bool enabled, float minDepth, float maxDepth );

/// @}

/// \defgroup mdi Multi Draw Indirect (MDI)
/// API for dispatching multiple instanced draw commands.
/// The multi draw indirect extensions allow multiple sets of DrawInstancedIndirect to be submitted in one API call.
/// The draw calls are issued on the GPU's command processor (CP), potentially saving the significant CPU overheads incurred by submitting the equivalent draw calls on the CPU.
///
/// The extension allows the following code:
/// \code{.cpp}
///     // Submit n batches of DrawIndirect calls
///     for ( int i = 0; i < n; i++ )
///         deviceContext->DrawIndexedInstancedIndirect( buffer, i * sizeof( cmd ) );
/// \endcode
/// To be replaced by the following call:
/// \code{.cpp}
///     // Submit all n batches in one call
///     agsDriverExtensionsDX11_MultiDrawIndexedInstancedIndirect( m_agsContext, deviceContext, n, buffer, 0, sizeof( cmd ) );
/// \endcode
///
/// The buffer used for the indirect args must be of the following formats:
/// \code{.cpp}
///     // Buffer layout for agsDriverExtensions_MultiDrawInstancedIndirect
///     struct DrawInstancedIndirectArgs
///     {
///         UINT VertexCountPerInstance;
///         UINT InstanceCount;
///         UINT StartVertexLocation;
///         UINT StartInstanceLocation;
///     };
///
///     // Buffer layout for agsDriverExtensions_MultiDrawIndexedInstancedIndirect
///     struct DrawIndexedInstancedIndirectArgs
///     {
///         UINT IndexCountPerInstance;
///         UINT InstanceCount;
///         UINT StartIndexLocation;
///         UINT BaseVertexLocation;
///         UINT StartInstanceLocation;
///     };
/// \endcode
///
/// Example usage can be seen in AMD's GeometryFX (https://github.com/GPUOpen-Effects/GeometryFX).  In particular, in this file: https://github.com/GPUOpen-Effects/GeometryFX/blob/master/amd_geometryfx/src/AMD_GeometryFX_Filtering.cpp
///
/// @{

///
/// Function used to submit a batch of draws via MultiDrawIndirect
///
/// \param [in] context                             Pointer to a context.
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
/// \param [in] drawCount                           The number of draws.
/// \param [in] pBufferForArgs                      The args buffer.
/// \param [in] alignedByteOffsetForArgs            The offset into the args buffer.
/// \param [in] byteStrideForArgs                   The per element stride of the args buffer.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_MultiDrawInstancedIndirect( AGSContext* context, ID3D11DeviceContext* dxContext, unsigned int drawCount, ID3D11Buffer* pBufferForArgs, unsigned int alignedByteOffsetForArgs, unsigned int byteStrideForArgs );

///
/// Function used to submit a batch of draws via MultiDrawIndirect
///
/// \param [in] context                             Pointer to a context.
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
/// \param [in] drawCount                           The number of draws.
/// \param [in] pBufferForArgs                      The args buffer.
/// \param [in] alignedByteOffsetForArgs            The offset into the args buffer.
/// \param [in] byteStrideForArgs                   The per element stride of the args buffer.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_MultiDrawIndexedInstancedIndirect( AGSContext* context, ID3D11DeviceContext* dxContext, unsigned int drawCount, ID3D11Buffer* pBufferForArgs, unsigned int alignedByteOffsetForArgs, unsigned int byteStrideForArgs );

///
/// Function used to submit a batch of draws via MultiDrawIndirect
///
/// \param [in] context                             Pointer to a context.
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
/// \param [in] pBufferForDrawCount                 The draw count buffer.
/// \param [in] alignedByteOffsetForDrawCount       The offset into the draw count buffer.
/// \param [in] pBufferForArgs                      The args buffer.
/// \param [in] alignedByteOffsetForArgs            The offset into the args buffer.
/// \param [in] byteStrideForArgs                   The per element stride of the args buffer.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_MultiDrawInstancedIndirectCountIndirect( AGSContext* context, ID3D11DeviceContext* dxContext, ID3D11Buffer* pBufferForDrawCount, unsigned int alignedByteOffsetForDrawCount, ID3D11Buffer* pBufferForArgs, unsigned int alignedByteOffsetForArgs, unsigned int byteStrideForArgs );

///
/// Function used to submit a batch of draws via MultiDrawIndirect
///
/// \param [in] context                             Pointer to a context.
/// \param [in] dxContext                           Pointer to the DirectX device context.  If this is to work using the non-immediate context, then you need to check support.  If nullptr is specified, then the immediate context is assumed.
/// \param [in] pBufferForDrawCount                 The draw count buffer.
/// \param [in] alignedByteOffsetForDrawCount       The offset into the draw count buffer.
/// \param [in] pBufferForArgs                      The args buffer.
/// \param [in] alignedByteOffsetForArgs            The offset into the args buffer.
/// \param [in] byteStrideForArgs                   The per element stride of the args buffer.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_MultiDrawIndexedInstancedIndirectCountIndirect( AGSContext* context, ID3D11DeviceContext* dxContext, ID3D11Buffer* pBufferForDrawCount, unsigned int alignedByteOffsetForDrawCount, ID3D11Buffer* pBufferForArgs, unsigned int alignedByteOffsetForArgs, unsigned int byteStrideForArgs );

/// @}

/// \defgroup shadercompiler Shader Compiler Controls
/// API for controlling DirectX11 shader compilation.
/// Check support for this feature using the AGS_DX11_EXTENSION_CREATE_SHADER_CONTROLS bit.
/// Supported in Radeon Software Version 16.9.2 (driver version 16.40.2311) onwards.
/// @{

///
/// This method can be used to limit the maximum number of threads the driver uses for asynchronous shader compilation.
/// Setting it to 0 will disable asynchronous compilation completely and force the shaders to be compiled "inline" on the threads that call Create*Shader.
///
/// This method can only be called before any shaders are created and being compiled by the driver.
/// If this method is called after shaders have been created the function will return AGS_FAILURE.
/// This function only sets an upper limit.The driver may create fewer threads than allowed by this function.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] numberOfThreads                     The maximum number of threads to use.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_SetMaxAsyncCompileThreadCount( AGSContext* context, unsigned int numberOfThreads );

///
/// This method can be used to determine the total number of asynchronous shader compile jobs that are either
/// queued for waiting for compilation or being compiled by the driver's asynchronous compilation threads.
/// This method can be called at any during the lifetime of the driver.
///
/// \param [in] context                             Pointer to a context.
/// \param [out] numberOfJobs                       Pointer to the number of jobs in flight currently.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NumPendingAsyncCompileJobs( AGSContext* context, unsigned int* numberOfJobs );

///
/// This method can be used to enable or disable the disk based shader cache.
/// Enabling/disabling the disk cache is not supported if is it disabled explicitly via Radeon Settings or by an app profile.
/// Calling this method under these conditions will result in AGS_FAILURE being returned.
/// It is recommended that this method be called before any shaders are created by the application and being compiled by the driver.
/// Doing so at any other time may result in the cache being left in an inconsistent state.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] enable                              Whether to enable the disk cache. 0 to disable, 1 to enable.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_SetDiskShaderCacheEnabled( AGSContext* context, int enable );

/// @}

/// \defgroup multiview Multiview
/// API for multiview broadcasting.
/// Check support for this feature using the AGS_DX11_EXTENSION_MULTIVIEW bit.
/// Supported in Radeon Software Version 16.12.1 (driver version 16.50.2001) onwards.
/// @{

///
/// Function to control draw calls replication to multiple viewports and RT slices.
/// Setting any mask to 0 disables draw replication.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] vpMask                              Viewport control bit mask.
/// \param [in] rtSliceMask                         RT slice control bit mask.
/// \param [in] vpMaskPerRtSliceEnabled             If 0, 16 lower bits of vpMask apply to all RT slices; if 1 each 16 bits of 64-bit mask apply to corresponding 4 RT slices.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_SetViewBroadcastMasks( AGSContext* context, unsigned long long vpMask, unsigned long long rtSliceMask, int vpMaskPerRtSliceEnabled );

///
/// Function returns max number of supported clip rectangles.
///
/// \param [in] context                             Pointer to a context.
/// \param [out] maxRectCount                       Returned max number of clip rectangles.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_GetMaxClipRects( AGSContext* context, unsigned int* maxRectCount );

/// The clip rectangle struct used by \ref agsDriverExtensionsDX11_SetClipRects
typedef struct AGSClipRect
{
    /// The inclusion mode for the rect
    typedef enum Mode
    {
        ClipRectIncluded = 0,   ///< Include the rect
        ClipRectExcluded = 1    ///< Exclude the rect
    } Mode;

    Mode            mode; ///< Include/exclude rect region
    AGSRect         rect; ///< The rect to include/exclude
} AGSClipRect;

///
/// Function sets clip rectangles.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] clipRectCount                       Number of specified clip rectangles. Use 0 to disable clip rectangles.
/// \param [in] clipRects                           Array of clip rectangles.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_SetClipRects( AGSContext* context, unsigned int clipRectCount, const AGSClipRect* clipRects );

/// @}

/// \defgroup cfxapi Explicit Crossfire API
/// API for explicit control over Crossfire
/// @{

/// The Crossfire API transfer types
typedef enum AGSAfrTransferType
{
    AGS_AFR_TRANSFER_DEFAULT                                = 0,    ///< Default Crossfire driver resource tracking
    AGS_AFR_TRANSFER_DISABLE                                = 1,    ///< Turn off driver resource tracking
    AGS_AFR_TRANSFER_1STEP_P2P                              = 2,    ///< App controlled GPU to next GPU transfer
    AGS_AFR_TRANSFER_2STEP_NO_BROADCAST                     = 3,    ///< App controlled GPU to next GPU transfer using intermediate system memory
    AGS_AFR_TRANSFER_2STEP_WITH_BROADCAST                   = 4,    ///< App controlled GPU to all render GPUs transfer using intermediate system memory
} AGSAfrTransferType;

/// The Crossfire API transfer engines
typedef enum AGSAfrTransferEngine
{
    AGS_AFR_TRANSFERENGINE_DEFAULT                          = 0,    ///< Use default engine for Crossfire API transfers
    AGS_AFR_TRANSFERENGINE_3D_ENGINE                        = 1,    ///< Use 3D engine for Crossfire API transfers
    AGS_AFR_TRANSFERENGINE_COPY_ENGINE                      = 2,    ///< Use Copy engine for Crossfire API transfers
} AGSAfrTransferEngine;

///
/// Function to create a Direct3D11 resource with the specified AFR transfer type and specified transfer engine.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] desc                                Pointer to the D3D11 resource description.
/// \param [in] initialData                         Optional pointer to the initializing data for the resource.
/// \param [out] buffer                             Returned pointer to the resource.
/// \param [in] transferType                        The transfer behavior.
/// \param [in] transferEngine                      The transfer engine to use.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateBuffer( AGSContext* context, const D3D11_BUFFER_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Buffer** buffer, AGSAfrTransferType transferType, AGSAfrTransferEngine transferEngine );

///
/// Function to create a Direct3D11 resource with the specified AFR transfer type and specified transfer engine.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] desc                                Pointer to the D3D11 resource description.
/// \param [in] initialData                         Optional pointer to the initializing data for the resource.
/// \param [out] texture1D                          Returned pointer to the resource.
/// \param [in] transferType                        The transfer behavior.
/// \param [in] transferEngine                      The transfer engine to use.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateTexture1D( AGSContext* context, const D3D11_TEXTURE1D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Texture1D** texture1D, AGSAfrTransferType transferType, AGSAfrTransferEngine transferEngine );

///
/// Function to create a Direct3D11 resource with the specified AFR transfer type and specified transfer engine.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] desc                                Pointer to the D3D11 resource description.
/// \param [in] initialData                         Optional pointer to the initializing data for the resource.
/// \param [out] texture2D                          Returned pointer to the resource.
/// \param [in] transferType                        The transfer behavior.
/// \param [in] transferEngine                      The transfer engine to use.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateTexture2D( AGSContext* context, const D3D11_TEXTURE2D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Texture2D** texture2D, AGSAfrTransferType transferType, AGSAfrTransferEngine transferEngine );

///
/// Function to create a Direct3D11 resource with the specified AFR transfer type and specified transfer engine.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] desc                                Pointer to the D3D11 resource description.
/// \param [in] initialData                         Optional pointer to the initializing data for the resource.
/// \param [out] texture3D                          Returned pointer to the resource.
/// \param [in] transferType                        The transfer behavior.
/// \param [in] transferEngine                      The transfer engine to use.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateTexture3D( AGSContext* context, const D3D11_TEXTURE3D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Texture3D** texture3D, AGSAfrTransferType transferType, AGSAfrTransferEngine transferEngine );

///
/// Function to notify the driver that we have finished writing to the resource this frame.
/// This will initiate a transfer for AGS_AFR_TRANSFER_1STEP_P2P,
/// AGS_AFR_TRANSFER_2STEP_NO_BROADCAST, and AGS_AFR_TRANSFER_2STEP_WITH_BROADCAST.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] resource                            Pointer to the resource.
/// \param [in] transferRegions                     An array of transfer regions (can be null to specify the whole area).
/// \param [in] subresourceArray                    An array of subresource indices (can be null to specify all subresources).
/// \param [in] numSubresources                     The number of subresources in subresourceArray OR number of transferRegions. Use 0 to specify ALL subresources and one transferRegion (which may be null if specifying the whole area).
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NotifyResourceEndWrites( AGSContext* context, ID3D11Resource* resource, const D3D11_RECT* transferRegions, const unsigned int* subresourceArray, unsigned int numSubresources );

///
/// This will notify the driver that the app will begin read/write access to the resource.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] resource                            Pointer to the resource.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NotifyResourceBeginAllAccess( AGSContext* context, ID3D11Resource* resource );

///
/// This is used for AGS_AFR_TRANSFER_1STEP_P2P to notify when it is safe to initiate a transfer.
/// This call in frame N-(NumGpus-1) allows a 1 step P2P in frame N to start.
/// This should be called after agsDriverExtensionsDX11_NotifyResourceEndWrites.
///
/// \param [in] context                             Pointer to a context.
/// \param [in] resource                            Pointer to the resource.
///
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NotifyResourceEndAllAccess( AGSContext* context, ID3D11Resource* resource );

/// @}

/// @}

/// \defgroup typedefs Function pointer typedefs
/// List of function pointer typedefs for the API
/// @{

typedef AMD_AGS_API AGSDriverVersionResult  (*AGS_CHECKDRIVERVERSION)( const char*, unsigned int ); ///< \ref agsCheckDriverVersion
typedef AMD_AGS_API int                     (*AGS_GETVERSIONNUMBER)(); ///< \ref agsGetVersionNumber
typedef AMD_AGS_API AGSReturnCode           (*AGS_INITIALIZE)( int, const AGSConfiguration*, AGSContext**, AGSGPUInfo* ); ///< \ref agsInitialize
typedef AMD_AGS_API AGSReturnCode           (*AGS_DEINITIALIZE)( AGSContext* ); ///< \ref agsDeInitialize
typedef AMD_AGS_API AGSReturnCode           (*AGS_SETDISPLAYMODE)( AGSContext*, int, int, const AGSDisplaySettings* ); ///< \ref agsSetDisplayMode
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX12_CREATEDEVICE)( AGSContext*, const AGSDX12DeviceCreationParams*, const AGSDX12ExtensionParams*, AGSDX12ReturnedParams* ); ///< \ref agsDriverExtensionsDX12_CreateDevice
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX12_DESTROYDEVICE)( AGSContext*, ID3D12Device*, unsigned int* ); ///< \ref agsDriverExtensionsDX12_DestroyDevice
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX12_PUSHMARKER)( AGSContext*, ID3D12GraphicsCommandList*, const char* ); ///< \ref agsDriverExtensionsDX12_PushMarker
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX12_POPMARKER)( AGSContext*, ID3D12GraphicsCommandList* ); ///< \ref agsDriverExtensionsDX12_PopMarker
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX12_SETMARKER)( AGSContext*, ID3D12GraphicsCommandList*, const char* ); ///< \ref agsDriverExtensionsDX12_SetMarker
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_CREATEDEVICE)( AGSContext*, const AGSDX11DeviceCreationParams*, const AGSDX11ExtensionParams*, AGSDX11ReturnedParams* ); ///< \ref agsDriverExtensionsDX11_CreateDevice
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_DESTROYDEVICE)( AGSContext*, ID3D11Device*, unsigned int*, ID3D11DeviceContext*, unsigned int* ); ///< \ref agsDriverExtensionsDX11_DestroyDevice
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_WRITEBREADCRUMB)( AGSContext*, const AGSBreadcrumbMarker* ); ///< \ref agsDriverExtensionsDX11_WriteBreadcrumb
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_IASETPRIMITIVETOPOLOGY)( AGSContext*, enum D3D_PRIMITIVE_TOPOLOGY ); ///< \ref agsDriverExtensionsDX11_IASetPrimitiveTopology
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_BEGINUAVOVERLAP)( AGSContext*, ID3D11DeviceContext* ); ///< \ref agsDriverExtensionsDX11_BeginUAVOverlap
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_ENDUAVOVERLAP)( AGSContext*, ID3D11DeviceContext* ); ///< \ref agsDriverExtensionsDX11_EndUAVOverlap
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_SETDEPTHBOUNDS)( AGSContext*, ID3D11DeviceContext*, bool, float, float ); ///< \ref agsDriverExtensionsDX11_SetDepthBounds
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_MULTIDRAWINSTANCEDINDIRECT)( AGSContext*, ID3D11DeviceContext*, unsigned int, ID3D11Buffer*, unsigned int, unsigned int ); ///< \ref agsDriverExtensionsDX11_MultiDrawInstancedIndirect
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_MULTIDRAWINDEXEDINSTANCEDINDIRECT)( AGSContext*, ID3D11DeviceContext*, unsigned int, ID3D11Buffer*, unsigned int, unsigned int ); ///< \ref agsDriverExtensionsDX11_MultiDrawIndexedInstancedIndirect
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_MULTIDRAWINSTANCEDINDIRECTCOUNTINDIRECT)( AGSContext*, ID3D11DeviceContext*, ID3D11Buffer*, unsigned int, ID3D11Buffer*, unsigned int, unsigned int ); ///< \ref agsDriverExtensionsDX11_MultiDrawInstancedIndirectCountIndirect
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_MULTIDRAWINDEXEDINSTANCEDINDIRECTCOUNTINDIRECT)( AGSContext*, ID3D11DeviceContext*, ID3D11Buffer*, unsigned int, ID3D11Buffer*, unsigned int, unsigned int ); ///< \ref agsDriverExtensionsDX11_MultiDrawIndexedInstancedIndirectCountIndirect
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_SETMAXASYNCCOMPILETHREADCOUNT)( AGSContext*, unsigned int ); ///< \ref agsDriverExtensionsDX11_SetMaxAsyncCompileThreadCount
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_NUMPENDINGASYNCOMPILEJOBS)( AGSContext*, unsigned int* ); ///< \ref agsDriverExtensionsDX11_NumPendingAsyncCompileJobs
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_SETDISKSHADERCACHEENABLED)( AGSContext*, int ); ///< \ref agsDriverExtensionsDX11_SetDiskShaderCacheEnabled
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_SETVIEWBROADCASTMASKS)( AGSContext*, unsigned long long, unsigned long long, int ); ///< \ref agsDriverExtensionsDX11_SetViewBroadcastMasks
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_GETMAXCLIPRECTS)( AGSContext*, unsigned int* ); ///< \ref agsDriverExtensionsDX11_GetMaxClipRects
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_SETCLIPRECTS)( AGSContext*, unsigned int, const AGSClipRect* ); ///< \ref agsDriverExtensionsDX11_SetClipRects
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_CREATEBUFFER)( AGSContext*, const D3D11_BUFFER_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Buffer**, AGSAfrTransferType, AGSAfrTransferEngine ); ///< \ref agsDriverExtensionsDX11_CreateBuffer
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_CREATETEXTURE1D)( AGSContext*, const D3D11_TEXTURE1D_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Texture1D**, AGSAfrTransferType, AGSAfrTransferEngine ); ///< \ref agsDriverExtensionsDX11_CreateTexture1D
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_CREATETEXTURE2D)( AGSContext*, const D3D11_TEXTURE2D_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Texture2D**, AGSAfrTransferType, AGSAfrTransferEngine ); ///< \ref agsDriverExtensionsDX11_CreateTexture2D
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_CREATETEXTURE3D)( AGSContext*, const D3D11_TEXTURE3D_DESC*, const D3D11_SUBRESOURCE_DATA*, ID3D11Texture3D**, AGSAfrTransferType, AGSAfrTransferEngine ); ///< \ref agsDriverExtensionsDX11_CreateTexture3D
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_NOTIFYRESOURCEENDWRITES)( AGSContext*, ID3D11Resource*, const D3D11_RECT*, const unsigned int*, unsigned int ); ///< \ref agsDriverExtensionsDX11_NotifyResourceEndWrites
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_NOTIFYRESOURCEBEGINALLACCESS)( AGSContext*, ID3D11Resource* ); ///< \ref agsDriverExtensionsDX11_NotifyResourceBeginAllAccess
typedef AMD_AGS_API AGSReturnCode           (*AGS_DRIVEREXTENSIONSDX11_NOTIFYRESOURCEENDALLACCESS)( AGSContext*, ID3D11Resource* ); ///< \ref agsDriverExtensionsDX11_NotifyResourceEndAllAccess
/// @}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // AMD_AGS_H
