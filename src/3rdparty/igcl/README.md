# Intel Graphics Control Library (IGCL)
Header, wrapper library and samples of IGCL version 1.0
 

# Introduction
IGCL is meant to be a collection of high level APIs for all control aspects of hardware, primarily graphics. This is replacement of legacy Intel CUISDK which used to be released only to OEM's and selected customers. IGCL allows global control and tweaking of display, media & 3D capabilities.

# Notes
* IGCL binaries are distributed as part of Intel Graphics driver package.
* Header & library wrapper code are provided here to help developers with their application development.
* For API/spec questions or issues, for now, use the "Issues" tab under Github. For issues related to an already shipped binary of this spec, contact standard Intel customer support for Graphics.
* Performance & Telemetry API's, i.e., Engine/Fan/Telemetry/Frequency/Memory/Overclock/PCI/Power/Temperature are limited to 64-bit applications as of now. This is a Level0 limitation.

# Usage 
cmake.exe -B <output_folder> -S <cmake_source_folder> -G "Visual Studio 17 2022" -A x64
