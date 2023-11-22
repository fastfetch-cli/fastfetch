#!/usr/bin/env python3

import json, subprocess

if __name__ == "__main__":
    data: dict[str, str] = json.loads(subprocess.check_output(['fastfetch', '--help-raw']))
    for item in data:
        print(item)
