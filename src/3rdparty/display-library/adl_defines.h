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

/// \file adl_defines.h
/// \brief Contains all definitions exposed by ADL for \ALL platforms.\n <b>Included in ADL SDK</b>
///
/// This file contains all definitions used by ADL.
/// The ADL definitions include the following:
/// \li ADL error codes
/// \li Enumerations for the ADLDisplayInfo structure
/// \li Maximum limits
///

#ifndef ADL_DEFINES_H_
#define ADL_DEFINES_H_

/// \defgroup DEFINES Constants and Definitions
/// @{

/// \defgroup define_misc Miscellaneous Constant Definitions
/// @{

/// \name General Definitions
/// @{

/// Defines ADL_TRUE
#define ADL_TRUE    1
/// Defines ADL_FALSE
#define ADL_FALSE        0

/// Defines the maximum string length
#define ADL_MAX_CHAR                                    4096
/// Defines the maximum string length
#define ADL_MAX_PATH                                    256
/// Defines the maximum number of supported adapters
#define ADL_MAX_ADAPTERS                               250
/// Defines the maxumum number of supported displays
#define ADL_MAX_DISPLAYS                                150
/// Defines the maxumum string length for device name
#define ADL_MAX_DEVICENAME                                32
/// Defines for all adapters
#define ADL_ADAPTER_INDEX_ALL                            -1
///    Defines APIs with iOption none
#define ADL_MAIN_API_OPTION_NONE                        0
/// @}

/// \name Definitions for iOption parameter used by
/// ADL_Display_DDCBlockAccess_Get()
/// @{

/// Switch to DDC line 2 before sending the command to the display.
#define ADL_DDC_OPTION_SWITCHDDC2              0x00000001
/// Save command in the registry under a unique key, corresponding to parameter \b iCommandIndex
#define ADL_DDC_OPTION_RESTORECOMMAND 0x00000002
/// Combine write-read DDC block access command.
#define ADL_DDC_OPTION_COMBOWRITEREAD 0x00000010
/// Direct DDC access to the immediate device connected to graphics card.
/// MST with this option set: DDC command is sent to first branch.
/// MST with this option not set: DDC command is sent to the end node sink device.
#define ADL_DDC_OPTION_SENDTOIMMEDIATEDEVICE 0x00000020
/// @}

/// \name Values for
/// ADLI2C.iAction used with ADL_Display_WriteAndReadI2C()
/// @{

#define ADL_DL_I2C_ACTIONREAD								0x00000001
#define ADL_DL_I2C_ACTIONWRITE								0x00000002
#define ADL_DL_I2C_ACTIONREAD_REPEATEDSTART    				0x00000003
#define ADL_DL_I2C_ACTIONIS_PRESENT							0x00000004
/// @}


/// @}        //Misc

/// \defgroup define_adl_results Result Codes
/// This group of definitions are the various results returned by all ADL functions \n
/// @{
/// All OK, but need to wait
#define ADL_OK_WAIT                4
/// All OK, but need restart
#define ADL_OK_RESTART                3
/// All OK but need mode change
#define ADL_OK_MODE_CHANGE            2
/// All OK, but with warning
#define ADL_OK_WARNING                1
/// ADL function completed successfully
#define ADL_OK                    0
/// Generic Error. Most likely one or more of the Escape calls to the driver failed!
#define ADL_ERR                    -1
/// ADL not initialized
#define ADL_ERR_NOT_INIT            -2
/// One of the parameter passed is invalid
#define ADL_ERR_INVALID_PARAM            -3
/// One of the parameter size is invalid
#define ADL_ERR_INVALID_PARAM_SIZE        -4
/// Invalid ADL index passed
#define ADL_ERR_INVALID_ADL_IDX            -5
/// Invalid controller index passed
#define ADL_ERR_INVALID_CONTROLLER_IDX        -6
/// Invalid display index passed
#define ADL_ERR_INVALID_DIPLAY_IDX        -7
/// Function  not supported by the driver
#define ADL_ERR_NOT_SUPPORTED            -8
/// Null Pointer error
#define ADL_ERR_NULL_POINTER            -9
/// Call can't be made due to disabled adapter
#define ADL_ERR_DISABLED_ADAPTER        -10
/// Invalid Callback
#define ADL_ERR_INVALID_CALLBACK            -11
/// Display Resource conflict
#define ADL_ERR_RESOURCE_CONFLICT                -12
//Failed to update some of the values. Can be returned by set request that include multiple values if not all values were successfully committed.
#define ADL_ERR_SET_INCOMPLETE                 -20
/// There's no Linux XDisplay in Linux Console environment
#define ADL_ERR_NO_XDISPLAY                    -21
/// escape call failed becuse of incompatiable driver found in driver store
#define ADL_ERR_CALL_TO_INCOMPATIABLE_DRIVER            -22
/// not running as administrator
#define ADL_ERR_NO_ADMINISTRATOR_PRIVILEGES            -23
/// Feature Sync Start api is not called yet
#define ADL_ERR_FEATURESYNC_NOT_STARTED            -24
/// Adapter is in an invalid power state
#define ADL_ERR_INVALID_POWER_STATE             -25

/// @}
/// </A>

/// \defgroup define_display_type Display Type
/// Define Monitor/CRT display type
/// @{
/// Define Monitor display type
#define ADL_DT_MONITOR                  0
/// Define TV display type
#define ADL_DT_TELEVISION                    1
/// Define LCD display type
#define ADL_DT_LCD_PANEL                       2
/// Define DFP display type
#define ADL_DT_DIGITAL_FLAT_PANEL        3
/// Define Componment Video display type
#define ADL_DT_COMPONENT_VIDEO               4
/// Define Projector display type
#define ADL_DT_PROJECTOR                       5
/// @}

/// \defgroup define_display_connection_type Display Connection Type
/// @{
/// Define unknown display output type
#define ADL_DOT_UNKNOWN                0
/// Define composite display output type
#define ADL_DOT_COMPOSITE            1
/// Define SVideo display output type
#define ADL_DOT_SVIDEO                2
/// Define analog display output type
#define ADL_DOT_ANALOG                3
/// Define digital display output type
#define ADL_DOT_DIGITAL                4
/// @}

/// \defgroup define_color_type Display Color Type and Source
/// Define  Display Color Type and Source
/// @{
#define ADL_DISPLAY_COLOR_BRIGHTNESS    (1 << 0)
#define ADL_DISPLAY_COLOR_CONTRAST    (1 << 1)
#define ADL_DISPLAY_COLOR_SATURATION    (1 << 2)
#define ADL_DISPLAY_COLOR_HUE        (1 << 3)
#define ADL_DISPLAY_COLOR_TEMPERATURE    (1 << 4)

/// Color Temperature Source is EDID
#define ADL_DISPLAY_COLOR_TEMPERATURE_SOURCE_EDID    (1 << 5)
/// Color Temperature Source is User
#define ADL_DISPLAY_COLOR_TEMPERATURE_SOURCE_USER    (1 << 6)
/// @}

/// \defgroup define_adjustment_capabilities Display Adjustment Capabilities
/// Display adjustment capabilities values.  Returned by ADL_Display_AdjustCaps_Get
/// @{
#define ADL_DISPLAY_ADJUST_OVERSCAN        (1 << 0)
#define ADL_DISPLAY_ADJUST_VERT_POS        (1 << 1)
#define ADL_DISPLAY_ADJUST_HOR_POS        (1 << 2)
#define ADL_DISPLAY_ADJUST_VERT_SIZE        (1 << 3)
#define ADL_DISPLAY_ADJUST_HOR_SIZE        (1 << 4)
#define ADL_DISPLAY_ADJUST_SIZEPOS        (ADL_DISPLAY_ADJUST_VERT_POS | ADL_DISPLAY_ADJUST_HOR_POS | ADL_DISPLAY_ADJUST_VERT_SIZE | ADL_DISPLAY_ADJUST_HOR_SIZE)
#define ADL_DISPLAY_CUSTOMMODES            (1<<5)
#define ADL_DISPLAY_ADJUST_UNDERSCAN        (1<<6)
/// @}

///Down-scale support
#define ADL_DISPLAY_CAPS_DOWNSCALE        (1 << 0)

/// Sharpness support
#define ADL_DISPLAY_CAPS_SHARPNESS      (1 << 0)

/// \defgroup define_desktop_config Desktop Configuration Flags
/// These flags are used by ADL_DesktopConfig_xxx
/// \deprecated This API has been deprecated because it was only used for RandR 1.1 (Red Hat 5.x) distributions which is now not supported.
/// @{
#define ADL_DESKTOPCONFIG_UNKNOWN    0          /* UNKNOWN desktop config   */
#define ADL_DESKTOPCONFIG_SINGLE     (1 <<  0)    /* Single                   */
#define ADL_DESKTOPCONFIG_CLONE      (1 <<  2)    /* Clone                    */
#define ADL_DESKTOPCONFIG_BIGDESK_H  (1 <<  4)    /* Big Desktop Horizontal   */
#define ADL_DESKTOPCONFIG_BIGDESK_V  (1 <<  5)    /* Big Desktop Vertical     */
#define ADL_DESKTOPCONFIG_BIGDESK_HR (1 <<  6)    /* Big Desktop Reverse Horz */
#define ADL_DESKTOPCONFIG_BIGDESK_VR (1 <<  7)    /* Big Desktop Reverse Vert */
#define ADL_DESKTOPCONFIG_RANDR12    (1 <<  8)    /* RandR 1.2 Multi-display */
/// @}

/// needed for ADLDDCInfo structure
#define ADL_MAX_DISPLAY_NAME                                256

/// \defgroup define_edid_flags Values for ulDDCInfoFlag
/// defines for ulDDCInfoFlag EDID flag
/// @{
#define ADL_DISPLAYDDCINFOEX_FLAG_PROJECTORDEVICE       (1 << 0)
#define ADL_DISPLAYDDCINFOEX_FLAG_EDIDEXTENSION         (1 << 1)
#define ADL_DISPLAYDDCINFOEX_FLAG_DIGITALDEVICE         (1 << 2)
#define ADL_DISPLAYDDCINFOEX_FLAG_HDMIAUDIODEVICE       (1 << 3)
#define ADL_DISPLAYDDCINFOEX_FLAG_SUPPORTS_AI           (1 << 4)
#define ADL_DISPLAYDDCINFOEX_FLAG_SUPPORT_xvYCC601      (1 << 5)
#define ADL_DISPLAYDDCINFOEX_FLAG_SUPPORT_xvYCC709      (1 << 6)
/// @}

/// \defgroup define_displayinfo_connector Display Connector Type
/// defines for ADLDisplayInfo.iDisplayConnector
/// @{
#define ADL_DISPLAY_CONTYPE_UNKNOWN                 0
#define ADL_DISPLAY_CONTYPE_VGA                     1
#define ADL_DISPLAY_CONTYPE_DVI_D                   2
#define ADL_DISPLAY_CONTYPE_DVI_I                   3
#define ADL_DISPLAY_CONTYPE_ATICVDONGLE_NTSC        4
#define ADL_DISPLAY_CONTYPE_ATICVDONGLE_JPN         5
#define ADL_DISPLAY_CONTYPE_ATICVDONGLE_NONI2C_JPN  6
#define ADL_DISPLAY_CONTYPE_ATICVDONGLE_NONI2C_NTSC 7
#define ADL_DISPLAY_CONTYPE_PROPRIETARY                8
#define ADL_DISPLAY_CONTYPE_HDMI_TYPE_A             10
#define ADL_DISPLAY_CONTYPE_HDMI_TYPE_B             11
#define ADL_DISPLAY_CONTYPE_SVIDEO                   12
#define ADL_DISPLAY_CONTYPE_COMPOSITE               13
#define ADL_DISPLAY_CONTYPE_RCA_3COMPONENT          14
#define ADL_DISPLAY_CONTYPE_DISPLAYPORT             15
#define ADL_DISPLAY_CONTYPE_EDP                     16
#define ADL_DISPLAY_CONTYPE_WIRELESSDISPLAY         17
#define ADL_DISPLAY_CONTYPE_USB_TYPE_C              18
/// @}

/// TV Capabilities and Standards
/// \defgroup define_tv_caps TV Capabilities and Standards
/// \deprecated Dropping support for TV displays
/// @{
#define ADL_TV_STANDARDS            (1 << 0)
#define ADL_TV_SCART                (1 << 1)

/// TV Standards Definitions
#define ADL_STANDARD_NTSC_M        (1 << 0)
#define ADL_STANDARD_NTSC_JPN        (1 << 1)
#define ADL_STANDARD_NTSC_N        (1 << 2)
#define ADL_STANDARD_PAL_B        (1 << 3)
#define ADL_STANDARD_PAL_COMB_N        (1 << 4)
#define ADL_STANDARD_PAL_D        (1 << 5)
#define ADL_STANDARD_PAL_G        (1 << 6)
#define ADL_STANDARD_PAL_H        (1 << 7)
#define ADL_STANDARD_PAL_I        (1 << 8)
#define ADL_STANDARD_PAL_K        (1 << 9)
#define ADL_STANDARD_PAL_K1        (1 << 10)
#define ADL_STANDARD_PAL_L        (1 << 11)
#define ADL_STANDARD_PAL_M        (1 << 12)
#define ADL_STANDARD_PAL_N        (1 << 13)
#define ADL_STANDARD_PAL_SECAM_D    (1 << 14)
#define ADL_STANDARD_PAL_SECAM_K    (1 << 15)
#define ADL_STANDARD_PAL_SECAM_K1    (1 << 16)
#define ADL_STANDARD_PAL_SECAM_L    (1 << 17)
/// @}


/// \defgroup define_video_custom_mode Video Custom Mode flags
/// Component Video Custom Mode flags.  This is used by the iFlags parameter in ADLCustomMode
/// @{
#define ADL_CUSTOMIZEDMODEFLAG_MODESUPPORTED    (1 << 0)
#define ADL_CUSTOMIZEDMODEFLAG_NOTDELETETABLE    (1 << 1)
#define ADL_CUSTOMIZEDMODEFLAG_INSERTBYDRIVER    (1 << 2)
#define ADL_CUSTOMIZEDMODEFLAG_INTERLACED    (1 << 3)
#define ADL_CUSTOMIZEDMODEFLAG_BASEMODE        (1 << 4)
/// @}

/// \defgroup define_ddcinfoflag Values used for DDCInfoFlag
/// ulDDCInfoFlag field values used by the ADLDDCInfo structure
/// @{
#define ADL_DISPLAYDDCINFOEX_FLAG_PROJECTORDEVICE    (1 << 0)
#define ADL_DISPLAYDDCINFOEX_FLAG_EDIDEXTENSION        (1 << 1)
#define ADL_DISPLAYDDCINFOEX_FLAG_DIGITALDEVICE        (1 << 2)
#define ADL_DISPLAYDDCINFOEX_FLAG_HDMIAUDIODEVICE    (1 << 3)
#define ADL_DISPLAYDDCINFOEX_FLAG_SUPPORTS_AI        (1 << 4)
#define ADL_DISPLAYDDCINFOEX_FLAG_SUPPORT_xvYCC601    (1 << 5)
#define ADL_DISPLAYDDCINFOEX_FLAG_SUPPORT_xvYCC709    (1 << 6)
/// @}

/// \defgroup define_cv_dongle Values used by ADL_CV_DongleSettings_xxx
/// The following is applicable to ADL_DISPLAY_CONTYPE_ATICVDONGLE_JP and ADL_DISPLAY_CONTYPE_ATICVDONGLE_NONI2C_D only
/// \deprecated Dropping support for Component Video displays
/// @{
#define ADL_DISPLAY_CV_DONGLE_D1          (1 << 0)
#define ADL_DISPLAY_CV_DONGLE_D2          (1 << 1)
#define ADL_DISPLAY_CV_DONGLE_D3          (1 << 2)
#define ADL_DISPLAY_CV_DONGLE_D4          (1 << 3)
#define ADL_DISPLAY_CV_DONGLE_D5          (1 << 4)

/// The following is applicable to ADL_DISPLAY_CONTYPE_ATICVDONGLE_NA and ADL_DISPLAY_CONTYPE_ATICVDONGLE_NONI2C only

#define ADL_DISPLAY_CV_DONGLE_480I        (1 << 0)
#define ADL_DISPLAY_CV_DONGLE_480P        (1 << 1)
#define ADL_DISPLAY_CV_DONGLE_540P        (1 << 2)
#define ADL_DISPLAY_CV_DONGLE_720P        (1 << 3)
#define ADL_DISPLAY_CV_DONGLE_1080I       (1 << 4)
#define ADL_DISPLAY_CV_DONGLE_1080P       (1 << 5)
#define ADL_DISPLAY_CV_DONGLE_16_9        (1 << 6)
#define ADL_DISPLAY_CV_DONGLE_720P50      (1 << 7)
#define ADL_DISPLAY_CV_DONGLE_1080I25     (1 << 8)
#define ADL_DISPLAY_CV_DONGLE_576I25      (1 << 9)
#define ADL_DISPLAY_CV_DONGLE_576P50      (1 << 10)
#define ADL_DISPLAY_CV_DONGLE_1080P24      (1 << 11)
#define ADL_DISPLAY_CV_DONGLE_1080P25      (1 << 12)
#define ADL_DISPLAY_CV_DONGLE_1080P30      (1 << 13)
#define ADL_DISPLAY_CV_DONGLE_1080P50      (1 << 14)
/// @}

/// \defgroup define_formats_ovr    Formats Override Settings
/// Display force modes flags
/// @{
///
#define ADL_DISPLAY_FORMAT_FORCE_720P        0x00000001
#define ADL_DISPLAY_FORMAT_FORCE_1080I        0x00000002
#define ADL_DISPLAY_FORMAT_FORCE_1080P        0x00000004
#define ADL_DISPLAY_FORMAT_FORCE_720P50        0x00000008
#define ADL_DISPLAY_FORMAT_FORCE_1080I25    0x00000010
#define ADL_DISPLAY_FORMAT_FORCE_576I25        0x00000020
#define ADL_DISPLAY_FORMAT_FORCE_576P50        0x00000040
#define ADL_DISPLAY_FORMAT_FORCE_1080P24    0x00000080
#define ADL_DISPLAY_FORMAT_FORCE_1080P25    0x00000100
#define ADL_DISPLAY_FORMAT_FORCE_1080P30    0x00000200
#define ADL_DISPLAY_FORMAT_FORCE_1080P50    0x00000400

///< Below are \b EXTENDED display mode flags

#define ADL_DISPLAY_FORMAT_CVDONGLEOVERIDE  0x00000001
#define ADL_DISPLAY_FORMAT_CVMODEUNDERSCAN  0x00000002
#define ADL_DISPLAY_FORMAT_FORCECONNECT_SUPPORTED  0x00000004
#define ADL_DISPLAY_FORMAT_RESTRICT_FORMAT_SELECTION 0x00000008
#define ADL_DISPLAY_FORMAT_SETASPECRATIO 0x00000010
#define ADL_DISPLAY_FORMAT_FORCEMODES    0x00000020
#define ADL_DISPLAY_FORMAT_LCDRTCCOEFF   0x00000040
/// @}

/// Defines used by OD5
#define ADL_PM_PARAM_DONT_CHANGE    0

/// The following defines Bus types
/// @{
#define ADL_BUSTYPE_PCI           0       /* PCI bus                          */
#define ADL_BUSTYPE_AGP           1       /* AGP bus                          */
#define ADL_BUSTYPE_PCIE          2       /* PCI Express bus                  */
#define ADL_BUSTYPE_PCIE_GEN2     3       /* PCI Express 2nd generation bus   */
#define ADL_BUSTYPE_PCIE_GEN3     4       /* PCI Express 3rd generation bus   */
#define ADL_BUSTYPE_PCIE_GEN4     5       /* PCI Express 4th generation bus   */
/// @}

/// \defgroup define_ws_caps    Workstation Capabilities
/// Workstation values
/// @{

/// This value indicates that the workstation card supports active stereo though stereo output connector
#define ADL_STEREO_SUPPORTED        (1 << 2)
/// This value indicates that the workstation card supports active stereo via "blue-line"
#define ADL_STEREO_BLUE_LINE        (1 << 3)
/// This value is used to turn off stereo mode.
#define ADL_STEREO_OFF                0
/// This value indicates that the workstation card supports active stereo.  This is also used to set the stereo mode to active though the stereo output connector
#define ADL_STEREO_ACTIVE             (1 << 1)
/// This value indicates that the workstation card supports auto-stereo monitors with horizontal interleave. This is also used to set the stereo mode to use the auto-stereo monitor with horizontal interleave
#define ADL_STEREO_AUTO_HORIZONTAL    (1 << 30)
/// This value indicates that the workstation card supports auto-stereo monitors with vertical interleave. This is also used to set the stereo mode to use the auto-stereo monitor with vertical interleave
#define ADL_STEREO_AUTO_VERTICAL    (1 << 31)
/// This value indicates that the workstation card supports passive stereo, ie. non stereo sync
#define ADL_STEREO_PASSIVE              (1 << 6)
/// This value indicates that the workstation card supports auto-stereo monitors with vertical interleave. This is also used to set the stereo mode to use the auto-stereo monitor with vertical interleave
#define ADL_STEREO_PASSIVE_HORIZ        (1 << 7)
/// This value indicates that the workstation card supports auto-stereo monitors with vertical interleave. This is also used to set the stereo mode to use the auto-stereo monitor with vertical interleave
#define ADL_STEREO_PASSIVE_VERT         (1 << 8)
/// This value indicates that the workstation card supports auto-stereo monitors with Samsung.
#define ADL_STEREO_AUTO_SAMSUNG        (1 << 11)
/// This value indicates that the workstation card supports auto-stereo monitors with Tridility.
#define ADL_STEREO_AUTO_TSL         (1 << 12)
/// This value indicates that the workstation card supports DeepBitDepth (10 bpp)
#define ADL_DEEPBITDEPTH_10BPP_SUPPORTED   (1 << 5)

/// This value indicates that the workstation supports 8-Bit Grayscale
#define ADL_8BIT_GREYSCALE_SUPPORTED   (1 << 9)
/// This value indicates that the workstation supports CUSTOM TIMING
#define ADL_CUSTOM_TIMING_SUPPORTED   (1 << 10)

/// Load balancing is supported.
#define ADL_WORKSTATION_LOADBALANCING_SUPPORTED         0x00000001
/// Load balancing is available.
#define ADL_WORKSTATION_LOADBALANCING_AVAILABLE         0x00000002

/// Load balancing is disabled.
#define ADL_WORKSTATION_LOADBALANCING_DISABLED          0x00000000
/// Load balancing is Enabled.
#define ADL_WORKSTATION_LOADBALANCING_ENABLED           0x00000001



/// @}

/// \defgroup define_adapterspeed speed setting from the adapter
/// @{
#define ADL_CONTEXT_SPEED_UNFORCED        0        /* default asic running speed */
#define ADL_CONTEXT_SPEED_FORCEHIGH        1        /* asic running speed is forced to high */
#define ADL_CONTEXT_SPEED_FORCELOW        2        /* asic running speed is forced to low */

#define ADL_ADAPTER_SPEEDCAPS_SUPPORTED        (1 << 0)    /* change asic running speed setting is supported */
/// @}

/// \defgroup define_glsync Genlock related values
/// GL-Sync port types (unique values)
/// @{
/// Unknown port of GL-Sync module
#define ADL_GLSYNC_PORT_UNKNOWN        0
/// BNC port of of GL-Sync module
#define ADL_GLSYNC_PORT_BNC            1
/// RJ45(1) port of of GL-Sync module
#define ADL_GLSYNC_PORT_RJ45PORT1    2
/// RJ45(2) port of of GL-Sync module
#define ADL_GLSYNC_PORT_RJ45PORT2    3

// GL-Sync Genlock settings mask (bit-vector)

/// None of the ADLGLSyncGenlockConfig members are valid
#define ADL_GLSYNC_CONFIGMASK_NONE                0
/// The ADLGLSyncGenlockConfig.lSignalSource member is valid
#define ADL_GLSYNC_CONFIGMASK_SIGNALSOURCE        (1 << 0)
/// The ADLGLSyncGenlockConfig.iSyncField member is valid
#define ADL_GLSYNC_CONFIGMASK_SYNCFIELD            (1 << 1)
/// The ADLGLSyncGenlockConfig.iSampleRate member is valid
#define ADL_GLSYNC_CONFIGMASK_SAMPLERATE        (1 << 2)
/// The ADLGLSyncGenlockConfig.lSyncDelay member is valid
#define ADL_GLSYNC_CONFIGMASK_SYNCDELAY            (1 << 3)
/// The ADLGLSyncGenlockConfig.iTriggerEdge member is valid
#define ADL_GLSYNC_CONFIGMASK_TRIGGEREDGE        (1 << 4)
/// The ADLGLSyncGenlockConfig.iScanRateCoeff member is valid
#define ADL_GLSYNC_CONFIGMASK_SCANRATECOEFF        (1 << 5)
/// The ADLGLSyncGenlockConfig.lFramelockCntlVector member is valid
#define ADL_GLSYNC_CONFIGMASK_FRAMELOCKCNTL        (1 << 6)


// GL-Sync Framelock control mask (bit-vector)

/// Framelock is disabled
#define ADL_GLSYNC_FRAMELOCKCNTL_NONE            0
/// Framelock is enabled
#define ADL_GLSYNC_FRAMELOCKCNTL_ENABLE            ( 1 << 0)

#define ADL_GLSYNC_FRAMELOCKCNTL_DISABLE        ( 1 << 1)
#define ADL_GLSYNC_FRAMELOCKCNTL_SWAP_COUNTER_RESET    ( 1 << 2)
#define ADL_GLSYNC_FRAMELOCKCNTL_SWAP_COUNTER_ACK    ( 1 << 3)
#define ADL_GLSYNC_FRAMELOCKCNTL_VERSION_KMD    (1 << 4)

#define ADL_GLSYNC_FRAMELOCKCNTL_STATE_ENABLE        ( 1 << 0)
#define ADL_GLSYNC_FRAMELOCKCNTL_STATE_KMD        (1 << 4)

// GL-Sync Framelock counters mask (bit-vector)
#define ADL_GLSYNC_COUNTER_SWAP                ( 1 << 0 )

// GL-Sync Signal Sources (unique values)

/// GL-Sync signal source is undefined
#define ADL_GLSYNC_SIGNALSOURCE_UNDEFINED    0x00000100
/// GL-Sync signal source is Free Run
#define ADL_GLSYNC_SIGNALSOURCE_FREERUN      0x00000101
/// GL-Sync signal source is the BNC GL-Sync port
#define ADL_GLSYNC_SIGNALSOURCE_BNCPORT      0x00000102
/// GL-Sync signal source is the RJ45(1) GL-Sync port
#define ADL_GLSYNC_SIGNALSOURCE_RJ45PORT1    0x00000103
/// GL-Sync signal source is the RJ45(2) GL-Sync port
#define ADL_GLSYNC_SIGNALSOURCE_RJ45PORT2    0x00000104


// GL-Sync Signal Types (unique values)

/// GL-Sync signal type is unknown
#define ADL_GLSYNC_SIGNALTYPE_UNDEFINED      0
/// GL-Sync signal type is 480I
#define ADL_GLSYNC_SIGNALTYPE_480I           1
/// GL-Sync signal type is 576I
#define ADL_GLSYNC_SIGNALTYPE_576I           2
/// GL-Sync signal type is 480P
#define ADL_GLSYNC_SIGNALTYPE_480P           3
/// GL-Sync signal type is 576P
#define ADL_GLSYNC_SIGNALTYPE_576P           4
/// GL-Sync signal type is 720P
#define ADL_GLSYNC_SIGNALTYPE_720P           5
/// GL-Sync signal type is 1080P
#define ADL_GLSYNC_SIGNALTYPE_1080P          6
/// GL-Sync signal type is 1080I
#define ADL_GLSYNC_SIGNALTYPE_1080I          7
/// GL-Sync signal type is SDI
#define ADL_GLSYNC_SIGNALTYPE_SDI            8
/// GL-Sync signal type is TTL
#define ADL_GLSYNC_SIGNALTYPE_TTL            9
/// GL_Sync signal type is Analog
#define ADL_GLSYNC_SIGNALTYPE_ANALOG        10

// GL-Sync Sync Field options (unique values)

///GL-Sync sync field option is undefined
#define ADL_GLSYNC_SYNCFIELD_UNDEFINED        0
///GL-Sync sync field option is Sync to Field 1 (used for Interlaced signal types)
#define ADL_GLSYNC_SYNCFIELD_BOTH            1
///GL-Sync sync field option is Sync to Both fields (used for Interlaced signal types)
#define ADL_GLSYNC_SYNCFIELD_1                2


// GL-Sync trigger edge options (unique values)

/// GL-Sync trigger edge is undefined
#define ADL_GLSYNC_TRIGGEREDGE_UNDEFINED     0
/// GL-Sync trigger edge is the rising edge
#define ADL_GLSYNC_TRIGGEREDGE_RISING        1
/// GL-Sync trigger edge is the falling edge
#define ADL_GLSYNC_TRIGGEREDGE_FALLING       2
/// GL-Sync trigger edge is both the rising and the falling edge
#define ADL_GLSYNC_TRIGGEREDGE_BOTH          3


// GL-Sync scan rate coefficient/multiplier options (unique values)

/// GL-Sync scan rate coefficient/multiplier is undefined
#define ADL_GLSYNC_SCANRATECOEFF_UNDEFINED   0
/// GL-Sync scan rate coefficient/multiplier is 5
#define ADL_GLSYNC_SCANRATECOEFF_x5          1
/// GL-Sync scan rate coefficient/multiplier is 4
#define ADL_GLSYNC_SCANRATECOEFF_x4          2
/// GL-Sync scan rate coefficient/multiplier is 3
#define ADL_GLSYNC_SCANRATECOEFF_x3          3
/// GL-Sync scan rate coefficient/multiplier is 5:2 (SMPTE)
#define ADL_GLSYNC_SCANRATECOEFF_x5_DIV_2    4
/// GL-Sync scan rate coefficient/multiplier is 2
#define ADL_GLSYNC_SCANRATECOEFF_x2          5
/// GL-Sync scan rate coefficient/multiplier is 3 : 2
#define ADL_GLSYNC_SCANRATECOEFF_x3_DIV_2    6
/// GL-Sync scan rate coefficient/multiplier is 5 : 4
#define ADL_GLSYNC_SCANRATECOEFF_x5_DIV_4    7
/// GL-Sync scan rate coefficient/multiplier is 1 (default)
#define ADL_GLSYNC_SCANRATECOEFF_x1          8
/// GL-Sync scan rate coefficient/multiplier is 4 : 5
#define ADL_GLSYNC_SCANRATECOEFF_x4_DIV_5    9
/// GL-Sync scan rate coefficient/multiplier is 2 : 3
#define ADL_GLSYNC_SCANRATECOEFF_x2_DIV_3    10
/// GL-Sync scan rate coefficient/multiplier is 1 : 2
#define ADL_GLSYNC_SCANRATECOEFF_x1_DIV_2    11
/// GL-Sync scan rate coefficient/multiplier is 2 : 5 (SMPTE)
#define ADL_GLSYNC_SCANRATECOEFF_x2_DIV_5    12
/// GL-Sync scan rate coefficient/multiplier is 1 : 3
#define ADL_GLSYNC_SCANRATECOEFF_x1_DIV_3    13
/// GL-Sync scan rate coefficient/multiplier is 1 : 4
#define ADL_GLSYNC_SCANRATECOEFF_x1_DIV_4    14
/// GL-Sync scan rate coefficient/multiplier is 1 : 5
#define ADL_GLSYNC_SCANRATECOEFF_x1_DIV_5    15


// GL-Sync port (signal presence) states (unique values)

/// GL-Sync port state is undefined
#define ADL_GLSYNC_PORTSTATE_UNDEFINED       0
/// GL-Sync port is not connected
#define ADL_GLSYNC_PORTSTATE_NOCABLE         1
/// GL-Sync port is Idle
#define ADL_GLSYNC_PORTSTATE_IDLE            2
/// GL-Sync port has an Input signal
#define ADL_GLSYNC_PORTSTATE_INPUT           3
/// GL-Sync port is Output
#define ADL_GLSYNC_PORTSTATE_OUTPUT          4


// GL-Sync LED types (used index within ADL_Workstation_GLSyncPortState_Get returned ppGlSyncLEDs array) (unique values)

/// Index into the ADL_Workstation_GLSyncPortState_Get returned ppGlSyncLEDs array for the one LED of the BNC port
#define ADL_GLSYNC_LEDTYPE_BNC               0
/// Index into the ADL_Workstation_GLSyncPortState_Get returned ppGlSyncLEDs array for the Left LED of the RJ45(1) or RJ45(2) port
#define ADL_GLSYNC_LEDTYPE_RJ45_LEFT         0
/// Index into the ADL_Workstation_GLSyncPortState_Get returned ppGlSyncLEDs array for the Right LED of the RJ45(1) or RJ45(2) port
#define ADL_GLSYNC_LEDTYPE_RJ45_RIGHT        1


// GL-Sync LED colors (unique values)

/// GL-Sync LED undefined color
#define ADL_GLSYNC_LEDCOLOR_UNDEFINED        0
/// GL-Sync LED is unlit
#define ADL_GLSYNC_LEDCOLOR_NOLIGHT          1
/// GL-Sync LED is yellow
#define ADL_GLSYNC_LEDCOLOR_YELLOW           2
/// GL-Sync LED is red
#define ADL_GLSYNC_LEDCOLOR_RED              3
/// GL-Sync LED is green
#define ADL_GLSYNC_LEDCOLOR_GREEN            4
/// GL-Sync LED is flashing green
#define ADL_GLSYNC_LEDCOLOR_FLASH_GREEN      5


// GL-Sync Port Control (refers one GL-Sync Port) (unique values)

/// Used to configure the RJ54(1) or RJ42(2) port of GL-Sync is as Idle
#define ADL_GLSYNC_PORTCNTL_NONE             0x00000000
/// Used to configure the RJ54(1) or RJ42(2) port of GL-Sync is as Output
#define ADL_GLSYNC_PORTCNTL_OUTPUT           0x00000001


// GL-Sync Mode Control (refers one Display/Controller) (bitfields)

/// Used to configure the display to use internal timing (not genlocked)
#define ADL_GLSYNC_MODECNTL_NONE             0x00000000
/// Bitfield used to configure the display as genlocked (either as Timing Client or as Timing Server)
#define ADL_GLSYNC_MODECNTL_GENLOCK          0x00000001
/// Bitfield used to configure the display as Timing Server
#define ADL_GLSYNC_MODECNTL_TIMINGSERVER     0x00000002

// GL-Sync Mode Status
/// Display is currently not genlocked
#define ADL_GLSYNC_MODECNTL_STATUS_NONE         0x00000000
/// Display is currently genlocked
#define ADL_GLSYNC_MODECNTL_STATUS_GENLOCK   0x00000001
/// Display requires a mode switch
#define ADL_GLSYNC_MODECNTL_STATUS_SETMODE_REQUIRED 0x00000002
/// Display is capable of being genlocked
#define ADL_GLSYNC_MODECNTL_STATUS_GENLOCK_ALLOWED 0x00000004

#define ADL_MAX_GLSYNC_PORTS                            8
#define ADL_MAX_GLSYNC_PORT_LEDS                        8

/// @}

/// \defgroup define_crossfirestate CrossfireX state of a particular adapter CrossfireX combination
/// @{
#define ADL_XFIREX_STATE_NOINTERCONNECT            ( 1 << 0 )    /* Dongle / cable is missing */
#define ADL_XFIREX_STATE_DOWNGRADEPIPES            ( 1 << 1 )    /* CrossfireX can be enabled if pipes are downgraded */
#define ADL_XFIREX_STATE_DOWNGRADEMEM            ( 1 << 2 )    /* CrossfireX cannot be enabled unless mem downgraded */
#define ADL_XFIREX_STATE_REVERSERECOMMENDED        ( 1 << 3 )    /* Card reversal recommended, CrossfireX cannot be enabled. */
#define ADL_XFIREX_STATE_3DACTIVE            ( 1 << 4 )    /* 3D client is active - CrossfireX cannot be safely enabled */
#define ADL_XFIREX_STATE_MASTERONSLAVE            ( 1 << 5 )    /* Dongle is OK but master is on slave */
#define ADL_XFIREX_STATE_NODISPLAYCONNECT        ( 1 << 6 )    /* No (valid) display connected to master card. */
#define ADL_XFIREX_STATE_NOPRIMARYVIEW            ( 1 << 7 )    /* CrossfireX is enabled but master is not current primary device */
#define ADL_XFIREX_STATE_DOWNGRADEVISMEM        ( 1 << 8 )    /* CrossfireX cannot be enabled unless visible mem downgraded */
#define ADL_XFIREX_STATE_LESSTHAN8LANE_MASTER        ( 1 << 9 )     /* CrossfireX can be enabled however performance not optimal due to <8 lanes */
#define ADL_XFIREX_STATE_LESSTHAN8LANE_SLAVE        ( 1 << 10 )    /* CrossfireX can be enabled however performance not optimal due to <8 lanes */
#define ADL_XFIREX_STATE_PEERTOPEERFAILED        ( 1 << 11 )    /* CrossfireX cannot be enabled due to failed peer to peer test */
#define ADL_XFIREX_STATE_MEMISDOWNGRADED        ( 1 << 16 )    /* Notification that memory is currently downgraded */
#define ADL_XFIREX_STATE_PIPESDOWNGRADED        ( 1 << 17 )    /* Notification that pipes are currently downgraded */
#define ADL_XFIREX_STATE_XFIREXACTIVE            ( 1 << 18 )    /* CrossfireX is enabled on current device */
#define ADL_XFIREX_STATE_VISMEMISDOWNGRADED        ( 1 << 19 )    /* Notification that visible FB memory is currently downgraded */
#define ADL_XFIREX_STATE_INVALIDINTERCONNECTION        ( 1 << 20 )    /* Cannot support current inter-connection configuration */
#define ADL_XFIREX_STATE_NONP2PMODE            ( 1 << 21 )    /* CrossfireX will only work with clients supporting non P2P mode */
#define ADL_XFIREX_STATE_DOWNGRADEMEMBANKS        ( 1 << 22 )    /* CrossfireX cannot be enabled unless memory banks downgraded */
#define ADL_XFIREX_STATE_MEMBANKSDOWNGRADED        ( 1 << 23 )    /* Notification that memory banks are currently downgraded */
#define ADL_XFIREX_STATE_DUALDISPLAYSALLOWED        ( 1 << 24 )    /* Extended desktop or clone mode is allowed. */
#define ADL_XFIREX_STATE_P2P_APERTURE_MAPPING        ( 1 << 25 )    /* P2P mapping was through peer aperture */
#define ADL_XFIREX_STATE_P2PFLUSH_REQUIRED        ADL_XFIREX_STATE_P2P_APERTURE_MAPPING    /* For back compatible */
#define ADL_XFIREX_STATE_XSP_CONNECTED            ( 1 << 26 )    /* There is CrossfireX side port connection between GPUs */
#define ADL_XFIREX_STATE_ENABLE_CF_REBOOT_REQUIRED    ( 1 << 27 )    /* System needs a reboot bofore enable CrossfireX */
#define ADL_XFIREX_STATE_DISABLE_CF_REBOOT_REQUIRED    ( 1 << 28 )    /* System needs a reboot after disable CrossfireX */
#define ADL_XFIREX_STATE_DRV_HANDLE_DOWNGRADE_KEY    ( 1 << 29 )    /* Indicate base driver handles the downgrade key updating */
#define ADL_XFIREX_STATE_CF_RECONFIG_REQUIRED        ( 1 << 30 )    /* CrossfireX need to be reconfigured by CCC because of a LDA chain broken */
#define ADL_XFIREX_STATE_ERRORGETTINGSTATUS        ( 1 << 31 )    /* Could not obtain current status */
/// @}

///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_ADJUSTMENT_PIXELFORMAT adjustment values
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
/// \defgroup define_pixel_formats Pixel Formats values
/// This group defines the various Pixel Formats that a particular digital display can support. \n
/// Since a display can support multiple formats, these values can be bit-or'ed to indicate the various formats \n
/// @{
#define ADL_DISPLAY_PIXELFORMAT_UNKNOWN             0
#define ADL_DISPLAY_PIXELFORMAT_RGB                       (1 << 0)
#define ADL_DISPLAY_PIXELFORMAT_YCRCB444                  (1 << 1)    //Limited range
#define ADL_DISPLAY_PIXELFORMAT_YCRCB422                 (1 << 2)    //Limited range
#define ADL_DISPLAY_PIXELFORMAT_RGB_LIMITED_RANGE      (1 << 3)
#define ADL_DISPLAY_PIXELFORMAT_RGB_FULL_RANGE    ADL_DISPLAY_PIXELFORMAT_RGB  //Full range
#define ADL_DISPLAY_PIXELFORMAT_YCRCB420              (1 << 4)
/// @}

/// \defgroup define_contype Connector Type Values
/// ADLDisplayConfig.ulConnectorType defines
/// @{
#define ADL_DL_DISPLAYCONFIG_CONTYPE_UNKNOWN      0
#define ADL_DL_DISPLAYCONFIG_CONTYPE_CV_NONI2C_JP 1
#define ADL_DL_DISPLAYCONFIG_CONTYPE_CV_JPN       2
#define ADL_DL_DISPLAYCONFIG_CONTYPE_CV_NA        3
#define ADL_DL_DISPLAYCONFIG_CONTYPE_CV_NONI2C_NA 4
#define ADL_DL_DISPLAYCONFIG_CONTYPE_VGA          5
#define ADL_DL_DISPLAYCONFIG_CONTYPE_DVI_D        6
#define ADL_DL_DISPLAYCONFIG_CONTYPE_DVI_I        7
#define ADL_DL_DISPLAYCONFIG_CONTYPE_HDMI_TYPE_A  8
#define ADL_DL_DISPLAYCONFIG_CONTYPE_HDMI_TYPE_B  9
#define ADL_DL_DISPLAYCONFIG_CONTYPE_DISPLAYPORT  10
/// @}

///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_DISPLAYINFO_ Definitions
// for ADLDisplayInfo.iDisplayInfoMask and ADLDisplayInfo.iDisplayInfoValue
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
/// \defgroup define_displayinfomask Display Info Mask Values
/// @{
#define ADL_DISPLAY_DISPLAYINFO_DISPLAYCONNECTED            0x00000001
#define ADL_DISPLAY_DISPLAYINFO_DISPLAYMAPPED                0x00000002
#define ADL_DISPLAY_DISPLAYINFO_NONLOCAL                    0x00000004
#define ADL_DISPLAY_DISPLAYINFO_FORCIBLESUPPORTED            0x00000008
#define ADL_DISPLAY_DISPLAYINFO_GENLOCKSUPPORTED            0x00000010
#define ADL_DISPLAY_DISPLAYINFO_MULTIVPU_SUPPORTED            0x00000020
#define ADL_DISPLAY_DISPLAYINFO_LDA_DISPLAY                    0x00000040
#define ADL_DISPLAY_DISPLAYINFO_MODETIMING_OVERRIDESSUPPORTED            0x00000080

#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_SINGLE            0x00000100
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_CLONE            0x00000200

/// Legacy support for XP
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_2VSTRETCH        0x00000400
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_2HSTRETCH        0x00000800
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_EXTENDED        0x00001000

/// More support manners
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_NSTRETCH1GPU    0x00010000
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_NSTRETCHNGPU    0x00020000
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_RESERVED2        0x00040000
#define ADL_DISPLAY_DISPLAYINFO_MANNER_SUPPORTED_RESERVED3        0x00080000

/// Projector display type
#define ADL_DISPLAY_DISPLAYINFO_SHOWTYPE_PROJECTOR                0x00100000

/// @}


///////////////////////////////////////////////////////////////////////////
// ADL_ADAPTER_DISPLAY_MANNER_SUPPORTED_ Definitions
// for ADLAdapterDisplayCap of ADL_Adapter_Display_Cap()
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
/// \defgroup define_adaptermanner Adapter Manner Support Values
/// @{
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_NOTACTIVE        0x00000001
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_SINGLE            0x00000002
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_CLONE            0x00000004
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_NSTRETCH1GPU    0x00000008
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_NSTRETCHNGPU    0x00000010

/// Legacy support for XP
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_2VSTRETCH        0x00000020
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_2HSTRETCH        0x00000040
#define ADL_ADAPTER_DISPLAYCAP_MANNER_SUPPORTED_EXTENDED        0x00000080

#define ADL_ADAPTER_DISPLAYCAP_PREFERDISPLAY_SUPPORTED            0x00000100
#define ADL_ADAPTER_DISPLAYCAP_BEZEL_SUPPORTED                    0x00000200


///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_DISPLAYMAP_MANNER_ Definitions
// for ADLDisplayMap.iDisplayMapMask and ADLDisplayMap.iDisplayMapValue
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
#define ADL_DISPLAY_DISPLAYMAP_MANNER_RESERVED            0x00000001
#define ADL_DISPLAY_DISPLAYMAP_MANNER_NOTACTIVE            0x00000002
#define ADL_DISPLAY_DISPLAYMAP_MANNER_SINGLE            0x00000004
#define ADL_DISPLAY_DISPLAYMAP_MANNER_CLONE                0x00000008
#define ADL_DISPLAY_DISPLAYMAP_MANNER_RESERVED1            0x00000010  // Removed NSTRETCH
#define ADL_DISPLAY_DISPLAYMAP_MANNER_HSTRETCH            0x00000020
#define ADL_DISPLAY_DISPLAYMAP_MANNER_VSTRETCH            0x00000040
#define ADL_DISPLAY_DISPLAYMAP_MANNER_VLD                0x00000080

/// @}

///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_DISPLAYMAP_OPTION_ Definitions
// for iOption in function ADL_Display_DisplayMapConfig_Get
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
#define ADL_DISPLAY_DISPLAYMAP_OPTION_GPUINFO            0x00000001

///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_DISPLAYTARGET_ Definitions
// for ADLDisplayTarget.iDisplayTargetMask and ADLDisplayTarget.iDisplayTargetValue
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
#define ADL_DISPLAY_DISPLAYTARGET_PREFERRED            0x00000001

///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_POSSIBLEMAPRESULT_VALID Definitions
// for ADLPossibleMapResult.iPossibleMapResultMask and ADLPossibleMapResult.iPossibleMapResultValue
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
#define ADL_DISPLAY_POSSIBLEMAPRESULT_VALID                0x00000001
#define ADL_DISPLAY_POSSIBLEMAPRESULT_BEZELSUPPORTED    0x00000002
#define ADL_DISPLAY_POSSIBLEMAPRESULT_OVERLAPSUPPORTED    0x00000004

///////////////////////////////////////////////////////////////////////////
// ADL_DISPLAY_MODE_ Definitions
// for ADLMode.iModeMask, ADLMode.iModeValue, and ADLMode.iModeFlag
// (bit-vector)
///////////////////////////////////////////////////////////////////////////
/// \defgroup define_displaymode Display Mode Values
/// @{
#define ADL_DISPLAY_MODE_COLOURFORMAT_565                0x00000001
#define ADL_DISPLAY_MODE_COLOURFORMAT_8888                0x00000002
#define ADL_DISPLAY_MODE_ORIENTATION_SUPPORTED_000        0x00000004
#define ADL_DISPLAY_MODE_ORIENTATION_SUPPORTED_090        0x00000008
#define ADL_DISPLAY_MODE_ORIENTATION_SUPPORTED_180        0x00000010
#define ADL_DISPLAY_MODE_ORIENTATION_SUPPORTED_270        0x00000020
#define ADL_DISPLAY_MODE_REFRESHRATE_ROUNDED            0x00000040
#define ADL_DISPLAY_MODE_REFRESHRATE_ONLY                0x00000080

#define ADL_DISPLAY_MODE_PROGRESSIVE_FLAG    0
#define ADL_DISPLAY_MODE_INTERLACED_FLAG    2
/// @}

///////////////////////////////////////////////////////////////////////////
// ADL_OSMODEINFO Definitions
///////////////////////////////////////////////////////////////////////////
/// \defgroup define_osmode OS Mode Values
/// @{
#define ADL_OSMODEINFOXPOS_DEFAULT                -640
#define ADL_OSMODEINFOYPOS_DEFAULT                0
#define ADL_OSMODEINFOXRES_DEFAULT                640
#define ADL_OSMODEINFOYRES_DEFAULT                480
#define ADL_OSMODEINFOXRES_DEFAULT800            800
#define ADL_OSMODEINFOYRES_DEFAULT600            600
#define ADL_OSMODEINFOREFRESHRATE_DEFAULT        60
#define ADL_OSMODEINFOCOLOURDEPTH_DEFAULT        8
#define ADL_OSMODEINFOCOLOURDEPTH_DEFAULT16        16
#define ADL_OSMODEINFOCOLOURDEPTH_DEFAULT24        24
#define ADL_OSMODEINFOCOLOURDEPTH_DEFAULT32        32
#define ADL_OSMODEINFOORIENTATION_DEFAULT        0
#define ADL_OSMODEINFOORIENTATION_DEFAULT_WIN7    DISPLAYCONFIG_ROTATION_FORCE_UINT32
#define ADL_OSMODEFLAG_DEFAULT                    0
/// @}

///////////////////////////////////////////////////////////////////////////
// ADLThreadingModel Enumeration
///////////////////////////////////////////////////////////////////////////
/// \defgroup thread_model
/// Used with \ref ADL_Main_ControlX2_Create and \ref ADL2_Main_ControlX2_Create to specify how ADL handles API calls when executed by multiple threads concurrently.
/// \brief Declares ADL threading behavior.
/// @{
typedef enum ADLThreadingModel
{
    ADL_THREADING_UNLOCKED    = 0, /*!< Default behavior. ADL will not enforce serialization of ADL API executions by multiple threads.  Multiple threads will be allowed to enter to ADL at the same time. Note that ADL library is not guaranteed to be thread-safe. Client that calls ADL_Main_Control_Create have to provide its own mechanism for ADL calls serialization. */
    ADL_THREADING_LOCKED     /*!< ADL will enforce serialization of ADL API when called by multiple threads.  Only single thread will be allowed to enter ADL API at the time. This option makes ADL calls thread-safe. You shouldn't use this option if ADL calls will be executed on Linux on x-server rendering thread. It can cause the application to hung.  */
}ADLThreadingModel;

/// @}
///////////////////////////////////////////////////////////////////////////
// ADLPurposeCode Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLPurposeCode
{
    ADL_PURPOSECODE_NORMAL    = 0,
    ADL_PURPOSECODE_HIDE_MODE_SWITCH,
    ADL_PURPOSECODE_MODE_SWITCH,
    ADL_PURPOSECODE_ATTATCH_DEVICE,
    ADL_PURPOSECODE_DETACH_DEVICE,
    ADL_PURPOSECODE_SETPRIMARY_DEVICE,
    ADL_PURPOSECODE_GDI_ROTATION,
    ADL_PURPOSECODE_ATI_ROTATION
};
///////////////////////////////////////////////////////////////////////////
// ADLAngle Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLAngle
{
    ADL_ANGLE_LANDSCAPE = 0,
    ADL_ANGLE_ROTATERIGHT = 90,
    ADL_ANGLE_ROTATE180 = 180,
    ADL_ANGLE_ROTATELEFT = 270,
};

///////////////////////////////////////////////////////////////////////////
// ADLOrientationDataType Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLOrientationDataType
{
    ADL_ORIENTATIONTYPE_OSDATATYPE,
    ADL_ORIENTATIONTYPE_NONOSDATATYPE
};

///////////////////////////////////////////////////////////////////////////
// ADLPanningMode Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLPanningMode
{
    ADL_PANNINGMODE_NO_PANNING = 0,
    ADL_PANNINGMODE_AT_LEAST_ONE_NO_PANNING = 1,
    ADL_PANNINGMODE_ALLOW_PANNING = 2,
};

///////////////////////////////////////////////////////////////////////////
// ADLLARGEDESKTOPTYPE Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLLARGEDESKTOPTYPE
{
    ADL_LARGEDESKTOPTYPE_NORMALDESKTOP = 0,
    ADL_LARGEDESKTOPTYPE_PSEUDOLARGEDESKTOP = 1,
    ADL_LARGEDESKTOPTYPE_VERYLARGEDESKTOP = 2
};

///////////////////////////////////////////////////////////////////////////
// ADLPlatform Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLPlatForm
{
    GRAPHICS_PLATFORM_DESKTOP  = 0,
    GRAPHICS_PLATFORM_MOBILE   = 1
};

///////////////////////////////////////////////////////////////////////////
// ADLGraphicCoreGeneration Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLGraphicCoreGeneration
{
    ADL_GRAPHIC_CORE_GENERATION_UNDEFINED                   = 0,
    ADL_GRAPHIC_CORE_GENERATION_PRE_GCN                     = 1,
    ADL_GRAPHIC_CORE_GENERATION_GCN                         = 2,
    ADL_GRAPHIC_CORE_GENERATION_RDNA                        = 3
};

// Other Definitions for internal use

// Values for ADL_Display_WriteAndReadI2CRev_Get()

#define ADL_I2C_MAJOR_API_REV           0x00000001
#define ADL_I2C_MINOR_DEFAULT_API_REV   0x00000000
#define ADL_I2C_MINOR_OEM_API_REV       0x00000001

// Values for ADL_Display_WriteAndReadI2C()
#define ADL_DL_I2C_LINE_OEM                0x00000001
#define ADL_DL_I2C_LINE_OD_CONTROL         0x00000002
#define ADL_DL_I2C_LINE_OEM2               0x00000003
#define ADL_DL_I2C_LINE_OEM3               0x00000004
#define ADL_DL_I2C_LINE_OEM4               0x00000005
#define ADL_DL_I2C_LINE_OEM5               0x00000006
#define ADL_DL_I2C_LINE_OEM6               0x00000007
#define ADL_DL_I2C_LINE_GPIO               0x00000008

// Max size of I2C data buffer
#define ADL_DL_I2C_MAXDATASIZE             0x00000018
#define ADL_DL_I2C_MAXWRITEDATASIZE        0x0000000C
#define ADL_DL_I2C_MAXADDRESSLENGTH        0x00000006
#define ADL_DL_I2C_MAXOFFSETLENGTH         0x00000004

// I2C clock speed in KHz
#define ADL_DL_I2C_SPEED_50K               50
#define ADL_DL_I2C_SPEED_100K              100
#define ALD_DL_I2C_SPEED_400K              400
#define ADL_DL_I2C_SPEED_1M                1000
#define ADL_DL_I2C_SPEED_2M                2300

/// Values for ADLDisplayProperty.iPropertyType
#define ADL_DL_DISPLAYPROPERTY_TYPE_UNKNOWN              0
#define ADL_DL_DISPLAYPROPERTY_TYPE_EXPANSIONMODE        1
#define ADL_DL_DISPLAYPROPERTY_TYPE_USEUNDERSCANSCALING     2
/// Enables ITC processing for HDMI panels that are capable of the feature
#define ADL_DL_DISPLAYPROPERTY_TYPE_ITCFLAGENABLE        9
#define ADL_DL_DISPLAYPROPERTY_TYPE_DOWNSCALE            11
#define ADL_DL_DISPLAYPROPERTY_TYPE_INTEGER_SCALING      12


/// Values for ADLDisplayContent.iContentType
/// Certain HDMI panels that support ITC have support for a feature such that, the display on the panel
/// can be adjusted to optimize the view of the content being displayed, depending on the type of content.
#define ADL_DL_DISPLAYCONTENT_TYPE_GRAPHICS        1
#define ADL_DL_DISPLAYCONTENT_TYPE_PHOTO        2
#define ADL_DL_DISPLAYCONTENT_TYPE_CINEMA        4
#define ADL_DL_DISPLAYCONTENT_TYPE_GAME            8



//values for ADLDisplayProperty.iExpansionMode
#define ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_CENTER        0
#define ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_FULLSCREEN    1
#define ADL_DL_DISPLAYPROPERTY_EXPANSIONMODE_ASPECTRATIO   2


///\defgroup define_dither_states Dithering options
/// @{
/// Dithering disabled.
#define ADL_DL_DISPLAY_DITHER_DISABLED              0
/// Use default driver settings for dithering. Note that the default setting could be dithering disabled.
#define ADL_DL_DISPLAY_DITHER_DRIVER_DEFAULT        1
/// Temporal dithering to 6 bpc. Note that if the input is 12 bits, the two least significant bits will be truncated.
#define ADL_DL_DISPLAY_DITHER_FM6                   2
/// Temporal dithering to 8 bpc.
#define ADL_DL_DISPLAY_DITHER_FM8                   3
/// Temporal dithering to 10 bpc.
#define ADL_DL_DISPLAY_DITHER_FM10                  4
/// Spatial dithering to 6 bpc. Note that if the input is 12 bits, the two least significant bits will be truncated.
#define ADL_DL_DISPLAY_DITHER_DITH6                 5
/// Spatial dithering to 8 bpc.
#define ADL_DL_DISPLAY_DITHER_DITH8                 6
/// Spatial dithering to 10 bpc.
#define ADL_DL_DISPLAY_DITHER_DITH10                7
/// Spatial dithering to 6 bpc. Random number generators are reset every frame, so the same input value of a certain pixel will always be dithered to the same output value. Note that if the input is 12 bits, the two least significant bits will be truncated.
#define ADL_DL_DISPLAY_DITHER_DITH6_NO_FRAME_RAND   8
/// Spatial dithering to 8 bpc. Random number generators are reset every frame, so the same input value of a certain pixel will always be dithered to the same output value.
#define ADL_DL_DISPLAY_DITHER_DITH8_NO_FRAME_RAND   9
/// Spatial dithering to 10 bpc. Random number generators are reset every frame, so the same input value of a certain pixel will always be dithered to the same output value.
#define ADL_DL_DISPLAY_DITHER_DITH10_NO_FRAME_RAND  10
/// Truncation to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN6                 11
/// Truncation to 8 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN8                 12
/// Truncation to 10 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN10                13
/// Truncation to 10 bpc followed by spatial dithering to 8 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN10_DITH8          14
/// Truncation to 10 bpc followed by spatial dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN10_DITH6          15
/// Truncation to 10 bpc followed by temporal dithering to 8 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN10_FM8            16
/// Truncation to 10 bpc followed by temporal dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN10_FM6            17
/// Truncation to 10 bpc followed by spatial dithering to 8 bpc and temporal dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN10_DITH8_FM6      18
/// Spatial dithering to 10 bpc followed by temporal dithering to 8 bpc.
#define ADL_DL_DISPLAY_DITHER_DITH10_FM8            19
/// Spatial dithering to 10 bpc followed by temporal dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_DITH10_FM6            20
/// Truncation to 8 bpc followed by spatial dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN8_DITH6           21
/// Truncation to 8 bpc followed by temporal dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_TRUN8_FM6             22
/// Spatial dithering to 8 bpc followed by temporal dithering to 6 bpc.
#define ADL_DL_DISPLAY_DITHER_DITH8_FM6             23
#define ADL_DL_DISPLAY_DITHER_LAST                  ADL_DL_DISPLAY_DITHER_DITH8_FM6
/// @}


/// Display Get Cached EDID flag
#define ADL_MAX_EDIDDATA_SIZE              256 // number of UCHAR
#define ADL_MAX_OVERRIDEEDID_SIZE          512 // number of UCHAR
#define ADL_MAX_EDID_EXTENSION_BLOCKS      3

#define ADL_DL_CONTROLLER_OVERLAY_ALPHA         0
#define ADL_DL_CONTROLLER_OVERLAY_ALPHAPERPIX   1

#define ADL_DL_DISPLAY_DATA_PACKET__INFO_PACKET_RESET      0x00000000
#define ADL_DL_DISPLAY_DATA_PACKET__INFO_PACKET_SET        0x00000001
#define ADL_DL_DISPLAY_DATA_PACKET__INFO_PACKET_SCAN       0x00000002

///\defgroup define_display_packet Display Data Packet Types
/// @{
#define ADL_DL_DISPLAY_DATA_PACKET__TYPE__AVI              0x00000001
#define ADL_DL_DISPLAY_DATA_PACKET__TYPE__GAMMUT           0x00000002
#define ADL_DL_DISPLAY_DATA_PACKET__TYPE__VENDORINFO       0x00000004
#define ADL_DL_DISPLAY_DATA_PACKET__TYPE__HDR              0x00000008
#define ADL_DL_DISPLAY_DATA_PACKET__TYPE__SPD              0x00000010
/// @}

// matrix types
#define ADL_GAMUT_MATRIX_SD         1   // SD matrix i.e. BT601
#define ADL_GAMUT_MATRIX_HD         2   // HD matrix i.e. BT709

///\defgroup define_clockinfo_flags Clock flags
/// Used by ADLAdapterODClockInfo.iFlag
/// @{
#define ADL_DL_CLOCKINFO_FLAG_FULLSCREEN3DONLY         0x00000001
#define ADL_DL_CLOCKINFO_FLAG_ALWAYSFULLSCREEN3D       0x00000002
#define ADL_DL_CLOCKINFO_FLAG_VPURECOVERYREDUCED       0x00000004
#define ADL_DL_CLOCKINFO_FLAG_THERMALPROTECTION        0x00000008
/// @}

// Supported GPUs
// ADL_Display_PowerXpressActiveGPU_Get()
#define ADL_DL_POWERXPRESS_GPU_INTEGRATED        1
#define ADL_DL_POWERXPRESS_GPU_DISCRETE            2

// Possible values for lpOperationResult
// ADL_Display_PowerXpressActiveGPU_Get()
#define ADL_DL_POWERXPRESS_SWITCH_RESULT_STARTED         1 // Switch procedure has been started - Windows platform only
#define ADL_DL_POWERXPRESS_SWITCH_RESULT_DECLINED        2 // Switch procedure cannot be started - All platforms
#define ADL_DL_POWERXPRESS_SWITCH_RESULT_ALREADY         3 // System already has required status  - All platforms
#define ADL_DL_POWERXPRESS_SWITCH_RESULT_DEFERRED        5  // Switch was deferred and requires an X restart - Linux platform only

// PowerXpress support version
// ADL_Display_PowerXpressVersion_Get()
#define ADL_DL_POWERXPRESS_VERSION_MAJOR            2    // Current PowerXpress support version 2.0
#define ADL_DL_POWERXPRESS_VERSION_MINOR            0

#define ADL_DL_POWERXPRESS_VERSION    (((ADL_DL_POWERXPRESS_VERSION_MAJOR) << 16) | ADL_DL_POWERXPRESS_VERSION_MINOR)

//values for ADLThermalControllerInfo.iThermalControllerDomain
#define ADL_DL_THERMAL_DOMAIN_OTHER      0
#define ADL_DL_THERMAL_DOMAIN_GPU        1

//values for ADLThermalControllerInfo.iFlags
#define ADL_DL_THERMAL_FLAG_INTERRUPT    1
#define ADL_DL_THERMAL_FLAG_FANCONTROL   2

///\defgroup define_fanctrl Fan speed cotrol
/// Values for ADLFanSpeedInfo.iFlags
/// @{
#define ADL_DL_FANCTRL_SUPPORTS_PERCENT_READ     1
#define ADL_DL_FANCTRL_SUPPORTS_PERCENT_WRITE    2
#define ADL_DL_FANCTRL_SUPPORTS_RPM_READ         4
#define ADL_DL_FANCTRL_SUPPORTS_RPM_WRITE        8
/// @}

//values for ADLFanSpeedValue.iSpeedType
#define ADL_DL_FANCTRL_SPEED_TYPE_PERCENT    1
#define ADL_DL_FANCTRL_SPEED_TYPE_RPM        2

//values for ADLFanSpeedValue.iFlags
#define ADL_DL_FANCTRL_FLAG_USER_DEFINED_SPEED   1

// MVPU interfaces
#define ADL_DL_MAX_MVPU_ADAPTERS   4
#define MVPU_ADAPTER_0          0x00000001
#define MVPU_ADAPTER_1          0x00000002
#define MVPU_ADAPTER_2          0x00000004
#define MVPU_ADAPTER_3          0x00000008
#define ADL_DL_MAX_REGISTRY_PATH   256

//values for ADLMVPUStatus.iStatus
#define ADL_DL_MVPU_STATUS_OFF   0
#define ADL_DL_MVPU_STATUS_ON    1

// values for ASIC family
///\defgroup define_Asic_type Detailed asic types
/// Defines for Adapter ASIC family type
/// @{
#define ADL_ASIC_UNDEFINED      0
#define ADL_ASIC_DISCRETE       (1 << 0)
#define ADL_ASIC_INTEGRATED     (1 << 1)
#define ADL_ASIC_WORKSTATION    (1 << 2)
#define ADL_ASIC_FIREMV         (1 << 3)
#define ADL_ASIC_XGP            (1 << 4)
#define ADL_ASIC_FUSION         (1 << 5)
#define ADL_ASIC_FIRESTREAM     (1 << 6)
#define ADL_ASIC_EMBEDDED       (1 << 7)
// Backward compatibility
#define ADL_ASIC_FIREGL  ADL_ASIC_WORKSTATION
/// @}

///\defgroup define_detailed_timing_flags Detailed Timimg Flags
/// Defines for ADLDetailedTiming.sTimingFlags field
/// @{
#define ADL_DL_TIMINGFLAG_DOUBLE_SCAN              0x0001
//sTimingFlags is set when the mode is INTERLACED, if not PROGRESSIVE
#define ADL_DL_TIMINGFLAG_INTERLACED               0x0002
//sTimingFlags is set when the Horizontal Sync is POSITIVE, if not NEGATIVE
#define ADL_DL_TIMINGFLAG_H_SYNC_POLARITY          0x0004
//sTimingFlags is set when the Vertical Sync is POSITIVE, if not NEGATIVE
#define ADL_DL_TIMINGFLAG_V_SYNC_POLARITY          0x0008
/// @}

///\defgroup define_modetiming_standard Timing Standards
/// Defines for ADLDisplayModeInfo.iTimingStandard field
/// @{
#define ADL_DL_MODETIMING_STANDARD_CVT             0x00000001 // CVT Standard
#define ADL_DL_MODETIMING_STANDARD_GTF             0x00000002 // GFT Standard
#define ADL_DL_MODETIMING_STANDARD_DMT             0x00000004 // DMT Standard
#define ADL_DL_MODETIMING_STANDARD_CUSTOM          0x00000008 // User-defined standard
#define ADL_DL_MODETIMING_STANDARD_DRIVER_DEFAULT  0x00000010 // Remove Mode from overriden list
#define ADL_DL_MODETIMING_STANDARD_CVT_RB           0x00000020 // CVT-RB Standard
/// @}

// \defgroup define_xserverinfo driver x-server info
/// These flags are used by ADL_XServerInfo_Get()
// @

/// Xinerama is active in the x-server, Xinerama extension may report it to be active but it
/// may not be active in x-server
#define ADL_XSERVERINFO_XINERAMAACTIVE            (1<<0)

/// RandR 1.2 is supported by driver, RandR extension may report version 1.2
/// but driver may not support it
#define ADL_XSERVERINFO_RANDR12SUPPORTED          (1<<1)
// @


///\defgroup define_eyefinity_constants Eyefinity Definitions
/// @{

#define ADL_CONTROLLERVECTOR_0        1    // ADL_CONTROLLERINDEX_0 = 0, (1 << ADL_CONTROLLERINDEX_0)
#define ADL_CONTROLLERVECTOR_1        2    // ADL_CONTROLLERINDEX_1 = 1, (1 << ADL_CONTROLLERINDEX_1)

#define ADL_DISPLAY_SLSGRID_ORIENTATION_000        0x00000001
#define ADL_DISPLAY_SLSGRID_ORIENTATION_090        0x00000002
#define ADL_DISPLAY_SLSGRID_ORIENTATION_180        0x00000004
#define ADL_DISPLAY_SLSGRID_ORIENTATION_270        0x00000008
#define ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_LANDSCAPE     0x00000001
#define ADL_DISPLAY_SLSGRID_CAP_OPTION_RELATIVETO_CURRENTANGLE     0x00000002
#define ADL_DISPLAY_SLSGRID_PORTAIT_MODE                         0x00000004
#define ADL_DISPLAY_SLSGRID_KEEPTARGETROTATION                  0x00000080

#define ADL_DISPLAY_SLSGRID_SAMEMODESLS_SUPPORT        0x00000010
#define ADL_DISPLAY_SLSGRID_MIXMODESLS_SUPPORT        0x00000020
#define ADL_DISPLAY_SLSGRID_DISPLAYROTATION_SUPPORT    0x00000040
#define ADL_DISPLAY_SLSGRID_DESKTOPROTATION_SUPPORT    0x00000080


#define ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FIT        0x0100
#define ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_FILL       0x0200
#define ADL_DISPLAY_SLSMAP_SLSLAYOUTMODE_EXPAND     0x0400

#define ADL_DISPLAY_SLSMAP_IS_SLS        0x1000
#define ADL_DISPLAY_SLSMAP_IS_SLSBUILDER 0x2000
#define ADL_DISPLAY_SLSMAP_IS_CLONEVT     0x4000

#define ADL_DISPLAY_SLSMAPCONFIG_GET_OPTION_RELATIVETO_LANDSCAPE         0x00000001
#define ADL_DISPLAY_SLSMAPCONFIG_GET_OPTION_RELATIVETO_CURRENTANGLE     0x00000002

#define ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_LANDSCAPE         0x00000001
#define ADL_DISPLAY_SLSMAPCONFIG_CREATE_OPTION_RELATIVETO_CURRENTANGLE     0x00000002

#define ADL_DISPLAY_SLSMAPCONFIG_REARRANGE_OPTION_RELATIVETO_LANDSCAPE     0x00000001
#define ADL_DISPLAY_SLSMAPCONFIG_REARRANGE_OPTION_RELATIVETO_CURRENTANGLE     0x00000002

#define ADL_SLS_SAMEMODESLS_SUPPORT         0x0001
#define ADL_SLS_MIXMODESLS_SUPPORT          0x0002
#define ADL_SLS_DISPLAYROTATIONSLS_SUPPORT  0x0004
#define ADL_SLS_DESKTOPROTATIONSLS_SUPPORT  0x0008

#define ADL_SLS_TARGETS_INVALID     0x0001
#define ADL_SLS_MODES_INVALID       0x0002
#define ADL_SLS_ROTATIONS_INVALID   0x0004
#define ADL_SLS_POSITIONS_INVALID   0x0008
#define ADL_SLS_LAYOUTMODE_INVALID  0x0010

#define ADL_DISPLAY_SLSDISPLAYOFFSET_VALID        0x0002

#define ADL_DISPLAY_SLSGRID_RELATIVETO_LANDSCAPE         0x00000010
#define ADL_DISPLAY_SLSGRID_RELATIVETO_CURRENTANGLE     0x00000020


/// The bit mask identifies displays is currently in bezel mode.
#define ADL_DISPLAY_SLSMAP_BEZELMODE            0x00000010
/// The bit mask identifies displays from this map is arranged.
#define ADL_DISPLAY_SLSMAP_DISPLAYARRANGED        0x00000002
/// The bit mask identifies this map is currently in used for the current adapter.
#define ADL_DISPLAY_SLSMAP_CURRENTCONFIG        0x00000004

///For onlay active SLS  map info
#define ADL_DISPLAY_SLSMAPINDEXLIST_OPTION_ACTIVE        0x00000001

///For Bezel
#define ADL_DISPLAY_BEZELOFFSET_STEPBYSTEPSET            0x00000004
#define ADL_DISPLAY_BEZELOFFSET_COMMIT                    0x00000008

typedef enum SLS_ImageCropType {
    Fit = 1,
    Fill = 2,
    Expand = 3
}SLS_ImageCropType;


typedef enum DceSettingsType {
    DceSetting_HdmiLq,
    DceSetting_DpSettings,
    DceSetting_Protection

} DceSettingsType;

typedef enum DpLinkRate {
    DPLinkRate_Unknown,
    DPLinkRate_RBR,
	DPLinkRate_2_16Gbps,
	DPLinkRate_2_43Gbps,
    DPLinkRate_HBR,
	DPLinkRate_4_32Gbps,
    DPLinkRate_HBR2,
    DPLinkRate_HBR3,
	DPLinkRate_UHBR10,
	DPLinkRate_UHBR13D5,
	DPLinkRate_UHBR20

} DpLinkRate;

/// @}

///\defgroup define_powerxpress_constants PowerXpress Definitions
/// @{

/// The bit mask identifies PX caps for ADLPXConfigCaps.iPXConfigCapMask and ADLPXConfigCaps.iPXConfigCapValue
#define    ADL_PX_CONFIGCAPS_SPLASHSCREEN_SUPPORT        0x0001
#define    ADL_PX_CONFIGCAPS_CF_SUPPORT                0x0002
#define    ADL_PX_CONFIGCAPS_MUXLESS                    0x0004
#define    ADL_PX_CONFIGCAPS_PROFILE_COMPLIANT            0x0008
#define    ADL_PX_CONFIGCAPS_NON_AMD_DRIVEN_DISPLAYS    0x0010
#define ADL_PX_CONFIGCAPS_FIXED_SUPPORT             0x0020
#define ADL_PX_CONFIGCAPS_DYNAMIC_SUPPORT           0x0040
#define ADL_PX_CONFIGCAPS_HIDE_AUTO_SWITCH            0x0080

/// The bit mask identifies PX schemes for ADLPXSchemeRange
#define ADL_PX_SCHEMEMASK_FIXED                        0x0001
#define ADL_PX_SCHEMEMASK_DYNAMIC                    0x0002

/// PX Schemes
typedef enum ADLPXScheme
{
    ADL_PX_SCHEME_INVALID   = 0,
    ADL_PX_SCHEME_FIXED     = ADL_PX_SCHEMEMASK_FIXED,
    ADL_PX_SCHEME_DYNAMIC   = ADL_PX_SCHEMEMASK_DYNAMIC
}ADLPXScheme;

/// Just keep the old definitions for compatibility, need to be removed later
typedef enum PXScheme
{
    PX_SCHEME_INVALID   = 0,
    PX_SCHEME_FIXED     = 1,
    PX_SCHEME_DYNAMIC   = 2
} PXScheme;


/// @}

///\defgroup define_appprofiles For Application Profiles
/// @{

#define ADL_APP_PROFILE_FILENAME_LENGTH        256
#define ADL_APP_PROFILE_TIMESTAMP_LENGTH    32
#define ADL_APP_PROFILE_VERSION_LENGTH        32
#define ADL_APP_PROFILE_PROPERTY_LENGTH        64

enum ApplicationListType
{
    ADL_PX40_MRU,
    ADL_PX40_MISSED,
    ADL_PX40_DISCRETE,
    ADL_PX40_INTEGRATED,
    ADL_MMD_PROFILED,
    ADL_PX40_TOTAL
};

typedef enum ADLProfilePropertyType
{
    ADL_PROFILEPROPERTY_TYPE_BINARY        = 0,
    ADL_PROFILEPROPERTY_TYPE_BOOLEAN,
    ADL_PROFILEPROPERTY_TYPE_DWORD,
    ADL_PROFILEPROPERTY_TYPE_QWORD,
    ADL_PROFILEPROPERTY_TYPE_ENUMERATED,
    ADL_PROFILEPROPERTY_TYPE_STRING
}ADLProfilePropertyType;


//Virtual display type returning virtual display type and for request for creating a dummy target ID (xInput or remote play)
typedef enum ADL_VIRTUALDISPLAY_TYPE
{
	ADL_VIRTUALDISPLAY_NONE = 0,
	ADL_VIRTUALDISPLAY_XINPUT = 1,			//Requested for xInput
	ADL_VIRTUALDISPLAY_REMOTEPLAY = 2,		//Requested for emulated display during remote play
	ADL_VIRTUALDISPLAY_GENERIC = 10			//Generic virtual display, af a type different than any of the above special ones
}ADL_VIRTUALDISPLAY_TYPE;

/// @}

///\defgroup define_dp12 For Display Port 1.2
/// @{

/// Maximum Relative Address Link
#define ADL_MAX_RAD_LINK_COUNT    15

/// @}

///\defgroup defines_gamutspace Driver Supported Gamut Space
/// @{

/// The flags desribes that gamut is related to source or to destination and to overlay or to graphics
#define ADL_GAMUT_REFERENCE_SOURCE       (1 << 0)
#define ADL_GAMUT_GAMUT_VIDEO_CONTENT    (1 << 1)

/// The flags are used to describe the source of gamut and how read information from struct ADLGamutData
#define ADL_CUSTOM_WHITE_POINT           (1 << 0)
#define ADL_CUSTOM_GAMUT                 (1 << 1)
#define ADL_GAMUT_REMAP_ONLY             (1 << 2)

/// The define means the predefined gamut values  .
///Driver uses to find entry in the table and apply appropriate gamut space.
#define ADL_GAMUT_SPACE_CCIR_709     (1 << 0)
#define ADL_GAMUT_SPACE_CCIR_601     (1 << 1)
#define ADL_GAMUT_SPACE_ADOBE_RGB    (1 << 2)
#define ADL_GAMUT_SPACE_CIE_RGB      (1 << 3)
#define ADL_GAMUT_SPACE_CUSTOM       (1 << 4)
#define ADL_GAMUT_SPACE_CCIR_2020    (1 << 5)
#define ADL_GAMUT_SPACE_APPCTRL      (1 << 6)

/// Predefine white point values are structed similar to gamut .
#define ADL_WHITE_POINT_5000K       (1 << 0)
#define ADL_WHITE_POINT_6500K       (1 << 1)
#define ADL_WHITE_POINT_7500K       (1 << 2)
#define ADL_WHITE_POINT_9300K       (1 << 3)
#define ADL_WHITE_POINT_CUSTOM      (1 << 4)

///gamut and white point coordinates are from 0.0 -1.0 and divider is used to find the real value .
/// X float = X int /divider
#define ADL_GAMUT_WHITEPOINT_DIVIDER           10000

///gamma a0 coefficient uses the following divider:
#define ADL_REGAMMA_COEFFICIENT_A0_DIVIDER       10000000
///gamma a1 ,a2,a3 coefficients use the following divider:
#define ADL_REGAMMA_COEFFICIENT_A1A2A3_DIVIDER   1000

///describes whether the coefficients are from EDID or custom user values.
#define ADL_EDID_REGAMMA_COEFFICIENTS          (1 << 0)
///Used for struct ADLRegamma. Feature if set use gamma ramp, if missing use regamma coefficents
#define ADL_USE_GAMMA_RAMP                     (1 << 4)
///Used for struct ADLRegamma. If the gamma ramp flag is used then the driver could apply de gamma corretion to the supplied curve and this depends on this flag
#define ADL_APPLY_DEGAMMA                      (1 << 5)
///specifies that standard SRGB gamma should be applied
#define ADL_EDID_REGAMMA_PREDEFINED_SRGB       (1 << 1)
///specifies that PQ gamma curve should be applied
#define ADL_EDID_REGAMMA_PREDEFINED_PQ         (1 << 2)
///specifies that PQ gamma curve should be applied, lower max nits
#define ADL_EDID_REGAMMA_PREDEFINED_PQ_2084_INTERIM (1 << 3)
///specifies that 3.6 gamma should be applied
#define ADL_EDID_REGAMMA_PREDEFINED_36         (1 << 6)
///specifies that BT709 gama should be applied
#define ADL_EDID_REGAMMA_PREDEFINED_BT709      (1 << 7)
///specifies that regamma should be disabled, and application controls regamma content (of the whole screen)
#define ADL_EDID_REGAMMA_PREDEFINED_APPCTRL    (1 << 8)

/// @}

/// \defgroup define_ddcinfo_pixelformats DDCInfo Pixel Formats
/// @{
/// defines for iPanelPixelFormat  in struct ADLDDCInfo2
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB656                       0x00000001L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB666                       0x00000002L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB888                       0x00000004L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB101010                    0x00000008L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB161616                    0x00000010L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB_RESERVED1                0x00000020L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB_RESERVED2                0x00000040L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_RGB_RESERVED3                0x00000080L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_XRGB_BIAS101010              0x00000100L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR444_8BPCC               0x00000200L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR444_10BPCC              0x00000400L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR444_12BPCC              0x00000800L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR422_8BPCC               0x00001000L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR422_10BPCC              0x00002000L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR422_12BPCC              0x00004000L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR420_8BPCC               0x00008000L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR420_10BPCC              0x00010000L
#define ADL_DISPLAY_DDCINFO_PIXEL_FORMAT_YCBCR420_12BPCC              0x00020000L
/// @}

/// \defgroup define_source_content_TF ADLSourceContentAttributes transfer functions (gamma)
/// @{
/// defines for iTransferFunction in ADLSourceContentAttributes
#define ADL_TF_sRGB                0x0001      ///< sRGB
#define ADL_TF_BT709            0x0002      ///< BT.709
#define ADL_TF_PQ2084            0x0004      ///< PQ2084
#define ADL_TF_PQ2084_INTERIM    0x0008        ///< PQ2084-Interim
#define ADL_TF_LINEAR_0_1        0x0010      ///< Linear 0 - 1
#define ADL_TF_LINEAR_0_125        0x0020      ///< Linear 0 - 125
#define ADL_TF_DOLBYVISION        0x0040      ///< DolbyVision
#define ADL_TF_GAMMA_22         0x0080      ///< Plain 2.2 gamma curve
/// @}

/// \defgroup define_source_content_CS ADLSourceContentAttributes color spaces
/// @{
/// defines for iColorSpace in ADLSourceContentAttributes
#define ADL_CS_sRGB                0x0001      ///< sRGB
#define ADL_CS_BT601             0x0002      ///< BT.601
#define ADL_CS_BT709            0x0004      ///< BT.709
#define ADL_CS_BT2020            0x0008      ///< BT.2020
#define ADL_CS_ADOBE            0x0010      ///< Adobe RGB
#define ADL_CS_P3                0x0020      ///< DCI-P3
#define ADL_CS_scRGB_MS_REF        0x0040      ///< scRGB (MS Reference)
#define ADL_CS_DISPLAY_NATIVE    0x0080      ///< Display Native
#define ADL_CS_APP_CONTROL         0x0100      ///< Application Controlled
#define ADL_CS_DOLBYVISION      0x0200      ///< DolbyVision
/// @}

/// \defgroup define_HDR_support ADLDDCInfo2 HDR support options
/// @{
/// defines for iSupportedHDR in ADLDDCInfo2
#define ADL_HDR_CEA861_3        0x0001      ///< HDR10/CEA861.3 HDR supported
#define ADL_HDR_DOLBYVISION     0x0002      ///< \deprecated DolbyVision HDR supported
#define ADL_HDR_FREESYNC_HDR    0x0004      ///< FreeSync HDR supported
/// @}

/// \defgroup define_FreesyncFlags ADLDDCInfo2 Freesync HDR flags
/// @{
/// defines for iFreesyncFlags in ADLDDCInfo2
#define ADL_HDR_FREESYNC_BACKLIGHT_SUPPORT           0x0001      ///< Global backlight control supported
#define ADL_HDR_FREESYNC_LOCAL_DIMMING               0x0002      ///< Local dimming supported
/// @}

/// \defgroup define_source_content_flags ADLSourceContentAttributes flags
/// @{
/// defines for iFlags in ADLSourceContentAttributes
#define ADL_SCA_LOCAL_DIMMING_DISABLE    0x0001      ///< Disable local dimming
/// @}

/// \defgroup define_dbd_state Deep Bit Depth
/// @{

/// defines for ADL_Workstation_DeepBitDepth_Get and  ADL_Workstation_DeepBitDepth_Set functions
// This value indicates that the deep bit depth state is forced off
#define ADL_DEEPBITDEPTH_FORCEOFF     0
/// This value indicates that the deep bit depth state  is set to auto, the driver will automatically enable the
/// appropriate deep bit depth state depending on what connected display supports.
#define ADL_DEEPBITDEPTH_10BPP_AUTO     1
/// This value indicates that the deep bit depth state  is forced on to 10 bits per pixel, this is regardless if the display
/// supports 10 bpp.
#define ADL_DEEPBITDEPTH_10BPP_FORCEON     2

/// defines for ADLAdapterConfigMemory of ADL_Adapter_ConfigMemory_Get
/// If this bit is set, it indicates that the Deep Bit Depth pixel is set on the display
#define ADL_ADAPTER_CONFIGMEMORY_DBD            (1 << 0)
/// If this bit is set, it indicates that the display is rotated (90, 180 or 270)
#define ADL_ADAPTER_CONFIGMEMORY_ROTATE            (1 << 1)
/// If this bit is set, it indicates that passive stereo is set on the display
#define ADL_ADAPTER_CONFIGMEMORY_STEREO_PASSIVE    (1 << 2)
/// If this bit is set, it indicates that the active stereo is set on the display
#define ADL_ADAPTER_CONFIGMEMORY_STEREO_ACTIVE    (1 << 3)
/// If this bit is set, it indicates that the tear free vsync is set on the display
#define ADL_ADAPTER_CONFIGMEMORY_ENHANCEDVSYNC    (1 << 4)
#define ADL_ADAPTER_CONFIGMEMORY_TEARFREEVSYNC    (1 << 4)
/// @}

/// \defgroup define_adl_validmemoryrequiredfields Memory Type
/// @{

///  This group defines memory types in ADLMemoryRequired struct \n
/// Indicates that this is the visible memory
#define ADL_MEMORYREQTYPE_VISIBLE                (1 << 0)
/// Indicates that this is the invisible memory.
#define ADL_MEMORYREQTYPE_INVISIBLE                (1 << 1)
/// Indicates that this is amount of visible memory per GPU that should be reserved for all other allocations.
#define ADL_MEMORYREQTYPE_GPURESERVEDVISIBLE    (1 << 2)
/// @}

/// \defgroup define_adapter_tear_free_status
/// Used in ADL_Adapter_TEAR_FREE_Set and ADL_Adapter_TFD_Get functions to indicate the tear free
/// desktop status.
/// @{
/// Tear free desktop is enabled.
#define ADL_ADAPTER_TEAR_FREE_ON                1
/// Tear free desktop can't be enabled due to a lack of graphic adapter memory.
#define ADL_ADAPTER_TEAR_FREE_NOTENOUGHMEM        -1
/// Tear free desktop can't be enabled due to quad buffer stereo being enabled.
#define ADL_ADAPTER_TEAR_FREE_OFF_ERR_QUADBUFFERSTEREO    -2
/// Tear free desktop can't be enabled due to MGPU-SLS being enabled.
#define ADL_ADAPTER_TEAR_FREE_OFF_ERR_MGPUSLD    -3
/// Tear free desktop is disabled.
#define ADL_ADAPTER_TEAR_FREE_OFF                0
/// @}

/// \defgroup define_adapter_crossdisplay_platforminfo
/// Used in ADL_Adapter_CrossDisplayPlatformInfo_Get function to indicate the Crossdisplay platform info.
/// @{
/// CROSSDISPLAY platform.
#define ADL_CROSSDISPLAY_PLATFORM                    (1 << 0)
/// CROSSDISPLAY platform for Lasso station.
#define ADL_CROSSDISPLAY_PLATFORM_LASSO                (1 << 1)
/// CROSSDISPLAY platform for docking station.
#define ADL_CROSSDISPLAY_PLATFORM_DOCKSTATION        (1 << 2)
/// @}

/// \defgroup define_adapter_crossdisplay_option
/// Used in ADL_Adapter_CrossdisplayInfoX2_Set function to indicate cross display options.
/// @{
/// Checking if 3D application is runnning. If yes, not to do switch, return ADL_OK_WAIT; otherwise do switch.
#define ADL_CROSSDISPLAY_OPTION_NONE            0
/// Force switching without checking for running 3D applications
#define ADL_CROSSDISPLAY_OPTION_FORCESWITCH        (1 << 0)
/// @}

/// \defgroup define_adapter_states Adapter Capabilities
/// These defines the capabilities supported by an adapter. It is used by \ref ADL_Adapter_ConfigureState_Get
/// @{
/// Indicates that the adapter is headless (i.e. no displays can be connected to it)
#define ADL_ADAPTERCONFIGSTATE_HEADLESS ( 1 << 2 )
/// Indicates that the adapter is configured to define the main rendering capabilities. For example, adapters
/// in Crossfire(TM) configuration, this bit would only be set on the adapter driving the display(s).
#define ADL_ADAPTERCONFIGSTATE_REQUISITE_RENDER ( 1 << 0 )
/// Indicates that the adapter is configured to be used to unload some of the rendering work for a particular
/// requisite rendering adapter. For eample, for adapters in a Crossfire configuration, this bit would be set
/// on all adapters that are currently not driving the display(s)
#define ADL_ADAPTERCONFIGSTATE_ANCILLARY_RENDER ( 1 << 1 )
/// Indicates that scatter gather feature enabled on the adapter
#define ADL_ADAPTERCONFIGSTATE_SCATTERGATHER ( 1 << 4 )
/// @}

/// \defgroup define_controllermode_ulModifiers
/// These defines the detailed actions supported by set viewport. It is used by \ref ADL_Display_ViewPort_Set
/// @{
/// Indicate that the viewport set will change the view position
#define ADL_CONTROLLERMODE_CM_MODIFIER_VIEW_POSITION       0x00000001
/// Indicate that the viewport set will change the view PanLock
#define ADL_CONTROLLERMODE_CM_MODIFIER_VIEW_PANLOCK        0x00000002
/// Indicate that the viewport set will change the view size
#define ADL_CONTROLLERMODE_CM_MODIFIER_VIEW_SIZE           0x00000008
/// @}

/// \defgroup defines for Mirabilis
/// These defines are used for the Mirabilis feature
/// @{
///
/// Indicates the maximum number of audio sample rates
#define ADL_MAX_AUDIO_SAMPLE_RATE_COUNT                    16
/// @}

///////////////////////////////////////////////////////////////////////////
// ADLMultiChannelSplitStateFlag Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLMultiChannelSplitStateFlag
{
    ADLMultiChannelSplit_Unitialized = 0,
    ADLMultiChannelSplit_Disabled    = 1,
    ADLMultiChannelSplit_Enabled     = 2,
    ADLMultiChannelSplit_SaveProfile = 3
};

///////////////////////////////////////////////////////////////////////////
// ADLSampleRate Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLSampleRate
{
    ADLSampleRate_32KHz =0,
    ADLSampleRate_44P1KHz,
    ADLSampleRate_48KHz,
    ADLSampleRate_88P2KHz,
    ADLSampleRate_96KHz,
    ADLSampleRate_176P4KHz,
    ADLSampleRate_192KHz,
    ADLSampleRate_384KHz, //DP1.2
    ADLSampleRate_768KHz, //DP1.2
    ADLSampleRate_Undefined
};

/// \defgroup define_overdrive6_capabilities
/// These defines the capabilities supported by Overdrive 6. It is used by \ref ADL_Overdrive6_Capabilities_Get
/// @{
/// Indicate that core (engine) clock can be changed.
#define ADL_OD6_CAPABILITY_SCLK_CUSTOMIZATION               0x00000001
/// Indicate that memory clock can be changed.
#define ADL_OD6_CAPABILITY_MCLK_CUSTOMIZATION               0x00000002
/// Indicate that graphics activity reporting is supported.
#define ADL_OD6_CAPABILITY_GPU_ACTIVITY_MONITOR             0x00000004
/// Indicate that power limit can be customized.
#define ADL_OD6_CAPABILITY_POWER_CONTROL                    0x00000008
/// Indicate that SVI2 Voltage Control is supported.
#define ADL_OD6_CAPABILITY_VOLTAGE_CONTROL                  0x00000010
/// Indicate that OD6+ percentage adjustment is supported.
#define ADL_OD6_CAPABILITY_PERCENT_ADJUSTMENT               0x00000020
/// Indicate that Thermal Limit Unlock is supported.
#define ADL_OD6_CAPABILITY_THERMAL_LIMIT_UNLOCK             0x00000040
///Indicate that Fan speed needs to be displayed in RPM
#define ADL_OD6_CAPABILITY_FANSPEED_IN_RPM                    0x00000080
/// @}

/// \defgroup define_overdrive6_supported_states
/// These defines the power states supported by Overdrive 6. It is used by \ref ADL_Overdrive6_Capabilities_Get
/// @{
/// Indicate that overdrive is supported in the performance state.  This is currently the only state supported.
#define ADL_OD6_SUPPORTEDSTATE_PERFORMANCE                  0x00000001
/// Do not use.  Reserved for future use.
#define ADL_OD6_SUPPORTEDSTATE_POWER_SAVING                 0x00000002
/// @}

/// \defgroup define_overdrive6_getstateinfo
/// These defines the power states to get information about. It is used by \ref ADL_Overdrive6_StateInfo_Get
/// @{
/// Get default clocks for the performance state.
#define ADL_OD6_GETSTATEINFO_DEFAULT_PERFORMANCE            0x00000001
/// Do not use.  Reserved for future use.
#define ADL_OD6_GETSTATEINFO_DEFAULT_POWER_SAVING           0x00000002
/// Get clocks for current state.  Currently this is the same as \ref ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE
/// since only performance state is supported.
#define ADL_OD6_GETSTATEINFO_CURRENT                        0x00000003
/// Get the modified clocks (if any) for the performance state.  If clocks were not modified
/// through Overdrive 6, then this will return the same clocks as \ref ADL_OD6_GETSTATEINFO_DEFAULT_PERFORMANCE.
#define ADL_OD6_GETSTATEINFO_CUSTOM_PERFORMANCE             0x00000004
/// Do not use.  Reserved for future use.
#define ADL_OD6_GETSTATEINFO_CUSTOM_POWER_SAVING            0x00000005
/// @}

/// \defgroup define_overdrive6_getstate and define_overdrive6_getmaxclockadjust
/// These defines the power states to get information about. It is used by \ref ADL_Overdrive6_StateEx_Get and \ref ADL_Overdrive6_MaxClockAdjust_Get
/// @{
/// Get default clocks for the performance state.  Only performance state is currently supported.
#define ADL_OD6_STATE_PERFORMANCE            0x00000001
/// @}

/// \defgroup define_overdrive6_setstate
/// These define which power state to set customized clocks on. It is used by \ref ADL_Overdrive6_State_Set
/// @{
/// Set customized clocks for the performance state.
#define ADL_OD6_SETSTATE_PERFORMANCE                        0x00000001
/// Do not use.  Reserved for future use.
#define ADL_OD6_SETSTATE_POWER_SAVING                       0x00000002
/// @}

/// \defgroup define_overdrive6_thermalcontroller_caps
/// These defines the capabilities of the GPU thermal controller. It is used by \ref ADL_Overdrive6_ThermalController_Caps
/// @{
/// GPU thermal controller is supported.
#define ADL_OD6_TCCAPS_THERMAL_CONTROLLER                   0x00000001
/// GPU fan speed control is supported.
#define ADL_OD6_TCCAPS_FANSPEED_CONTROL                     0x00000002
/// Fan speed percentage can be read.
#define ADL_OD6_TCCAPS_FANSPEED_PERCENT_READ                0x00000100
/// Fan speed can be set by specifying a percentage value.
#define ADL_OD6_TCCAPS_FANSPEED_PERCENT_WRITE               0x00000200
/// Fan speed RPM (revolutions-per-minute) can be read.
#define ADL_OD6_TCCAPS_FANSPEED_RPM_READ                    0x00000400
/// Fan speed can be set by specifying an RPM value.
#define ADL_OD6_TCCAPS_FANSPEED_RPM_WRITE                   0x00000800
/// @}

/// \defgroup define_overdrive6_fanspeed_type
/// These defines the fan speed type being reported. It is used by \ref ADL_Overdrive6_FanSpeed_Get
/// @{
/// Fan speed reported in percentage.
#define ADL_OD6_FANSPEED_TYPE_PERCENT                       0x00000001
/// Fan speed reported in RPM.
#define ADL_OD6_FANSPEED_TYPE_RPM                           0x00000002
/// Fan speed has been customized by the user, and fan is not running in automatic mode.
#define ADL_OD6_FANSPEED_USER_DEFINED                       0x00000100
/// @}

/// \defgroup define_overdrive_EventCounter_type
/// These defines the EventCounter type being reported. It is used by \ref ADL2_OverdriveN_CountOfEvents_Get ,can be used on older OD version supported ASICs also.
/// @{
#define ADL_ODN_EVENTCOUNTER_THERMAL        0
#define ADL_ODN_EVENTCOUNTER_VPURECOVERY    1
/// @}

///////////////////////////////////////////////////////////////////////////
// ADLODNControlType Enumeration
///////////////////////////////////////////////////////////////////////////
enum ADLODNControlType
{
    ODNControlType_Current = 0,
    ODNControlType_Default,
    ODNControlType_Auto,
    ODNControlType_Manual
};

enum ADLODNDPMMaskType
{
     ADL_ODN_DPM_CLOCK               = 1 << 0,
     ADL_ODN_DPM_VDDC                = 1 << 1,
     ADL_ODN_DPM_MASK                = 1 << 2,
};

//ODN features Bits for ADLODNCapabilitiesX2
enum ADLODNFeatureControl
{
     ADL_ODN_SCLK_DPM                = 1 << 0,
     ADL_ODN_MCLK_DPM                = 1 << 1,
     ADL_ODN_SCLK_VDD                = 1 << 2,
     ADL_ODN_MCLK_VDD                = 1 << 3,
     ADL_ODN_FAN_SPEED_MIN           = 1 << 4,
     ADL_ODN_FAN_SPEED_TARGET        = 1 << 5,
     ADL_ODN_ACOUSTIC_LIMIT_SCLK     = 1 << 6,
     ADL_ODN_TEMPERATURE_FAN_MAX     = 1 << 7,
     ADL_ODN_TEMPERATURE_SYSTEM      = 1 << 8,
     ADL_ODN_POWER_LIMIT             = 1 << 9,
     ADL_ODN_SCLK_AUTO_LIMIT             = 1 << 10,
     ADL_ODN_MCLK_AUTO_LIMIT             = 1 << 11,
     ADL_ODN_SCLK_DPM_MASK_ENABLE        = 1 << 12,
     ADL_ODN_MCLK_DPM_MASK_ENABLE        = 1 << 13,
     ADL_ODN_MCLK_UNDERCLOCK_ENABLE      = 1 << 14,
     ADL_ODN_SCLK_DPM_THROTTLE_NOTIFY    = 1 << 15,
     ADL_ODN_POWER_UTILIZATION           = 1 << 16,
     ADL_ODN_PERF_TUNING_SLIDER          = 1 << 17,
     ADL_ODN_REMOVE_WATTMAN_PAGE         = 1 << 31 // Internal Only
};

//If any new feature is added, PPLIB only needs to add ext feature ID and Item ID(Seeting ID). These IDs should match the drive defined in CWDDEPM.h
enum ADLODNExtFeatureControl
{
	ADL_ODN_EXT_FEATURE_MEMORY_TIMING_TUNE = 1 << 0,
	ADL_ODN_EXT_FEATURE_FAN_ZERO_RPM_CONTROL = 1 << 1,
	ADL_ODN_EXT_FEATURE_AUTO_UV_ENGINE = 1 << 2,   //Auto under voltage
	ADL_ODN_EXT_FEATURE_AUTO_OC_ENGINE = 1 << 3,   //Auto OC Enine
	ADL_ODN_EXT_FEATURE_AUTO_OC_MEMORY = 1 << 4,   //Auto OC memory
	ADL_ODN_EXT_FEATURE_FAN_CURVE = 1 << 5    //Fan curve

};

//If any new feature is added, PPLIB only needs to add ext feature ID and Item ID(Seeting ID).These IDs should match the drive defined in CWDDEPM.h
enum ADLODNExtSettingId
{
	ADL_ODN_PARAMETER_AC_TIMING = 0,
	ADL_ODN_PARAMETER_FAN_ZERO_RPM_CONTROL,
	ADL_ODN_PARAMETER_AUTO_UV_ENGINE,
	ADL_ODN_PARAMETER_AUTO_OC_ENGINE,
	ADL_ODN_PARAMETER_AUTO_OC_MEMORY,
	ADL_ODN_PARAMETER_FAN_CURVE_TEMPERATURE_1,
	ADL_ODN_PARAMETER_FAN_CURVE_SPEED_1,
	ADL_ODN_PARAMETER_FAN_CURVE_TEMPERATURE_2,
	ADL_ODN_PARAMETER_FAN_CURVE_SPEED_2,
	ADL_ODN_PARAMETER_FAN_CURVE_TEMPERATURE_3,
	ADL_ODN_PARAMETER_FAN_CURVE_SPEED_3,
	ADL_ODN_PARAMETER_FAN_CURVE_TEMPERATURE_4,
	ADL_ODN_PARAMETER_FAN_CURVE_SPEED_4,
	ADL_ODN_PARAMETER_FAN_CURVE_TEMPERATURE_5,
	ADL_ODN_PARAMETER_FAN_CURVE_SPEED_5,
    ADL_ODN_POWERGAUGE,
	ODN_COUNT

} ;

//OD8 Capability features bits
enum ADLOD8FeatureControl
{
    ADL_OD8_GFXCLK_LIMITS = 1 << 0,
    ADL_OD8_GFXCLK_CURVE = 1 << 1,
    ADL_OD8_UCLK_MAX = 1 << 2,
    ADL_OD8_POWER_LIMIT = 1 << 3,
    ADL_OD8_ACOUSTIC_LIMIT_SCLK = 1 << 4,   //FanMaximumRpm
    ADL_OD8_FAN_SPEED_MIN = 1 << 5,   //FanMinimumPwm
    ADL_OD8_TEMPERATURE_FAN = 1 << 6,   //FanTargetTemperature
    ADL_OD8_TEMPERATURE_SYSTEM = 1 << 7,    //MaxOpTemp
    ADL_OD8_MEMORY_TIMING_TUNE = 1 << 8,
    ADL_OD8_FAN_ZERO_RPM_CONTROL = 1 << 9 ,
	ADL_OD8_AUTO_UV_ENGINE = 1 << 10,  //Auto under voltage
	ADL_OD8_AUTO_OC_ENGINE = 1 << 11,  //Auto overclock engine
	ADL_OD8_AUTO_OC_MEMORY = 1 << 12,  //Auto overclock memory
	ADL_OD8_FAN_CURVE = 1 << 13,   //Fan curve
	ADL_OD8_WS_AUTO_FAN_ACOUSTIC_LIMIT = 1 << 14, //Workstation Manual Fan controller
    ADL_OD8_GFXCLK_QUADRATIC_CURVE = 1 << 15,
    ADL_OD8_OPTIMIZED_GPU_POWER_MODE = 1 << 16,
    ADL_OD8_ODVOLTAGE_LIMIT = 1 << 17,
    ADL_OD8_ADV_OC_LIMITS = 1 << 18,  //Advanced OC limits.
    ADL_OD8_PER_ZONE_GFX_VOLTAGE_OFFSET = 1 << 19,  //Per Zone gfx voltage offset feature
    ADL_OD8_AUTO_CURVE_OPTIMIZER = 1 << 20,  //Auto per zone tuning.
    ADL_OD8_GFX_VOLTAGE_LIMIT = 1 << 21,  //Voltage limit slider
    ADL_OD8_TDC_LIMIT = 1 << 22,  //TDC slider
    ADL_OD8_FULL_CONTROL_MODE = 1 << 23,  //Full control
    ADL_OD8_POWER_SAVING_FEATURE_CONTROL = 1 << 24,  //Power saving feature control
    ADL_OD8_POWER_GAUGE = 1 << 25 //Power Gauge
};


typedef enum ADLOD8SettingId
{
	OD8_GFXCLK_FMAX = 0,
	OD8_GFXCLK_FMIN,
	OD8_GFXCLK_FREQ1,
	OD8_GFXCLK_VOLTAGE1,
	OD8_GFXCLK_FREQ2,
	OD8_GFXCLK_VOLTAGE2,
	OD8_GFXCLK_FREQ3,
	OD8_GFXCLK_VOLTAGE3,
	OD8_UCLK_FMAX,
	OD8_POWER_PERCENTAGE,
	OD8_FAN_MIN_SPEED,
	OD8_FAN_ACOUSTIC_LIMIT,
	OD8_FAN_TARGET_TEMP,
	OD8_OPERATING_TEMP_MAX,
	OD8_AC_TIMING,
	OD8_FAN_ZERORPM_CONTROL,
	OD8_AUTO_UV_ENGINE_CONTROL,
	OD8_AUTO_OC_ENGINE_CONTROL,
	OD8_AUTO_OC_MEMORY_CONTROL,
	OD8_FAN_CURVE_TEMPERATURE_1,
	OD8_FAN_CURVE_SPEED_1,
	OD8_FAN_CURVE_TEMPERATURE_2,
	OD8_FAN_CURVE_SPEED_2,
	OD8_FAN_CURVE_TEMPERATURE_3,
	OD8_FAN_CURVE_SPEED_3,
	OD8_FAN_CURVE_TEMPERATURE_4,
	OD8_FAN_CURVE_SPEED_4,
	OD8_FAN_CURVE_TEMPERATURE_5,
	OD8_FAN_CURVE_SPEED_5,
	OD8_WS_FAN_AUTO_FAN_ACOUSTIC_LIMIT,
    OD8_GFXCLK_CURVE_COEFFICIENT_A, // As part of the agreement with UI team, the min/max voltage limits for the
    OD8_GFXCLK_CURVE_COEFFICIENT_B, // quadratic curve graph will be stored in the min and max limits of
    OD8_GFXCLK_CURVE_COEFFICIENT_C, // coefficient a, b and c. A, b and c themselves do not have limits.
    OD8_GFXCLK_CURVE_VFT_FMIN,
    OD8_UCLK_FMIN,
    OD8_FAN_ZERO_RPM_STOP_TEMPERATURE,
    OD8_OPTIMZED_POWER_MODE,
    OD8_OD_VOLTAGE,// RSX - voltage offset feature
    OD8_ADV_OC_LIMITS_SETTING,
    OD8_PER_ZONE_GFX_VOLTAGE_OFFSET_POINT_1,
    OD8_PER_ZONE_GFX_VOLTAGE_OFFSET_POINT_2,
    OD8_PER_ZONE_GFX_VOLTAGE_OFFSET_POINT_3,
    OD8_PER_ZONE_GFX_VOLTAGE_OFFSET_POINT_4,
    OD8_PER_ZONE_GFX_VOLTAGE_OFFSET_POINT_5,
    OD8_PER_ZONE_GFX_VOLTAGE_OFFSET_POINT_6,
    OD8_AUTO_CURVE_OPTIMIZER_SETTING,
    OD8_GFX_VOLTAGE_LIMIT_SETTING,
    OD8_TDC_PERCENTAGE,
    OD8_FULL_CONTROL_MODE_SETTING,
    OD8_IDLE_POWER_SAVING_FEATURE_CONTROL,
    OD8_RUNTIME_POWER_SAVING_FEATURE_CONTROL,
    OD8_POWER_GAUGE,
    OD8_COUNT
} ADLOD8SettingId;


//Define Performance Metrics Log max sensors number
#define ADL_PMLOG_MAX_SENSORS  256

/// \deprecated Replaced with ADL_PMLOG_SENSORS
typedef enum ADLSensorType
{
    SENSOR_MAXTYPES             = 0,
    PMLOG_CLK_GFXCLK            = 1,    // Current graphic clock value in MHz
    PMLOG_CLK_MEMCLK            = 2,    // Current memory clock value in MHz
    PMLOG_CLK_SOCCLK            = 3,
    PMLOG_CLK_UVDCLK1           = 4,
    PMLOG_CLK_UVDCLK2           = 5,
    PMLOG_CLK_VCECLK            = 6,
    PMLOG_CLK_VCNCLK            = 7,
    PMLOG_TEMPERATURE_EDGE      = 8,    // Current edge of the die temperature value in C
    PMLOG_TEMPERATURE_MEM       = 9,
    PMLOG_TEMPERATURE_VRVDDC    = 10,
    PMLOG_TEMPERATURE_VRMVDD    = 11,
    PMLOG_TEMPERATURE_LIQUID    = 12,
    PMLOG_TEMPERATURE_PLX       = 13,
    PMLOG_FAN_RPM               = 14,   // Current fan RPM value
    PMLOG_FAN_PERCENTAGE        = 15,   // Current ratio of fan RPM and max RPM
    PMLOG_SOC_VOLTAGE           = 16,
    PMLOG_SOC_POWER             = 17,
    PMLOG_SOC_CURRENT           = 18,
    PMLOG_INFO_ACTIVITY_GFX     = 19,   // Current graphic activity level in percentage
    PMLOG_INFO_ACTIVITY_MEM     = 20,   // Current memory activity level in percentage
    PMLOG_GFX_VOLTAGE           = 21,   // Current graphic voltage in mV
    PMLOG_MEM_VOLTAGE           = 22,
    PMLOG_ASIC_POWER            = 23,   // Current ASIC power draw in Watt
    PMLOG_TEMPERATURE_VRSOC     = 24,
    PMLOG_TEMPERATURE_VRMVDD0   = 25,
    PMLOG_TEMPERATURE_VRMVDD1   = 26,
    PMLOG_TEMPERATURE_HOTSPOT   = 27,   // Current center of the die temperature value in C
    PMLOG_TEMPERATURE_GFX       = 28,
    PMLOG_TEMPERATURE_SOC       = 29,
    PMLOG_GFX_POWER             = 30,
    PMLOG_GFX_CURRENT           = 31,
    PMLOG_TEMPERATURE_CPU       = 32,
    PMLOG_CPU_POWER             = 33,
    PMLOG_CLK_CPUCLK            = 34,
    PMLOG_THROTTLER_STATUS      = 35,   // A bit map of GPU throttle information. If a bit is set, the bit represented type of thorttling occurred in the last metrics sampling period
    PMLOG_CLK_VCN1CLK1          = 36,
    PMLOG_CLK_VCN1CLK2          = 37,
    PMLOG_SMART_POWERSHIFT_CPU  = 38,
    PMLOG_SMART_POWERSHIFT_DGPU = 39,
    PMLOG_BUS_SPEED             = 40,   // Current PCIE bus speed running
    PMLOG_BUS_LANES             = 41,   // Current PCIE bus lanes using
    PMLOG_TEMPERATURE_LIQUID0   = 42,
    PMLOG_TEMPERATURE_LIQUID1   = 43,
    PMLOG_CLK_FCLK              = 44,
    PMLOG_THROTTLER_STATUS_CPU  = 45,
    PMLOG_SSPAIRED_ASICPOWER    = 46, // apuPower
    PMLOG_SSTOTAL_POWERLIMIT    = 47, // Total Power limit    
    PMLOG_SSAPU_POWERLIMIT      = 48, // APU Power limit
    PMLOG_SSDGPU_POWERLIMIT     = 49, // DGPU Power limit
    PMLOG_TEMPERATURE_HOTSPOT_GCD      = 50,
    PMLOG_TEMPERATURE_HOTSPOT_MCD      = 51,
    PMLOG_THROTTLER_TEMP_EDGE_PERCENTAGE        = 52,
    PMLOG_THROTTLER_TEMP_HOTSPOT_PERCENTAGE     = 53,
    PMLOG_THROTTLER_TEMP_HOTSPOT_GCD_PERCENTAGE = 54,
    PMLOG_THROTTLER_TEMP_HOTSPOT_MCD_PERCENTAGE = 55,
    PMLOG_THROTTLER_TEMP_MEM_PERCENTAGE     = 56,
    PMLOG_THROTTLER_TEMP_VR_GFX_PERCENTAGE  = 57,
    PMLOG_THROTTLER_TEMP_VR_MEM0_PERCENTAGE = 58,
    PMLOG_THROTTLER_TEMP_VR_MEM1_PERCENTAGE = 59,
    PMLOG_THROTTLER_TEMP_VR_SOC_PERCENTAGE  = 60,
    PMLOG_THROTTLER_TEMP_LIQUID0_PERCENTAGE = 61,
    PMLOG_THROTTLER_TEMP_LIQUID1_PERCENTAGE = 62,
    PMLOG_THROTTLER_TEMP_PLX_PERCENTAGE = 63,
    PMLOG_THROTTLER_TDC_GFX_PERCENTAGE  = 64,
    PMLOG_THROTTLER_TDC_SOC_PERCENTAGE  = 65,
    PMLOG_THROTTLER_TDC_USR_PERCENTAGE  = 66,
    PMLOG_THROTTLER_PPT0_PERCENTAGE     = 67,
    PMLOG_THROTTLER_PPT1_PERCENTAGE     = 68,
    PMLOG_THROTTLER_PPT2_PERCENTAGE     = 69,
    PMLOG_THROTTLER_PPT3_PERCENTAGE     = 70,
    PMLOG_THROTTLER_FIT_PERCENTAGE           = 71,
    PMLOG_THROTTLER_GFX_APCC_PLUS_PERCENTAGE = 72,
    PMLOG_BOARD_POWER                        = 73,
    PMLOG_MAX_SENSORS_REAL
} ADLSensorType;


//Throttle Status
typedef enum ADL_THROTTLE_NOTIFICATION
{
	ADL_PMLOG_THROTTLE_POWER = 1 << 0,
	ADL_PMLOG_THROTTLE_THERMAL = 1 << 1,
	ADL_PMLOG_THROTTLE_CURRENT = 1 << 2,
} ADL_THROTTLE_NOTIFICATION;

typedef enum ADL_PMLOG_SENSORS
{
    ADL_SENSOR_MAXTYPES             = 0,
    ADL_PMLOG_CLK_GFXCLK            = 1,
    ADL_PMLOG_CLK_MEMCLK            = 2,
    ADL_PMLOG_CLK_SOCCLK            = 3,
    ADL_PMLOG_CLK_UVDCLK1           = 4,
    ADL_PMLOG_CLK_UVDCLK2           = 5,
    ADL_PMLOG_CLK_VCECLK            = 6,
    ADL_PMLOG_CLK_VCNCLK            = 7,
    ADL_PMLOG_TEMPERATURE_EDGE      = 8,
    ADL_PMLOG_TEMPERATURE_MEM       = 9,
    ADL_PMLOG_TEMPERATURE_VRVDDC    = 10,
    ADL_PMLOG_TEMPERATURE_VRMVDD    = 11,
    ADL_PMLOG_TEMPERATURE_LIQUID    = 12,
    ADL_PMLOG_TEMPERATURE_PLX       = 13,
    ADL_PMLOG_FAN_RPM               = 14,
    ADL_PMLOG_FAN_PERCENTAGE        = 15,
    ADL_PMLOG_SOC_VOLTAGE           = 16,
    ADL_PMLOG_SOC_POWER             = 17,
    ADL_PMLOG_SOC_CURRENT           = 18,
    ADL_PMLOG_INFO_ACTIVITY_GFX     = 19,
    ADL_PMLOG_INFO_ACTIVITY_MEM     = 20,
    ADL_PMLOG_GFX_VOLTAGE           = 21,
    ADL_PMLOG_MEM_VOLTAGE           = 22,
    ADL_PMLOG_ASIC_POWER            = 23,
    ADL_PMLOG_TEMPERATURE_VRSOC     = 24,
    ADL_PMLOG_TEMPERATURE_VRMVDD0   = 25,
    ADL_PMLOG_TEMPERATURE_VRMVDD1   = 26,
    ADL_PMLOG_TEMPERATURE_HOTSPOT   = 27,
    ADL_PMLOG_TEMPERATURE_GFX       = 28,
    ADL_PMLOG_TEMPERATURE_SOC       = 29,
    ADL_PMLOG_GFX_POWER             = 30,
    ADL_PMLOG_GFX_CURRENT           = 31,
    ADL_PMLOG_TEMPERATURE_CPU       = 32,
    ADL_PMLOG_CPU_POWER             = 33,
    ADL_PMLOG_CLK_CPUCLK            = 34,
    ADL_PMLOG_THROTTLER_STATUS      = 35,   // GFX
    ADL_PMLOG_CLK_VCN1CLK1          = 36,
    ADL_PMLOG_CLK_VCN1CLK2          = 37,
    ADL_PMLOG_SMART_POWERSHIFT_CPU  = 38,
    ADL_PMLOG_SMART_POWERSHIFT_DGPU = 39,
    ADL_PMLOG_BUS_SPEED             = 40,
    ADL_PMLOG_BUS_LANES             = 41,
    ADL_PMLOG_TEMPERATURE_LIQUID0   = 42,
    ADL_PMLOG_TEMPERATURE_LIQUID1   = 43,
    ADL_PMLOG_CLK_FCLK              = 44,
    ADL_PMLOG_THROTTLER_STATUS_CPU  = 45,
    ADL_PMLOG_SSPAIRED_ASICPOWER    = 46, // apuPower
    ADL_PMLOG_SSTOTAL_POWERLIMIT    = 47, // Total Power limit
    ADL_PMLOG_SSAPU_POWERLIMIT      = 48, // APU Power limit
    ADL_PMLOG_SSDGPU_POWERLIMIT     = 49, // DGPU Power limit
    ADL_PMLOG_TEMPERATURE_HOTSPOT_GCD      = 50,
    ADL_PMLOG_TEMPERATURE_HOTSPOT_MCD      = 51,
    ADL_PMLOG_THROTTLER_TEMP_EDGE_PERCENTAGE        = 52,
    ADL_PMLOG_THROTTLER_TEMP_HOTSPOT_PERCENTAGE     = 53,
    ADL_PMLOG_THROTTLER_TEMP_HOTSPOT_GCD_PERCENTAGE = 54,
    ADL_PMLOG_THROTTLER_TEMP_HOTSPOT_MCD_PERCENTAGE = 55,
    ADL_PMLOG_THROTTLER_TEMP_MEM_PERCENTAGE     = 56,
    ADL_PMLOG_THROTTLER_TEMP_VR_GFX_PERCENTAGE  = 57,
    ADL_PMLOG_THROTTLER_TEMP_VR_MEM0_PERCENTAGE = 58,
    ADL_PMLOG_THROTTLER_TEMP_VR_MEM1_PERCENTAGE = 59,
    ADL_PMLOG_THROTTLER_TEMP_VR_SOC_PERCENTAGE  = 60,
    ADL_PMLOG_THROTTLER_TEMP_LIQUID0_PERCENTAGE = 61,
    ADL_PMLOG_THROTTLER_TEMP_LIQUID1_PERCENTAGE = 62,
    ADL_PMLOG_THROTTLER_TEMP_PLX_PERCENTAGE = 63,
    ADL_PMLOG_THROTTLER_TDC_GFX_PERCENTAGE  = 64,
    ADL_PMLOG_THROTTLER_TDC_SOC_PERCENTAGE  = 65,
    ADL_PMLOG_THROTTLER_TDC_USR_PERCENTAGE  = 66,
    ADL_PMLOG_THROTTLER_PPT0_PERCENTAGE     = 67,
    ADL_PMLOG_THROTTLER_PPT1_PERCENTAGE     = 68,
    ADL_PMLOG_THROTTLER_PPT2_PERCENTAGE     = 69,
    ADL_PMLOG_THROTTLER_PPT3_PERCENTAGE     = 70,
    ADL_PMLOG_THROTTLER_FIT_PERCENTAGE           = 71,
    ADL_PMLOG_THROTTLER_GFX_APCC_PLUS_PERCENTAGE = 72,
    ADL_PMLOG_BOARD_POWER                        = 73,
    ADL_PMLOG_MAX_SENSORS_REAL
} ADL_PMLOG_SENSORS;

/// \defgroup define_ecc_mode_states
/// These defines the ECC(Error Correction Code) state. It is used by \ref ADL_Workstation_ECC_Get,ADL_Workstation_ECC_Set
/// @{
/// Error Correction is OFF.
#define ECC_MODE_OFF 0
/// Error Correction is ECCV2.
#define ECC_MODE_ON 2
/// Error Correction is HBM.
#define ECC_MODE_HBM 3
/// @}

/// \defgroup define_board_layout_flags
/// These defines are the board layout flags state which indicates what are the valid properties of \ref ADLBoardLayoutInfo . It is used by \ref ADL_Adapter_BoardLayout_Get
/// @{
/// Indicates the number of slots is valid.
#define ADL_BLAYOUT_VALID_NUMBER_OF_SLOTS 0x1
/// Indicates the slot sizes are valid. Size of the slot consists of the length and width.
#define ADL_BLAYOUT_VALID_SLOT_SIZES 0x2
/// Indicates the connector offsets are valid.
#define ADL_BLAYOUT_VALID_CONNECTOR_OFFSETS 0x4
/// Indicates the connector lengths is valid.
#define ADL_BLAYOUT_VALID_CONNECTOR_LENGTHS 0x8
/// @}

/// \defgroup define_max_constants
/// These defines are the maximum value constants.
/// @{
/// Indicates the Maximum supported slots on board.
#define ADL_ADAPTER_MAX_SLOTS 4
/// Indicates the Maximum supported connectors on slot.
#define ADL_ADAPTER_MAX_CONNECTORS 10
/// Indicates the Maximum supported properties of connection
#define ADL_MAX_CONNECTION_TYPES 32
/// Indicates the Maximum relative address link count.
#define ADL_MAX_RELATIVE_ADDRESS_LINK_COUNT 15
/// Indicates the Maximum size of EDID data block size
#define ADL_MAX_DISPLAY_EDID_DATA_SIZE 1024
/// Indicates the Maximum count of Error Records.
#define ADL_MAX_ERROR_RECORDS_COUNT  256
/// Indicates the maximum number of power states supported
#define ADL_MAX_POWER_POLICY    6
/// @}

/// \defgroup define_connection_types
/// These defines are the connection types constants which indicates  what are the valid connection type of given connector. It is used by \ref ADL_Adapter_SupportedConnections_Get
/// @{
/// Indicates the VGA connection type is valid.
#define ADL_CONNECTION_TYPE_VGA 0
/// Indicates the DVI_I connection type is valid.
#define ADL_CONNECTION_TYPE_DVI 1
/// Indicates the DVI_SL connection type is valid.
#define ADL_CONNECTION_TYPE_DVI_SL 2
/// Indicates the HDMI connection type is valid.
#define ADL_CONNECTION_TYPE_HDMI 3
/// Indicates the DISPLAY PORT connection type is valid.
#define ADL_CONNECTION_TYPE_DISPLAY_PORT 4
/// Indicates the Active dongle DP->DVI(single link) connection type is valid.
#define ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_DVI_SL 5
/// Indicates the Active dongle DP->DVI(double link) connection type is valid.
#define ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_DVI_DL 6
/// Indicates the Active dongle DP->HDMI connection type is valid.
#define ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_HDMI 7
/// Indicates the Active dongle DP->VGA connection type is valid.
#define ADL_CONNECTION_TYPE_ACTIVE_DONGLE_DP_VGA 8
/// Indicates the Passive dongle DP->HDMI connection type is valid.
#define ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_HDMI 9
/// Indicates the Active dongle DP->VGA connection type is valid.
#define ADL_CONNECTION_TYPE_PASSIVE_DONGLE_DP_DVI 10
/// Indicates the MST type is valid.
#define ADL_CONNECTION_TYPE_MST 11
/// Indicates the active dongle, all types.
#define ADL_CONNECTION_TYPE_ACTIVE_DONGLE          12
/// Indicates the Virtual Connection Type.
#define ADL_CONNECTION_TYPE_VIRTUAL    13
/// Macros for generating bitmask from index.
#define ADL_CONNECTION_BITMAST_FROM_INDEX(index) (1 << index)
/// @}

/// \defgroup define_connection_properties
/// These defines are the connection properties which indicates what are the valid properties of given connection type. It is used by \ref ADL_Adapter_SupportedConnections_Get
/// @{
/// Indicates the property Bitrate is valid.
#define ADL_CONNECTION_PROPERTY_BITRATE 0x1
/// Indicates the property number of lanes is valid.
#define ADL_CONNECTION_PROPERTY_NUMBER_OF_LANES 0x2
/// Indicates the property 3D caps is valid.
#define ADL_CONNECTION_PROPERTY_3DCAPS  0x4
/// Indicates the property output bandwidth is valid.
#define ADL_CONNECTION_PROPERTY_OUTPUT_BANDWIDTH 0x8
/// Indicates the property colordepth is valid.
#define ADL_CONNECTION_PROPERTY_COLORDEPTH  0x10
/// @}

/// \defgroup define_lanecount_constants
/// These defines are the Lane count constants which will be used in DP & etc.
/// @{
/// Indicates if lane count is unknown
#define ADL_LANECOUNT_UNKNOWN 0
/// Indicates if lane count is 1
#define ADL_LANECOUNT_ONE 1
/// Indicates if lane count is 2
#define ADL_LANECOUNT_TWO 2
/// Indicates if lane count is 4
#define ADL_LANECOUNT_FOUR 4
/// Indicates if lane count is 8
#define ADL_LANECOUNT_EIGHT 8
/// Indicates default value of lane count
#define ADL_LANECOUNT_DEF ADL_LANECOUNT_FOUR
/// @}

/// \defgroup define_linkrate_constants
/// These defines are the link rate constants which will be used in DP & etc.
/// @{
/// Indicates if link rate is unknown
#define ADL_LINK_BITRATE_UNKNOWN 0
/// Indicates if link rate is 1.62Ghz
#define ADL_LINK_BITRATE_1_62_GHZ 0x06
/// Indicates if link rate is 2.7Ghz
#define ADL_LINK_BITRATE_2_7_GHZ 0x0A
/// Indicates if link rate is 5.4Ghz
#define ADL_LINK_BITRATE_5_4_GHZ 0x14

/// Indicates if link rate is 8.1Ghz
#define ADL_LINK_BITRATE_8_1_GHZ 0x1E
/// Indicates default value of link rate
#define ADL_LINK_BITRATE_DEF ADL_LINK_BITRATE_2_7_GHZ
/// @}

/// \defgroup define_colordepth_constants
/// These defines are the color depth constants which will be used in DP & etc.
/// @{
#define ADL_CONNPROP_S3D_ALTERNATE_TO_FRAME_PACK            0x00000001
/// @}


/// \defgroup define_colordepth_constants
/// These defines are the color depth constants which will be used in DP & etc.
/// @{
/// Indicates if color depth is unknown
#define ADL_COLORDEPTH_UNKNOWN 0
/// Indicates if color depth is 666
#define ADL_COLORDEPTH_666 1
/// Indicates if color depth is 888
#define ADL_COLORDEPTH_888 2
/// Indicates if color depth is 101010
#define ADL_COLORDEPTH_101010 3
/// Indicates if color depth is 121212
#define ADL_COLORDEPTH_121212 4
/// Indicates if color depth is 141414
#define ADL_COLORDEPTH_141414 5
/// Indicates if color depth is 161616
#define ADL_COLORDEPTH_161616 6
/// Indicates default value of color depth
#define ADL_COLOR_DEPTH_DEF ADL_COLORDEPTH_888
/// @}


/// \defgroup define_emulation_status
/// These defines are the status of emulation
/// @{
/// Indicates if real device is connected.
#define ADL_EMUL_STATUS_REAL_DEVICE_CONNECTED 0x1
/// Indicates if emulated device is presented.
#define ADL_EMUL_STATUS_EMULATED_DEVICE_PRESENT 0x2
/// Indicates if emulated device is used.
#define ADL_EMUL_STATUS_EMULATED_DEVICE_USED  0x4
/// In case when last active real/emulated device used (when persistence is enabled but no emulation enforced then persistence will use last connected/emulated device).
#define ADL_EMUL_STATUS_LAST_ACTIVE_DEVICE_USED 0x8
/// @}

/// \defgroup define_emulation_mode
/// These defines are the modes of emulation
/// @{
/// Indicates if no emulation is used
#define ADL_EMUL_MODE_OFF 0
/// Indicates if emulation is used when display connected
#define ADL_EMUL_MODE_ON_CONNECTED 1
/// Indicates if emulation is used when display dis connected
#define ADL_EMUL_MODE_ON_DISCONNECTED 2
/// Indicates if emulation is used always
#define ADL_EMUL_MODE_ALWAYS 3
/// @}

/// \defgroup define_emulation_query
/// These defines are the modes of emulation
/// @{
/// Indicates Data from real device
#define ADL_QUERY_REAL_DATA 0
/// Indicates Emulated data
#define ADL_QUERY_EMULATED_DATA 1
/// Indicates Data currently in use
#define ADL_QUERY_CURRENT_DATA 2
/// @}

/// \defgroup define_persistence_state
/// These defines are the states of persistence
/// @{
/// Indicates persistence is disabled
#define ADL_EDID_PERSISTANCE_DISABLED 0
/// Indicates persistence is enabled
#define ADL_EDID_PERSISTANCE_ENABLED 1
/// @}

/// \defgroup define_connector_types Connector Type
/// defines for ADLConnectorInfo.iType
/// @{
/// Indicates unknown Connector type
#define ADL_CONNECTOR_TYPE_UNKNOWN                 0
/// Indicates VGA Connector type
#define ADL_CONNECTOR_TYPE_VGA                     1
/// Indicates DVI-D Connector type
#define ADL_CONNECTOR_TYPE_DVI_D                   2
/// Indicates DVI-I Connector type
#define ADL_CONNECTOR_TYPE_DVI_I                   3
/// Indicates Active Dongle-NA Connector type
#define ADL_CONNECTOR_TYPE_ATICVDONGLE_NA          4
/// Indicates Active Dongle-JP Connector type
#define ADL_CONNECTOR_TYPE_ATICVDONGLE_JP          5
/// Indicates Active Dongle-NONI2C Connector type
#define ADL_CONNECTOR_TYPE_ATICVDONGLE_NONI2C      6
/// Indicates Active Dongle-NONI2C-D Connector type
#define ADL_CONNECTOR_TYPE_ATICVDONGLE_NONI2C_D    7
/// Indicates HDMI-Type A Connector type
#define ADL_CONNECTOR_TYPE_HDMI_TYPE_A             8
/// Indicates HDMI-Type B Connector type
#define ADL_CONNECTOR_TYPE_HDMI_TYPE_B             9
/// Indicates Display port Connector type
#define ADL_CONNECTOR_TYPE_DISPLAYPORT             10
/// Indicates EDP Connector type
#define ADL_CONNECTOR_TYPE_EDP                     11
/// Indicates MiniDP Connector type
#define ADL_CONNECTOR_TYPE_MINI_DISPLAYPORT        12
/// Indicates Virtual Connector type
#define ADL_CONNECTOR_TYPE_VIRTUAL                   13
/// Indicates USB type C Connector type
#define ADL_CONNECTOR_TYPE_USB_TYPE_C              14
/// @}

/// \defgroup define_freesync_usecase
/// These defines are to specify use cases in which FreeSync should be enabled
/// They are a bit mask. To specify FreeSync for more than one use case, the input value
/// should be set to include multiple bits set
/// @{
/// Indicates FreeSync is enabled for Static Screen case
#define ADL_FREESYNC_USECASE_STATIC                 0x1
/// Indicates FreeSync is enabled for Video use case
#define ADL_FREESYNC_USECASE_VIDEO                  0x2
/// Indicates FreeSync is enabled for Gaming use case
#define ADL_FREESYNC_USECASE_GAMING                 0x4
/// @}

/// \defgroup define_freesync_caps
/// These defines are used to retrieve FreeSync display capabilities.
/// GPU support flag also indicates whether the display is
/// connected to a GPU that actually supports FreeSync
/// @{
#define ADL_FREESYNC_CAP_SUPPORTED                      (1 << 0)
#define ADL_FREESYNC_CAP_GPUSUPPORTED                   (1 << 1)
#define ADL_FREESYNC_CAP_DISPLAYSUPPORTED               (1 << 2)
#define ADL_FREESYNC_CAP_CURRENTMODESUPPORTED           (1 << 3)
#define ADL_FREESYNC_CAP_NOCFXORCFXSUPPORTED            (1 << 4)
#define ADL_FREESYNC_CAP_NOGENLOCKORGENLOCKSUPPORTED    (1 << 5)
#define ADL_FREESYNC_CAP_BORDERLESSWINDOWSUPPORTED      (1 << 6)
/// @}

/// \defgroup define_freesync_labelIndex
/// These defines are used to retrieve which FreeSync label to use
/// @{
#define ADL_FREESYNC_LABEL_UNSUPPORTED            0
#define ADL_FREESYNC_LABEL_FREESYNC               1
#define ADL_FREESYNC_LABEL_ADAPTIVE_SYNC          2
#define ADL_FREESYNC_LABEL_VRR                    3
#define ADL_FREESYNC_LABEL_FREESYNC_PREMIUM       4
#define ADL_FREESYNC_LABEL_FREESYNC_PREMIUM_PRO   5
/// @}

/// Freesync Power optimization masks
/// @{
#define ADL_FREESYNC_POWEROPTIMIZATION_SUPPORTED_MASK		(1 << 0)
#define ADL_FREESYNC_POWEROPTIMIZATION_ENABLED_MASK			(1 << 1)
#define ADL_FREESYNC_POWEROPTIMIZATION_DEFAULT_VALUE_MASK	(1 << 2)
/// @}

/// \defgroup define_MST_CommandLine_execute
/// @{
/// Indicates the MST command line for branch message if the bit is set. Otherwise, it is display message
#define ADL_MST_COMMANDLINE_PATH_MSG                 0x1
/// Indicates the MST command line to send message in broadcast way it the bit is set
#define ADL_MST_COMMANDLINE_BROADCAST                  0x2

/// @}


/// \defgroup define_Adapter_CloneTypes_Get
/// @{
/// Indicates there is crossGPU clone with non-AMD dispalys
#define ADL_CROSSGPUDISPLAYCLONE_AMD_WITH_NONAMD                 0x1
/// Indicates there is crossGPU clone
#define ADL_CROSSGPUDISPLAYCLONE                  0x2

/// @}

/// \defgroup define_D3DKMT_HANDLE
/// @{
/// Handle can be used to create Device Handle when using CreateDevice()
typedef unsigned int ADL_D3DKMT_HANDLE;
/// @}


// End Bracket for Constants and Definitions. Add new groups ABOVE this line!

/// @}


typedef enum ADL_RAS_ERROR_INJECTION_MODE
{
	ADL_RAS_ERROR_INJECTION_MODE_SINGLE = 1,
	ADL_RAS_ERROR_INJECTION_MODE_MULTIPLE = 2
}ADL_RAS_ERROR_INJECTION_MODE;


typedef enum ADL_RAS_BLOCK_ID
{
	ADL_RAS_BLOCK_ID_UMC = 0,
	ADL_RAS_BLOCK_ID_SDMA,
	ADL_RAS_BLOCK_ID_GFX_HUB,
	ADL_RAS_BLOCK_ID_MMHUB,
	ADL_RAS_BLOCK_ID_ATHUB,
	ADL_RAS_BLOCK_ID_PCIE_BIF,
	ADL_RAS_BLOCK_ID_HDP,
	ADL_RAS_BLOCK_ID_XGMI_WAFL,
	ADL_RAS_BLOCK_ID_DF,
	ADL_RAS_BLOCK_ID_SMN,
	ADL_RAS_BLOCK_ID_SEM,
	ADL_RAS_BLOCK_ID_MP0,
	ADL_RAS_BLOCK_ID_MP1,
	ADL_RAS_BLOCK_ID_FUSE
}ADL_RAS_BLOCK_ID;

typedef enum ADL_MEM_SUB_BLOCK_ID
{
	ADL_RAS__UMC_HBM = 0,
	ADL_RAS__UMC_SRAM = 1
}ADL_MEM_SUB_BLOCK_ID;

typedef enum  _ADL_RAS_ERROR_TYPE
{
	ADL_RAS_ERROR__NONE = 0,
	ADL_RAS_ERROR__PARITY = 1,
	ADL_RAS_ERROR__SINGLE_CORRECTABLE = 2,
	ADL_RAS_ERROR__PARITY_SINGLE_CORRECTABLE = 3,
	ADL_RAS_ERROR__MULTI_UNCORRECTABLE = 4,
	ADL_RAS_ERROR__PARITY_MULTI_UNCORRECTABLE = 5,
	ADL_RAS_ERROR__SINGLE_CORRECTABLE_MULTI_UNCORRECTABLE = 6,
	ADL_RAS_ERROR__PARITY_SINGLE_CORRECTABLE_MULTI_UNCORRECTABLE = 7,
	ADL_RAS_ERROR__POISON = 8,
	ADL_RAS_ERROR__PARITY_POISON = 9,
	ADL_RAS_ERROR__SINGLE_CORRECTABLE_POISON = 10,
	ADL_RAS_ERROR__PARITY_SINGLE_CORRECTABLE_POISON = 11,
	ADL_RAS_ERROR__MULTI_UNCORRECTABLE_POISON = 12,
	ADL_RAS_ERROR__PARITY_MULTI_UNCORRECTABLE_POISON = 13,
	ADL_RAS_ERROR__SINGLE_CORRECTABLE_MULTI_UNCORRECTABLE_POISON = 14,
	ADL_RAS_ERROR__PARITY_SINGLE_CORRECTABLE_MULTI_UNCORRECTABLE_POISON = 15
}ADL_RAS_ERROR_TYPE;

typedef enum ADL_RAS_INJECTION_METHOD
{
	ADL_RAS_ERROR__UMC_METH_COHERENT = 0,
	ADL_RAS_ERROR__UMC_METH_SINGLE_SHOT = 1,
	ADL_RAS_ERROR__UMC_METH_PERSISTENT = 2,
	ADL_RAS_ERROR__UMC_METH_PERSISTENT_DISABLE = 3
}ADL_RAS_INJECTION_METHOD;

// Driver event types
typedef enum ADL_DRIVER_EVENT_TYPE
{
	ADL_EVENT_ID_AUTO_FEATURE_COMPLETED = 30,
	ADL_EVENT_ID_FEATURE_AVAILABILITY = 31,

} ADL_DRIVER_EVENT_TYPE;


//UIFeature Ids
typedef enum ADL_UIFEATURES_GROUP
{
	ADL_UIFEATURES_GROUP_DVR = 0,
	ADL_UIFEATURES_GROUP_TURBOSYNC = 1,
	ADL_UIFEATURES_GROUP_FRAMEMETRICSMONITOR = 2,
	ADL_UIFEATURES_GROUP_FRTC = 3,
	ADL_UIFEATURES_GROUP_XVISION = 4,
	ADL_UIFEATURES_GROUP_BLOCKCHAIN = 5,
	ADL_UIFEATURES_GROUP_GAMEINTELLIGENCE = 6,
	ADL_UIFEATURES_GROUP_CHILL = 7,
	ADL_UIFEATURES_GROUP_DELAG = 8,
	ADL_UIFEATURES_GROUP_BOOST = 9,
	ADL_UIFEATURES_GROUP_USU = 10,
	ADL_UIFEATURES_GROUP_XGMI = 11,
	ADL_UIFEATURES_GROUP_PROVSR = 12,
    ADL_UIFEATURES_GROUP_SMA = 13,
    ADL_UIFEATURES_GROUP_CAMERA = 14,
    ADL_UIFEATURES_GROUP_FRTCPRO = 15
} ADL_UIFEATURES_GROUP;



/// Maximum brightness supported by Radeon LED interface
#define ADL_RADEON_LED_MAX_BRIGHTNESS		2

/// Maximum speed supported by Radeon LED interface
#define ADL_RADEON_LED_MAX_SPEED	        4

/// Maximum RGB supported by Radeon LED interface
#define ADL_RADEON_LED_MAX_RGB	            255

/// Maximum MORSE code supported string
#define ADL_RADEON_LED_MAX_MORSE_CODE       260

/// Maximum LED ROW ON GRID
#define ADL_RADEON_LED_MAX_LED_ROW_ON_GRID      7

/// Maximum LED COLUMN ON GRID
#define ADL_RADEON_LED_MAX_LED_COLUMN_ON_GRID   24

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief
///
///
///
///
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef enum ADL_RADEON_USB_LED_BAR_CONTROLS
{
   RadeonLEDBarControl_OFF = 0,
   RadeonLEDBarControl_Static,
   RadeonLEDBarControl_Rainbow,
   RadeonLEDBarControl_Swirl,
   RadeonLEDBarControl_Chase,
   RadeonLEDBarControl_Bounce,
   RadeonLEDBarControl_MorseCode,
   RadeonLEDBarControl_ColorCycle,
   RadeonLEDBarControl_Breathing,
   RadeonLEDBarControl_CustomPattern,
   RadeonLEDBarControl_MAX
}ADL_RADEON_USB_LED_BAR_CONTROLS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief
///
///
///
///
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned int RadeonLEDBARSupportedControl;


/////////////////////////////////////////////////////////////////////////////////////////////
///\brief
///
///
///
///
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef enum ADL_RADEON_USB_LED_CONTROL_CONFIGS
{
   RadeonLEDPattern_Speed = 0,
   RadeonLEDPattern_Brightness,
   RadeonLEDPattern_Direction,
   RadeonLEDPattern_Color,
   RadeonLEDPattern_MAX
}ADL_RADEON_USB_LED_CONTROL_CONFIGS;

/////////////////////////////////////////////////////////////////////////////////////////////
///\brief
///
///
///
///
/// \nosubgrouping
////////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned int RadeonLEDBARSupportedConfig;

//User blob feature settings
typedef enum ADL_USER_SETTINGS
{
    ADL_USER_SETTINGS_ENHANCEDSYNC = 1 << 0,          //notify Enhanced Sync settings change
    ADL_USER_SETTINGS_CHILL_PROFILE = 1 << 1,          //notify Chill settings change
    ADL_USER_SETTINGS_DELAG_PROFILE = 1 << 2,          //notify Delag settings change
    ADL_USER_SETTINGS_BOOST_PROFILE = 1 << 3,			//notify Boost settings change
    ADL_USER_SETTINGS_USU_PROFILE = 1 << 4,  		//notify USU settings change
    ADL_USER_SETTINGS_CVDC_PROFILE = 1 << 5,			//notify Color Vision Deficiency Corretion settings change
    ADL_USER_SETTINGS_SCE_PROFILE = 1 << 6,
    ADL_USER_SETTINGS_PROVSR = 1 << 7
   } ADL_USER_SETTINGS;

#define ADL_REG_DEVICE_FUNCTION_1            0x00000001
#endif /* ADL_DEFINES_H_ */


