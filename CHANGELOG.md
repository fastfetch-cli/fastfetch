# 1.10.1

New release for debugging #421

# 1.10.0

Notable Changes:

* With the support of Win32 platform, original Windows 64bit artifact file is renamed to Win64 to avoid possible confusion

Features:
* Bluetooth module
* Sound module
* Gamepad module
* Support colored percentage numbers output (#409)
* Support `--localip-compact-type` option (#408)
* Terminator terminal font detection (@Zerogiven, #415)
* Windows 32bit compatibility
* Support global configuration in MSYS2 environment (Windows)
* Support GPU driver version detection on Windows 11
* Support scaled resolution detection for wayland (Linux)

Bugfixes:

* Fix build with older libnm versions
* Fix a rare case that fails to detect terminal
* Fix Muffin detection (@Zerogiven, #411)
* Fix IPv6 detection (Windows)
* Fix scoop package count detection when scoop is installed in non-default path (Windows, #417)
* Fix UB reported by clang
* Honor $SCOOP when detecting scoop packages (#417)

Other:

* Simplified wmtheme output format (Windows)
* Improved GPU detection performance on Windows 11
* Lastest Mac model identifier support (macOS)

# 1.9.1

Bugfixes:

* Fix builds on s390x (@jonathanspw, #402)
* Fix zero refresh rate on some monitors (macOS)
* Fix default formatting of Wifi module

# 1.9.0

Notable Changes:
* fastfetch no longer creates a sample config file silently. Use `--gen-config` to generate one.
* fastfetch now search for user config file in the order of `fastfetch --list-config-paths`
* Unknown disks are hidden by default.
* `Resolution` module is renamed to `Display`. (#393)

Features:
* `--logo-padding-top` option (@CarterLi, #372)
* Raw image file as logo support (@CarterLi)
* Look for config files in `$APPDATA` ([RoamingAppData](https://superuser.com/questions/21458/why-are-there-directories-called-local-locallow-and-roaming-under-users-user#answer-21462)) (Windows)
* Look for config files in `~/Library/Preferences` (macOS)
* Add `--list-config-paths` option which list search paths of config files
* Add `--list-data-paths` option which list search paths for presets and logos
* Add `Brightness` module support
* Add `Battery` module support for FreeBSD
* Add `--disk-show-unknown` option for Disk module
* Add `--disk-show-subvolumes` option for Disk module
* Add `--gpu-hide-integrated` option (#379)
* Add `--gpu-hide-discrete` option (#379)
* Detect terminal version when available
* Support `WezTerm` terminal font detection (requires [`wezterm` executable](https://wezfurlong.org/wezterm/cli/general.html) being available)
* Add `--shell-version` and `--terminal-version` options to disable shell / terminal version detection
* Enhance `--percent-type` to allow hiding other texts (#387)
* Add Wifi module support for Linux
* Detect scaled resolutions (Windows, macOS)
* Optimise font module printing (Windows)
* Detect pacman package count inside MSYS2 environment (Windows)
* Add Wifi / Battery module support for Android
* Disk name support for Linux

Logos:
* Raspbian (@IamNoRobot, #373)

Bugfixes:
* `--logo-type` now does accept `iterm` too (@CarterLi, #374)
* Fix mintty terminal font detection (Windows)
* Fix bug that line buffering doesn't work properly (Windows)
* Fix rpm package count detection (Linux)
* Fix cpu temp detection (Linux)

Other:
* Fixed a Typo in iterm error message (@jessebot, #376)
* Don't try to load config file in `/etc` (Windows)

# 1.8.2

Bugfixes:

* Fix memleaks Users module (Windows)
* Fix shell detection when installed with scoop (Windows)
* Don't use libcJSON as wlanapi's dll name (Windows)
* Align artifact names to other platforms (Windows)

# 1.8.1

Notable Changes:

* `Song` was used as an alias to `Media` module. It's removed to avoid confusion. All song related flags (`--song-key`, etc) should change to media (`--media-key`, etc). (@CarterLi)

Bugfixes:

* Mountpoint paths on linux get decoded correctly (#364)
* Color parsing once again works (@IanManske, #365)
* Using a custom key with a placeholder for the local ip module now does work correctly if multiple interfaces are present (#368)

# 1.8.0

This release introduces Windows support! Fastfetch now fully support all major desktop OSes (Linux, macOS, Windows and FreeBSD)

Notable Changes:
* Bios / Board / Chassis modules are splitted against Host module for performance reasons
* Caching is removed. Option `--nocache` is removed accordingly

Features:
* Windows (7 and newer) is officially and fully supported
* FreeBSD support is improved greatly (Bios, Cpu Temp, Cpu Usage, Disk, Host, Processes, Swap, Terminal / Shell, Uptime)
* Adds a new flag `--stat`, which prints time usage for individual modules
* Adds Wifi module which supports Windows and macOS
* Adds data source option for logo printing
* Detects Homebrew Cellar and Cask seperately
* Detects WSL version
* Detects disk based on mount point
* Exposes more chafa configs
* Improves performance for Cpu Usage, Public IP, Weather modules
* Improves performance for Kitty image protocol when both image width / height specified
* Improves performance for large file loading
* Improves performance for macOS WM and Host detection
* Improves shell and terminal detection on macOS
* Supports Deepin Terminal terminal font
* Supports GPU detection on Android
* Supports Kitty Terminal terminal font
* Supports bar output for percentage values
* Supports Bios module on macOS
* Supports eopkg package manager detection
* Supports iTerm image logo protocol
* Supports image logo printing on macOS
* Supports tcsh version detection
* Vulkan module on macOS no longer requires vulkan-loader to work

Logos:
* Alpine
* CRUX
* EndeavourOS
* Enso
* Garuda small
* Nobara
* OpenMandriva
* Parabola GNU/Linux-libre
* Rocky
* Rosa
* Solus
* Univalent
* Vanilla OS

Bugfixes:
* Fixes disk size detection on 32bit Linux (#337)
* Fixes cpu freq detection in WSL
* Fixes internal bug of FFstrbuf
* Fixes some memory leaks
* Fixes segfault if 0 is given as argument index
* Lots of code refactors

# 1.7.5

Fixes a crash on linux that could happen when getting zsh version (#285)

# 1.7.4

The last element in the default structure (currently the color blocks) is now printed again (#283)

# 1.7.3

A lot of small improvements for MacOS & BSD platforms.

Features:
* BSD is now officially supported (#228)
* MacPorts package manager support (@SladeGetz, #234)
* Battery support for MacOS (@CarterLi, #235)
* Processes, swap & terminal font support for MacOS(@CarterLi, #237)
* Media support for MacOS (@CarterLi, #242)
* Player support for MacOS (@CarterLi, #245)
* WM theme support for MacOS (@CarterLi, #246)
* CPU usage support for MacOS (@CarterLi, #247)
* Power Adapter module (@CarterLi, #249)
* Windows terminal font for WSL (@CarterLi, #254)
* Temps & Font support for MacOS (@CarterLi, #258)
* Terminal font support for Termux (@CarterLi, #263)
* Weather module (@CarterLi, #266)

Logos
* Crystal linux (@AloneER0, #239)
* FreeBSD (@draumaz, #244)
* New Ubuntu (@AloneER0, #259)

Bugfixes:
* Don't segfault in GPU code on Intel Macs (@CarterLi, #236)
* Don't use hardcoded size units in presets (@dr460nf1r3, #255)
* Don't crash with some format strings (#252)
* --logo none keeps key color now (#264)

# 1.7.2

Fixes the bash completions

# 1.7.1

This release brings a lot of bug fixes and improvements for MacOS. Big thanks to @CarterLi for the help on this!

Features:
* The color of the title and the keys can now be configured individually, using `--color-keys` and `--color-title` respectively. Some distros have different defaults now, similar to neofetch
* Swap module, similar to the Memory module, but for swap. Add `Swap` to your structure to enable it (#225)

Logos:
* Slackware (#227)

Bugfixes:
* Used disk space is now calculated much more accurately
* On Linux, GPU names are no longer truncated, if they are longer than 32 characters (#224)
* On Linux, NVIDIA GPUs once again have a proper name

* On M1 platforms, showing the GPU name no longer crashes the program (#222)
* Brew package count does now work on M1 platforms too
* The Vulkan module now does work on MacOS too
* The OpenGL and OpenCL modules now work on MacOS too (@CarterLi, #226)
* The LocalIp module now works on MacOS too (@CarterLi, #232)
* Detecting custom WMs on MacOS does now work

Other:
* GitHub actions now builds a dmg file for MacOS, as you can see in the release page

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
