from requests import get as http_get

class PciDeviceModel:
    def __init__(self, id: int, name: str):
        self.id = id
        self.name = name

class PciVendorModel:
    def __init__(self, id: int, name: str):
        self.id = id
        self.name = name
        self.devices = []

def main(keep_vendor_list: set):
    vendors = []
    try:
        with open('pci.ids', 'r') as f:
            full_text = f.read()
    except FileNotFoundError:
        response = http_get('https://pci-ids.ucw.cz/v2.2/pci.ids')
        full_text = response.text

    dev_list_text = full_text[:full_text.rfind('\n\n\n')]  # remove known classes
    for line in dev_list_text.split('\n'):
        if not line or line[0] == '#':
            continue
        if line[0] != '\t':
            id, name = line.split('  ', maxsplit=1)
            vendors.append(PciVendorModel(int(id, 16), name))
        elif line[1] != '\t':
            id, name = line[1:].split('  ', maxsplit=1)
            vendors[-1].devices.append(PciDeviceModel(int(id, 16), name))

    code = """\
// SPDX-License-Identifier: BSD-3-Clause
// https://opensource.org/license/BSD-3-Clause
// Generated from https://pci-ids.ucw.cz/v2.2/pci.ids

#include <stdint.h>
#include <stddef.h>

typedef struct FFPciDevice
{
    const uint32_t id;
    const char* name;
} FFPciDevice;

typedef struct FFPciVendor
{
    const uint32_t id;
    const char* name;
    const FFPciDevice* devices;
    const uint32_t nDevices;
} FFPciVendor;
"""

    if keep_vendor_list:
        vendors = [vendor for vendor in vendors if vendor.id in keep_vendor_list]

    for vendor in vendors:
        if vendor.devices:
            piece = ',\n    '.join('{{ 0x{:04X}, "{}" }}'.format(device.id, device.name.replace('"', '\\"')) for device in vendor.devices)
            code += f"""
// {vendor.name}
static const FFPciDevice pciDevices_{vendor.id:04X}[] = {{
    {piece},
    {{}},
}};
"""

    piece = ',\n    '.join('{{ 0x{:04X}, "{}", {}, {} }}'.format(vendor.id, vendor.name.replace('"', '\\"'), vendor.devices and f"pciDevices_{vendor.id:04X}" or "NULL", len(vendor.devices)) for vendor in vendors)
    code += f"""
const FFPciVendor ffPciVendors[] = {{
    {piece},
    {{}},
}};"""

    print(code)

if __name__ == '__main__':
    # From <src/detection/gpu/gpu.c>
    main({
        0x106b, # Apple
        0x1002, 0x1022, # AMD
        0x8086, 0x8087, 0x03e7, # Intel
        0x0955, 0x10de, 0x12d2, # Nvidia
        0x1ed5, # MThreads
        0x5143, # Qualcomm
        0x14c3, # MTK
        0x15ad, # VMware
        0x1af4, # RedHat
        0x1ab8, # Parallel
        0x1414, # Microsoft
        0x108e, # Oracle
    })
