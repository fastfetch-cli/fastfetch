#pragma once

#include "3rdparty/display-library/adl_sdk.h"

// https://gpuopen-librariesandsdks.github.io/adl/modules.html

// Function to initialize the ADL2 interface and to obtain client's context handle.
extern int ADL2_Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters, ADL_CONTEXT_HANDLE* context);

// Destroy client's ADL context.
extern int ADL2_Main_Control_Destroy(ADL_CONTEXT_HANDLE context);

// Retrieves adapter information for given adapter or all OS-known adapters.
// Return ADL_OK on success, DESPITE THE OFFICIAL DOCUMENT SAYS IT RETURNS 1 FOR SUCCESS!
extern int ADL2_Adapter_AdapterInfoX3_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);

// Function to retrieve Graphic Core Info.
extern int ADL2_Adapter_Graphic_Core_Info_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLGraphicCoreInfo* pGraphicCoreInfo);

// Function to retrieve memory information from the adapter. Version 2
extern int ADL2_Adapter_MemoryInfo2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLMemoryInfo2* lpMemoryInfo2);

// This function retrieves the Dedicated VRAM usage of given adapter.
extern int ADL2_Adapter_DedicatedVRAMUsage_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iVRAMUsageInMB);

// Function to get the ASICFamilyType from the adapter.
extern int ADL2_Adapter_ASICFamilyType_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpAsicTypes, int* lpValids);


// Function to retrieve current power management capabilities.
extern int ADL2_Overdrive_Caps(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iSupported, int* iEnabled, int* iVersion);


/////////// Overdrive 6 functions

// Function to retrieve current Overdrive and performance-related activity.
extern int ADL2_Overdrive6_CurrentStatus_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD6CurrentStatus* lpCurrentStatus);

// Function to retrieve GPU temperature from the thermal controller.
extern int ADL2_Overdrive6_Temperature_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpTemperature);

// Function to retrieve the current or default Overdrive clock ranges.
extern int ADL2_Overdrive6_StateInfo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iStateType, ADLOD6StateInfo* lpStateInfo);


/// Overdrive N functions

// Despite the name (N means Next), this is actually Overdrive7 API
// https://github.com/GPUOpen-LibrariesAndSDKs/display-library/blob/master/Sample/OverdriveN/OverdriveN.cpp#L209

// Function to retrieve the OverdriveN capabilities.
extern int ADL2_OverdriveN_CapabilitiesX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNCapabilitiesX2* lpODCapabilities);

// Function to retrieve the current OD performance status.
extern int ADL2_OverdriveN_PerformanceStatus_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceStatus *lpODPerformanceStatus);

// Function to retrieve the current temperature.
extern int ADL2_OverdriveN_Temperature_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iTemperatureType, int *iTemperature);

// Function to retrieve the current GPU clocks settings.
extern int ADL2_OverdriveN_SystemClocksX2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLODNPerformanceLevelsX2 *lpODPerformanceLevels);


/// Overdrive 8 functions

// Function to retrieve the Overdrive8 current settings.
extern int ADL2_Overdrive8_Current_Setting_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD8CurrentSetting *lpCurrentSetting);

// Function to retrieve the Overdrive8 current settings.
extern int ADL2_New_QueryPMLogData_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLPMLogDataOutput *lpDataOutput);
