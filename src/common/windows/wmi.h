#pragma once

#include <windef.h>
#include <wmistr.h>
#include <assert.h>

/**
 * The WmiOpenBlock function opens the WMI data block object for the specified WMI class.
 *
 * \param Guid Specifies the GUID for WMI class.
 * \param DesiredAccess Specifies the desired access rights to the data block object.
 * \param DataBlockHandle Pointer to a memory location where the routine returns a handle to the data block object.
 * \return ULONG Successful or errant status.
 */
NTSYSAPI ULONG NTAPI
WmiOpenBlock(
    _In_ LPCGUID Guid,
    _In_ ACCESS_MASK DesiredAccess,
    _Out_ PHANDLE DataBlockHandle
);

/**
 * The WmiQueryAllDataW function returns all WMI data blocks that implement a given WMI class (Unicode).
 *
 * \param DataBlockHandle Handle to a WMI data block object.
 * \param BufferLength Pointer to a memory location that specifies the size of the buffer.
 * \param Buffer Pointer to the buffer where the routine returns the WMI data.
 * \return ULONG Successful or errant status.
 */
NTSYSAPI ULONG NTAPI
WmiQueryAllDataW(
    _In_ HANDLE DataBlockHandle,
    _Inout_ PULONG BufferLength,
    _Out_writes_bytes_opt_(*BufferLength) PVOID Buffer
);

/**
 * The WmiCloseBlock function closes a WMI data block object.
 *
 * \param DataBlockHandle Handle to the data block object to be closed.
 * \return ULONG Successful or errant status.
 */
NTSYSAPI ULONG NTAPI
WmiCloseBlock(
    _In_ HANDLE DataBlockHandle
);

static inline void ffCloseWmiBlock(HANDLE* hBlock) {
    assert(hBlock);
    if (*hBlock) {
        WmiCloseBlock(*hBlock);
    }
}

#define FF_AUTO_CLOSE_WMI_BLOCK __attribute__((cleanup(ffCloseWmiBlock)))

// MOF: https://github.com/tpn/winsdk-10/blob/master/Include/10.0.16299.0/km/wmicore.mof
