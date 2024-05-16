#include "bootmgr.h"

// https://uefi.org/specs/UEFI/2.10/10_Protocols_Device_Path_Protocol.html#generic-device-path-structures
typedef struct ffEfiDevicePathProtocol
{
    uint8_t Type;
    uint8_t SubType;
    uint16_t Length;
    uint8_t SpecificDevicePathData[];
} ffEfiDevicePathProtocol;

// https://uefi.org/specs/UEFI/2.10/03_Boot_Manager.html#load-options
typedef struct FFEfiLoadOption
{
    uint32_t Attributes;
    uint16_t FilePathListLength;
    uint16_t Description[];
    // ffEfiDevicePathProtocol FilePathList[];
    // uint8_t OptionalData[];
} FFEfiLoadOption;

bool ffEfiFillLoadOption(const FFEfiLoadOption* efiOption, FFBootmgrResult* result);

#define FF_EFI_GLOBAL_GUID "8be4df61-93ca-11d2-aa0d-00e098032b8c"
