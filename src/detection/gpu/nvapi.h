// References:
// https://github.com/NVIDIA/nvapi (MIT License)
// https://github.com/deathcamp/NVOC/blob/master/nvoc.c (Public Domain)

typedef enum NvApiGPUMemoryType
{
    NVAPI_GPU_MEMORY_TYPE_UNKNOWN = 0,
    NVAPI_GPU_MEMORY_TYPE_SDRAM,
    NVAPI_GPU_MEMORY_TYPE_DDR1,
    NVAPI_GPU_MEMORY_TYPE_DDR2,
    NVAPI_GPU_MEMORY_TYPE_GDDR2,
    NVAPI_GPU_MEMORY_TYPE_GDDR3,
    NVAPI_GPU_MEMORY_TYPE_GDDR4,
    NVAPI_GPU_MEMORY_TYPE_DDR3,
    NVAPI_GPU_MEMORY_TYPE_GDDR5,
    NVAPI_GPU_MEMORY_TYPE_LPDDR2,
    NVAPI_GPU_MEMORY_TYPE_GDDR5X,
    NVAPI_GPU_MEMORY_TYPE_LPDDR3,
    NVAPI_GPU_MEMORY_TYPE_LPDDR4,
    NVAPI_GPU_MEMORY_TYPE_LPDDR5,
    NVAPI_GPU_MEMORY_TYPE_GDDR6,
    NVAPI_GPU_MEMORY_TYPE_GDDR6X,
    NVAPI_GPU_MEMORY_TYPE_GDDR7,
} NvApiGPUMemoryType;

typedef enum
{
    NV_SYSTEM_TYPE_GPU_UNKNOWN     = 0,
    NV_SYSTEM_TYPE_IGPU            = 1, // Integrated
    NV_SYSTEM_TYPE_DGPU            = 2, // Discrete
} NvApiGPUType;

typedef int NvAPI_Status; // 0 = success; < 0 = error
typedef struct NvPhysicalGpuHandle* NvPhysicalGpuHandle;

typedef enum
{
    NVAPI_INTERFACE_OFFSET_INITIALIZE = 0x0150E828,
    NVAPI_INTERFACE_OFFSET_UNLOAD = 0xD22BDD7E,
    NVAPI_INTERFACE_OFFSET_ENUM_PHYSICAL_GPUS = 0xE5AC921F,
    NVAPI_INTERFACE_OFFSET_GPU_GET_RAM_TYPE = 0x57F7CAAC,
    NVAPI_INTERFACE_OFFSET_GPU_GET_GPU_TYPE = 0xC33BAEB1,

    NVAPI_INTERFACE_OFFSET_FORCE_UINT32 = 0xFFFFFFFF
} NvApiInterfaceOffsets;

extern void* nvapi_QueryInterface(NvApiInterfaceOffsets offset);

extern NvAPI_Status nvapi_Initialize(void);
extern NvAPI_Status nvapi_Unload(void);
extern NvAPI_Status nvapi_EnumPhysicalGPUs(NvPhysicalGpuHandle* handles, int* count);
extern NvAPI_Status nvapi_GPU_GetRamType(NvPhysicalGpuHandle handle, NvApiGPUMemoryType* memtype);
extern NvAPI_Status nvapi_GPU_GetGPUType(NvPhysicalGpuHandle handle, NvApiGPUType* gpuType);
