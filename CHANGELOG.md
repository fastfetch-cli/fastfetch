# 2.0.0 (beta)

Fastfetch v2 introduces a new configuration file format: JSON config. Please refer to <https://github.com/fastfetch-cli/fastfetch/wiki/Configuration> for details.

Changes:
* Drop the dependency of cJSON. We now use [yyjson](https://ibireme.github.io/yyjson/doc/doxygen/html/index.html) to parse JSON documents.
* Remove `--shell-version` and `--terminal-version`. They are always enabled (Terminal / Shell)
* Remove `--*-error-format`, which seems to be useless
* Remove `--display-detect-name`. Display name is always detected, and will be printed if multiple displays are detected
* Deprecate `--set` and `--set-keyless`; they may be removed in future releases. Use JSON config with Custom module instead
* Remove the special handling of Command module (it can be set once in the triditional `config.conf`). Use JSON config with Command module instead
* Change `--wm-theme-*` to `--wmtheme-*`. Affect `key` and `format` (WMTheme)
* Change `--terminal-font-*` to `--terminalfont-*`. Affect `key` and `format` (TerminalFont)
* Module `Command` uses `/bin/sh` as the default shell on systems other than Windows (Command)
* Fix M2 CPU temperature detection (CPU, macOS)
* Detect monitor name when available, instead of using DRM connector name (Display / Brightness, Linux)

Features:
* FreeBSD support is improved greatly, and actually tested in a physical machine
* Add `--no-buffer` option for easier debugging. CMake option `ENABLE_BUFFER` is removed and always enabled.
* Support `--*-key-color` option to change the key color of specified module
* Support `--colors-symbol` and `--colors-padding-left` (Colors)
* Add LM (Login Manager) module. Currently requires systemd installed (thus Linux only)
* Add `--wmi-timeout` option (Windows)
* Add `--logo-type small` to search for small logos
* Support detecting brightness of external displays with DDC/CI (guard behind `--allow-slow-operations`) (Brightness)
* Add option `--size-ndigits` and `--size-max-prefix` (#494)
* Add option `--processing-timeout` to the timeout when waiting for child processes.
* Public IP module prints the IP location if `--publicip-url` is not set (PublicIP)
* Add option `--localip-default-route-only` (LocalIP)
* Add option `--weather-location` (Weather)
* Support iTerm non-ascii font detection (Terminal, macOS)
* Add option `--title-color-user`, `--title-color-at` and `--title-color-host` (Title)
* Add Exherbo logo and package manager count (Packages, Linux, #503)
* Add module `Terminal Size` which prints the number of terminal width and height in charactors and pixels
* Add new option `--temperature-unit`
* Better CPU and Host detection for Android (Android)
* Support yakuake terminal version & font detection (Terminal, Linux)
* Add new option `--bright-color` which can be used to disable the default bright color of keys, title and ASCII logo.
* Add module `Monitor` which prints physical parameters (native resolutions and demensions) of connected monitors
* Support path with environment variables for `--logo-source` and `--load-config`.

Bugfixes:
* Fix possible hanging (TerminalFont, #493)
* Fix heap-buffer-overflow reading (DisplayServer, Linux)
* Fix false errors when built without libnm support (Wifi, Linux)
* Properly detect CPU on POWER (CPU, Linux)
* Fix compatibility with Fig (Terminal, macOS)
* Fix option `--title-fqdn` doesn't work (Title)
* Fix used spaces calculation (Disk, Linux / BSD / macOS, #508)
* Fix `--brightness-format` (Brightness)
* Fix specifying `--set-keyless` with the same key second time won't override the value set before (#517)
* Fix specifying `--color` second time won't clear the value set before (#517)

Logo:
* Change the special handling of `kitty` protocol with `.png` image file to a new image protocol `kitty-direct`. This is the fastest image protocol because fastfetch doesn't need to pre-encode the image to base64 or something and the image content doesn't need to be transmitted via tty. Note:
    1. Although konsole was said to support `kitty` image protocol, it doesn't support `kitty-direct`
    2. wezterm support more image formats other than `.png` (tested with `.jpg` and `.webp`)
* Port all logos supported by neo(wo)fetch. Fastfetch now has 350 builtin logos in total.

# 1.12.2

Features:
* Support terminator terminal version detection (Linux, Terminal)
* Support `pkgtool` package manager detection (Linux, Packages)
* Support `Far` shell version detection (Windows, Shell)

Bugfixes:
* Fix ConEmu terminal detection in some special cases (Windows, Terminal, #488)
* Fix incorrect Host on M2 Mac Studio with M2 Max CPU (macOS, Host, #490)

# 1.12.1

Bugfixes:
* Fix compiling error on Apple Slicon (Bios, macOS)

# 1.12.0

This release backports some changes from dev branch, and fixes 2 crashing issues

Features:
* Support KDE / LXQT / MATE / Cinnamon wallpaper detection (Wallpaper, Linux)
* Support QTerminal version & terminal font detection
* Support MATE Terminal version & terminal font detection
* Set `--pipe true` automatically if stdout is not a tty
* Detect new macs released on WWDC 2023 (macOS, Host)
* Count cached memory as free memory (FreeBSD, Memory)
* Support sound detection (FreeBSD, Sound)

Bugfixes:
* Fix DE detection on Windows 8.1 (Windows, DE)
* Fix `--logo-padding-left` doesn't work when `--logo-padding-top` is set (Logo)
* Fix KDE version detection on Fedora (DE)
* Include limits.h when needed (Linux, #472)
* Fix Windows drives detection in WSL (Linux, Disk)
* Fix CPU temp detection (FreeBSD, CPU)
* Fix disk detection (Android, Disk)
* Fix Gnome Terminal version and font detection (FreeBSD, TerminalFont)
* Fix crash on newer wayland desktops (Linux, Display, #477)
* Fix vendor detection for Intel GPU (macOS, GPU)
* Fix possible crashes on Windows Server (Windows, GPU, #484)

Logo:
* Add bsd, freebsd_small, ghostbsd
* Make Windows 11 logo colorable

# 1.11.3

Bugfixes:
* Fix a segfault bug, regression of `1.11.1` (Linux, wmtheme, #467)

# 1.11.2

This release should be the last version of fastfetch 1.x (if no serious bugs found, hopefully)

Features:
* Support display name, type, rotation detection on Wayland (Linux, Display)
* Print more useful display name instead of intel_backlight (Linux, Brightness)
* Icons module supports Windows (Windows, Icons)
* Add Wallpaper module which shows the current wallpaper image path
* Add mac address detection `--localip-show-mac` (LocalIP, #451)

Bugfixes:
* Fix Gnome version detection on Fedora (DE)
* Fix Windows drives detection in WSL (Disk)

Changes:
* In order to make Icons module consistant between different platforms, `--icons-format` no longer supports individual GTK / QT icon params.
* `--theme-format` no longer supports individual GTK / plasma theme params.
* `--local-ip-*` and `--public-ip-*` have been changed to `--localip-*` and `--publicip-*`
* `--localip-compact-type` is no longer supported. Fastfetch now display IPs as `--localip-compat-type multiline` by default, with `--local-compact true` can be set as an alias of `--localip-compact-type oneline`
* `--localip-v6first` is no longer supported.

# 1.11.1

Features:
* Support xonsh detection (TerminalShell)
* Support Tabby version / terminal font detection (TerminalFont)

Bugfixes:
* Fix name of Pro Controller (Gamepad, Windows)
* Fix compile error with imagemagick enabled (Windows)
* Fix copy-and-paste errors (Gamepad)
* Flatpak: Fix user package count
* Flatpak: Count runtime packages too (#441)
* Fix flatpak package count (#441)
* Don't print white color blocks with `--pipe` (#450)
* Fix iTerm being detected as iTermServer-* sometimes
* Fix sound device volume being incorrectly detected as muted sometimes (Sound)
* Fix memleaks reported by LeakSanitizer (Linux)
* Fix potential memory curruption bug in unicode.c (Windows)

Logo:
* Update Windows 11 ASCII logo to look more visually consistent (#445)
* Add another font color index to arch icon (#446)
* Add SteamOS
* Add macOS small / small2

# 1.11.0

Features:
* Support linuxbrew (Packages, Linux)
* Support foot terminal (#431, Linux)
* Support cursor size detection on Windows (Cursor, Windows)
* Support cursor detection on macOS (Cursor, macOS)
* Support display name, display type and decimal refresh rate detection (Display, macOS / Windows)
* Support `--display-compact-type` to display multiple resolutions in one line (Display)
* Support flatpak-user (Packages, Linux, #436)
* Support `--gpu-force-vulkan` to force using vulkan to detect GPUs, which support video memory usage detection with `--allow-slow-operations` (GPU)

Bugfixes:
* Fix date time format
* Fix compiling with musl (Wifi, Linux, #429)
* Don't exit if libpci is failed to init (GPU, Linux, #433)
* Names of most well-known gamepads are correctly printed instead of `Wireless Controller` on Windows

Logo:
* Small update for nobara logo (#435, @regulargvy13)

# 1.10.3

Bugfixes:
* Fix uninitialized variables (GPU, Windows)
* Fix compiling errors (Windows)

Improvements:
* Improve preformance (WmTheme amd Font, Windows and macOS)
* Enable nonblocking public-ip / weather detection (Android)

# 1.10.2

Bugfixes:
* Handle `kAudioObjectPropertyElementMain` for macOS **SDK** < 12 (#425, @nandahkrishna)
* Add missing `NULL` for `ffProcessAppendStdOut` (#421)

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
