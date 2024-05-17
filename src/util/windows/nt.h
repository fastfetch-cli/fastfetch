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
