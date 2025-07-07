#pragma once

#include <winnt.h>
#include <winternl.h>

typedef struct _PROCESSOR_POWER_INFORMATION {
  ULONG Number;
  ULONG MaxMhz;
  ULONG CurrentMhz;
  ULONG MhzLimit;
  ULONG MaxIdleState;
  ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

NTSTATUS NTAPI NtPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength);


NTSTATUS NTAPI RtlGetVersion(
    _Inout_ PRTL_OSVERSIONINFOW lpVersionInformation
);

#if __has_include(<d3dkmthk.h>)
#include <d3dkmthk.h>
#else
typedef UINT D3DKMT_HANDLE;

typedef struct _D3DKMT_OPENADAPTERFROMLUID
{
    LUID            AdapterLuid;
    D3DKMT_HANDLE   hAdapter;
} D3DKMT_OPENADAPTERFROMLUID;
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTOpenAdapterFromLuid(_Inout_ CONST D3DKMT_OPENADAPTERFROMLUID*);

typedef struct _D3DKMT_CLOSEADAPTER
{
    D3DKMT_HANDLE   hAdapter;   // in: adapter handle
} D3DKMT_CLOSEADAPTER;
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTCloseAdapter(_In_ CONST D3DKMT_CLOSEADAPTER*);

typedef struct _D3DKMT_ADAPTERTYPE
{
    union
    {
        struct
        {
            UINT   RenderSupported              :  1;
            UINT   DisplaySupported             :  1;
            UINT   SoftwareDevice               :  1;
            UINT   PostDevice                   :  1;
            UINT   HybridDiscrete               :  1;
            UINT   HybridIntegrated             :  1;
            UINT   IndirectDisplayDevice        :  1;
            UINT   Paravirtualized              :  1;
            UINT   ACGSupported                 :  1;
            UINT   SupportSetTimingsFromVidPn   :  1;
            UINT   Detachable                   :  1;
            UINT   ComputeOnly                  :  1;
            UINT   Prototype                    :  1;
            UINT   RuntimePowerManagement       :  1;
            UINT   Reserved                     : 18;
        };
        UINT Value;
    };
} D3DKMT_ADAPTERTYPE;

typedef enum _KMTQUERYADAPTERINFOTYPE
{
    KMTQAITYPE_ADAPTERTYPE = 15,
} KMTQUERYADAPTERINFOTYPE;
typedef struct _D3DKMT_QUERYADAPTERINFO
{
    D3DKMT_HANDLE           hAdapter;
    KMTQUERYADAPTERINFOTYPE Type;
    VOID*                   pPrivateDriverData;
    UINT                    PrivateDriverDataSize;
} D3DKMT_QUERYADAPTERINFO;
EXTERN_C _Check_return_ NTSTATUS APIENTRY D3DKMTQueryAdapterInfo(_Inout_ CONST D3DKMT_QUERYADAPTERINFO*);
#endif
