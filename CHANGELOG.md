# 2.12.0

Changes:
* The long deprecated options `--set` and `--set-keyless` are removed.
* `Kernel` module now prints kernel name by default

Features:
* Support `st` terminal font detection for font configuration compiled in `st` binary (TerminalFont, Linux)
* Add option `--color-output` to change output color of all modules except `Title`, `Separator`
    * `display.color.output` in JSONC config file
* Add option `--<module>-output-color` to change output color of one specified module, which overrides the global option `--color-output`
* Add option `--publicip-ipv6` to print IPv6 address (PublicIP)
* Add new module `Loadavg` to print load averages (Loadavg)
* Add new module `PhysicalMemory` to print information of physical memory devices (PhysicalMemory)
    * Requires root permission to work on Linux and FreeBSD
* Support specifying `--logo-width` only for `--kitty-direct` and `--iterm` (Logo)
* Add option `--localip-show-all-ips` to show all IPs assigned to the same interface (LocalIP)
    * By default first IP will be shown only

Bugfixes:
* Rename option `--temperature-unit` to `--temp-unit` as stated in help messages
* Fix compatibility with `(*term)` (#909, Terminal, macOS)
* Fix alternate logo doesn't work with type builtin (Logo)
* Fix DahliaOS detection (Logo)

Logos:
* Add openSUSE Slowroll
* Add macOS3
* Add Quirinux

# 2.11.5

Bugfix:
* Fix logo printing for OpenMandriva (#896)
* Remove `--os-file` in help messages

# 2.11.4

Changes:
* Fastfetch will print a colorless ascii logo in `--pipe` mode for better `lolcat` compatibility. `fastfetch | lolcat` should work and no `--pipe false` needed.
    * Previously the logo would be disabled in `--pipe` mode.
    * Use `--pipe -l none` to get the old beheavior
* `--os-file` was removed and CMake option `-DCUSTOM_OS_RELEASE_PATH=/path/to/os-release` was introduced for configuring at compile time by package managers if needed. This option should not used in most cases.

Bugfixes:
* Fix possible out-of-bound memory access (#868)
* Fix Apple Terminal detection (#878, macOS, Terminal)
* Fix deprecation warning for macOS 14.0 hopefully (#860, macOS, Camera)
* Fix memory leaks when passing informative options (#888)
* Fix JSON config `size.ndigits` doesn't work 

Features:
* Enable `--pipe` mode if environment variable `$NO_COLOR` is set
* Support Armbian and Proxmox distro detection (OS, Linux)

Logo:
* Add Armbian

# 2.11.3

Hotfix for nix (https://github.com/NixOS/nixpkgs/issues/308849#issuecomment-2093962376)

Features:
* Add cmake option `CUSTOM_AMDGPU_IDS_PATH` for specifying custom path of `amdgpu.ids`

Bugfixes:
* Fix hanging when detecting disconnected network drive (Disk, Windows)
* Ensure line ending is printed when printing image logo errors (Logo)
* Revert image logo limitation change in 2.11.2; allow image logo in SSH session and tmux again (#861, Logo)
* Fix doubled output in custom formation (PhysicalDisk, Windows)

# 2.11.2

Hotfix for Debian 11

Changes:
* Error messages when trying to print image logo will only be printed with `--show-errors`
* When generating JSON output, fastfetch will generate an empty array when no result is detected, instead of an error.

Bugfixes:
* Fix segfault in Debian 11 and some old kernels. Regression introduced in 2.11.0 (#845, GPU, Linux)
* Don't try detecting version of raw `sh` shell (#849, Shell, Linux)
* Trim `\r` on Windows

Features:
* Check xdg state home for nix user packages (#837, Packages, Linux)
* Disable image logos in ssh and tmux sessions (#839)
* Support MX Linux distro detection (#847, OS, Linux)

Logo:
* Add KernelOS
* Fix name of DraugerOS
* Add missing `FF_LOGO_LINE_TYPE_SMALL_BIT` flags
* Add MX2

# 2.11.1

Hotfix for Android

Bugfixes:
* Fix uninitialized variables which can cause crashes (#760 #838, Battery, Android)
* Don't detect hyfetch as shell when used as backend of [hyfetch](https://github.com/hykilpikonna/hyfetch)
* Fix incorrect information in man page (#828)

Features:
* Support sorcery package manager detection (Packages, Linux)
* Make `--custom-format` optional (Custom)
* Make `/` an alias of `C:\` for `--disk-folders` (Disk, Windows)
* Build for Linux armv7

Logo:
* Fix colors of Source Mage logo

# 2.11.0

Changes:
* Default `hideCursor` to false. It doesn't make much difference but makes user's terminal unusable if fastfetch is not exited correctly.
* Linux amd64 binaries are built with Ubuntu 20.04 again (#808)

Bugfixes:
* Fix swap usage detection in x86-32 build (Windows, Swap)
* Fix minimum cmake version support (#810)
* Fix wifi detection on platforms that don't use NetworkManager (#811, Wifi, Linux)
* Fix NixOS wrapped process name (#814, Terminal, Linux)
* Fix GPU type detection for AMD cards (#816, GPU, Linux)
* Silence system deprecation warnings (#822, Camera, macOS)

Features:
* Add basic support DE detection support for UKUI (DE, Linux)
* Support printing total number of nix / flatpak / brew packages (Packages)
    * See `fastfetch -h packages-format` for detail
* Better max CPU frequency detection support with `CPUID / 16H` instruction (CPU, Windows)
    * This requires Intel Core I Gen 6 or newer, and with `Virtual Machine Platform` Windows feature disabled. X86 only.
* Improve performance of nix packages detection (Packages, Linux)
* Make config specified in JSONC overridable by command line flags
    * Note this change only make global config overridable; module configs are still not
* Suggest increasing `--processing-timeout` when child process timeouts
* Only detect folders that specified by `--disk-folders`
    * Previously `--disk-folders` only omits unmatched disks from output
    * This option can be used to improve detection performance by ignoring slow network drives

# 2.10.2

Bugfixes:
* Fix a regression that detect x11 as wayland (#805, WM, Linux)

# 2.10.1

Bugfixes:
* Fix build with `-DENABLE_DBUS=OFF` (Linux)

# 2.10.0

Changes:
* We now always detect max frequency of GPUs for consistent, instead of current frequency

Features:
* Improve display detection for wlroots based WMs. Fastfetch now correctly reports fractional scale factors in hyprland (Display, Linux)
* Improve GPU detection on Linux (GPU, Linux)
    * Support GPU memory usage detection for AMD GPUs
    * Support GPU frequency detection for Intel GPUs
* Improve performance of Gnome version detection (DE, Linux)
* Improve performance of kitty version detection (Terminal, Linux)
* Detect refresh rate when using `--ds-force-drm sysfs-only` (Display, Linux)
* Add option `--ts-version` to disable terminal and shell version detection. Mainly for benchmarking purposes
* Improve performance of detecting WSL version (Host, Linux)

Bugfixes:
* Correctly detect `/bin/sh` as current shell if it's used as default shell (#798, Shell, Linux)
* Work around an issue which CPU module reports incorrect CPU frequency that is too high (#800, CPU, Linux)
* Don't print ANSI escape codes in `--pipe` mode

# 2.9.2

Changes:
* To make use of the newly introduced `yyjson` flag `YYJSON_WRITE_NEWLINE_AT_END`, fastfetch now requires `yyjson` 0.9.0 or later

Features:
* Always add a final new-line when generating JSON output
* Detect partition create time, which can be used as OS installation time (Disk)
* Print time string when generating JSON result instead of UNIX epoch time number, which is more human-readable

Bugfixes:
* Fix a memory leak
* Better portable mode detection of Windows Terminal (TerminalFont, Windows)
* Fix parsing of option `--packages-disabled` (Packages)
* Don't use command `time` as a shell (Shell)

Logos:
* Add openSUSE MicroOS
* Fix color of AOSC OS

# 2.9.1

Features:
* Support weston-terminal (missed commit in v2.9.0) (TerminalFont, Linux)
* Support hyprcursor detection (#776, Cursor, Linux)

Bugfixes:
* Fix `fastfetch --gen-config` raises SIGSEGV when `~/.config/fastfetch` doesn't exist. Regression of `2.9.0` (#778)

# 2.9.0

Features:
* Support Lxterminal version detection (Terminal, Linux)
* Support weston-terminal version detection (Terminal, Linux)
* Support `am` package manager detection (#771, Packages, Linux)
* Support network prefix length detection for IPv6 (LocalIP)
* Display all IPs when multiple IPs are assigned to the same interface (LocalIP)
* Add option `--localip-show-prefix-len` to show network prefix length for both IPv4 and IPv6. Defaults to `true` (LocalIP)

Bugfixes:
* Fix network prefix length detection when the value is greater than 24 (#773, LocalIP, Linux)
* For xfce4-terminal, use system mono font if no config file is found (TerminalFont, Linux)

# 2.8.10

Bugfixes:
* Don't display 0.00 GHz (CPU, FreeBSD)
* Don't detect manufactor of Qualcomm as ARM (CPU, Android)
* Ignore `chezmoi` (Terminal, Linux)
* Trim trailing possible whitespaces (PublicIP)
* Fix detection compatibility for KDE 6 (Font, Linux)
* Always use Metal API to detect vmem size (GPU, macOS)

Features:
* Improve stability; print more useful error message; avoid misuse (PublicIP / Weather)
* Use MS-DOS device name as mountFrom result, instead of useless GUID volume name (Windows, Disk)
* Some adjustments to Terminal detection (Terminal, Windows)
    * Don't pretty print CMD
    * Print conhost as Windows Console
    * Don't detect `wininit` as Terminal

Logo:
* Fix color of Arco Linux

# 2.8.9

Bugfixes:
* Don't detect `SessionLeader` as terminal, actually (Terminal, Linux)
* Fix blurry chafa result when specifying both width and height (#757, Logo)

Features:
* Support new MacBook Air (Host, macOS)
* Distinguish min frequency and base frequency (CPU)

Logo:
* Fix proxmox

# 2.8.8

Bugfixes:
* Fix old fish version compatibility (#744)
* Fix truncated texts in `--help format` (#745)
* Fix old vulkan-header and libdrm library compatibility (#748, Linux)
* Fix possible segfaults in `--help *-format` (#749)
* Fix invalid resolution detection when using libdrm (Linux, Display)
* Fix segfault when `/sys/devices/system/cpu/cpufreq/` doesn't exist (#750, CPU, Linux)
* Don't detect `SessionLeader` as terminal (Terminal, Linux)
* Fix detection of client IP (Users, Linux)

# 2.8.7

Bugfixes:
* Fix max CPU frequency detection for some cases (CPU, Linux)
* Fix some memory leaks
* Fix ddcutil 2.1 compatibility (Brightness, Linux)
* Workaround `permission denied` error when reading `/proc/uptime` (Uptime, Android)

Features:
* Support zellij version detection (Linux, Terminal)

Logo:
* Fix PostMarketOS

# 2.8.6

Changes:
* Due to newly introduced configs, JSONC option `{ "temperatureUnit": "C" }` has been changed to `{ "temp": { "unit": "C" } }`

Bugfixes:
* Fix incorrect GPU name detection for Intel iGPU on Linux (#736, GPU, Linux)

Features:
* Support additional temperature formatting options (#737)
    * `{ "temp": { "ndigits": 1 } }`
    * `{ "temp": { "color": { "green": "green", "yellow": "yellow", "red": "red" } } }`
* Support specifying custom `pci.ids` path for Linux (GPU, Linux)
* Support warp-linux terminal version & terminal font detection (Terminal, Linux)

# 2.8.5

Bugfixes:
* Fix uninitialized variables

# 2.8.4

Bugfixes:
* Fix segfault if we fail to find `Vendor ID` in `lscpu` (#718, CPU, Linux)
* Fix multi-device bcachefs filesystem compatibility (#731, Disk, Linux)

Features:
* Support portable Windows Terminal settings (#720, Terminal, Windows)
* Support `--color-block-width` and `--color-block-range` (#721, Colors)
* Support `--diskio-detect-total` to show total bytes read/written (DiskIO)
* Support `--netio-detect-total` to show total bytes received/sent (NetIO)
* Support `--packages-disabled` to disable specified package manager (#729, Packages)
* Support `--display-order` to sort multiple displays in a specific order (Display)
* Support `--display-compact-type original-with-refresh-rate` to show refresh rates in compact (oneline) mode (Display)

# 2.8.3

Bugfixes:
* Fix GPU name detection for AMD graphic cards (GPU, Linux / FreeBSD)

# 2.8.2

Changes:
* The linux binaries are now built with glibc 2.35, which means they no longer support Debian 11 and Ubuntu 20.04. Users using these distros may download the artifacts `fastfetch-linux-old` from GitHub Actions.

Features:
* Rewrite GPU module, drop libpci dependency (GPU, Linux)
* Detect marketing name of Apple Silicon CPUs for asahi linux (CPU, Linux)
* Add new module `Camera`, which prints the name and resolution of connected cameras

Bugfixes:
* Fix compatibility with packages installed by flatpak (Terminal, Linux)
* Don't show an empty battery if no battery is detected (macOS, Battery)
* Don't show `not connected` if no power adapter is found (macOS / Linux, PowerAdapter)
* Make format of battery status be consistent with other platforms (Linux, Battery)

Logo:
* Print Asahi logo in asahi linux (Logo, Linux)
* Add Asahi2, z/OS, Tatra, PikaOS

# 2.7.1

Features:
* Config presets in app folder now work with symlinks

Bugfixes:
* Fix a possible segfault when detecting terminal (Terminal, Linux)

# 2.7.0

Features:
* Add new module `TerminalTheme`, which prints the foreground and background color of the current terminal window. Currently doesn't work on Windows.
* Allow command substitution when expanding paths. For example, now it's possible to use `"source": "$(ls ~/path/to/images/*.png | shuf -n 1)"` in JSONC config file to randomly choose an image to display. (#698)
* Use native methods instead of pciutils to detect GPUs in FreeBSD. (GPU, FreeBSD)

Bugfixes:
* Fix text formatting (Wifi, Linux)
* Fix terminal detection in some cases (Terminal)
* Remove trailing `\0` in JSON results (FreeBSD)
* Fix uninitialized variables (GPU, Linux)
* Fix a possible segfault (OpenCL)

Logo:
* Add ASCII logos for fedora immutable variants (#700)

# 2.6.3

Bugfixes:
* Fix module not working (Bluetooth)

# 2.6.2

Bugfixes:
* Fix building for GCC in Windows (Windows)

# 2.6.1

Features:
* Improve xonsh shell detection (Shell)
* Support colored percentage values (Bluetooth / Gamepad / Sound)
* Add `--<module>-percent-[green|yellow]` options to specify threshold of percentage colors
    * eg. `--disk-percent-green 20 --disk-percent-yellow 50` will show green if disk usage is less than 20%, yellow if disk usage is less then 50%, and red otherwise.
* Add `--percent-color-[green|yellow|red]` options to specify color of different percent value states.
    * eg. `--percent-color-green blue` will show blue color if percent value falls in green state.
* Improve Intel macbook support (macOS)

Bugfixes:
* Fix segfault in CPU module when running in aarch64 machine without `lscpu` installed (CPU, Linux)
* Don't use `login` as terminal process (Terminal, Linux)
* Silence warnings when building in 32bit machines.
* Create sub folders when writing config file (#690)
* Improve user specific locale detection; fix locale detection in Windows 7 (Locale)
* Fix GPU type detection (GPU, macOS)

# 2.6.0

Changes:
* Remove support of option `--battery-dir`. We detect a lot of things in `/sys/class/*` and only module `Battery` supports specifying a custom directory for some reason, which is weird.
* Remove `--chassis-use-wmi` which is no longer used.

Features:
* Add `ENABLE_PROPRIETARY_GPU_DRIVER_API` cmake option to disable using of proprietary GPU driver APIs (GPU)
* Support wallpaper detection for macOS Sonoma (Wallpaper, macOS)
* Support power adapter detection for Asahi Linux (PowerAdapter, Linux)
* Support battery serial number and manufacturer date detection (Battery)
* Support host serial number and UUID detection (Host)
* Support battery level detection for gamepads where possible (Gamepad)
* Support maximum CPU clock detection. Previously base clock was printed (CPU, Windows)
* Support manufacture date and serial number detection for physical monitors (Monitor)
* Support ash (default shell of BusyBox) version detection (Shell, Linux)
* Sound module in FreeBSD now uses native `ioctl`s. Pulseaudio dependency is no longer used.
* Locale module in Windows now prints the same format as in Linux and other posix systems.

Bugfixes:
* Fix overall memory leaks (macOS)
* Remove trailing `\0` in JSON results (FreeBSD)
* Fix physical monitor detection with Nvidia drivers (Monitor, Linux)
* Don't print llvmpipe in vulkan module (Vulkan)
* Fix system yyjson usage in `fastfetch.c`. Previously embedded `3rdparty/yyjson/yyjson.h` was used in `fastfetch.c` even if `ENABLE_SYSTEM_YYJSON` was set (CMake)
* Fix locale module printing unexpected results in specific environments (Locale)
* Fix battery temperature detection in Windows. Note only smart batteries report temperatures but few laptops uses smart battery (Battery, Windows)
* Print device name if no backlight name is available, so we don't print empty parentheses (Brightness, FreeBSD)

# 2.5.0

Changes:
* `--gpu-use-nvml` has been renamed to `--gpu-driver-specific` due to using of `IGCL` and `AGS`
* We now detect external partitions more conservatively in Linux. USB partitions will not be detected as external always ( eg. The Linux kernel itself is installed in a USB drive )

Features:
* Support more authentication type detection for macOS Sonoma (Wifi, macOS)
* Default preset names to `.jsonc`. For example, `fastfetch -c all` will load `presets/all.jsonc` (#666)
* Use Intel Graphics Control Library (IGCL) to detect more GPU information. Windows only (GPU, Windows)
* Improve support of Asahi Linux (Brightness / CPU / GPU / Disk, Linux)
* Support more properties of physical disks (PhysicalDisk)
* Support SSD temperature detection with `--physicaldisk-temp` (PhysicalDisk)
* Support partition label detection (Disk, FreeBSD)
* Support platform specific graphic API version detection (GPU, macOS / Windows)

Bugfixes:
* Fix Windows partition detection for WSL2 (Linux, Disk)
* Fix Btrfs subvolumes being detected as external partitions some times (Linux, Disk)
* Fix battery cycle counts in some places (Battery)
* Fix CodeWhisperer compatibility (#676, Terminal, macOS)

# 2.4.0

**We are deprecating flags based config files (will be removed in v3.0.0). We suggest you migrate to json based config files.** One may use `-c /path/to/config.conf --gen-config` to migrate existing flag based config files.

Changes:
* All flag based presets are removed

Features:
* Improve performance of detecting rpm and pkg package count (Packages, Linux / FreeBSD)
* Support Apple M3X temperature detection (CPU / GPU, macOS)
* `--ds-force-drm` support a new option `sysfs-only`
* Improve xfce4 version detection
* Detect WM and DE by enumerating running processes (WM / DE, FreeBSD)
* Add a new module `Physical Disk`, which detects product name, full size, serial number and so on.

Bugfixes:
* Fix crashes sometimes when `--logo-padding-top` is not set (Logo)
* Fix memory usage counting algorithm (Memory, macOS)
* Fix the behavior of `--no-buffer` in Windows
* Fix possible segfault in some devices (Display, Linux)
* Fix segfaults on first use of new images with Sixel flag (Image) 

Logo:
* Remove unnecessary escaping for Adelie logo
* Add EshanizedOS

# 2.3.4

Bugfixes:
* Fix `--help` doesn't work when built without python

Features:
* Use `MemAvailable` if available (Memory, Linux)
* Improve performance of detecting dpkg package count (Packages, Linux)

# 2.3.3

Bugfixes:
* Fix `--help` doesn't work in Windows and some other platforms

# 2.3.2

Bugfixes:
* Fix fish completion script, and install the script correctly

Logo:
* Fix Xray-OS logo name

# 2.3.1

Bugfixes:
* Fix man page install location

# 2.3.0

**We are deprecating flags based config files (will be removed in v3.0.0). We suggest you migrate to json based config files.**

Config related changes:
* The deprecated flag `--gen-config conf` is removed
* Flag `--gen-config` now does the same thing as `--migrate-config`, which can be used as config migration and default config file generation. Flag `--migrate-config` is removed
* Fastfetch now searches for config files in the order of `fastfetch --list-config-paths`, and won't load other config if one is found.
* The undocumented flag `--load-user-config` is removed. As an alternative, `--config none` can be used to disable loading config files.
* `--config` (previously named `--load-config`) is now supported for command line arguments only. If specified, other config files won't be loaded, which works like other programs.
* Config files will always be loaded before other command line flags being parsed. That is to say, command line flags will always override options defined in config files.
* the value of GPUType `integrated` contained a typo and was fixed. Existing config files may need to be updated.

Features:
* Support Oils and elvish shell version detection (Shell)
* Support Windows Server Core (Windows)
* Better ddcutil 2.x compatibility (Brightness, Linux)
* Add completion support for fish (natively) and nushell (via [carapace-bin](https://github.com/rsteube/carapace-bin))
* Support nix in macOS (Packages, macOS)
* Print module description for `--list-modules`
* Support `alacritty.toml` (TerminalFont)
* Support board detection on macOS. It simplily prints machine model identifier as for now (Board, macOS)
* Add general method to query product name (Host, macOS)
* Use `libdrm` as a better fall back for detecting displays, which correctly detects current mode; supports refresh rate detection and maybe also faster than using `/sys/class/drm` (Display, Linux)
* Support physical disk size detection (DiskIO)
* Support physical disk name and type detection (DiskIO, FreeBSD)

Bugfixes:
* End `va_list` before returning (@VoltrexKeyva)
* Don't use background color when printing blocks (Color)
* Fix lots of typos
* Fix compatibility with Linux containers (Linux)
* Don't report disabled monitors when using DRM (Linux)
* Fix bad performance in some cases when using X11 (Display, Linux)
* Fix some memory leaks
* Fix used swap space detection (Swap, FreeBSD)
* Don't leak fds to child processes (Linux)
* Fix possible issues when reading procfs (Linux, @apocelipes)

Logos:
* Add Adelie, Ironclad
* Update parch

# 2.2.3

Features:
* Update the latest mac models (Host, macOS)

Bugfixes:
* Fix local ips detection on Android. Regression from `2.0.0` (LocalIP, Android)
* Fix terminal detection on NixOS (Terminal)

# 2.2.2

Changes:
* `--percent-type` now defaults to 9 (colored percentage numbers)
* `fastfetch` now prints LocalIp module by default

Features:
* LocalIP module now prints netmask in CIDR format for IPv4 (LocalIP)
* Bios module now detects system firmware type (Bios)
* Improve detection of `Battery`
    * Detect cycle count on supported platforms
    * Detect temperature on Linux when supported
    * Status detection on macOS has been adjusted to be consistent with other platforms
* Linux binaries are built with imagemagick7 support

Bugfixes:
* Fix uninitialized variables (#609)
* Fix spelling of `--preserve-aspect-ratio` (#614)

Logos:

* Update NixOS_small

# 2.2.1

Hotfix release for #606

Bugfixes:
* Fix broken presets due to the breaking changes introduced in 2.2.0

Features:
* Pretty print `fastfetch --help`

# 2.2.0

This release introduces a new option `--migrate-config`, which migrates old flag based config file to new JSONC format

Changes:
* `--pipe` and `--stat` are moved from `general` options to `display` options. This affects cjson configuration.
* Display keys `percent*` and `size*` in JSON config are restructured. e.g. `{ "sizeNdigits": 1 }` is now `{ "size": { "ndigits": 1 } }`
* With the introduction of `--migrate-config`, the old flag based config file is deprecated, and will be removed in 3.0.0 (next major version)
* Support of `--gen-config conf` is deprecated accordingly, and will be removed in 2.3.0 (next minor version)
* The global flag `--allow-slow-operations` is split into some explicit flags in different modules
    * `--packages-winget`: control whether `winget` packages count should be detected. Note it's a very slow operation, please enable it with caution.
    * `--chassis-use-wmi`: control whether `WMI` query should be used to detect chassis type, which detects more information, but slower. This flag only affects `--chassis-format` and `--format json`.
    * `--battery-use-setup-api`: control whether `SetupAPI` should be used on Windows to detect battery info, which supports multi batteries, but slower.
    * `--wm-detect-plugin`: control whether WM plugins should be detected. Note it's implemented with global processes enumeration and can report false results.
    * `--de-slow-version-detection`: control DE version should be detected with slow operations. It's usually not necessary and only provided as a backup.
* `--localip-default-route-only` and `--netio-default-route-only` defaults to true to avoid large number of results

Features:
* Quirks for MIPS platforms (CPU, Linux)
* Use devicetree path for OBP hosts (Host, Linux)
* Detect `tmux: server` as tmux (Terminal, Linux)
* Support urxvt version detection (Terminal, Linux)
* Support st version detection (Terminal, Linux)
* Support st terminal font detection (TerminalFont, Linux)
* Support xfce4-terminal 1.1.0+ terminal font detection (TerminalFont, Linux)
* Add option `--migrate-config <?target-file-path>`
* Support Nvidia GPU temp and cuda core count detection via nvml. Use `--gpu-use-nvml` to enable it (GPU)
* Try supporting Wifi authentication type detection in macOS Sonoma. Please file a feature request if you get `to be supported (num)` with result of `/System/Library/PrivateFrameworks/Apple80211.framework/Resources/airport -I | grep auth` (Wifi, macOS)

Bugfixes:
* Better GPU memory and type detection (GPU, Windows)
* Don't print display type twice (Display)
* Detect BSSID instead of Wifi MAC address to align with other platforms (Wifi, macOS)
* Remove support of used GPU memory detection, which is not reliable and only supported with `--gpu-force-vulkan`. (GPU)
* Fix flag `--brightness-ddcci-sleep` (Brightness, Linux)
* Fix hanging if a child process prints to both stdout and stderr (Linux)

Logos:
* Add Black Mesa
* Add cycledream
* Add Evolinx
* Add azos
* Add Interix

# 2.1.2

Bugfixes:
* Fix icon detection on Windows. It shows enabled system icons in desktop (`This PC`, `Recycle Bin`, etc) (Icon, Windows)
* Fix compatibility with ddcutil 2.0 (Brightness, Linux)
* Fix a compile warning (CPUUsage, FreeBSD)

# 2.1.1

Features:
* Support opkg (Packages, Linux)
* Support GNOME Console terminal version and font detection (Terminal, Linux)
* Add `--cpu-freq-ndigits` to set number of digits for CPU frequency (CPU)
* New module to detect physical disk I/O usage (DiskIO)
* Add `--cpuusage-separate` to display CPU usage per CPU logical core
* Add `--brightness-ddcci-sleep` to set the sleep times (in ms) when sending DDC/CI requests (Brightness, #580)

Bugfixes:
* Fix possible crashes on Windows 7 (Disk, Windows)
* Fix possible crashes caused by uninitialized strings (Users, Windows)
* Improve support of `--help *-format` and several bugs are found and fixed
* Don't incorrectly print `No active sound devices found` when using a non-controllable sound device (Sound, macOS)
* Fix implementation processes counting (Processes, Linux)
* Work around a issue that SSID cannot be detected on macOS Sonoma (Wifi, macOS)

Logo:
* Add Chimera Linux
* Add EndeavourSmall
* Add Xenia
* Add MainsailOS
* Fix phyOS

# 2.1.0

This release introduces a new output format: JSON result

Changes:
* Users module detects and prints user login time by default. Specifying `--users-compact` to disable it
* Fastfetch now requires yyjson 0.8.0 or later, which is embedded in fastfetch source tree. If you build fastfetch with `-DENABLE_SYSTEM_YYJSON` cmake option, you must upgrade your yyjson package
* Reduced information supported by `--terminal-format`, `--shell-format`
* Some config presets (`devinfo` and `verbose`) are obsolete and removed. They are barely maintained and can be replaced with `--format json` now.
* Custom strings in `--module-key` and `--module-format` are no longer trimmed.
* `/boot` is hidden by default (FreeBSD, Disk)

Features:
* Add `--format json`, which prints system information as JSON format
* Add fast path for xfce4 version detection (DE, FreeBSD)
* Support contour terminal version and font detection (Terminal / TerminalFont)
* Support `kitty-direct` / `iterm` without specifying logo width / height. Note: in this case, the entre screen will be cleared.
* Support new flag `--logo-separate`. If true, print modules at bottom of the logo
* Support Apple Silicon CPU frequency detection (CPU, macOS)
* Support user login time detection (Users)
* Support winget package manager detection, guarded behind `--allow-slow-operations` (Packages, Windows)
* Print monitor type (built-in or external) (Display)
* Support full GPU detection in WSL (Linux, GPU)
* Add `--module-key " "` as a special case for hiding keys
* Support `--title-format`. See `fastfetch --help title-format` for detail
* Support `--colors-key` (Colors)
* Add `-c` as a shortcut of `--load-config`. Note it was used as the shortcut of `--color` before 2.0.5
* Support Windows Service Pack version detection (Kernel, Windows)
* Support Debian point releases detection (OS, Linux)
* Add new module `NetIO` which prints network throughput (usage) of specified interface. Note this module costs about 1 second to finish.
* Use `lscpu` to detect CPU name for ARM CPUs (CPU, Linux)

Bugfixes:
* Fix fastfetch hanging in specific environment (#561)
* Fix short read when reading from stdin (Logo)
* Fix `poll() timeout or failed` error when image is very large (Logo)
* Fix Termux Monet terminal version detection (Terminal)
* Fix zpool volumes detection (Disk, Linux)
* Fix external volumes detection (Disk, Linux)
* Fix snap package number detection on systems other than Ubuntu (Packages, Linux)
* Fix dpkg / apt package number detection (Packages, Linux)
* Fix bluetooth mac address detection (Bluetooth, Windows)

Logo:
* Add Afterglow
* Add Elbrus
* Update EvolutionOS
* Update AOSC OS
* Update Ubuntu_old
* Update Windows 11_small
* Add Amazon Linux
* Add LainOS
* Fix colors of Slackware

# 2.0.5

Bugfixes:
* Fix segfault when using libxrandr (#544, Display, Linux)
* Don't print 0px (#544, Cursor)

Features:
* Add option `--disk-use-available` (#543)
* Add option `--disk-show-readonly`

# 2.0.4

Bugfixes:
* Fix building on 32-bit FreeBSD (Memory, FreeBSD)
* Fix `--file-raw` doesn't work (Logo)

Features:
* Trait `-` as an alias for `/dev/stdin`. Available for `--file`, `--file-raw` and `--raw` (Logo)

# 2.0.3

Bugfixes:
* Fix typo in config parsing for --color-title (#534)
* Fix percent formatting for `--*-format` (#535)
* Fix loading presets for homebrew (macOS)

Features:
* Add option `--percent-ndigits`
* Add command flag `--config` as an alias of `--load-config`
* Windows packages now include presets (Windows)

# 2.0.2

Bugfixes:
* Workaround [a compiler bug of GCC](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282) (Windows)
* Fix presets not detected by file name (#529)

Logo:
* Add Tuxedo OS

# 2.0.1

First stable release of Fastfetch V2

Changes:
* Unescape strings only when parsing `.conf` files
    * Previously: `$ NO_CONFIG=1 fastfetch --os-key \\\\ -s os -l none` prints `\: *`. Note the backslashes are unescaped twice (once by shell and once by fastfetch).
    * Now: `$ NO_CONFIG=1 fastfetch --os-key \\\\ -s os -l none` prints `\\: *`
* Remove option shortcut `-c` (alias of `--color`), which is more commonly used as alias of `--config`
* Rename `--recache` to `--logo-recache` (which is used for regenerate image logo cache). Remove option shortcut `-r` (alias of `--recache`).
* Detecting brightness of external displays with DDC/CI is no longer guarded behind `--allow-slow-operations` (Brightness)

Features:
* Add `--key-width` for aligning the left edge of values, supported both for global `--key-width` and specific module `--module-key-width`
* Add `--bar-char-elapsed`, `--bar-char-total`, `--bar-width` and `--bar-border` options
* Add CMake option `ENABLE_SYSTEM_YYJSON`, which allow building fastfetch with system-provided yyjson (for package managers)
* Add new module `Version`, which prints fastfetch version (like `fastfetch --version`)

Bugfixes:
* Fix label detection. Use `--disk-key 'Disk ({2})'` to display it (Disk, Linux)
* Fix some module options were not inited
* Fix terminal version and font detection on NixOS (Terminal, Linux)

# 2.0.0-beta

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
* Add module `Terminal Size` which prints the number of terminal width and height in characters and pixels
* Add new option `--temperature-unit`
* Better CPU and Host detection for Android (Android)
* Support yakuake terminal version & font detection (Terminal, Linux)
* Add new option `--bright-color` which can be used to disable the default bright color of keys, title and ASCII logo.
* Add module `Monitor` which prints physical parameters (native resolutions and dimensions) of connected monitors
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
* In order to make Icons module consistent between different platforms, `--icons-format` no longer supports individual GTK / QT icon params.
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
* Fix potential memory corruption bug in unicode.c (Windows)

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
* Improve performance (WmTheme amd Font, Windows and macOS)
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
* Latest Mac model identifier support (macOS)

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
* Bios / Board / Chassis modules are split against Host module for performance reasons
* Caching is removed. Option `--nocache` is removed accordingly

Features:
* Windows (7 and newer) is officially and fully supported
* FreeBSD support is improved greatly (Bios, Cpu Temp, Cpu Usage, Disk, Host, Processes, Swap, Terminal / Shell, Uptime)
* Adds a new flag `--stat`, which prints time usage for individual modules
* Adds Wifi module which supports Windows and macOS
* Adds data source option for logo printing
* Detects Homebrew Cellar and Cask separately
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
