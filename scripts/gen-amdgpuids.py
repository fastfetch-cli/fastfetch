#!/usr/bin/env python3

import sys

def main(amdgpu_ids_path: str):
    with open(amdgpu_ids_path, 'r') as f:
        full_text = f.read()

    products = []
    for line in full_text.split('\n'):
        if not line or line[0] == '#' or not ',\t' in line:
            continue
        device, revision, name = line.split(',\t', maxsplit=2)
        products.append((device, revision, name))

    code = """\
// SPDX-License-Identifier: MIT
// https://opensource.org/license/mit
// Generated from https://gitlab.freedesktop.org/mesa/drm/-/raw/main/data/amdgpu.ids

#include <stdint.h>
#include <stddef.h>

typedef struct FFArmGpuProduct
{
    const uint32_t id; // device << 8 | revision
    const char* name;
} FFArmGpuProduct;

const FFArmGpuProduct ffAmdGpuProducts[] = {
"""

    for device, revision, name in products:
        code += f"    {{ 0x{device} << 8 | 0x{revision}, \"{name}\" }},\n"

    code += "};\n"

    print(code)

if __name__ == '__main__':
    len(sys.argv) == 2 or sys.exit('Usage: gen-amdgpuids.py </path/to/amdgpu.ids>')

    main(sys.argv[1])
