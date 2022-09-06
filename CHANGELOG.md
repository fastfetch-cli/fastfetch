# 1.7.0

This release brings support for MacOS!  
The basics things are working, but it is far from feature parity with Linux.
I developed this in a VM, so bugs on real hardware are likely.
If you have a Mac and no idea what to do with your free time, i am very happy to accept pull requests / work on issues.

A lot of things were changed under the hood to make this possible, which should bring better performance and stability on all platforms. 

Besides that, the following things have changed:

Features:
* The binary prefix used can now be configured, and is used consistently across all modules. Set `--binary-prefix` to `iec` (default), `si` or `jedec`.
* AMD GPUs now have a much better name, if the file `/usr/share/libdrm/amdgpu.ids` exists. For example my dedicated GPU, which was displayed as `AMD/ATI Radeon RX 5600 OEM/5600 XT / 5700/5700 XT`, is now `AMD Radeon RX 5600M`.

Logos:
* MacOS
* CachyOS small (@sirlucjan, #220)
* MSYS2 (#219)

Bugfixes:
* the `--file` option, which can be used to display the contents of a file as the logo, is now working again.

# 1.6.5

Fixes parsing quoted values in config files

# 1.6.4

Releasing this, so fedora can package fastfetch. Thanks to @jonathanspw for doing that!

Features:
* --set-keyless option (#215)
* Replace `\n`, `\t`, `\e` and `\\` in user provided strings, just like c would do it (#215)
* APK (Alpine Package Keeper) support (@mxkrsv, #216)

Logos:
* Alma Linux (@jonathanspw, #214)

Bugfixes:
* replace deprecated gethostbyname call with getaddrinfo (#217)

# 1.6.3

Fixes installing presets in their own directory (@ceamac, #212)

# 1.6.2

Releasing this, so void linux can package fastfetch.

Logos:
* Rosa linux (#206)
* KISS linux (@draumaz, #207)
* LangitKetujuh (@hervyqa, #208)

Bugfixes:
* Using musl as libc does work now (#210)
* XBPS packages are once again printed (#209)
* configured target dirs are applied to install directories too
* empty XDG_* env vars don't cause a crash anymore

# 1.6.1

Fixes build on android (#205)

# 1.6.0

Features:
* Detect QT on more DEs than just KDE Plasma. The [Plasma] category was therefore renamed to [QT]
* Alacritty font detection
* Load `/etc/fastfetch/config.conf` before user config
* Disk: print one decimal point if size < 100GB
* `--title-fqdn` option, to print fully qualified domain name instead of host name in title
 
Logos:
* updated old NixOS logo

Bugfixes:
* Correctly detect GTK on DEs that store their settings in dconf
* Correctly detect NixOS packages
* Mutter WM detected once again
* Show full NixOS version in OS output
* Don't segfault if an invalid structure is given
* WSL doesn't output GPU anymore, as the name is always meaningless
