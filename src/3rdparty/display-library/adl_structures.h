//
// Copyright (c) 2016 - 2022 Advanced Micro Devices, Inc. All rights reserved.
//
// MIT LICENSE:
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// \file adl_structures.h
///\brief This file contains the structure declarations that are used by the public ADL interfaces for \ALL platforms.\n <b>Included in ADL SDK</b>
///
/// All data structures used in AMD Display Library (ADL) public interfaces should be defined in this header file.
///

#ifndef ADL_STRUCTURES_H_
#define ADL_STRUCTURES_H_

#include "adl_defines.h"
#include <stdbool.h>
/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the graphics adapter.
///
/// This structure is used to store various information about the graphics adapter.  This
/// information can be returned to the user. Alternatively, it can be used to access various driver calls to set
/// or fetch various settings upon the user's request.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct AdapterInfo
{
/// \ALL_STRUCT_MEM

/// Size of the structure.
    int iSize;
/// The ADL index handle. One GPU may be associated with one or two index handles
    int iAdapterIndex;
/// The unique device ID associated with this adapter.
    char strUDID[ADL_MAX_PATH];
/// The BUS number associated with this adapter.
    int iBusNumber;
/// The driver number associated with this adapter.
    int iDeviceNumber;
/// The function number.
    int iFunctionNumber;
/// The vendor ID associated with this adapter.
    int iVendorID;
/// Adapter name.
    char strAdapterName[ADL_MAX_PATH];
/// Display name. For example, "\\\\Display0" for Windows or ":0:0" for Linux.
    char strDisplayName[ADL_MAX_PATH];
/// Present or not; 1 if present and 0 if not present.It the logical adapter is present, the display name such as \\\\.\\Display1 can be found from OS
    int iPresent;

#if defined (_WIN32) || defined (_WIN64)
/// \WIN_STRUCT_MEM

/// Exist or not; 1 is exist and 0 is not present.
    int iExist;
/// Driver registry path.
    char strDriverPath[ADL_MAX_PATH];
/// Driver registry path Ext for.
    char strDriverPathExt[ADL_MAX_PATH];
/// PNP string from Windows.
    char strPNPString[ADL_MAX_PATH];
/// It is generated from EnumDisplayDevices.
    int iOSDisplayIndex;

#endif /* (_WIN32) || (_WIN64) */

#if defined (LINUX)
/// \LNX_STRUCT_MEM

/// Internal X screen number from GPUMapInfo (DEPRICATED use XScreenInfo)
    int iXScreenNum;
/// Internal driver index from GPUMapInfo
    int iDrvIndex;
/// \deprecated Internal x config file screen identifier name. Use XScreenInfo instead.
    char strXScreenConfigName[ADL_MAX_PATH];

#endif /* (LINUX) */
} AdapterInfo, *LPAdapterInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Linux X screen information.
///
/// This structure is used to store the current screen number and xorg.conf ID name assoicated with an adapter index.
/// This structure is updated during ADL_Main_Control_Refresh or ADL_ScreenInfo_Update.
/// Note:  This structure should be used in place of iXScreenNum and strXScreenConfigName in AdapterInfo as they will be
/// deprecated.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
#if defined (LINUX)
typedef struct XScreenInfo
{
/// Internal X screen number from GPUMapInfo.
    int iXScreenNum;
/// Internal x config file screen identifier name.
    char strXScreenConfigName[ADL_MAX_PATH];
} XScreenInfo, *LPXScreenInfo;
#endif /* (LINUX) */

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an controller mode
///
/// This structure is used to store information of an controller mode
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterCaps
{
	/// AdapterID for this adapter
	int iAdapterID;
	/// Number of controllers for this adapter
	int iNumControllers;
	/// Number of displays for this adapter
	int iNumDisplays;
	/// Number of overlays for this adapter
	int iNumOverlays;
	/// Number of GLSyncConnectors
	int iNumOfGLSyncConnectors;
	/// The bit mask identifies the adapter caps
	int iCapsMask;
	/// The bit identifies the adapter caps \ref define_adapter_caps
	int iCapsValue;
}ADLAdapterCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing additional information about the ASIC memory
///
/// This structure is used to store additional information about the ASIC memory.  This
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryInfo2
{
	/// Memory size in bytes.
	long long iMemorySize;
	/// Memory type in string.
	char strMemoryType[ADL_MAX_PATH];
	/// Highest default performance level Memory bandwidth in Mbytes/s
	long long iMemoryBandwidth;
	/// HyperMemory size in bytes.
	long long iHyperMemorySize;

	/// Invisible Memory size in bytes.
	long long iInvisibleMemorySize;
	/// Visible Memory size in bytes.
	long long iVisibleMemorySize;
} ADLMemoryInfo2, *LPADLMemoryInfo2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing additional information about the ASIC memory
///
/// This structure is used to store additional information about the ASIC memory.  This
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryInfo3
{
    /// Memory size in bytes.
    long long iMemorySize;
    /// Memory type in string.
    char strMemoryType[ADL_MAX_PATH];
    /// Highest default performance level Memory bandwidth in Mbytes/s
    long long iMemoryBandwidth;
    /// HyperMemory size in bytes.
    long long iHyperMemorySize;

    /// Invisible Memory size in bytes.
    long long iInvisibleMemorySize;
    /// Visible Memory size in bytes.
    long long iVisibleMemorySize;
    /// Vram vendor ID
    long long iVramVendorRevId;
} ADLMemoryInfo3, *LPADLMemoryInfo3;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing additional information about the ASIC memory
///
/// This structure is used to store additional information about the ASIC memory.  This
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryInfoX4
{
    /// Memory size in bytes.
    long long iMemorySize;
    /// Memory type in string.
    char strMemoryType[ADL_MAX_PATH];
    /// Highest default performance level Memory bandwidth in Mbytes/s
    long long iMemoryBandwidth;
    /// HyperMemory size in bytes.
    long long iHyperMemorySize;

    /// Invisible Memory size in bytes.
    long long iInvisibleMemorySize;
    /// Visible Memory size in bytes.
    long long iVisibleMemorySize;
    /// Vram vendor ID
    long long iVramVendorRevId;
    /// Memory Bandiwidth that is calculated and finalized on the driver side, grab and go.
    long long iMemoryBandwidthX2;
    /// Memory Bit Rate that is calculated and finalized on the driver side, grab and go.
    long long iMemoryBitRateX2;

} ADLMemoryInfoX4, *LPADLMemoryInfoX4;

///////////////////////////////////////////////////////////////////////////
// ADLvRamVendor Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLvRamVendors
{
    ADLvRamVendor_Unsupported = 0x0,
    ADLvRamVendor_SAMSUNG,
    ADLvRamVendor_INFINEON,
    ADLvRamVendor_ELPIDA,
    ADLvRamVendor_ETRON,
    ADLvRamVendor_NANYA,
    ADLvRamVendor_HYNIX,
    ADLvRamVendor_MOSEL,
    ADLvRamVendor_WINBOND,
    ADLvRamVendor_ESMT,
    ADLvRamVendor_MICRON = 0xF,
    ADLvRamVendor_Undefined
};

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about components of ASIC GCN architecture
///
///  Elements of GCN info are compute units, number of Tex (Texture filtering units)  , number of ROPs (render back-ends).
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLGcnInfo
{
	int CuCount; //Number of compute units on the ASIC.
	int TexCount; //Number of texture mapping units.
	int RopCount; //Number of Render backend Units.
	int ASICFamilyId; //Such SI, VI. See /inc/asic_reg/atiid.h for family ids
	int ASICRevisionId; //Such as Ellesmere, Fiji.   For example - VI family revision ids are stored in /inc/asic_reg/vi_id.h
}ADLGcnInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related virtual segment config information.
///
/// This structure is used to store information related virtual segment config
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLVirtualSegmentSettingsOutput
{
	int                      virtualSegmentSupported;   // 1 - subsequent values are valid
	int                      virtualSegmentDefault;     //virtual segment default, 1: enable, 0: disable
	int                      virtualSegmentCurrent;     //virtual segment current, 1: enable, 0: disable
	int                      iMinSizeInMB;              //minimum value
	int                      iMaxSizeInMB;              //maximum value
	int                      icurrentSizeInMB;          //last configured otherwise same as factory default
	int                      idefaultSizeInMB;          //factory default
	int                      iMask;                     //fileds for extension in the future
	int                      iValue;                    //fileds for extension in the future
} ADLVirtualSegmentSettingsOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about the Chipset.
///
/// This structure is used to store various information about the Chipset.  This
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLChipSetInfo
{
	int iBusType; 		///< Bus type.
	int iBusSpeedType;	///Maximum Bus Speed of the current platform
	int iMaxPCIELaneWidth; 	///< Number of PCIE lanes.
	int iCurrentPCIELaneWidth;  ///< Current PCIE Lane Width
	int iSupportedAGPSpeeds;    ///< Bit mask or AGP transfer speed.
	int iCurrentAGPSpeed;       ///< Current AGP speed
} ADLChipSetInfo, *LPADLChipSetInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the ASIC memory.
///
/// This structure is used to store various information about the ASIC memory.  This
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryInfo
{
/// Memory size in bytes.
    long long iMemorySize;
/// Memory type in string.
    char strMemoryType[ADL_MAX_PATH];
/// Memory bandwidth in Mbytes/s.
    long long iMemoryBandwidth;
} ADLMemoryInfo, *LPADLMemoryInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about memory required by type
///
/// This structure is returned by ADL_Adapter_ConfigMemory_Get, which given a desktop and display configuration
/// will return the Memory used.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryRequired
{
    long long iMemoryReq;        /// Memory in bytes required
    int iType;                    /// Type of Memory \ref define_adl_validmemoryrequiredfields
    int iDisplayFeatureValue;   /// Display features \ref define_adl_visiblememoryfeatures that are using this type of memory
} ADLMemoryRequired, *LPADLMemoryRequired;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the features associated with a display
///
/// This structure is a parameter to ADL_Adapter_ConfigMemory_Get, which given a desktop and display configuration
/// will return the Memory used.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMemoryDisplayFeatures
{
    int iDisplayIndex;            /// ADL Display index
    int iDisplayFeatureValue;    /// features that the display is using \ref define_adl_visiblememoryfeatures
} ADLMemoryDisplayFeatures, *LPADLMemoryDisplayFeatures;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing DDC information.
///
/// This structure is used to store various DDC information that can be returned to the user.
/// Note that all fields of type int are actually defined as unsigned int types within the driver.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDDCInfo
{
/// Size of the structure
    int  ulSize;
/// Indicates whether the attached display supports DDC. If this field is zero on return, no other DDC information fields will be used.
    int  ulSupportsDDC;
/// Returns the manufacturer ID of the display device. Should be zeroed if this information is not available.
    int  ulManufacturerID;
/// Returns the product ID of the display device. Should be zeroed if this information is not available.
    int  ulProductID;
/// Returns the name of the display device. Should be zeroed if this information is not available.
    char cDisplayName[ADL_MAX_DISPLAY_NAME];
/// Returns the maximum Horizontal supported resolution. Should be zeroed if this information is not available.
    int  ulMaxHResolution;
/// Returns the maximum Vertical supported resolution. Should be zeroed if this information is not available.
    int  ulMaxVResolution;
/// Returns the maximum supported refresh rate. Should be zeroed if this information is not available.
    int  ulMaxRefresh;
/// Returns the display device preferred timing mode's horizontal resolution.
    int  ulPTMCx;
/// Returns the display device preferred timing mode's vertical resolution.
    int  ulPTMCy;
/// Returns the display device preferred timing mode's refresh rate.
    int  ulPTMRefreshRate;
/// Return EDID flags.
    int  ulDDCInfoFlag;
} ADLDDCInfo, *LPADLDDCInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing DDC information.
///
/// This structure is used to store various DDC information that can be returned to the user.
/// Note that all fields of type int are actually defined as unsigned int types within the driver.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDDCInfo2
{
/// Size of the structure
    int  ulSize;
/// Indicates whether the attached display supports DDC. If this field is zero on return, no other DDC
/// information fields will be used.
    int  ulSupportsDDC;
/// Returns the manufacturer ID of the display device. Should be zeroed if this information is not available.
    int  ulManufacturerID;
/// Returns the product ID of the display device. Should be zeroed if this information is not available.
    int  ulProductID;
/// Returns the name of the display device. Should be zeroed if this information is not available.
    char cDisplayName[ADL_MAX_DISPLAY_NAME];
/// Returns the maximum Horizontal supported resolution. Should be zeroed if this information is not available.
    int  ulMaxHResolution;
/// Returns the maximum Vertical supported resolution. Should be zeroed if this information is not available.
    int  ulMaxVResolution;
/// Returns the maximum supported refresh rate. Should be zeroed if this information is not available.
    int  ulMaxRefresh;
/// Returns the display device preferred timing mode's horizontal resolution.
    int  ulPTMCx;
/// Returns the display device preferred timing mode's vertical resolution.
    int  ulPTMCy;
/// Returns the display device preferred timing mode's refresh rate.
    int  ulPTMRefreshRate;
/// Return EDID flags.
    int  ulDDCInfoFlag;
/// Returns 1 if the display supported packed pixel, 0 otherwise
    int bPackedPixelSupported;
/// Returns the Pixel formats the display supports \ref define_ddcinfo_pixelformats
    int iPanelPixelFormat;
/// Return EDID serial ID.
    int  ulSerialID;
/// Return minimum monitor luminance data
    int ulMinLuminanceData;
/// Return average monitor luminance data
    int ulAvgLuminanceData;
/// Return maximum monitor luminance data
    int ulMaxLuminanceData;

/// Bit vector of supported transfer functions \ref define_source_content_TF
    int iSupportedTransferFunction;

/// Bit vector of supported color spaces \ref define_source_content_CS
    int iSupportedColorSpace;

/// Display Red Chromaticity X coordinate multiplied by 10000
    int iNativeDisplayChromaticityRedX;
/// Display Red Chromaticity Y coordinate multiplied by 10000
    int iNativeDisplayChromaticityRedY;
/// Display Green Chromaticity X coordinate multiplied by 10000
    int iNativeDisplayChromaticityGreenX;
/// Display Green Chromaticity Y coordinate multiplied by 10000
    int iNativeDisplayChromaticityGreenY;
/// Display Blue Chromaticity X coordinate multiplied by 10000
    int iNativeDisplayChromaticityBlueX;
/// Display Blue Chromaticity Y coordinate multiplied by 10000
    int iNativeDisplayChromaticityBlueY;
/// Display White Point X coordinate multiplied by 10000
    int iNativeDisplayChromaticityWhitePointX;
/// Display White Point Y coordinate multiplied by 10000
    int iNativeDisplayChromaticityWhitePointY;
/// Display diffuse screen reflectance 0-1 (100%) in units of 0.01
    int iDiffuseScreenReflectance;
/// Display specular screen reflectance 0-1 (100%) in units of 0.01
    int iSpecularScreenReflectance;
/// Bit vector of supported color spaces \ref define_HDR_support
    int iSupportedHDR;
/// Bit vector for freesync flags
    int iFreesyncFlags;

/// Return minimum monitor luminance without dimming data
    int ulMinLuminanceNoDimmingData;

    int ulMaxBacklightMaxLuminanceData;
    int ulMinBacklightMaxLuminanceData;
    int ulMaxBacklightMinLuminanceData;
    int ulMinBacklightMinLuminanceData;

    // Reserved for future use
    int iReserved[4];
} ADLDDCInfo2, *LPADLDDCInfo2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information controller Gamma settings.
///
/// This structure is used to store the red, green and blue color channel information for the.
/// controller gamma setting. This information is returned by ADL, and it can also be used to
/// set the controller gamma setting.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGamma
{
/// Red color channel gamma value.
    float fRed;
/// Green color channel gamma value.
    float fGreen;
/// Blue color channel gamma value.
    float fBlue;
} ADLGamma, *LPADLGamma;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about component video custom modes.
///
/// This structure is used to store the component video custom mode.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLCustomMode
{
/// Custom mode flags.  They are returned by the ADL driver.
    int iFlags;
/// Custom mode width.
    int iModeWidth;
/// Custom mode height.
    int iModeHeight;
/// Custom mode base width.
    int iBaseModeWidth;
/// Custom mode base height.
    int iBaseModeHeight;
/// Custom mode refresh rate.
    int iRefreshRate;
} ADLCustomMode, *LPADLCustomMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing Clock information for OD5 calls.
///
/// This structure is used to retrieve clock information for OD5 calls.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGetClocksOUT
{
    long ulHighCoreClock;
    long ulHighMemoryClock;
    long ulHighVddc;
    long ulCoreMin;
    long ulCoreMax;
    long ulMemoryMin;
    long ulMemoryMax;
    long ulActivityPercent;
    long ulCurrentCoreClock;
    long ulCurrentMemoryClock;
    long ulReserved;
} ADLGetClocksOUT;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing HDTV information for display calls.
///
/// This structure is used to retrieve HDTV information information for display calls.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayConfig
{
/// Size of the structure
  long ulSize;
/// HDTV connector type.
  long ulConnectorType;
/// HDTV capabilities.
  long ulDeviceData;
/// Overridden HDTV capabilities.
  long ulOverridedDeviceData;
/// Reserved field
  long ulReserved;
} ADLDisplayConfig;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display device.
///
/// This structure is used to store display device information
/// such as display index, type, name, connection status, mapped adapter and controller indexes,
/// whether or not multiple VPUs are supported, local display connections or not (through Lasso), etc.
/// This information can be returned to the user. Alternatively, it can be used to access various driver calls to set
/// or fetch various display device related settings upon the user's request.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayID
{
/// The logical display index belonging to this adapter.
    int iDisplayLogicalIndex;

///\brief The physical display index.
/// For example, display index 2 from adapter 2 can be used by current adapter 1.\n
/// So current adapter may enumerate this adapter as logical display 7 but the physical display
/// index is still 2.
    int iDisplayPhysicalIndex;

/// The persistent logical adapter index for the display.
    int iDisplayLogicalAdapterIndex;

///\brief The persistent physical adapter index for the display.
/// It can be the current adapter or a non-local adapter. \n
/// If this adapter index is different than the current adapter,
/// the Display Non Local flag is set inside DisplayInfoValue.
    int iDisplayPhysicalAdapterIndex;
} ADLDisplayID, *LPADLDisplayID;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display device.
///
/// This structure is used to store various information about the display device.  This
/// information can be returned to the user, or used to access various driver calls to set
/// or fetch various display-device-related settings upon the user's request
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayInfo
{
/// The DisplayID structure
    ADLDisplayID displayID;

///\deprecated The controller index to which the display is mapped.\n Will not be used in the future\n
    int  iDisplayControllerIndex;

/// The display's EDID name.
    char strDisplayName[ADL_MAX_PATH];

/// The display's manufacturer name.
    char strDisplayManufacturerName[ADL_MAX_PATH];

/// The Display type. For example: CRT, TV, CV, DFP.
    int  iDisplayType;

/// The display output type. For example: HDMI, SVIDEO, COMPONMNET VIDEO.
    int  iDisplayOutputType;

/// The connector type for the device.
    int  iDisplayConnector;

///\brief The bit mask identifies the number of bits ADLDisplayInfo is currently using. \n
/// It will be the sum all the bit definitions in ADL_DISPLAY_DISPLAYINFO_xxx.
    int  iDisplayInfoMask;

/// The bit mask identifies the display status. \ref define_displayinfomask
    int  iDisplayInfoValue;
} ADLDisplayInfo, *LPADLDisplayInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display port MST device.
///
/// This structure is used to store various MST information about the display port device.  This
/// information can be returned to the user, or used to access various driver calls to
/// fetch various display-device-related settings upon the user's request
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayDPMSTInfo
{
    /// The ADLDisplayID structure
    ADLDisplayID displayID;

    /// total bandwidth available on the DP connector
    int    iTotalAvailableBandwidthInMpbs;
    /// bandwidth allocated to this display
    int    iAllocatedBandwidthInMbps;

    // info from DAL DpMstSinkInfo
    /// string identifier for the display
    char    strGlobalUniqueIdentifier[ADL_MAX_PATH];

    /// The link count of relative address, rad[0] upto rad[linkCount] are valid
    int        radLinkCount;
    /// The physical connector ID, used to identify the physical DP port
    int        iPhysicalConnectorID;

    /// Relative address, address scheme starts from source side
    char    rad[ADL_MAX_RAD_LINK_COUNT];
} ADLDisplayDPMSTInfo, *LPADLDisplayDPMSTInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the display mode definition used per controller.
///
/// This structure is used to store the display mode definition used per controller.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayMode
{
/// Vertical resolution (in pixels).
   int  iPelsHeight;
/// Horizontal resolution (in pixels).
   int  iPelsWidth;
/// Color depth.
   int  iBitsPerPel;
/// Refresh rate.
   int  iDisplayFrequency;
} ADLDisplayMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing detailed timing parameters.
///
/// This structure is used to store the detailed timing parameters.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDetailedTiming
{
/// Size of the structure.
     int   iSize;
/// Timing flags. \ref define_detailed_timing_flags
     short sTimingFlags;
/// Total width (columns).
     short sHTotal;
/// Displayed width.
     short sHDisplay;
/// Horizontal sync signal offset.
     short sHSyncStart;
/// Horizontal sync signal width.
     short sHSyncWidth;
/// Total height (rows).
     short sVTotal;
/// Displayed height.
     short sVDisplay;
/// Vertical sync signal offset.
     short sVSyncStart;
/// Vertical sync signal width.
     short sVSyncWidth;
/// Pixel clock value.
     short sPixelClock;
/// Overscan right.
     short sHOverscanRight;
/// Overscan left.
     short sHOverscanLeft;
/// Overscan bottom.
     short sVOverscanBottom;
/// Overscan top.
     short sVOverscanTop;
     short sOverscan8B;
     short sOverscanGR;
} ADLDetailedTiming;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing display mode information.
///
/// This structure is used to store the display mode information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayModeInfo
{
/// Timing standard of the current mode. \ref define_modetiming_standard
  int  iTimingStandard;
/// Applicable timing standards for the current mode.
  int  iPossibleStandard;
/// Refresh rate factor.
  int  iRefreshRate;
/// Num of pixels in a row.
  int  iPelsWidth;
/// Num of pixels in a column.
  int  iPelsHeight;
/// Detailed timing parameters.
  ADLDetailedTiming  sDetailedTiming;
} ADLDisplayModeInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about display property.
///
/// This structure is used to store the display property for the current adapter.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayProperty
{
/// Must be set to sizeof the structure
  int iSize;
/// Must be set to \ref ADL_DL_DISPLAYPROPERTY_TYPE_EXPANSIONMODE or \ref ADL_DL_DISPLAYPROPERTY_TYPE_USEUNDERSCANSCALING
  int iPropertyType;
/// Get or Set \ref ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_CENTER or \ref ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_FULLSCREEN or \ref ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_ASPECTRATIO or \ref ADL_DL_DISPLAYPROPERTY_TYPE_ITCFLAGENABLE
  int iExpansionMode;
/// Display Property supported? 1: Supported, 0: Not supported
  int iSupport;
/// Display Property current value
  int iCurrent;
/// Display Property Default value
  int iDefault;
} ADLDisplayProperty;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Clock.
///
/// This structure is used to store the clock information for the current adapter
/// such as core clock and memory clock info.
///\nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLClockInfo
{
/// Core clock in 10 KHz.
    int iCoreClock;
/// Memory clock in 10 KHz.
    int iMemoryClock;
} ADLClockInfo, *LPADLClockInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about I2C.
///
/// This structure is used to store the I2C information for the current adapter.
/// This structure is used by the ADL_Display_WriteAndReadI2C() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLI2C
{
/// Size of the structure
    int iSize;
/// Numerical value representing hardware I2C.
    int iLine;
/// The 7-bit I2C slave device address, shifted one bit to the left.
    int iAddress;
/// The offset of the data from the address.
    int iOffset;
/// Read from or write to slave device. \ref ADL_DL_I2C_ACTIONREAD or \ref ADL_DL_I2C_ACTIONWRITE or \ref ADL_DL_I2C_ACTIONREAD_REPEATEDSTART
    int iAction;
/// I2C clock speed in KHz.
    int iSpeed;
/// A numerical value representing the number of bytes to be sent or received on the I2C bus.
    int iDataSize;
/// Address of the characters which are to be sent or received on the I2C bus.
    char *pcData;
} ADLI2C;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about EDID data.
///
/// This structure is used to store the information about EDID data for the adapter.
/// This structure is used by the ADL_Display_EdidData_Get() and ADL_Display_EdidData_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayEDIDData
{
/// Size of the structure
  int iSize;
/// Set to 0
  int iFlag;
  /// Size of cEDIDData. Set by ADL_Display_EdidData_Get() upon return
  int iEDIDSize;
/// 0, 1 or 2. If set to 3 or above an error ADL_ERR_INVALID_PARAM is generated
  int iBlockIndex;
/// EDID data
  char cEDIDData[ADL_MAX_EDIDDATA_SIZE];
/// Reserved
  int iReserved[4];
}ADLDisplayEDIDData;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about input of controller overlay adjustment.
///
/// This structure is used to store the information about input of controller overlay adjustment for the adapter.
/// This structure is used by the ADL_Display_ControllerOverlayAdjustmentCaps_Get, ADL_Display_ControllerOverlayAdjustmentData_Get, and
/// ADL_Display_ControllerOverlayAdjustmentData_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLControllerOverlayInput
{
/// Should be set to the sizeof the structure
  int  iSize;
///\ref ADL_DL_CONTROLLER_OVERLAY_ALPHA or \ref ADL_DL_CONTROLLER_OVERLAY_ALPHAPERPIX
  int  iOverlayAdjust;
/// Data.
  int  iValue;
/// Should be 0.
  int  iReserved;
} ADLControllerOverlayInput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about overlay adjustment.
///
/// This structure is used to store the information about overlay adjustment for the adapter.
/// This structure is used by the ADLControllerOverlayInfo() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdjustmentinfo
{
/// Default value
  int iDefault;
/// Minimum value
  int iMin;
/// Maximum Value
  int iMax;
/// Step value
  int iStep;
} ADLAdjustmentinfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about controller overlay information.
///
/// This structure is used to store information about controller overlay info for the adapter.
/// This structure is used by the ADL_Display_ControllerOverlayAdjustmentCaps_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLControllerOverlayInfo
{
/// Should be set to the sizeof the structure
  int                    iSize;
/// Data.
  ADLAdjustmentinfo        sOverlayInfo;
/// Should be 0.
  int                    iReserved[3];
} ADLControllerOverlayInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync module information.
///
/// This structure is used to retrieve GL-Sync module information for
/// Workstation Framelock/Genlock.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGLSyncModuleID
{
/// Unique GL-Sync module ID.
    int        iModuleID;
/// GL-Sync GPU port index (to be passed into ADLGLSyncGenlockConfig.lSignalSource and ADLGlSyncPortControl.lSignalSource).
    int        iGlSyncGPUPort;
/// GL-Sync module firmware version of Boot Sector.
    int        iFWBootSectorVersion;
/// GL-Sync module firmware version of User Sector.
    int        iFWUserSectorVersion;
} ADLGLSyncModuleID , *LPADLGLSyncModuleID;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync ports capabilities.
///
/// This structure is used to retrieve hardware capabilities for the ports of the GL-Sync module
/// for Workstation Framelock/Genlock (such as port type and number of associated LEDs).
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGLSyncPortCaps
{
/// Port type. Bitfield of ADL_GLSYNC_PORTTYPE_*  \ref define_glsync
    int        iPortType;
/// Number of LEDs associated for this port.
    int        iNumOfLEDs;
}ADLGLSyncPortCaps, *LPADLGLSyncPortCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync Genlock settings.
///
/// This structure is used to get and set genlock settings for the GPU ports of the GL-Sync module
/// for Workstation Framelock/Genlock.\n
/// \see define_glsync
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGLSyncGenlockConfig
{
/// Specifies what fields in this structure are valid \ref define_glsync
    int        iValidMask;
/// Delay (ms) generating a sync signal.
    int        iSyncDelay;
/// Vector of framelock control bits. Bitfield of ADL_GLSYNC_FRAMELOCKCNTL_* \ref define_glsync
    int        iFramelockCntlVector;
/// Source of the sync signal. Either GL_Sync GPU Port index or ADL_GLSYNC_SIGNALSOURCE_* \ref define_glsync
    int        iSignalSource;
/// Use sampled sync signal. A value of 0 specifies no sampling.
    int        iSampleRate;
/// For interlaced sync signals, the value can be ADL_GLSYNC_SYNCFIELD_1 or *_BOTH \ref define_glsync
    int        iSyncField;
/// The signal edge that should trigger synchronization. ADL_GLSYNC_TRIGGEREDGE_* \ref define_glsync
    int        iTriggerEdge;
/// Scan rate multiplier applied to the sync signal. ADL_GLSYNC_SCANRATECOEFF_* \ref define_glsync
    int        iScanRateCoeff;
}ADLGLSyncGenlockConfig, *LPADLGLSyncGenlockConfig;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync port information.
///
/// This structure is used to get status of the GL-Sync ports (BNC or RJ45s)
/// for Workstation Framelock/Genlock.
/// \see define_glsync
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncPortInfo
{
/// Type of GL-Sync port (ADL_GLSYNC_PORT_*).
    int        iPortType;
/// The number of LEDs for this port. It's also filled within ADLGLSyncPortCaps.
    int        iNumOfLEDs;
/// Port state ADL_GLSYNC_PORTSTATE_*  \ref define_glsync
    int        iPortState;
/// Scanned frequency for this port (vertical refresh rate in milliHz; 60000 means 60 Hz).
    int        iFrequency;
/// Used for ADL_GLSYNC_PORT_BNC. It is ADL_GLSYNC_SIGNALTYPE_*   \ref define_glsync
    int        iSignalType;
/// Used for ADL_GLSYNC_PORT_RJ45PORT*. It is GL_Sync GPU Port index or ADL_GLSYNC_SIGNALSOURCE_*.  \ref define_glsync
    int        iSignalSource;
} ADLGlSyncPortInfo, *LPADLGlSyncPortInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync port control settings.
///
/// This structure is used to configure the GL-Sync ports (RJ45s only)
/// for Workstation Framelock/Genlock.
/// \see define_glsync
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncPortControl
{
/// Port to control ADL_GLSYNC_PORT_RJ45PORT1 or ADL_GLSYNC_PORT_RJ45PORT2   \ref define_glsync
    int        iPortType;
/// Port control data ADL_GLSYNC_PORTCNTL_*   \ref define_glsync
    int        iControlVector;
/// Source of the sync signal. Either GL_Sync GPU Port index or ADL_GLSYNC_SIGNALSOURCE_*   \ref define_glsync
    int        iSignalSource;
} ADLGlSyncPortControl;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync mode of a display.
///
/// This structure is used to get and set GL-Sync mode settings for a display connected to
/// an adapter attached to a GL-Sync module for Workstation Framelock/Genlock.
/// \see define_glsync
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncMode
{
/// Mode control vector. Bitfield of ADL_GLSYNC_MODECNTL_*   \ref define_glsync
    int        iControlVector;
/// Mode status vector. Bitfield of ADL_GLSYNC_MODECNTL_STATUS_*   \ref define_glsync
    int        iStatusVector;
/// Index of GL-Sync connector used to genlock the display/controller.
    int        iGLSyncConnectorIndex;
} ADLGlSyncMode, *LPADLGlSyncMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing GL-Sync mode of a display.
///
/// This structure is used to get and set GL-Sync mode settings for a display connected to
/// an adapter attached to a GL-Sync module for Workstation Framelock/Genlock.
/// \see define_glsync
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGlSyncMode2
{
/// Mode control vector. Bitfield of ADL_GLSYNC_MODECNTL_*   \ref define_glsync
    int        iControlVector;
/// Mode status vector. Bitfield of ADL_GLSYNC_MODECNTL_STATUS_*   \ref define_glsync
    int        iStatusVector;
/// Index of GL-Sync connector used to genlock the display/controller.
    int        iGLSyncConnectorIndex;
/// Index of the display to which this GLSync applies to.
    int        iDisplayIndex;
} ADLGlSyncMode2, *LPADLGlSyncMode2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the packet info of a display.
///
/// This structure is used to get and set the packet information of a display.
/// This structure is used by ADLDisplayDataPacket.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct  ADLInfoPacket
{
    char hb0;
    char hb1;
    char hb2;
/// sb0~sb27
    char sb[28];
}ADLInfoPacket;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the AVI packet info of a display.
///
/// This structure is used to get and set AVI the packet info of a display.
/// This structure is used by ADLDisplayDataPacket.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAVIInfoPacket  //Valid user defined data/
{
/// byte 3, bit 7
   char bPB3_ITC;
/// byte 5, bit [7:4].
   char bPB5;
}ADLAVIInfoPacket;

// Overdrive clock setting structure definition.

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Overdrive clock setting.
///
/// This structure is used to get the Overdrive clock setting.
/// This structure is used by ADLAdapterODClockInfo.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODClockSetting
{
/// Deafult clock
    int iDefaultClock;
/// Current clock
    int iCurrentClock;
/// Maximum clcok
    int iMaxClock;
/// Minimum clock
    int iMinClock;
/// Requested clcock
    int iRequestedClock;
/// Step
    int iStepClock;
} ADLODClockSetting;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Overdrive clock information.
///
/// This structure is used to get the Overdrive clock information.
/// This structure is used by the ADL_Display_ODClockInfo_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterODClockInfo
{
/// Size of the structure
    int iSize;
/// Flag \ref define_clockinfo_flags
    int iFlags;
/// Memory Clock
    ADLODClockSetting sMemoryClock;
/// Engine Clock
    ADLODClockSetting sEngineClock;
} ADLAdapterODClockInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Overdrive clock configuration.
///
/// This structure is used to set the Overdrive clock configuration.
/// This structure is used by the ADL_Display_ODClockConfig_Set() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterODClockConfig
{
/// Size of the structure
  int iSize;
/// Flag \ref define_clockinfo_flags
  int iFlags;
/// Memory Clock
  int iMemoryClock;
/// Engine Clock
  int iEngineClock;
} ADLAdapterODClockConfig;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about current power management related activity.
///
/// This structure is used to store information about current power management related activity.
/// This structure (Overdrive 5 interfaces) is used by the ADL_PM_CurrentActivity_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPMActivity
{
/// Must be set to the size of the structure
    int iSize;
/// Current engine clock.
    int iEngineClock;
/// Current memory clock.
    int iMemoryClock;
/// Current core voltage.
    int iVddc;
/// GPU utilization.
    int iActivityPercent;
/// Performance level index.
    int iCurrentPerformanceLevel;
/// Current PCIE bus speed.
    int iCurrentBusSpeed;
/// Number of PCIE bus lanes.
    int iCurrentBusLanes;
/// Maximum number of PCIE bus lanes.
    int iMaximumBusLanes;
/// Reserved for future purposes.
    int iReserved;
} ADLPMActivity;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about thermal controller.
///
/// This structure is used to store information about thermal controller.
/// This structure is used by ADL_PM_ThermalDevices_Enum.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLThermalControllerInfo
{
/// Must be set to the size of the structure
  int iSize;
/// Possible valies: \ref ADL_DL_THERMAL_DOMAIN_OTHER or \ref ADL_DL_THERMAL_DOMAIN_GPU.
  int iThermalDomain;
///    GPU 0, 1, etc.
  int iDomainIndex;
/// Possible valies: \ref ADL_DL_THERMAL_FLAG_INTERRUPT or \ref ADL_DL_THERMAL_FLAG_FANCONTROL
  int iFlags;
} ADLThermalControllerInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about thermal controller temperature.
///
/// This structure is used to store information about thermal controller temperature.
/// This structure is used by the ADL_PM_Temperature_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLTemperature
{
/// Must be set to the size of the structure
  int iSize;
/// Temperature in millidegrees Celsius.
  int iTemperature;
} ADLTemperature;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about thermal controller fan speed.
///
/// This structure is used to store information about thermal controller fan speed.
/// This structure is used by the ADL_PM_FanSpeedInfo_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFanSpeedInfo
{
/// Must be set to the size of the structure
  int iSize;
/// \ref define_fanctrl
  int iFlags;
/// Minimum possible fan speed value in percents.
  int iMinPercent;
/// Maximum possible fan speed value in percents.
  int iMaxPercent;
/// Minimum possible fan speed value in RPM.
  int iMinRPM;
/// Maximum possible fan speed value in RPM.
  int iMaxRPM;
} ADLFanSpeedInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about fan speed reported by thermal controller.
///
/// This structure is used to store information about fan speed reported by thermal controller.
/// This structure is used by the ADL_Overdrive5_FanSpeed_Get() and ADL_Overdrive5_FanSpeed_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFanSpeedValue
{
/// Must be set to the size of the structure
  int iSize;
/// Possible valies: \ref ADL_DL_FANCTRL_SPEED_TYPE_PERCENT or \ref ADL_DL_FANCTRL_SPEED_TYPE_RPM
  int iSpeedType;
/// Fan speed value
  int iFanSpeed;
/// The only flag for now is: \ref ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED
  int iFlags;
} ADLFanSpeedValue;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the range of Overdrive parameter.
///
/// This structure is used to store information about the range of Overdrive parameter.
/// This structure is used by ADLODParameters.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODParameterRange
{
/// Minimum parameter value.
  int iMin;
/// Maximum parameter value.
  int iMax;
/// Parameter step value.
  int iStep;
} ADLODParameterRange;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive parameters.
///
/// This structure is used to store information about Overdrive parameters.
/// This structure is used by the ADL_Overdrive5_ODParameters_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODParameters
{
/// Must be set to the size of the structure
  int iSize;
/// Number of standard performance states.
  int iNumberOfPerformanceLevels;
/// Indicates whether the GPU is capable to measure its activity.
  int iActivityReportingSupported;
/// Indicates whether the GPU supports discrete performance levels or performance range.
  int iDiscretePerformanceLevels;
/// Reserved for future use.
  int iReserved;
/// Engine clock range.
  ADLODParameterRange sEngineClock;
/// Memory clock range.
  ADLODParameterRange sMemoryClock;
/// Core voltage range.
  ADLODParameterRange sVddc;
} ADLODParameters;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive level.
///
/// This structure is used to store information about Overdrive level.
/// This structure is used by ADLODPerformanceLevels.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODPerformanceLevel
{
/// Engine clock.
  int iEngineClock;
/// Memory clock.
  int iMemoryClock;
/// Core voltage.
  int iVddc;
} ADLODPerformanceLevel;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive performance levels.
///
/// This structure is used to store information about Overdrive performance levels.
/// This structure is used by the ADL_Overdrive5_ODPerformanceLevels_Get() and ADL_Overdrive5_ODPerformanceLevels_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODPerformanceLevels
{
/// Must be set to sizeof( \ref ADLODPerformanceLevels ) + sizeof( \ref ADLODPerformanceLevel ) * (ADLODParameters.iNumberOfPerformanceLevels - 1)
  int iSize;
  int iReserved;
/// Array of performance state descriptors. Must have ADLODParameters.iNumberOfPerformanceLevels elements.
  ADLODPerformanceLevel aLevels [1];
} ADLODPerformanceLevels;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the proper CrossfireX chains combinations.
///
/// This structure is used to store information about the CrossfireX chains combination for a particular adapter.
/// This structure is used by the ADL_Adapter_Crossfire_Caps(), ADL_Adapter_Crossfire_Get(), and ADL_Adapter_Crossfire_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLCrossfireComb
{
/// Number of adapters in this combination.
  int iNumLinkAdapter;
/// A list of ADL indexes of the linked adapters in this combination.
  int iAdaptLink[3];
} ADLCrossfireComb;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing CrossfireX state and error information.
///
/// This structure is used to store state and error information about a particular adapter CrossfireX combination.
/// This structure is used by the ADL_Adapter_Crossfire_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLCrossfireInfo
{
/// Current error code of this CrossfireX combination.
  int iErrorCode;
/// Current \ref define_crossfirestate
  int iState;
/// If CrossfireX is supported by this combination. The value is either \ref ADL_TRUE or \ref ADL_FALSE.
  int iSupported;
} ADLCrossfireInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about the BIOS.
///
/// This structure is used to store various information about the Chipset.  This
/// information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLBiosInfo
{
    char strPartNumber[ADL_MAX_PATH];    ///< Part number.
    char strVersion[ADL_MAX_PATH];        ///< Version number.
    char strDate[ADL_MAX_PATH];        ///< BIOS date in yyyy/mm/dd hh:mm format.
} ADLBiosInfo, *LPADLBiosInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about adapter location.
///
/// This structure is used to store information about adapter location.
/// This structure is used by ADLMVPUStatus.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterLocation
{
/// PCI Bus number : 8 bits
    int iBus;
/// Device number : 5 bits
    int iDevice;
/// Function number : 3 bits
    int iFunction;
} ADLAdapterLocation,ADLBdf;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing version information
///
/// This structure is used to store software version information, description of the display device and a web link to the latest installed Catalyst drivers.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLVersionsInfo
{
    /// Driver Release (Packaging) Version (e.g. 8.71-100128n-094835E-ATI)
    char strDriverVer[ADL_MAX_PATH];
    /// Catalyst Version(e.g. "10.1").
    char strCatalystVersion[ADL_MAX_PATH];
    /// Web link to an XML file with information about the latest AMD drivers and locations (e.g. "http://www.amd.com/us/driverxml" )
    char strCatalystWebLink[ADL_MAX_PATH];
} ADLVersionsInfo, *LPADLVersionsInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing version information
///
/// This structure is used to store software version information, description of the display device and a web link to the latest installed Catalyst drivers.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLVersionsInfoX2
{
    /// Driver Release (Packaging) Version (e.g. "16.20.1035-160621a-303814C")
    char strDriverVer[ADL_MAX_PATH];
    /// Catalyst Version(e.g. "15.8").
    char strCatalystVersion[ADL_MAX_PATH];
    /// Crimson Version(e.g. "16.6.2").
    char strCrimsonVersion[ADL_MAX_PATH];
    /// Web link to an XML file with information about the latest AMD drivers and locations (e.g. "http://support.amd.com/drivers/xml/driver_09_us.xml" )
    char strCatalystWebLink[ADL_MAX_PATH];
} ADLVersionsInfoX2, *LPADLVersionsInfoX2;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about MultiVPU capabilities.
///
/// This structure is used to store information about MultiVPU capabilities.
/// This structure is used by the ADL_Display_MVPUCaps_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMVPUCaps
{
/// Must be set to sizeof( ADLMVPUCaps ).
  int iSize;
/// Number of adapters.
  int iAdapterCount;
/// Bits set for all possible MVPU masters. \ref MVPU_ADAPTER_0 .. \ref MVPU_ADAPTER_3
  int iPossibleMVPUMasters;
/// Bits set for all possible MVPU slaves. \ref MVPU_ADAPTER_0 .. \ref MVPU_ADAPTER_3
  int iPossibleMVPUSlaves;
/// Registry path for each adapter.
  char cAdapterPath[ADL_DL_MAX_MVPU_ADAPTERS][ADL_DL_MAX_REGISTRY_PATH];
} ADLMVPUCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about MultiVPU status.
///
/// This structure is used to store information about MultiVPU status.
/// Ths structure is used by the ADL_Display_MVPUStatus_Get() function.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMVPUStatus
{
/// Must be set to sizeof( ADLMVPUStatus ).
  int iSize;
/// Number of active adapters.
  int iActiveAdapterCount;
/// MVPU status.
  int iStatus;
/// PCI Bus/Device/Function for each active adapter participating in MVPU.
  ADLAdapterLocation aAdapterLocation[ADL_DL_MAX_MVPU_ADAPTERS];
} ADLMVPUStatus;

// Displays Manager structures

///////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about the activatable source.
///
/// This structure is used to store activatable source information
/// This information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLActivatableSource
{
    /// The Persistent logical Adapter Index.
    int iAdapterIndex;
    /// The number of Activatable Sources.
    int iNumActivatableSources;
    /// The bit mask identifies the number of bits ActivatableSourceValue is using. (Not currnetly used)
    int iActivatableSourceMask;
    /// The bit mask identifies the status.  (Not currnetly used)
    int iActivatableSourceValue;
} ADLActivatableSource, *LPADLActivatableSource;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about display mode.
///
/// This structure is used to store the display mode for the current adapter
/// such as X, Y positions, screen resolutions, orientation,
/// color depth, refresh rate, progressive or interlace mode, etc.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLMode
{
/// Adapter index.
    int iAdapterIndex;
/// Display IDs.
    ADLDisplayID displayID;
/// Screen position X coordinate.
    int iXPos;
/// Screen position Y coordinate.
    int iYPos;
/// Screen resolution Width.
    int iXRes;
/// Screen resolution Height.
    int iYRes;
/// Screen Color Depth. E.g., 16, 32.
    int iColourDepth;
/// Screen refresh rate. Could be fractional E.g. 59.97
    float fRefreshRate;
/// Screen orientation. E.g., 0, 90, 180, 270.
    int iOrientation;
/// Vista mode flag indicating Progressive or Interlaced mode.
    int iModeFlag;
/// The bit mask identifying the number of bits this Mode is currently using. It is the sum of all the bit definitions defined in \ref define_displaymode
    int iModeMask;
/// The bit mask identifying the display status. The detailed definition is in  \ref define_displaymode
    int iModeValue;
} ADLMode, *LPADLMode;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about display target information.
///
/// This structure is used to store the display target information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayTarget
{
    /// The Display ID.
    ADLDisplayID displayID;

    /// The display map index identify this manner and the desktop surface.
    int iDisplayMapIndex;

    /// The bit mask identifies the number of bits DisplayTarget is currently using. It is the sum of all the bit definitions defined in \ref ADL_DISPLAY_DISPLAYTARGET_PREFERRED.
    int  iDisplayTargetMask;

    /// The bit mask identifies the display status. The detailed definition is in \ref ADL_DISPLAY_DISPLAYTARGET_PREFERRED.
    int  iDisplayTargetValue;
} ADLDisplayTarget, *LPADLDisplayTarget;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display SLS bezel Mode information.
///
/// This structure is used to store the display SLS bezel Mode information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct tagADLBezelTransientMode
{
    /// Adapter Index
    int iAdapterIndex;

    /// SLS Map Index
    int iSLSMapIndex;

    /// The mode index
    int iSLSModeIndex;

    /// The mode
    ADLMode displayMode;

    /// The number of bezel offsets belongs to this map
    int  iNumBezelOffset;

    /// The first bezel offset array index in the native mode array
    int  iFirstBezelOffsetArrayIndex;

    /// The bit mask identifies the bits this structure is currently using. It will be the total OR of all the bit definitions.
    int  iSLSBezelTransientModeMask;

    /// The bit mask identifies the display status. The detail definition is defined below.
    int  iSLSBezelTransientModeValue;
} ADLBezelTransientMode, *LPADLBezelTransientMode;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about the adapter display manner.
///
/// This structure is used to store adapter display manner information
/// This information can be returned to the user. Alternatively, it can be used to access various driver calls to
/// fetch various display device related display manner settings upon the user's request.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterDisplayCap
{
    /// The Persistent logical Adapter Index.
    int iAdapterIndex;
    /// The bit mask identifies the number of bits AdapterDisplayCap is currently using. Sum all the bits defined in ADL_ADAPTER_DISPLAYCAP_XXX
    int  iAdapterDisplayCapMask;
    /// The bit mask identifies the status. Refer to ADL_ADAPTER_DISPLAYCAP_XXX
    int  iAdapterDisplayCapValue;
} ADLAdapterDisplayCap, *LPADLAdapterDisplayCap;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about display mapping.
///
/// This structure is used to store the display mapping data such as display manner.
/// For displays with horizontal or vertical stretch manner,
/// this structure also stores the display order, display row, and column data.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayMap
{
/// The current display map index. It is the OS desktop index. For example, if the OS index 1 is showing clone mode, the display map will be 1.
    int iDisplayMapIndex;

/// The Display Mode for the current map
    ADLMode displayMode;

/// The number of display targets belongs to this map\n
    int iNumDisplayTarget;

/// The first target array index in the Target array\n
    int iFirstDisplayTargetArrayIndex;

/// The bit mask identifies the number of bits DisplayMap is currently using. It is the sum of all the bit definitions defined in ADL_DISPLAY_DISPLAYMAP_MANNER_xxx.
     int  iDisplayMapMask;

///The bit mask identifies the display status. The detailed definition is in ADL_DISPLAY_DISPLAYMAP_MANNER_xxx.
    int  iDisplayMapValue;
} ADLDisplayMap, *LPADLDisplayMap;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about the display device possible map for one GPU
///
/// This structure is used to store the display device possible map
/// This information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPossibleMap
{
    /// The current PossibleMap index. Each PossibleMap is assigned an index
    int iIndex;
    /// The adapter index identifying the GPU for which to validate these Maps & Targets
    int iAdapterIndex;
    /// Number of display Maps for this GPU to be validated
    int iNumDisplayMap;
    /// The display Maps list to validate
    ADLDisplayMap* displayMap;
    /// the number of display Targets for these display Maps
    int iNumDisplayTarget;
    /// The display Targets list for these display Maps to be validated.
    ADLDisplayTarget* displayTarget;
} ADLPossibleMap, *LPADLPossibleMap;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about display possible mapping.
///
/// This structure is used to store the display possible mapping's controller index for the current display.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPossibleMapping
{
    int iDisplayIndex;                ///< The display index. Each display is assigned an index.
    int iDisplayControllerIndex;    ///< The controller index to which display is mapped.
    int iDisplayMannerSupported;    ///< The supported display manner.
} ADLPossibleMapping, *LPADLPossibleMapping;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing information about the validated display device possible map result.
///
/// This structure is used to store the validated display device possible map result
/// This information can be returned to the user.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPossibleMapResult
{
    /// The current display map index. It is the OS Desktop index. For example, OS Index 1 showing clone mode. The Display Map will be 1.
    int iIndex;
    // The bit mask identifies the number of bits   PossibleMapResult is currently using. It will be the sum all the bit definitions defined in ADL_DISPLAY_POSSIBLEMAPRESULT_VALID.
    int iPossibleMapResultMask;
    /// The bit mask identifies the possible map result. The detail definition is defined in ADL_DISPLAY_POSSIBLEMAPRESULT_XXX.
    int iPossibleMapResultValue;
} ADLPossibleMapResult, *LPADLPossibleMapResult;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display SLS Grid information.
///
/// This structure is used to store the display SLS Grid information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSLSGrid
{
/// The Adapter index.
    int iAdapterIndex;

/// The grid index.
    int  iSLSGridIndex;

/// The grid row.
    int  iSLSGridRow;

/// The grid column.
    int  iSLSGridColumn;

/// The grid bit mask identifies the number of bits DisplayMap is currently using. Sum of all bits defined in ADL_DISPLAY_SLSGRID_ORIENTATION_XXX
    int  iSLSGridMask;

/// The grid bit value identifies the display status. Refer to ADL_DISPLAY_SLSGRID_ORIENTATION_XXX
    int  iSLSGridValue;
} ADLSLSGrid, *LPADLSLSGrid;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display SLS Map information.
///
/// This structure is used to store the display SLS Map information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct    ADLSLSMap
{
    /// The Adapter Index
    int iAdapterIndex;

    /// The current display map index. It is the OS Desktop index. For example, OS Index 1 showing clone mode. The Display Map will be 1.
    int iSLSMapIndex;

    /// Indicate the current grid
    ADLSLSGrid grid;

    /// OS surface index
    int  iSurfaceMapIndex;

     ///  Screen orientation. E.g., 0, 90, 180, 270
     int iOrientation;

    /// The number of display targets belongs to this map
    int  iNumSLSTarget;

    /// The first target array index in the Target array
    int  iFirstSLSTargetArrayIndex;

    /// The number of native modes belongs to this map
    int  iNumNativeMode;

    /// The first native mode array index in the native mode array
    int  iFirstNativeModeArrayIndex;

    /// The number of bezel modes belongs to this map
    int  iNumBezelMode;

    /// The first bezel mode array index in the native mode array
    int  iFirstBezelModeArrayIndex;

    /// The number of bezel offsets belongs to this map
    int  iNumBezelOffset;

    /// The first bezel offset array index in the
    int  iFirstBezelOffsetArrayIndex;

    /// The bit mask identifies the number of bits DisplayMap is currently using. Sum all the bit definitions defined in ADL_DISPLAY_SLSMAP_XXX.
    int  iSLSMapMask;

    /// The bit mask identifies the display map status. Refer to ADL_DISPLAY_SLSMAP_XXX
    int  iSLSMapValue;
} ADLSLSMap, *LPADLSLSMap;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display SLS Offset information.
///
/// This structure is used to store the display SLS Offset information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSLSOffset
{
    /// The Adapter Index
    int iAdapterIndex;

    /// The current display map index. It is the OS Desktop index. For example, OS Index 1 showing clone mode. The Display Map will be 1.
    int iSLSMapIndex;

    /// The Display ID.
    ADLDisplayID displayID;

    /// SLS Bezel Mode Index
    int iBezelModeIndex;

    /// SLS Bezel Offset X
    int iBezelOffsetX;

    /// SLS Bezel Offset Y
    int iBezelOffsetY;

    /// SLS Display Width
    int iDisplayWidth;

    /// SLS Display Height
    int iDisplayHeight;

    /// The bit mask identifies the number of bits Offset is currently using.
    int iBezelOffsetMask;

    /// The bit mask identifies the display status.
    int  iBezelffsetValue;
} ADLSLSOffset, *LPADLSLSOffset;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display SLS Mode information.
///
/// This structure is used to store the display SLS Mode information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSLSMode
{
    /// The Adapter Index
    int iAdapterIndex;

    /// The current display map index. It is the OS Desktop index. For example, OS Index 1 showing clone mode. The Display Map will be 1.
    int iSLSMapIndex;

    /// The mode index
    int iSLSModeIndex;

    /// The mode for this map.
    ADLMode displayMode;

    /// The bit mask identifies the number of bits Mode is currently using.
    int iSLSNativeModeMask;

    /// The bit mask identifies the display status.
    int iSLSNativeModeValue;
} ADLSLSMode, *LPADLSLSMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the display Possible SLS Map information.
///
/// This structure is used to store the display Possible SLS Map information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPossibleSLSMap
{
    /// The current display map index. It is the OS Desktop index.
    /// For example, OS Index 1 showing clone mode. The Display Map will be 1.
    int iSLSMapIndex;

    /// Number of display map to be validated.
    int iNumSLSMap;

    /// The display map list for validation
    ADLSLSMap* lpSLSMap;

    /// the number of display map config to be validated.
    int iNumSLSTarget;

    /// The display target list for validation.
    ADLDisplayTarget* lpDisplayTarget;
} ADLPossibleSLSMap, *LPADLPossibleSLSMap;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the SLS targets.
///
/// This structure is used to store the SLS targets information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSLSTarget
{
    /// the logic adapter index
    int iAdapterIndex;

    /// The SLS map index
    int iSLSMapIndex;

    /// The target ID
    ADLDisplayTarget displayTarget;

    /// Target postion X in SLS grid
    int iSLSGridPositionX;

    /// Target postion Y in SLS grid
    int iSLSGridPositionY;

    /// The view size width, height and rotation angle per SLS Target
    ADLMode viewSize;

    /// The bit mask identifies the bits in iSLSTargetValue are currently used
    int iSLSTargetMask;

    /// The bit mask identifies status info. It is for function extension purpose
    int iSLSTargetValue;
} ADLSLSTarget, *LPADLSLSTarget;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the Adapter offset stepping size.
///
/// This structure is used to store the Adapter offset stepping size information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLBezelOffsetSteppingSize
{
    /// the logic adapter index
    int iAdapterIndex;

    /// The SLS map index
    int iSLSMapIndex;

    /// Bezel X stepping size offset
    int iBezelOffsetSteppingSizeX;

    /// Bezel Y stepping size offset
    int iBezelOffsetSteppingSizeY;

    /// Identifies the bits this structure is currently using. It will be the total OR of all the bit definitions.
    int iBezelOffsetSteppingSizeMask;

    /// Bit mask identifies the display status.
    int iBezelOffsetSteppingSizeValue;
} ADLBezelOffsetSteppingSize, *LPADLBezelOffsetSteppingSize;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the overlap offset info for all the displays for each SLS mode.
///
/// This structure is used to store the no. of overlapped modes for each SLS Mode once user finishes the configuration from Overlap Widget
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSLSOverlappedMode
{
    /// the SLS mode for which the overlap is configured
    ADLMode SLSMode;
    /// the number of target displays in SLS.
    int iNumSLSTarget;
    /// the first target array index in the target array
    int iFirstTargetArrayIndex;
}ADLSLSTargetOverlap, *LPADLSLSTargetOverlap;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported PowerExpress Config Caps
///
/// This structure is used to store the driver supported PowerExpress Config Caps
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPXConfigCaps
{
    /// The Persistent logical Adapter Index.
    int iAdapterIndex;

    /// The bit mask identifies the number of bits PowerExpress Config Caps is currently using. It is the sum of all the bit definitions defined in ADL_PX_CONFIGCAPS_XXXX /ref define_powerxpress_constants.
    int  iPXConfigCapMask;

    /// The bit mask identifies the PowerExpress Config Caps value. The detailed definition is in ADL_PX_CONFIGCAPS_XXXX /ref define_powerxpress_constants.
    int  iPXConfigCapValue;
} ADLPXConfigCaps, *LPADLPXConfigCaps;

/////////////////////////////////////////////////////////////////////////////////////////
///\brief Enum containing PX or HG type
///
/// This enum is used to get PX or hG type
///
/// \nosubgrouping
//////////////////////////////////////////////////////////////////////////////////////////
typedef enum ADLPxType
{
	//Not AMD related PX/HG or not PX or HG at all
	ADL_PX_NONE = 0,
	//A+A PX
	ADL_SWITCHABLE_AMDAMD = 1,
	// A+A HG
	ADL_HG_AMDAMD = 2,
	//A+I PX
	ADL_SWITCHABLE_AMDOTHER = 3,
	//A+I HG
	ADL_HG_AMDOTHER = 4,
}ADLPxType;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an application
///
/// This structure is used to store basic information of an application
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLApplicationData
{
    /// Path Name
    char strPathName[ADL_MAX_PATH];
    /// File Name
    char strFileName[ADL_APP_PROFILE_FILENAME_LENGTH];
    /// Creation timestamp
    char strTimeStamp[ADL_APP_PROFILE_TIMESTAMP_LENGTH];
    /// Version
    char strVersion[ADL_APP_PROFILE_VERSION_LENGTH];
}ADLApplicationData;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an application
///
/// This structure is used to store basic information of an application
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLApplicationDataX2
{
    /// Path Name
    wchar_t strPathName[ADL_MAX_PATH];
    /// File Name
    wchar_t strFileName[ADL_APP_PROFILE_FILENAME_LENGTH];
    /// Creation timestamp
    wchar_t strTimeStamp[ADL_APP_PROFILE_TIMESTAMP_LENGTH];
    /// Version
    wchar_t strVersion[ADL_APP_PROFILE_VERSION_LENGTH];
}ADLApplicationDataX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an application
///
/// This structure is used to store basic information of an application including process id
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLApplicationDataX3
{
    /// Path Name
    wchar_t strPathName[ADL_MAX_PATH];
    /// File Name
    wchar_t strFileName[ADL_APP_PROFILE_FILENAME_LENGTH];
    /// Creation timestamp
    wchar_t strTimeStamp[ADL_APP_PROFILE_TIMESTAMP_LENGTH];
    /// Version
    wchar_t strVersion[ADL_APP_PROFILE_VERSION_LENGTH];
    //Application Process id
    unsigned int iProcessId;
}ADLApplicationDataX3;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information of a property of an application profile
///
/// This structure is used to store property information of an application profile
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct PropertyRecord
{
    /// Property Name
    char strName [ADL_APP_PROFILE_PROPERTY_LENGTH];
    /// Property Type
    ADLProfilePropertyType eType;
    /// Data Size in bytes
    int iDataSize;
    /// Property Value, can be any data type
    unsigned char uData[1];
}PropertyRecord;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an application profile
///
/// This structure is used to store information of an application profile
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLApplicationProfile
{
    /// Number of properties
    int iCount;
    /// Buffer to store all property records
    PropertyRecord record[1];
}ADLApplicationProfile;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an OD5 Power Control feature
///
/// This structure is used to store information of an Power Control feature
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPowerControlInfo
{
/// Minimum value.
int iMinValue;
/// Maximum value.
int iMaxValue;
/// The minimum change in between minValue and maxValue.
int iStepValue;
 } ADLPowerControlInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an controller mode
///
/// This structure is used to store information of an controller mode
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLControllerMode
{
    /// This falg indicates actions that will be applied by set viewport
    /// The value can be a combination of ADL_CONTROLLERMODE_CM_MODIFIER_VIEW_POSITION,
    /// ADL_CONTROLLERMODE_CM_MODIFIER_VIEW_PANLOCK and ADL_CONTROLLERMODE_CM_MODIFIER_VIEW_SIZE
    int iModifiers;

    /// Horizontal view starting position
    int iViewPositionCx;

    /// Vertical view starting position
    int iViewPositionCy;

    /// Horizontal left panlock position
    int iViewPanLockLeft;

    /// Horizontal right panlock position
    int iViewPanLockRight;

    /// Vertical top panlock position
    int iViewPanLockTop;

    /// Vertical bottom panlock position
    int iViewPanLockBottom;

    /// View resolution in pixels (width)
    int iViewResolutionCx;

    /// View resolution in pixels (hight)
    int iViewResolutionCy;
}ADLControllerMode;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about a display
///
/// This structure is used to store information about a display
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayIdentifier
{
    /// ADL display index
    long ulDisplayIndex;

    /// manufacturer ID of the display
    long ulManufacturerId;

    /// product ID of the display
    long ulProductId;

    /// serial number of the display
    long ulSerialNo;
} ADLDisplayIdentifier;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 clock range
///
/// This structure is used to store information about Overdrive 6 clock range
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6ParameterRange
{
    /// The starting value of the clock range
    int     iMin;
    /// The ending value of the clock range
    int     iMax;
    /// The minimum increment between clock values
    int     iStep;
} ADLOD6ParameterRange;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 capabilities
///
/// This structure is used to store information about Overdrive 6 capabilities
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6Capabilities
{
    /// Contains a bitmap of the OD6 capability flags.  Possible values: \ref ADL_OD6_CAPABILITY_SCLK_CUSTOMIZATION,
    /// \ref ADL_OD6_CAPABILITY_MCLK_CUSTOMIZATION, \ref ADL_OD6_CAPABILITY_GPU_ACTIVITY_MONITOR
    int     iCapabilities;
    /// Contains a bitmap indicating the power states
    /// supported by OD6.  Currently only the performance state
    /// is supported. Possible Values: \ref ADL_OD6_SUPPORTEDSTATE_PERFORMANCE
    int     iSupportedStates;
    /// Number of levels. OD6 will always use 2 levels, which describe
    /// the minimum to maximum clock ranges.
    /// The 1st level indicates the minimum clocks, and the 2nd level
    /// indicates the maximum clocks.
    int     iNumberOfPerformanceLevels;
    /// Contains the hard limits of the sclk range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLOD6ParameterRange     sEngineClockRange;
    /// Contains the hard limits of the mclk range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLOD6ParameterRange     sMemoryClockRange;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6Capabilities;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 clock values.
///
/// This structure is used to store information about Overdrive 6 clock values.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6PerformanceLevel
{
    /// Engine (core) clock.
    int iEngineClock;
    /// Memory clock.
    int iMemoryClock;
} ADLOD6PerformanceLevel;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 clocks.
///
/// This structure is used to store information about Overdrive 6 clocks.  This is a
/// variable-sized structure.  iNumberOfPerformanceLevels indicate how many elements
/// are contained in the aLevels array.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6StateInfo
{
    /// Number of levels.  OD6 uses clock ranges instead of discrete performance levels.
    /// iNumberOfPerformanceLevels is always 2.  The 1st level indicates the minimum clocks
    /// in the range.  The 2nd level indicates the maximum clocks in the range.
    int     iNumberOfPerformanceLevels;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;

    /// Variable-sized array of levels.
    /// The number of elements in the array is specified by iNumberofPerformanceLevels.
    ADLOD6PerformanceLevel aLevels [1];
} ADLOD6StateInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about current Overdrive 6 performance status.
///
/// This structure is used to store information about current Overdrive 6 performance status.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6CurrentStatus
{
    /// Current engine clock in 10 KHz.
    int     iEngineClock;
    /// Current memory clock in 10 KHz.
    int     iMemoryClock;
    /// Current GPU activity in percent.  This
    /// indicates how "busy" the GPU is.
    int     iActivityPercent;
    /// Not used.  Reserved for future use.
    int     iCurrentPerformanceLevel;
    /// Current PCI-E bus speed
    int     iCurrentBusSpeed;
    /// Current PCI-E bus # of lanes
    int     iCurrentBusLanes;
    /// Maximum possible PCI-E bus # of lanes
    int     iMaximumBusLanes;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6CurrentStatus;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 thermal contoller capabilities
///
/// This structure is used to store information about Overdrive 6 thermal controller capabilities
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6ThermalControllerCaps
{
    /// Contains a bitmap of thermal controller capability flags. Possible values: \ref ADL_OD6_TCCAPS_THERMAL_CONTROLLER, \ref ADL_OD6_TCCAPS_FANSPEED_CONTROL,
    /// \ref ADL_OD6_TCCAPS_FANSPEED_PERCENT_READ, \ref ADL_OD6_TCCAPS_FANSPEED_PERCENT_WRITE, \ref ADL_OD6_TCCAPS_FANSPEED_RPM_READ, \ref ADL_OD6_TCCAPS_FANSPEED_RPM_WRITE
    int     iCapabilities;
    /// Minimum fan speed expressed as a percentage
    int     iFanMinPercent;
    /// Maximum fan speed expressed as a percentage
    int     iFanMaxPercent;
    /// Minimum fan speed expressed in revolutions-per-minute
    int     iFanMinRPM;
    /// Maximum fan speed expressed in revolutions-per-minute
    int     iFanMaxRPM;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6ThermalControllerCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 fan speed information
///
/// This structure is used to store information about Overdrive 6 fan speed information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6FanSpeedInfo
{
    /// Contains a bitmap of the valid fan speed type flags.  Possible values: \ref ADL_OD6_FANSPEED_TYPE_PERCENT, \ref ADL_OD6_FANSPEED_TYPE_RPM, \ref ADL_OD6_FANSPEED_USER_DEFINED
    int     iSpeedType;
    /// Contains current fan speed in percent (if valid flag exists in iSpeedType)
    int     iFanSpeedPercent;
    /// Contains current fan speed in RPM (if valid flag exists in iSpeedType)
    int        iFanSpeedRPM;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6FanSpeedInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 fan speed value
///
/// This structure is used to store information about Overdrive 6 fan speed value
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6FanSpeedValue
{
    /// Indicates the units of the fan speed.  Possible values: \ref ADL_OD6_FANSPEED_TYPE_PERCENT, \ref ADL_OD6_FANSPEED_TYPE_RPM
    int     iSpeedType;
    /// Fan speed value (units as indicated above)
    int     iFanSpeed;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6FanSpeedValue;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 PowerControl settings.
///
/// This structure is used to store information about Overdrive 6 PowerControl settings.
/// PowerControl is the feature which allows the performance characteristics of the GPU
/// to be adjusted by changing the PowerTune power limits.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6PowerControlInfo
{
    /// The minimum PowerControl adjustment value
    int     iMinValue;
    /// The maximum PowerControl adjustment value
    int     iMaxValue;
    /// The minimum difference between PowerControl adjustment values
    int     iStepValue;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6PowerControlInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 PowerControl settings.
///
/// This structure is used to store information about Overdrive 6 PowerControl settings.
/// PowerControl is the feature which allows the performance characteristics of the GPU
/// to be adjusted by changing the PowerTune power limits.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6VoltageControlInfo
{
    /// The minimum VoltageControl adjustment value
    int     iMinValue;
    /// The maximum VoltageControl adjustment value
    int     iMaxValue;
    /// The minimum difference between VoltageControl adjustment values
    int     iStepValue;

    /// Value for future extension
    int     iExtValue;
    /// Mask for future extension
    int     iExtMask;
} ADLOD6VoltageControlInfo;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing ECC statistics namely SEC counts and DED counts
/// Single error count - count of errors that can be corrected
/// Doubt Error Detect -  count of errors that cannot be corrected
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLECCData
{
    // Single error count - count of errors that can be corrected
    int iSec;
    // Double error detect - count of errors that cannot be corrected
    int iDed;
} ADLECCData;

/// \brief Handle to ADL client context.
///
///  ADL clients obtain context handle from initial call to \ref ADL2_Main_Control_Create.
///  Clients have to pass the handle to each subsequent ADL call and finally destroy
///  the context with call to \ref ADL2_Main_Control_Destroy
/// \nosubgrouping
typedef void *ADL_CONTEXT_HANDLE;

/// \brief Handle to ADL Frame Monitor Token.
///
///  Frame Monitor clients obtain handle from initial call to \ref ADL2_Adapter_FrameMetrics_FrameDuration_Enable
///  Clients have to pass the handle to each subsequent ADL call to \ref ADL2_Adapter_FrameMetrics_FrameDuration_Get
///  and finally destroy the token with call to \ref ADL2_Adapter_FrameMetrics_FrameDuration_Disable
/// \nosubgrouping
typedef void *ADL_FRAME_DURATION_HANDLE;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the display mode definition used per controller.
///
/// This structure is used to store the display mode definition used per controller.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayModeX2
{
/// Horizontal resolution (in pixels).
   int  iWidth;
/// Vertical resolution (in lines).
   int  iHeight;
/// Interlaced/Progressive. The value will be set for Interlaced as ADL_DL_TIMINGFLAG_INTERLACED. If not set it is progressive. Refer define_detailed_timing_flags.
   int  iScanType;
/// Refresh rate.
   int  iRefreshRate;
/// Timing Standard. Refer define_modetiming_standard.
   int  iTimingStandard;
} ADLDisplayModeX2;

typedef enum ADLAppProcessState
{
	APP_PROC_INVALID = 0,          // Invalid Application
	APP_PROC_PREMPTION = 1,          // The Application is being set up for Process Creation
	APP_PROC_CREATION = 2,          // The Application's Main Process is created by the OS
	APP_PROC_READ = 3,          // The Application's Data is ready to be read
	APP_PROC_WAIT = 4,          // The Application is waiting for Timeout or Notification to Resume
	APP_PROC_RUNNING = 5,          // The Application is running
	APP_PROC_TERMINATE = 6           // The Application is about to terminate
}ADLAppProcessState;

typedef enum ADLAppInterceptionListType
{
	ADL_INVALID_FORMAT = 0,
	ADL_IMAGEFILEFORMAT = 1,
	ADL_ENVVAR = 2
}ADLAppInterceptionListType;

typedef struct ADLAppInterceptionInfo
{
	wchar_t                     AppName[ADL_MAX_PATH]; // the file name of the application or env var
	unsigned int                ProcessId;
	ADLAppInterceptionListType  AppFormat;
	ADLAppProcessState          AppState;
} ADLAppInterceptionInfo;

typedef enum ADL_AP_DATABASE // same as _SHARED_AP_DATABASE in "inc/shared/shared_escape.h"
{
	ADL_AP_DATABASE__SYSTEM,
	ADL_AP_DATABASE__USER,
	ADL_AP_DATABASE__OEM
} ADL_AP_DATABASE;

typedef struct ADLAppInterceptionInfoX2
{
	wchar_t                     AppName[ADL_MAX_PATH]; // the file name of the application or env var
	unsigned int                ProcessId;
	unsigned int                WaitForResumeNeeded;
	wchar_t                     CommandLine[ADL_MAX_PATH]; // The command line on app start/stop event
	ADLAppInterceptionListType  AppFormat;
	ADLAppProcessState          AppState;
} ADLAppInterceptionInfoX2;

typedef struct ADLAppInterceptionInfoX3
{
    wchar_t                     AppName[ADL_MAX_PATH]; // the file name of the application or env var
    unsigned int                ProcessId;
    unsigned int                WaitForResumeNeeded;
    unsigned int                RayTracingStatus; // returns the Ray Tracing status if it is enabled atleast once in session.
    wchar_t                     CommandLine[ADL_MAX_PATH]; // The command line on app start/stop event
    ADLAppInterceptionListType  AppFormat;
    ADLAppProcessState          AppState;
} ADLAppInterceptionInfoX3;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information info for a property record in a profile
///
/// This structure is used to store info for a property record in a profile
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPropertyRecordCreate
{
	/// Name of the property
	wchar_t * strPropertyName;
	/// Data type of the property
	ADLProfilePropertyType eType;
	// Value of the property
	wchar_t * strPropertyValue;
} ADLPropertyRecordCreate;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information info for an application record
///
/// This structure is used to store info for an application record
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLApplicationRecord
{
    /// Title of the application
    wchar_t * strTitle;
    /// File path of the application
    wchar_t * strPathName;
    /// File name of the application
    wchar_t * strFileName;
    /// File versin the application
    wchar_t * strVersion;
    /// Nostes on the application
    wchar_t * strNotes;
    /// Driver area which the application uses
    wchar_t * strArea;
    /// Name of profile assigned to the application
    wchar_t * strProfileName;
    // Source where this application record come from
    ADL_AP_DATABASE recordSource;
} ADLApplicationRecord;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 extension capabilities
///
/// This structure is used to store information about Overdrive 6 extension capabilities
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6CapabilitiesEx
{
    /// Contains a bitmap of the OD6 extension capability flags.  Possible values: \ref ADL_OD6_CAPABILITY_SCLK_CUSTOMIZATION,
    /// \ref ADL_OD6_CAPABILITY_MCLK_CUSTOMIZATION, \ref ADL_OD6_CAPABILITY_GPU_ACTIVITY_MONITOR,
    /// \ref ADL_OD6_CAPABILITY_POWER_CONTROL, \ref ADL_OD6_CAPABILITY_VOLTAGE_CONTROL, \ref ADL_OD6_CAPABILITY_PERCENT_ADJUSTMENT,
    //// \ref ADL_OD6_CAPABILITY_THERMAL_LIMIT_UNLOCK
    int iCapabilities;
    /// The Power states that support clock and power customization.  Only performance state is currently supported.
    /// Possible Values: \ref ADL_OD6_SUPPORTEDSTATE_PERFORMANCE
    int iSupportedStates;
    /// Returns the hard limits of the SCLK overdrive adjustment range.  Overdrive clocks should not be adjusted outside of this range.  The values are specified as +/- percentages.
    ADLOD6ParameterRange sEngineClockPercent;
    /// Returns the hard limits of the MCLK overdrive adjustment range.  Overdrive clocks should not be adjusted outside of this range.  The values are specified as +/- percentages.
    ADLOD6ParameterRange sMemoryClockPercent;
    /// Returns the hard limits of the Power Limit adjustment range.  Power limit should not be adjusted outside this range.  The values are specified as +/- percentages.
    ADLOD6ParameterRange sPowerControlPercent;
    /// Reserved for future expansion of the structure.
    int iExtValue;
    /// Reserved for future expansion of the structure.
    int iExtMask;
} ADLOD6CapabilitiesEx;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 extension state information
///
/// This structure is used to store information about Overdrive 6 extension state information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6StateEx
{
    /// The current engine clock adjustment value, specified as a +/- percent.
    int iEngineClockPercent;
    /// The current memory clock adjustment value, specified as a +/- percent.
    int iMemoryClockPercent;
    /// The current power control adjustment value, specified as a +/- percent.
    int iPowerControlPercent;
    /// Reserved for future expansion of the structure.
    int iExtValue;
    /// Reserved for future expansion of the structure.
    int iExtMask;
} ADLOD6StateEx;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive 6 extension recommended maximum clock adjustment values
///
/// This structure is used to store information about Overdrive 6 extension recommended maximum clock adjustment values
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD6MaxClockAdjust
{
    /// The recommended maximum engine clock adjustment in percent, for the specified power limit value.
    int iEngineClockMax;
    /// The recommended maximum memory clock adjustment in percent, for the specified power limit value.
    /// Currently the memory is independent of the Power Limit setting, so iMemoryClockMax will always return the maximum
    /// possible adjustment value.  This field is here for future enhancement in case we add a dependency between Memory Clock
    /// adjustment and Power Limit setting.
    int iMemoryClockMax;
    /// Reserved for future expansion of the structure.
    int iExtValue;
    /// Reserved for future expansion of the structure.
    int iExtMask;
} ADLOD6MaxClockAdjust;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Connector information
///
/// this structure is used to get the connector information like length, positions & etc.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLConnectorInfo
{
    ///index of the connector(0-based)
    int iConnectorIndex;
    ///used for disply identification/ordering
    int iConnectorId;
    ///index of the slot, 0-based index.
    int iSlotIndex;
    ///Type of the connector. \ref define_connector_types
    int iType;
    ///Position of the connector(in millimeters), from the right side of the slot.
    int iOffset;
    ///Length of the connector(in millimeters).
    int iLength;
} ADLConnectorInfo;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the slot information
///
/// this structure is used to get the slot information like length of the slot, no of connectors on the slot & etc.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLBracketSlotInfo
{
    ///index of the slot, 0-based index.
    int iSlotIndex;
    ///length of the slot(in millimeters).
    int iLength;
    ///width of the slot(in millimeters).
    int iWidth;
} ADLBracketSlotInfo;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing MST branch information
///
/// this structure is used to store the MST branch information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLMSTRad
{
    ///depth of the link.
    int iLinkNumber;
    /// Relative address, address scheme starts from source side
    char rad[ADL_MAX_RAD_LINK_COUNT];
} ADLMSTRad;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing port information
///
/// this structure is used to get the display or MST branch information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDevicePort
{
    ///index of the connector.
    int iConnectorIndex;
    ///Relative MST address. If MST RAD contains 0 it means DP or Root of the MST topology. For non DP connectors MST RAD is ignored.
    ADLMSTRad aMSTRad;
} ADLDevicePort;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing supported connection types and properties
///
/// this structure is used to get the supported connection types and supported properties of given connector
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSupportedConnections
{
    ///Bit vector of supported connections. Bitmask is defined in constants section. \ref define_connection_types
    int iSupportedConnections;
    ///Array of bitvectors. Each bit vector represents supported properties for one connection type. Index of this array is connection type (bit number in mask).
    int iSupportedProperties[ADL_MAX_CONNECTION_TYPES];
} ADLSupportedConnections;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing connection state of the connector
///
/// this structure is used to get the current Emulation status and mode of the given connector
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLConnectionState
{
    ///The value is bit vector. Each bit represents status. See masks constants for details. \ref define_emulation_status
    int iEmulationStatus;
    ///It contains information about current emulation mode. See constants for details. \ref define_emulation_mode
    int iEmulationMode;
    ///If connection is active it will contain display id, otherwise CWDDEDI_INVALID_DISPLAY_INDEX
    int iDisplayIndex;
} ADLConnectionState;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing connection properties information
///
/// this structure is used to retrieve the properties of connection type
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLConnectionProperties
{
    //Bit vector. Represents actual properties. Supported properties for specific connection type. \ref define_connection_properties
    int iValidProperties;
    //Bitrate(in MHz). Could be used for MST branch, DP or DP active dongle. \ref define_linkrate_constants
    int iBitrate;
    //Number of lanes in DP connection. \ref define_lanecount_constants
    int iNumberOfLanes;
    //Color depth(in bits). \ref define_colordepth_constants
    int iColorDepth;
    //3D capabilities. It could be used for some dongles. For instance: alternate framepack. Value of this property is bit vector.
    int iStereo3DCaps;
    ///Output Bandwidth. Could be used for MST branch, DP or DP Active dongle. \ref define_linkrate_constants
    int iOutputBandwidth;
} ADLConnectionProperties;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing connection information
///
/// this structure is used to retrieve the data from driver which includes
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLConnectionData
{
    ///Connection type. based on the connection type either iNumberofPorts or IDataSize,EDIDdata is valid, \ref define_connection_types
    int iConnectionType;
    ///Specifies the connection properties.
    ADLConnectionProperties aConnectionProperties;
    ///Number of ports
    int iNumberofPorts;
    ///Number of Active Connections
    int iActiveConnections;
    ///actual size of EDID data block size.
    int iDataSize;
    ///EDID Data
    char EdidData[ADL_MAX_DISPLAY_EDID_DATA_SIZE];
} ADLConnectionData;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an controller mode including Number of Connectors
///
/// This structure is used to store information of an controller mode
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLAdapterCapsX2
{
    /// AdapterID for this adapter
    int iAdapterID;
    /// Number of controllers for this adapter
    int iNumControllers;
    /// Number of displays for this adapter
    int iNumDisplays;
    /// Number of overlays for this adapter
    int iNumOverlays;
    /// Number of GLSyncConnectors
    int iNumOfGLSyncConnectors;
    /// The bit mask identifies the adapter caps
    int iCapsMask;
    /// The bit identifies the adapter caps \ref define_adapter_caps
    int iCapsValue;
    /// Number of Connectors for this adapter
    int iNumConnectors;
}ADLAdapterCapsX2;

typedef enum ADL_ERROR_RECORD_SEVERITY
{
    ADL_GLOBALLY_UNCORRECTED  = 1,
    ADL_LOCALLY_UNCORRECTED   = 2,
    ADL_DEFFERRED             = 3,
    ADL_CORRECTED             = 4
}ADL_ERROR_RECORD_SEVERITY;

typedef union _ADL_ECC_EDC_FLAG
{
    struct
    {
        unsigned int isEccAccessing        : 1;
        unsigned int reserved              : 31;
    }bits;
    unsigned int u32All;
}ADL_ECC_EDC_FLAG;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about EDC Error Record
///
/// This structure is used to store EDC Error Record
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLErrorRecord
{
    // Severity of error
    ADL_ERROR_RECORD_SEVERITY Severity;

    // Is the counter valid?
    int  countValid;

    // Counter value, if valid
    unsigned int count;

    // Is the location information valid?
    int locationValid;

    // Physical location of error
    unsigned int CU; // CU number on which error occurred, if known
    char StructureName[32]; // e.g. LDS, TCC, etc.

    // Time of error record creation (e.g. time of query, or time of poison interrupt)
    char tiestamp[32];

    unsigned int padding[3];
}ADLErrorRecord;

typedef enum ADL_EDC_BLOCK_ID
{
    ADL_EDC_BLOCK_ID_SQCIS = 1,
    ADL_EDC_BLOCK_ID_SQCDS = 2,
    ADL_EDC_BLOCK_ID_SGPR  = 3,
    ADL_EDC_BLOCK_ID_VGPR  = 4,
    ADL_EDC_BLOCK_ID_LDS   = 5,
    ADL_EDC_BLOCK_ID_GDS   = 6,
    ADL_EDC_BLOCK_ID_TCL1  = 7,
    ADL_EDC_BLOCK_ID_TCL2  = 8
}ADL_EDC_BLOCK_ID;

typedef enum ADL_ERROR_INJECTION_MODE
{
    ADL_ERROR_INJECTION_MODE_SINGLE      = 1,
    ADL_ERROR_INJECTION_MODE_MULTIPLE    = 2,
    ADL_ERROR_INJECTION_MODE_ADDRESS     = 3
}ADL_ERROR_INJECTION_MODE;

typedef union _ADL_ERROR_PATTERN
{
    struct
    {
        unsigned long  EccInjVector         :  16;
        unsigned long  EccInjEn             :  9;
        unsigned long  EccBeatEn            :  4;
        unsigned long  EccChEn              :  4;
        unsigned long  reserved             :  31;
    } bits;
    unsigned long long u64Value;
} ADL_ERROR_PATTERN;

typedef struct ADL_ERROR_INJECTION_DATA
{
    unsigned long long errorAddress;
    ADL_ERROR_PATTERN errorPattern;
}ADL_ERROR_INJECTION_DATA;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about EDC Error Injection
///
/// This structure is used to store EDC Error Injection
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLErrorInjection
{
    ADL_EDC_BLOCK_ID blockId;
    ADL_ERROR_INJECTION_MODE errorInjectionMode;
}ADLErrorInjection;

typedef struct ADLErrorInjectionX2
{
    ADL_EDC_BLOCK_ID blockId;
    ADL_ERROR_INJECTION_MODE errorInjectionMode;
    ADL_ERROR_INJECTION_DATA errorInjectionData;
}ADLErrorInjectionX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing per display FreeSync capability information.
///
/// This structure is used to store the FreeSync capability of both the display and
/// the GPU the display is connected to.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFreeSyncCap
{
    /// FreeSync capability flags. \ref define_freesync_caps
    int iCaps;
    /// Reports minimum FreeSync refresh rate supported by the display in micro hertz
    int iMinRefreshRateInMicroHz;
    /// Reports maximum FreeSync refresh rate supported by the display in micro hertz
    int iMaxRefreshRateInMicroHz;
    /// Index of FreeSync Label to use:  ADL_FREESYNC_LABEL_*
    unsigned char ucLabelIndex;
    /// Reserved
    char cReserved[3];
    int iReserved[4];
} ADLFreeSyncCap;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing per display Display Connectivty Experience Settings
///
/// This structure is used to store the Display Connectivity Experience settings of a
/// display
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDceSettings
{
    DceSettingsType type;                       // Defines which structure is in the union below
    union
    {
        struct
        {
            bool qualityDetectionEnabled;
        } HdmiLq;
        struct
        {
            DpLinkRate linkRate;                // Read-only
            unsigned int numberOfActiveLanes;   // Read-only
            unsigned int numberofTotalLanes;    // Read-only
            int relativePreEmphasis;            // Allowable values are -2 to +2
            int relativeVoltageSwing;           // Allowable values are -2 to +2
            int persistFlag;
        } DpLink;
        struct
        {
            bool linkProtectionEnabled;         // Read-only
        } Protection;
    } Settings;
    int iReserved[15];
} ADLDceSettings;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Graphic Core
///
/// This structure is used to get Graphic Core Info
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLGraphicCoreInfo
{
    /// indicate the graphic core generation
    int iGCGen;

    union
    {
        /// Total number of CUs. Valid for GCN (iGCGen == GCN)
        int iNumCUs;
        /// Total number of WGPs. Valid for RDNA (iGCGen == RDNA)
        int iNumWGPs;
    };

    union
    {
        /// Number of processing elements per CU. Valid for GCN (iGCGen == GCN)
        int iNumPEsPerCU;
        /// Number of processing elements per WGP. Valid for RDNA (iGCGen == RDNA)
        int iNumPEsPerWGP;
    };

    /// Total number of SIMDs. Valid for Pre GCN (iGCGen == Pre-GCN)
    int iNumSIMDs;

    /// Total number of ROPs. Valid for both GCN and Pre GCN
    int iNumROPs;

    /// reserved for future use
    int iReserved[11];
}ADLGraphicCoreInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N clock range
///
/// This structure is used to store information about Overdrive N clock range
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNParameterRange
{
    /// The starting value of the clock range
    int     iMode;
    /// The starting value of the clock range
    int     iMin;
    /// The ending value of the clock range
    int     iMax;
    /// The minimum increment between clock values
    int     iStep;
    /// The default clock values
    int     iDefault;
} ADLODNParameterRange;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N capabilities
///
/// This structure is used to store information about Overdrive N capabilities
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNCapabilities
{
    /// Number of levels which describe the minimum to maximum clock ranges.
    /// The 1st level indicates the minimum clocks, and the 2nd level
    /// indicates the maximum clocks.
    int     iMaximumNumberOfPerformanceLevels;
    /// Contains the hard limits of the sclk range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     sEngineClockRange;
    /// Contains the hard limits of the mclk range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     sMemoryClockRange;
    /// Contains the hard limits of the vddc range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     svddcRange;
    /// Contains the hard limits of the power range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     power;
    /// Contains the hard limits of the power range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     powerTuneTemperature;
    /// Contains the hard limits of the Temperature range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     fanTemperature;
    /// Contains the hard limits of the Fan range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     fanSpeed;
    /// Contains the hard limits of the Fan range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     minimumPerformanceClock;
} ADLODNCapabilities;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N capabilities
///
/// This structure is used to store information about Overdrive N capabilities
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNCapabilitiesX2
{
    /// Number of levels which describe the minimum to maximum clock ranges.
    /// The 1st level indicates the minimum clocks, and the 2nd level
    /// indicates the maximum clocks.
    int     iMaximumNumberOfPerformanceLevels;
    /// bit vector, which tells what are the features are supported.
    /// \ref: ADLODNFEATURECONTROL
    int iFlags;
    /// Contains the hard limits of the sclk range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     sEngineClockRange;
    /// Contains the hard limits of the mclk range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     sMemoryClockRange;
    /// Contains the hard limits of the vddc range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     svddcRange;
    /// Contains the hard limits of the power range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     power;
    /// Contains the hard limits of the power range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     powerTuneTemperature;
    /// Contains the hard limits of the Temperature range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     fanTemperature;
    /// Contains the hard limits of the Fan range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     fanSpeed;
    /// Contains the hard limits of the Fan range.  Overdrive
    /// clocks cannot be set outside this range.
    ADLODNParameterRange     minimumPerformanceClock;
    /// Contains the hard limits of the throttleNotification
    ADLODNParameterRange throttleNotificaion;
    /// Contains the hard limits of the Auto Systemclock
    ADLODNParameterRange autoSystemClock;
} ADLODNCapabilitiesX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive level.
///
/// This structure is used to store information about Overdrive level.
/// This structure is used by ADLODPerformanceLevels.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNPerformanceLevel
{
    /// clock.
    int iClock;
    /// VDCC.
    int iVddc;
    /// enabled
    int iEnabled;
} ADLODNPerformanceLevel;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N performance levels.
///
/// This structure is used to store information about Overdrive performance levels.
/// This structure is used by the ADL_OverdriveN_ODPerformanceLevels_Get() and ADL_OverdriveN_ODPerformanceLevels_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNPerformanceLevels
{
    int iSize;
    //Automatic/manual
    int iMode;
    /// Must be set to sizeof( \ref ADLODPerformanceLevels ) + sizeof( \ref ADLODPerformanceLevel ) * (ADLODParameters.iNumberOfPerformanceLevels - 1)
    int iNumberOfPerformanceLevels;
    /// Array of performance state descriptors. Must have ADLODParameters.iNumberOfPerformanceLevels elements.
    ADLODNPerformanceLevel aLevels[1];
} ADLODNPerformanceLevels;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N Fan Speed.
///
/// This structure is used to store information about Overdrive Fan control .
/// This structure is used by the ADL_OverdriveN_ODPerformanceLevels_Get() and ADL_OverdriveN_ODPerformanceLevels_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNFanControl
{
    int iMode;
    int iFanControlMode;
    int iCurrentFanSpeedMode;
    int iCurrentFanSpeed;
    int iTargetFanSpeed;
    int iTargetTemperature;
    int iMinPerformanceClock;
    int iMinFanLimit;
} ADLODNFanControl;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N power limit.
///
/// This structure is used to store information about Overdrive power limit.
/// This structure is used by the ADL_OverdriveN_ODPerformanceLevels_Get() and ADL_OverdriveN_ODPerformanceLevels_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNPowerLimitSetting
{
    int iMode;
    int iTDPLimit;
    int iMaxOperatingTemperature;
} ADLODNPowerLimitSetting;

typedef struct ADLODNPerformanceStatus
{
    int iCoreClock;
    int iMemoryClock;
    int iDCEFClock;
    int iGFXClock;
    int iUVDClock;
    int iVCEClock;
    int iGPUActivityPercent;
    int iCurrentCorePerformanceLevel;
    int iCurrentMemoryPerformanceLevel;
    int iCurrentDCEFPerformanceLevel;
    int iCurrentGFXPerformanceLevel;
    int iUVDPerformanceLevel;
    int iVCEPerformanceLevel;
    int iCurrentBusSpeed;
    int iCurrentBusLanes;
    int iMaximumBusLanes;
    int iVDDC;
    int iVDDCI;
} ADLODNPerformanceStatus;

///\brief Structure containing information about Overdrive level.
///
/// This structure is used to store information about Overdrive level.
/// This structure is used by ADLODPerformanceLevels.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNPerformanceLevelX2
{
    /// clock.
    int iClock;
    /// VDCC.
    int iVddc;
    /// enabled
    int iEnabled;
    /// MASK
    int iControl;
} ADLODNPerformanceLevelX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive N performance levels.
///
/// This structure is used to store information about Overdrive performance levels.
/// This structure is used by the ADL_OverdriveN_ODPerformanceLevels_Get() and ADL_OverdriveN_ODPerformanceLevels_Set() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLODNPerformanceLevelsX2
{
    int iSize;
    //Automatic/manual
    int iMode;
    /// Must be set to sizeof( \ref ADLODPerformanceLevels ) + sizeof( \ref ADLODPerformanceLevel ) * (ADLODParameters.iNumberOfPerformanceLevels - 1)
    int iNumberOfPerformanceLevels;
    /// Array of performance state descriptors. Must have ADLODParameters.iNumberOfPerformanceLevels elements.
    ADLODNPerformanceLevelX2 aLevels[1];
} ADLODNPerformanceLevelsX2;

typedef enum ADLODNCurrentPowerType
{
    ODN_GPU_TOTAL_POWER = 0,
    ODN_GPU_PPT_POWER,
    ODN_GPU_SOCKET_POWER,
    ODN_GPU_CHIP_POWER
} ADLODNCurrentPowerType;

// in/out: CWDDEPM_CURRENTPOWERPARAMETERS
typedef struct ADLODNCurrentPowerParameters
{
    int   size;
    ADLODNCurrentPowerType   powerType;
    int  currentPower;
} ADLODNCurrentPowerParameters;

//ODN Ext range data structure
typedef struct ADLODNExtSingleInitSetting
{
	int mode;
	int minValue;
	int maxValue;
	int step;
	int defaultValue;
} ADLODNExtSingleInitSetting;

//OD8 Ext range data structure
typedef struct ADLOD8SingleInitSetting
{
    int featureID;
    int minValue;
    int maxValue;
    int defaultValue;
} ADLOD8SingleInitSetting;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive8 initial setting
///
/// This structure is used to store information about Overdrive8 initial setting
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD8InitSetting
{
    int count;
    int overdrive8Capabilities;
    ADLOD8SingleInitSetting  od8SettingTable[OD8_COUNT];
} ADLOD8InitSetting;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive8 current setting
///
/// This structure is used to store information about Overdrive8 current setting
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLOD8CurrentSetting
{
    int count;
    int Od8SettingTable[OD8_COUNT];
} ADLOD8CurrentSetting;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Overdrive8 set setting
///
/// This structure is used to store information about Overdrive8 set setting
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLOD8SingleSetSetting
{
    int value;
    int requested;      // 0 - default , 1 - requested
    int reset;          // 0 - do not reset , 1 - reset setting back to default
} ADLOD8SingleSetSetting;

typedef struct ADLOD8SetSetting
{
    int count;
    ADLOD8SingleSetSetting  od8SettingTable[OD8_COUNT];
} ADLOD8SetSetting;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Performance Metrics data
///
/// This structure is used to store information about Performance Metrics data output
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSingleSensorData
{
    int supported;
    int  value;
} ADLSingleSensorData;

typedef struct ADLPMLogDataOutput
{
    int size;
    ADLSingleSensorData sensors[ADL_PMLOG_MAX_SENSORS];
}ADLPMLogDataOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about PPLog settings.
///
/// This structure is used to store information about PPLog settings.
/// This structure is used by the ADL2_PPLogSettings_Set() and ADL2_PPLogSettings_Get() functions.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPPLogSettings
{
    int BreakOnAssert;
    int BreakOnWarn;
    int LogEnabled;
    int LogFieldMask;
    int LogDestinations;
    int LogSeverityEnabled;
    int LogSourceMask;
    int PowerProfilingEnabled;
    int PowerProfilingTimeInterval;
}ADLPPLogSettings;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related Frames Per Second for AC and DC.
///
/// This structure is used to store information related AC and DC Frames Per Second settings
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFPSSettingsOutput
{
    /// size
    int ulSize;
    /// FPS Monitor is enabled in the AC state if 1
    int bACFPSEnabled;
    /// FPS Monitor is enabled in the DC state if 1
    int bDCFPSEnabled;
    /// Current Value of FPS Monitor in AC state
    int ulACFPSCurrent;
    /// Current Value of FPS Monitor in DC state
    int ulDCFPSCurrent;
    /// Maximum FPS Threshold allowed in PPLib for AC
    int ulACFPSMaximum;
    /// Minimum FPS Threshold allowed in PPLib for AC
    int ulACFPSMinimum;
    /// Maximum FPS Threshold allowed in PPLib for DC
    int ulDCFPSMaximum;
    /// Minimum FPS Threshold allowed in PPLib for DC
    int ulDCFPSMinimum;
} ADLFPSSettingsOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related Frames Per Second for AC and DC.
///
/// This structure is used to store information related AC and DC Frames Per Second settings
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFPSSettingsInput
{
    /// size
    int ulSize;
    /// Settings are for Global FPS (used by CCC)
    int bGlobalSettings;
    /// Current Value of FPS Monitor in AC state
    int ulACFPSCurrent;
    /// Current Value of FPS Monitor in DC state
    int ulDCFPSCurrent;
    /// Reserved
    int ulReserved[6];
} ADLFPSSettingsInput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related power management logging.
///
/// This structure is used to store support information for power management logging.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
enum { ADL_PMLOG_MAX_SUPPORTED_SENSORS = 256 };

typedef struct ADLPMLogSupportInfo
{
    /// list of sensors defined by ADL_PMLOG_SENSORS
    unsigned short usSensors[ADL_PMLOG_MAX_SUPPORTED_SENSORS];
    /// Reserved
    int ulReserved[16];
} ADLPMLogSupportInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information to start power management logging.
///
/// This structure is used as input to ADL2_Adapter_PMLog_Start
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPMLogStartInput
{
    /// list of sensors defined by ADL_PMLOG_SENSORS
    unsigned short usSensors[ADL_PMLOG_MAX_SUPPORTED_SENSORS];
    /// Sample rate in milliseconds
    unsigned long ulSampleRate;
    /// Reserved
    int ulReserved[15];
} ADLPMLogStartInput;

typedef struct ADLPMLogData
{
    /// Structure version
    unsigned int ulVersion;
    /// Current driver sample rate
    unsigned int ulActiveSampleRate;
    /// Timestamp of last update
    unsigned long long ulLastUpdated;
    /// 2D array of senesor and values
    unsigned int ulValues[ADL_PMLOG_MAX_SUPPORTED_SENSORS][2];
    /// Reserved
    unsigned int ulReserved[256];
} ADLPMLogData;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information to start power management logging.
///
/// This structure is returned as output from ADL2_Adapter_PMLog_Start
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPMLogStartOutput
{
    /// Pointer to memory address containing logging data
    union
    {
        void* pLoggingAddress;
        unsigned long long ptr_LoggingAddress;
    };
    /// Reserved
    int ulReserved[14];
} ADLPMLogStartOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information to query limts of power management logging.
///
/// This structure is returned as output from ADL2_Adapter_PMLog_SensorLimits_Get
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLPMLogSensorLimits
{
    int SensorLimits[ADL_PMLOG_MAX_SENSORS][2]; //index 0: min, 1: max
} ADLPMLogSensorLimits;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Get Error Counts Information
///
/// This structure is used to store RAS Error Counts Get Input Information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASGetErrorCountsInput
{
    unsigned int                Reserved[16];
} ADLRASGetErrorCountsInput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Get Error Counts Information
///
/// This structure is used to store RAS Error Counts Get Output Information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASGetErrorCountsOutput
{
    unsigned int                CorrectedErrors;    // includes both DRAM and SRAM ECC
    unsigned int                UnCorrectedErrors;  // includes both DRAM and SRAM ECC
    unsigned int                Reserved[14];
} ADLRASGetErrorCountsOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Get Error Counts Information
///
/// This structure is used to store RAS Error Counts Get Information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASGetErrorCounts
{
    unsigned int                InputSize;
    ADLRASGetErrorCountsInput   Input;
    unsigned int                OutputSize;
    ADLRASGetErrorCountsOutput  Output;
} ADLRASGetErrorCounts;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Error Counts Reset Information
///
/// This structure is used to store RAS Error Counts Reset Input Information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASResetErrorCountsInput
{
    unsigned int                Reserved[8];
} ADLRASResetErrorCountsInput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Error Counts Reset Information
///
/// This structure is used to store RAS Error Counts Reset Output Information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASResetErrorCountsOutput
{
    unsigned int                Reserved[8];
} ADLRASResetErrorCountsOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Error Counts Reset Information
///
/// This structure is used to store RAS Error Counts Reset Information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASResetErrorCounts
{
    unsigned int                    InputSize;
    ADLRASResetErrorCountsInput     Input;
    unsigned int                    OutputSize;
    ADLRASResetErrorCountsOutput    Output;
} ADLRASResetErrorCounts;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Error Injection information
///
/// This structure is used to store RAS Error Injection input information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASErrorInjectonInput
{
    unsigned long long Address;
    ADL_RAS_INJECTION_METHOD Value;
    ADL_RAS_BLOCK_ID BlockId;
    ADL_RAS_ERROR_TYPE InjectErrorType;
    ADL_MEM_SUB_BLOCK_ID SubBlockIndex;
    unsigned int padding[9];
} ADLRASErrorInjectonInput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Error Injection information
///
/// This structure is used to store RAS Error Injection output information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASErrorInjectionOutput
{
    unsigned int ErrorInjectionStatus;
    unsigned int padding[15];
} ADLRASErrorInjectionOutput;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related RAS Error Injection information
///
/// This structure is used to store RAS Error Injection information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLRASErrorInjection
{
    unsigned int                           InputSize;
    ADLRASErrorInjectonInput               Input;
    unsigned int                           OutputSize;
    ADLRASErrorInjectionOutput             Output;
} ADLRASErrorInjection;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about an application
///
/// This structure is used to store basic information of a recently ran or currently running application
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSGApplicationInfo
{
    /// Application file name
    wchar_t strFileName[ADL_MAX_PATH];
    /// Application file path
    wchar_t strFilePath[ADL_MAX_PATH];
    /// Application version
    wchar_t strVersion[ADL_MAX_PATH];
    /// Timestamp at which application has run
    long long int timeStamp;
    /// Holds whether the applicaition profile exists or not
    unsigned int iProfileExists;
    /// The GPU on which application runs
    unsigned int iGPUAffinity;
    /// The BDF of the GPU on which application runs
    ADLBdf GPUBdf;
} ADLSGApplicationInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related Frames Per Second for AC and DC.
///
/// This structure is used to store information related AC and DC Frames Per Second settings
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
enum { ADLPreFlipPostProcessingInfoInvalidLUTIndex = 0xFFFFFFFF };

enum ADLPreFlipPostProcessingLUTAlgorithm
{
    ADLPreFlipPostProcessingLUTAlgorithm_Default = 0,
    ADLPreFlipPostProcessingLUTAlgorithm_Full,
    ADLPreFlipPostProcessingLUTAlgorithm_Approximation
};

typedef struct ADLPreFlipPostProcessingInfo
{
    /// size
    int ulSize;
    /// Current active state
    int bEnabled;
    /// Current selected LUT index.  0xFFFFFFF returned if nothing selected.
    int ulSelectedLUTIndex;
    /// Current selected LUT Algorithm
    int ulSelectedLUTAlgorithm;
    /// Reserved
    int ulReserved[12];
} ADLPreFlipPostProcessingInfo;

typedef struct ADL_ERROR_REASON
{
    int boost; //ON, when boost is Enabled
    int delag; //ON, when delag is Enabled
    int chill; //ON, when chill is Enabled
    int proVsr; //ON, when proVsr is Enabled
}ADL_ERROR_REASON;

typedef struct ADL_ERROR_REASON2
{
    int boost; //ON, when boost is Enabled
    int delag; //ON, when delag is Enabled
    int chill; //ON, when chill is Enabled
    int proVsr; //ON, when proVsr is Enabled
    int upscale; //ON, when RSR is Enabled
}ADL_ERROR_REASON2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about DELAG Settings change reason
///
///  Elements of DELAG settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_DELAG_NOTFICATION_REASON
{
	int HotkeyChanged; //Set when Hotkey value is changed
	int GlobalEnableChanged; //Set when Global enable value is changed
	int GlobalLimitFPSChanged; //Set when Global enable value is changed
}ADL_DELAG_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about DELAG Settings
///
///  Elements of DELAG settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_DELAG_SETTINGS
{
	int Hotkey; // Hotkey value
	int GlobalEnable; //Global enable value
	int GlobalLimitFPS; //Global Limit FPS
	int GlobalLimitFPS_MinLimit; //Gloabl Limit FPS slider min limit value
	int GlobalLimitFPS_MaxLimit; //Gloabl Limit FPS slider max limit value
	int GlobalLimitFPS_Step; //Gloabl Limit FPS step  value
}ADL_DELAG_SETTINGS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about BOOST Settings change reason
///
///  Elements of BOOST settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_BOOST_NOTFICATION_REASON
{
	int HotkeyChanged; //Set when Hotkey value is changed
	int GlobalEnableChanged; //Set when Global enable value is changed
	int GlobalMinResChanged; //Set when Global min resolution value is changed
}ADL_BOOST_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about BOOST Settings
///
///  Elements of BOOST settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_BOOST_SETTINGS
{
	int Hotkey; // Hotkey value
	int GlobalEnable; //Global enable value
	int GlobalMinRes; //Gloabl Min Resolution value
	int GlobalMinRes_MinLimit; //Gloabl Min Resolution slider min limit value
	int GlobalMinRes_MaxLimit; //Gloabl Min Resolution slider max limit value
	int GlobalMinRes_Step; //Gloabl Min Resolution step  value
}ADL_BOOST_SETTINGS;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about ProVSR Settings change reason
///
///  Elements of ProVSR settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_PROVSR_NOTFICATION_REASON
{
	int HotkeyChanged; //Set when Hotkey value is changed
	int GlobalEnableChanged; //Set when Global enable value is changed
}ADL_PROVSR_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Pro VSR Settings
///
///  Elements of ProVSR settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_PROVSR_SETTINGS
{
	int Hotkey; // Hotkey value
	int GlobalEnable; //Global enable value
}ADL_PROVSR_SETTINGS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about Image Boost(OGL) Settings change reason
///
///  Elements of Image Boost settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_IMAGE_BOOST_NOTFICATION_REASON
{
    int HotkeyChanged; //Set when Hotkey value is changed
    int GlobalEnableChanged; //Set when Global enable value is changed
}ADL_IMAGE_BOOST_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about OGL IMAGE BOOST Settings
///
///  Elements of OGL IMAGE BOOST settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_IMAGE_BOOST_SETTINGS
{
    int Hotkey; // Hotkey value
    int GlobalEnable; //Global enable value
}ADL_IMAGE_BOOST_SETTINGS;

/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about RIS Settings change reason
///
///  Elements of RIS settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_RIS_NOTFICATION_REASON
{
	unsigned int GlobalEnableChanged; //Set when Global enable value is changed
	unsigned int GlobalSharpeningDegreeChanged; //Set when Global sharpening Degree value is changed
}ADL_RIS_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about RIS Settings
///
///  Elements of RIS settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_RIS_SETTINGS
{
	int GlobalEnable; //Global enable value
	int GlobalSharpeningDegree; //Global sharpening value
	int GlobalSharpeningDegree_MinLimit; //Gloabl sharpening slider min limit value
	int GlobalSharpeningDegree_MaxLimit; //Gloabl sharpening slider max limit value
	int GlobalSharpeningDegree_Step; //Gloabl sharpening step  value
}ADL_RIS_SETTINGS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about CHILL Settings change reason
///
///  Elements of Chiil settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_CHILL_NOTFICATION_REASON
{
	int HotkeyChanged; //Set when Hotkey value is changed
	int GlobalEnableChanged; //Set when Global enable value is changed
	int GlobalMinFPSChanged; //Set when Global min FPS value is changed
	int GlobalMaxFPSChanged; //Set when Global max FPS value is changed
}ADL_CHILL_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about CHILL Settings
///
///  Elements of Chill settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_CHILL_SETTINGS
{
	int Hotkey; // Hotkey value
	int GlobalEnable; //Global enable value
	int GlobalMinFPS; //Global Min FPS value
	int GlobalMaxFPS; //Global Max FPS value
	int GlobalFPS_MinLimit; //Gloabl FPS slider min limit value
	int GlobalFPS_MaxLimit; //Gloabl FPS slider max limit value
	int GlobalFPS_Step; //Gloabl FPS Slider step  value
}ADL_CHILL_SETTINGS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about DRIVERUPSCALE Settings change reason
///
///  Elements of DRIVERUPSCALE settings changed reason.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_DRIVERUPSCALE_NOTFICATION_REASON
{
    int ModeOverrideEnabledChanged;     //Set when Global min resolution value is changed
    int GlobalEnabledChanged;           //Set when Global enable value is changed
}ADL_DRIVERUPSCALE_NOTFICATION_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about DRIVERUPSCALE Settings
///
///  Elements of DRIVERUPSCALE settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_DRIVERUPSCALE_SETTINGS
{
    int ModeOverrideEnabled;
    int GlobalEnabled;
}ADL_DRIVERUPSCALE_SETTINGS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief  Structure Containing R G B values for Radeon USB LED Bar
///
/// Elements of RGB Values.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_RADEON_LED_COLOR_CONFIG
{
	unsigned short R : 8; // Red Value
	unsigned short G : 8; // Green Value
	unsigned short B : 8; // Blue Value
}ADL_RADEON_LED_COLOR_CONFIG;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure Containing All Generic LED configuration for user requested LED pattern. The driver will apply the confgiuration as requested
///
///  Elements of Radeon USB LED configuration.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_RADEON_LED_PATTERN_CONFIG_GENERIC
{
	short brightness : 8; // Brightness of LED
	short speed : 8; // Speed of LED pattern
	bool directionCounterClockWise; //Direction of LED Pattern
	ADL_RADEON_LED_COLOR_CONFIG colorConfig; // RGB value of LED pattern
	char morseCodeText[ADL_RADEON_LED_MAX_MORSE_CODE]; // Morse Code user input for Morse Code LED pattern
	char morseCodeTextOutPut[ADL_RADEON_LED_MAX_MORSE_CODE]; // Driver set output representation of Morse Code
	int  morseCodeTextOutPutLen; // Length of Morse Code output
}ADL_RADEON_LED_PATTERN_CONFIG_GENERIC;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure Containing All custom grid pattern LED configuration for user requested LED grid pattern. The driver will apply the confgiuration as requested
///
///  Elements of Radeon USB LED custom grid configuration.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_RADEON_LED_CUSTOM_LED_CONFIG
{
	short brightness : 8; // Brightness of LED
	ADL_RADEON_LED_COLOR_CONFIG colorConfig[ADL_RADEON_LED_MAX_LED_ROW_ON_GRID][ADL_RADEON_LED_MAX_LED_COLUMN_ON_GRID]; // Full grid array representation of Radeon LED to be populated by user
}ADL_RADEON_LED_CUSTOM_GRID_LED_CONFIG;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure Containing All Radeon USB LED requests and controls.
///
/// Elements of Radeon USB LED Controls.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_RADEON_LED_PATTERN_CONFIG
{
	ADL_RADEON_USB_LED_BAR_CONTROLS control; //Requested LED pattern

    union
    {
		ADL_RADEON_LED_PATTERN_CONFIG_GENERIC genericPararmeters; //Requested pattern configuration settings
		ADL_RADEON_LED_CUSTOM_GRID_LED_CONFIG customGridConfig; //Requested custom grid configuration settings
    };
}ADL_RADEON_LED_PATTERN_CONFIG;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about the graphics adapter with extended caps
///
/// This structure is used to store various information about the graphics adapter.  This
/// information can be returned to the user. Alternatively, it can be used to access various driver calls to set
/// or fetch various settings upon the user's request.
/// This AdapterInfoX2 struct extends the AdapterInfo struct in adl_structures.h
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct AdapterInfoX2
{
    /// \ALL_STRUCT_MEM

    /// Size of the structure.
    int iSize;
    /// The ADL index handle. One GPU may be associated with one or two index handles
    int iAdapterIndex;
    /// The unique device ID associated with this adapter.
    char strUDID[ADL_MAX_PATH];
    /// The BUS number associated with this adapter.
    int iBusNumber;
    /// The driver number associated with this adapter.
    int iDeviceNumber;
    /// The function number.
    int iFunctionNumber;
    /// The vendor ID associated with this adapter.
    int iVendorID;
    /// Adapter name.
    char strAdapterName[ADL_MAX_PATH];
    /// Display name. For example, "\\\\Display0"
    char strDisplayName[ADL_MAX_PATH];
    /// Present or not; 1 if present and 0 if not present.It the logical adapter is present, the display name such as \\\\.\\Display1 can be found from OS
    int iPresent;
    /// Exist or not; 1 is exist and 0 is not present.
    int iExist;
    /// Driver registry path.
    char strDriverPath[ADL_MAX_PATH];
    /// Driver registry path Ext for.
    char strDriverPathExt[ADL_MAX_PATH];
    /// PNP string from Windows.
    char strPNPString[ADL_MAX_PATH];
    /// It is generated from EnumDisplayDevices.
    int iOSDisplayIndex;
    /// The bit mask identifies the adapter info
    int iInfoMask;
    /// The bit identifies the adapter info \ref define_adapter_info
    int iInfoValue;
} AdapterInfoX2, *LPAdapterInfoX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver  gamut space , whether it is related to source or to destination, overlay or graphics
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef  struct ADLGamutReference
{
    /// mask whether it is related to source or to destination, overlay or graphics
    int      iGamutRef;
}ADLGamutReference;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported gamut spaces , capability method
///
/// This structure is used to get driver all supported gamut spaces
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLGamutInfo
{
    ///Any combination of following ADL_GAMUT_SPACE_CCIR_709 - ADL_GAMUT_SPACE_CUSTOM
    int    SupportedGamutSpace;

    ///Any combination of following ADL_WHITE_POINT_5000K - ADL_WHITE_POINT_CUSTOM
    int    SupportedWhitePoint;
} ADLGamutInfo;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver point coordinates
///
/// This structure is used to store the driver point coodinates for gamut and white point
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLPoint
{
    /// x coordinate
    int          iX;
    /// y coordinate
    int          iY;
} ADLPoint;
/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver supported gamut coordinates
///
/// This structure is used to store the driver supported supported gamut coordinates
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLGamutCoordinates
{
    /// red channel chromasity coordinate
    ADLPoint      Red;
    /// green channel chromasity coordinate
    ADLPoint      Green;
    /// blue channel chromasity coordinate
    ADLPoint      Blue;
} ADLGamutCoordinates;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about driver current gamut space , parent struct for ADLGamutCoordinates and ADLWhitePoint
/// This structure is used to get/set driver supported gamut space
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef  struct ADLGamutData
{
    ///used as mask and could be 4 options
    ///BIT_0 If flag ADL_GAMUT_REFERENCE_SOURCE is asserted set operation is related to gamut source ,
    ///if not gamut destination
    ///BIT_1 If flag ADL_GAMUT_GAMUT_VIDEO_CONTENT is asserted
    ///BIT_2,BIT_3 used as mask and could be 4 options custom (2) + predefined (2)
    ///0.  Gamut predefined,        white point predefined -> 0                | 0
    ///1.  Gamut predefined,        white point custom     -> 0                | ADL_CUSTOM_WHITE_POINT
    ///2.  White point predefined,  gamut custom           -> 0                | ADL_CUSTOM_GAMUT
    ///3.  White point custom,      gamut custom           -> ADL_CUSTOM_GAMUT | ADL_CUSTOM_WHITE_POINT
    int        iFeature;

    ///one of ADL_GAMUT_SPACE_CCIR_709 - ADL_GAMUT_SPACE_CIE_RGB
    int         iPredefinedGamut;

    ///one of ADL_WHITE_POINT_5000K - ADL_WHITE_POINT_9300K
    int         iPredefinedWhitePoint;

    ///valid when in mask avails ADL_CUSTOM_WHITE_POINT
    ADLPoint             CustomWhitePoint;

    ///valid when in mask avails ADL_CUSTOM_GAMUT
    ADLGamutCoordinates  CustomGamut;
} ADLGamutData;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing detailed timing parameters.
///
/// This structure is used to store the detailed timing parameters.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDetailedTimingX2
{
    /// Size of the structure.
    int   iSize;
    /// Timing flags. \ref define_detailed_timing_flags
    int sTimingFlags;
    /// Total width (columns).
    int sHTotal;
    /// Displayed width.
    int sHDisplay;
    /// Horizontal sync signal offset.
    int sHSyncStart;
    /// Horizontal sync signal width.
    int sHSyncWidth;
    /// Total height (rows).
    int sVTotal;
    /// Displayed height.
    int sVDisplay;
    /// Vertical sync signal offset.
    int sVSyncStart;
    /// Vertical sync signal width.
    int sVSyncWidth;
    /// Pixel clock value.
    int sPixelClock;
    /// Overscan right.
    short sHOverscanRight;
    /// Overscan left.
    short sHOverscanLeft;
    /// Overscan bottom.
    short sVOverscanBottom;
    /// Overscan top.
    short sVOverscanTop;
    short sOverscan8B;
    short sOverscanGR;
} ADLDetailedTimingX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing display mode information.
///
/// This structure is used to store the display mode information.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLDisplayModeInfoX2
{
    /// Timing standard of the current mode. \ref define_modetiming_standard
    int  iTimingStandard;
    /// Applicable timing standards for the current mode.
    int  iPossibleStandard;
    /// Refresh rate factor.
    int  iRefreshRate;
    /// Num of pixels in a row.
    int  iPelsWidth;
    /// Num of pixels in a column.
    int  iPelsHeight;
    /// Detailed timing parameters.
    ADLDetailedTimingX2  sDetailedTiming;
} ADLDisplayModeInfoX2;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about I2C.
///
/// This structure is used to store the I2C information for the current adapter.
/// This structure is used by \ref ADL_Display_WriteAndReadI2CLargePayload
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLI2CLargePayload
{
    /// Size of the structure
    int iSize;
    /// Numerical value representing hardware I2C.
    int iLine;
    /// The 7-bit I2C slave device address.
    int iAddress;
    /// The offset of the data from the address.
    int iOffset;
    /// Read from or write to slave device. \ref ADL_DL_I2C_ACTIONREAD or \ref ADL_DL_I2C_ACTIONWRITE
    int iAction;
    /// I2C clock speed in KHz.
    int iSpeed;
    /// I2C option flags.  \ref define_ADLI2CLargePayload
    int iFlags;
    /// A numerical value representing the number of bytes to be sent or received on the I2C bus.
    int iDataSize;
    /// Address of the characters which are to be sent or received on the I2C bus.
    char *pcData;
} ADLI2CLargePayload;

/// Size in bytes of the Feature Name
#define ADL_FEATURE_NAME_LENGTH 	16

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing the Multimedia Feature Name
///
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFeatureName
{
    /// The Feature Name
    char FeatureName[ADL_FEATURE_NAME_LENGTH];
}	ADLFeatureName, *LPADLFeatureName;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about MM Feature Capabilities.
///
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFeatureCaps
{
    /// The Feature Name
    ADLFeatureName	Name;
    //	char strFeatureName[ADL_FEATURE_NAME_LENGTH];

    /// Group ID. All Features in the same group are shown sequentially in the same UI Page.
    int  iGroupID;

    /// Visual ID. Places one or more features in a Group Box. If zero, no Group Box is added.
    int  iVisualID;

    /// Page ID. All Features with the same Page ID value are shown together on the same UI page.
    int iPageID;

    /// Feature Property Mask. Indicates which are the valid bits for iFeatureProperties.
    int iFeatureMask;

    /// Feature Property Values. See definitions for ADL_FEATURE_PROPERTIES_XXX
    int  iFeatureProperties;

    /// Apperance of the User-Controlled Boolean.
    int  iControlType;

    /// Style of the User-Controlled Boolean.
    int  iControlStyle;

    /// Apperance of the Adjustment Controls.
    int  iAdjustmentType;

    /// Style of the Adjustment Controls.
    int  iAdjustmentStyle;

    /// Default user-controlled boolean value. Valid only if ADLFeatureCaps supports user-controlled boolean.
    int bDefault;

    /// Minimum integer value. Valid only if ADLFeatureCaps indicates support for integers.
    int iMin;

    /// Maximum integer value. Valid only if ADLFeatureCaps indicates support for integers.
    int iMax;

    /// Step integer value. Valid only if ADLFeatureCaps indicates support for integers.
    int iStep;

    /// Default integer value. Valid only if ADLFeatureCaps indicates support for integers.
    int iDefault;

    /// Minimum float value. Valid only if ADLFeatureCaps indicates support for floats.
    float fMin;

    /// Maximum float value. Valid only if ADLFeatureCaps indicates support for floats.
    float fMax;

    /// Step float value. Valid only if ADLFeatureCaps indicates support for floats.
    float fStep;

    /// Default float value. Valid only if ADLFeatureCaps indicates support for floats.
    float fDefault;

    /// The Mask for available bits for enumerated values.(If ADLFeatureCaps supports ENUM values)
    int EnumMask;
} ADLFeatureCaps, *LPADLFeatureCaps;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about MM Feature Values.
///
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLFeatureValues
{
    /// The Feature Name
    ADLFeatureName	Name;
    //	char strFeatureName[ADL_FEATURE_NAME_LENGTH];

    /// User controlled Boolean current value. Valid only if ADLFeatureCaps supports Boolean.
    int bCurrent;

    /// Current integer value. Valid only if ADLFeatureCaps indicates support for integers.
    int iCurrent;

    /// Current float value. Valid only if ADLFeatureCaps indicates support for floats.
    float fCurrent;

    /// The States for the available bits for enumerated values.
    int EnumStates;
} ADLFeatureValues, *LPADLFeatureValues;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing HDCP Settings info
///
/// This structure is used to store the HDCP settings of a
/// display
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLHDCPSettings
{
	int iHDCPProtectionVersion; // Version, starting from 1
	int iHDCPCaps; //Caps used to ensure at least one protection scheme is supported, 1 is HDCP1X and 2 is HDCP22
	int iAllowAll; //Allow all is true, disable all is false
	int iHDCPVale;
	int iHDCPMask;
} ADLHDCPSettings;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing Mantle App  info
///
/// This structure is used to store the Mantle Driver information
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ADLMantleAppInfo
{
	/// mantle api version
	int   apiVersion;
	/// mantle driver version
	long   driverVersion;
	/// mantle vendroe id
	long   vendorId;
	/// mantle device id
	long   deviceId;
	/// mantle gpu type;
	int     gpuType;
	/// gpu name
	char     gpuName[256];
	/// mem size
	int     maxMemRefsPerSubmission;
	/// virtual mem size
	long long virtualMemPageSize;
	/// mem update
	long long maxInlineMemoryUpdateSize;
	/// bound descriptot
	long     maxBoundDescriptorSets;
	/// thread group size
	long     maxThreadGroupSize;
	/// time stamp frequency
	long  long timestampFrequency;
	/// color target
	long     multiColorTargetClears;
}ADLMantleAppInfo, *LPADLMantleAppInfo;

////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about SDIData
///This structure is used to store information about the state of the SDI whether it is on
///or off and the current size of the segment or aperture size.
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSDIData
{
	/// The SDI state, ADL_SDI_ON or ADL_SDI_OFF, for the current SDI mode
	int iSDIState;
	/// Size of the memory segment for SDI (in MB).
	int iSizeofSDISegment;
} ADLSDIData, *LPADLSDIData;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about FRTCPRO Settings
///
///  Elements of FRTCPRO settings.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_FRTCPRO_Settings
{
    int DefaultState;              //The default status for FRTC pro
    int CurrentState;              //The current enable/disable status for FRTC pro
    unsigned int DefaultValue;     //The default FPS value for FRTC pro.
    unsigned int CurrentValue;      //The current FPS value for FRTC pro.
    unsigned int maxSupportedFps;      //The max value for FRTC pro.
    unsigned int minSupportedFps;      //The min value for FRTC pro.
}ADL_FRTCPRO_Settings, *LPADLFRTCProSettings;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information about FRTCPRO Settings changed reason
///
///  Reason of FRTCPRO changed.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_FRTCPRO_CHANGED_REASON
{
    int StateChanged;               // FRTCPro state changed
    int ValueChanged;               // FRTCPro value changed
}ADL_FRTCPRO_CHANGED_REASON;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure containing the display mode definition used per controller.
///
/// This structure is used to store the display mode definition used per controller.
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADL_DL_DISPLAY_MODE
{
    int  iPelsHeight;                      // Vertical resolution (in pixels).
    int  iPelsWidth;                       // Horizontal resolution (in pixels).
    int  iBitsPerPel;                      // Color depth.
    int  iDisplayFrequency;                // Refresh rate.
} ADL_DL_DISPLAY_MODE;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief Structure containing information related DCE support
///
/// This structure is used to store a bit vector of possible DCE support
///
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef union _ADLDCESupport
{
    struct
    {
        unsigned int PrePhasis : 1;
        unsigned int voltageSwing : 1;
        unsigned int reserved : 30;
    }bits;
    unsigned int u32All;
}ADLDCESupport;

/////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Structure for Smart shift 2.0 settings
///
/// This structure is used to return the smart shift settings
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef struct ADLSmartShiftSettings
{
	int iMinRange;
	int iMaxRange;
	int iDefaultMode; //Refer to CWDDEPM_ODN_CONTROL_TYPE
	int iDefaultValue;
	int iCurrentMode;
	int iCurrentValue;
    int iFlags; //refer to define_smartshift_bits
}ADLSmartShiftSettings, *LPADLSmartShiftSettings;
#endif /* ADL_STRUCTURES_H_ */
