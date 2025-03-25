#pragma once

#include "3rdparty/display-library/adl_sdk.h"

// https://gpuopen-librariesandsdks.github.io/adl/modules.html

// Function to initialize the ADL2 interface and to obtain client's context handle.
extern int ADL2_Main_Control_Create(ADL_MAIN_MALLOC_CALLBACK callback, int iEnumConnectedAdapters, ADL_CONTEXT_HANDLE* context);

// Destroy client's ADL context.
extern int ADL2_Main_Control_Destroy(ADL_CONTEXT_HANDLE context);

// Retrieves adapter information for given adapter or all OS-known adapters.
// Return 1 on success
extern int ADL2_Adapter_AdapterInfoX3_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* numAdapters, AdapterInfo** lppAdapterInfo);

// Function to retrieve Graphic Core Info.
// Return ADL_OK on success
extern int ADL2_Adapter_Graphic_Core_Info_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLGraphicCoreInfo* pGraphicCoreInfo);

// Function to retrieve memory information from the adapter. Version 2
// Return ADL_OK on success
extern int ADL2_Adapter_MemoryInfo2_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLMemoryInfo2* lpMemoryInfo2);

// This function retrieves the VRAM usage of given adapter.
// Return ADL_OK on success
extern int ADL2_Adapter_VRAMUsage_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iVRAMUsageInMB);

// This function retrieves the Dedicated VRAM usage of given adapter.
// Return ADL_OK on success
extern int ADL2_Adapter_DedicatedVRAMUsage_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iVRAMUsageInMB);

// Function to get the ASICFamilyType from the adapter.
// Return ADL_OK on success
extern int ADL2_Adapter_ASICFamilyType_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpAsicTypes, int* lpValids);

// Function to retrieve current Overdrive and performance-related activity.
// Return ADL_OK on success
extern int ADL2_Overdrive6_CurrentStatus_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, ADLOD6CurrentStatus* lpCurrentStatus);

// Function to retrieve GPU temperature from the thermal controller.
// Return ADL_OK on success
extern int ADL2_Overdrive6_Temperature_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpTemperature);

// Function to retrieve the current or default Overdrive clock ranges.
// Return ADL_OK on success
extern int ADL2_Overdrive6_StateInfo_Get(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int iStateType, ADLOD6StateInfo* lpStateInfo);
