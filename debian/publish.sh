#!/usr/bin/env bash

debuild -S -i -I
dput ppa:zhangsongcui3371/fastfetch ~/fastfetch_*.changes
