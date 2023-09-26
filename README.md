# Fastfetch

![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/fastfetch-cli/fastfetch/ci.yml)
![GitHub license](https://img.shields.io/github/license/fastfetch-cli/fastfetch)
![GitHub contributors](https://img.shields.io/github/contributors/fastfetch-cli/fastfetch)
![GitHub top language](https://img.shields.io/github/languages/top/fastfetch-cli/fastfetch?logo=c&label=)
![GitHub commit activity (branch)](https://img.shields.io/github/commit-activity/m/fastfetch-cli/fastfetch)  
![homebrew downloads](https://img.shields.io/homebrew/installs/dm/fastfetch?logo=homebrew)
![GitHub all releases](https://img.shields.io/github/downloads/fastfetch-cli/fastfetch/total?logo=github)  
![GitHub release (with filter)](https://img.shields.io/github/v/release/fastfetch-cli/fastfetch?logo=github)
[![latest packaged version(s)](https://repology.org/badge/latest-versions/fastfetch.svg)](https://repology.org/project/fastfetch/versions)
[![Packaging status](https://repology.org/badge/tiny-repos/fastfetch.svg)](https://repology.org/project/fastfetch/versions)

Fastfetch is a [neofetch](https://github.com/dylanaraps/neofetch)-like tool for fetching system information and displaying them in a pretty way. It is written mainly in C, with performance and customizability in mind. Currently, Linux, Android, FreeBSD, MacOS and Windows 7+ are supported.

<img src="screenshots/example1.png" width="49%" align="left" />
<img src="https://upload.wikimedia.org/wikipedia/commons/2/24/Transparent_Square_Tiles_Texture.png" width="49%" height="16px" align="left" />
<img src="screenshots/example4.png" width="49%" align="left" />
<img src="https://upload.wikimedia.org/wikipedia/commons/2/24/Transparent_Square_Tiles_Texture.png" width="49%" height="16px" align="left" />
<img src="screenshots/example2.png" width="48%" align="top" />
<img src="screenshots/example3.png" width="48%" align="top" />
<img src="screenshots/example5.png" height="15%" align="top" />

There are [screenshots on different platforms](https://github.com/fastfetch-cli/fastfetch/wiki)

## Customization

With customization and speed being two competing goals, this project actually builds two executables:

* The main one is `fastfetch`, which can be very greatly configured via flags. These flags can be made persistent by modifying `$XDG_CONFIG_HOME/fastfetch/config.conf`. Use `fastfetch --gen-config conf` to generate one. To view the available options, run `fastfetch --help`.
* The second executable is called `flashfetch`, which is configured at compile time to eliminate any possible overhead. Configuration of it can be very easily done in [`src/flashfetch.c`](src/flashfetch.c).

Currently, the performance difference is measurable, but too small to be recognizable by humans. But with more options planned, the leap will get bigger over time and on slow machines this might actually make a difference.

There are some premade config files in [`presets`](presets), including the ones used for the screenshots above. You can load them using `--load-config <filename>`. They may also serve as a good example for format arguments.

Logos can be heavily customized too; see the [logo documentation](https://github.com/fastfetch-cli/fastfetch/wiki/Logo-options) for more information.

### Customization with new JSONC format

A new, structure based, and user-friendly config file format was introduced in v2.0.0. This format is based on [JSONC](https://jsonc.org/). [See more details in Wiki](https://github.com/fastfetch-cli/fastfetch/wiki/Configuration)

## Dependencies

Fastfetch dynamically loads needed libraries if they are available. On Linux, its only hard dependencies are `libc` (any implementation of the c standard library), `libdl`, `libm` and [`libpthread`](https://man7.org/linux/man-pages/man7/pthreads.7.html) (if built with multithreading support). They are all shipped with [`glibc`](https://www.gnu.org/software/libc/), which is already installed on most Linux distributions.

The following libraries are used if present at runtime:

### Linux and FreeBSD

* [`libpci`](https://github.com/pciutils/pciutils): GPU output.
* [`libvulkan`](https://www.vulkan.org/): Vulkan module & fallback for GPU output.
* [`libxcb-randr`](https://xcb.freedesktop.org/),
    [`libXrandr`](https://gitlab.freedesktop.org/xorg/lib/libxrandr),
    [`libxcb`](https://xcb.freedesktop.org/),
    [`libX11`](https://gitlab.freedesktop.org/xorg/lib/libx11): At least one of them sould be present in X11 sessions for better display detection and faster WM detection. The `*randr` ones provide multi monitor support The `libxcb*` ones usually have better performance.
* [`libwayland-client`](https://wayland.freedesktop.org/): Better display performance and output in wayland sessions. Supports different refresh rates per monitor.
* [`libGIO`](https://developer.gnome.org/gio/unstable/): Needed for values that are only stored GSettings.
* [`libDConf`](https://developer.gnome.org/dconf/unstable/): Needed for values that are only stored in DConf + Fallback for GSettings.
* [`libmagickcore` (ImageMagick)](https://www.imagemagick.org/): Images in terminal using sixel or kitty graphics protocol.
* [`libchafa`](https://github.com/hpjansson/chafa): Image output as ascii art.
* [`libZ`](https://www.zlib.net/): Faster image output when using kitty graphics protocol.
* [`libDBus`](https://www.freedesktop.org/wiki/Software/dbus): Bluetooth, Player & Media detection.
* [`libEGL`](https://www.khronos.org/registry/EGL/),
    [`libGLX`](https://dri.freedesktop.org/wiki/GLX/),
    [`libOSMesa`](https://docs.mesa3d.org/osmesa.html): At least one of them is needed by the OpenGL module for gl context creation.
* [`libOpenCL`](https://www.khronos.org/opencl/): OpenCL module
* [`libXFConf`](https://gitlab.xfce.org/xfce/xfconf): Needed for XFWM theme and XFCE Terminal font.
* [`libsqlite3`](https://www.sqlite.org/index.html): Needed for pkg & rpm package count.
* [`librpm`](http://rpm.org/): Slower fallback for rpm package count. Needed on openSUSE.
* [`libnm`](https://networkmanager.dev/docs/libnm/latest/): Used for Wifi detection.
* [`libpulse`](https://freedesktop.org/software/pulseaudio/doxygen/): Used for Sound detection.
* [`libddcutil`](https://github.com/rockowitz/ddcutil): Used for brightness detection of external displays
* [`DirectX-Headers`](https://github.com/microsoft/DirectX-Headers): Used for GPU detection in WSL

### macOS

* [`MediaRemote`](https://iphonedev.wiki/index.php/MediaRemote.framework): Need for Media detection. It's a private framework provided by newer macOS system.
* [`DisplayServices`](https://developer.apple.com/forums/thread/666383#663154022): Need for screen brightness detection. It's a private framework provided by newer macOS system.
* [`MoltenVK`](https://github.com/KhronosGroup/MoltenVK): Vulkan driver for macOS. [`molten-vk`](https://github.com/Homebrew/homebrew-core/blob/HEAD/Formula/molten-vk.rb)
* [`libmagickcore` (ImageMagick)](https://www.imagemagick.org/): Images in terminal using sixel graphics protocol. [`imagemagick`](https://github.com/Homebrew/homebrew-core/blob/HEAD/Formula/imagemagick.rb)
* [`libchafa`](https://github.com/hpjansson/chafa): Image output as ascii art. [`chafa`](https://github.com/Homebrew/homebrew-core/blob/HEAD/Formula/chafa.rb)
* [`libsqlite3`](https://www.sqlite.org/index.html): Used for fast wallpaper detection ( fallback to AppleScript )

For the image logo, iTerm with iterm image protocol should work. Apple Terminal is not supported.

### Windows

* [`wlanapi`](https://learn.microsoft.com/en-us/windows/win32/api/wlanapi/): A system dll which isn't supported by Windows Server by default. Used for Wifi info detection.
* [`libvulkan`](https://www.vulkan.org/): Vulkan module. Usually has been provided by GPU drivers. [`vulkan-loader`](https://github.com/msys2/MINGW-packages/tree/master/mingw-w64-vulkan-loader) [`vulkan-headers`](https://github.com/msys2/MINGW-packages/tree/master/mingw-w64-vulkan-headers)
* [`libOpenCL`](https://www.khronos.org/opencl/): OpenCL module. [`opencl-icd`](https://github.com/msys2/MINGW-packages/tree/master/mingw-w64-opencl-icd)

Note: In Windows 7, 8 and 8.1, [ConEmu](https://conemu.github.io/en/AnsiEscapeCodes.html) is required to run fastfetch due to [the lack of ASCII escape code native support](https://en.wikipedia.org/wiki/ANSI_escape_code#DOS,_OS/2,_and_Windows). In addition, as fastfetch for Windows targets [UCRT](https://learn.microsoft.com/en-us/cpp/windows/universal-crt-deployment) C runtime library, [it must be installed manually](https://support.microsoft.com/en-us/topic/update-for-universal-c-runtime-in-windows-c0514201-7fe6-95a3-b0a5-287930f3560c) as UCRT is only pre-installed in Windows 10 and later.

For the image logo, WezTerm with iterm image protocol is known to work, surprisingly.

### Android

* [`freetype`](https://www.freetype.org/): Used for Termux font detection. [`freetype`](https://github.com/termux/termux-packages/tree/master/packages/freetype)
* [`libvulkan`](https://www.vulkan.org/): Vulkan module, also used for GPU detection. Usually has been provided by Android system. [`vulkan-loader-android`](https://github.com/termux/termux-packages/tree/master/packages/vulkan-loader-android) [`vulkan-headers`](https://github.com/termux/termux-packages/tree/master/packages/vulkan-headers)
* [`termux-api`](https://github.com/termux/termux-api-package): Used for Wifi / Battery detection. Read [the official doc](https://wiki.termux.com/wiki/Termux:API) for detail or if you hang on these modules (IMPORTANT). [`termux-api`](https://github.com/termux/termux-packages/tree/master/packages/termux-api)
* [`libandroid-wordexp-static`](https://github.com/termux/termux-packages/tree/master/packages/libandroid-wordexp): `wordexp.h` support for Android.

For the image logo, [Termux Monet](https://github.com/HardcodedCat/termux-monet) supports iterm image protocol.

## Support status
All categories not listed here should work without needing a specific implementation.

##### Available Modules
```
Battery, Bios, Bluetooth, Board, Break, Brightness, Colors, Command, CPU, CPUUsage, Cursor, Custom, Date, DateTime, DE, Disk, Display, Font, Gamepad, GPU, Host, Icons, Kernel, LM, Locale, LocalIP, Media, Memory, Monitor, NetUsage, OpenCL, OpenGL, OS, Packages, Player, Power Adapter, Processes, PublicIP, Separator, Shell, Sound, Swap, Terminal, Terminal Font, Terminal Size, Theme, Time, Title, Uptime, Version, Vulkan, Wallpaper, Weather, Wifi, WM, WMTheme
```

##### Builtin logos

<!-- `Content of src/logo/builtin.c`.split('\n').map(x => x.trim()).filter(x => x.startsWith('// ')).map(x => x.slice(3)).filter(x => x != 'LAST').sort((a, b)=>a.toUpperCase().localeCompare(b.toUpperCase())).join(', ') -->
```
AerOS, AIX, AlmaLinux, Alpine, Alpine2Small, AlpineSmall, Alter, Amazon, AmogOS, Anarchy, Android, AndroidSmall, Antergos, Antix, AoscOS, AoscOsRetro, AoscOsRetro_small, Aperture, Apple, AppleSmall, Apricity, Arch, Arch2, ArchBox, Archcraft, Archcraft2, Archlabs, ArchSmall, ArchStrike, ArcoLinux, ArcoLinuxSmall, ArseLinux, Artix, Artix2Small, ArtixSmall, Arya, Asahi, Aster, AsteroidOS, AstOS, Astra, Ataraxia, Athena, Bedrock, BigLinux, Bitrig, BlackArch, BlackPanther, BLAG, BlankOn, BlueLight, Bodhi, Bonsai, BSD, BunsenLabs, CachyOS, CachyOSSmall, Calculate, CalinixOS, CalinixOSSmall, Carbs, CBL-Mariner, CelOS, Center, CentOS, CentOSSmall, Chakra, ChaletOS, Chapeau, ChonkySealOS, Chrom, Cleanjaro, CleanjaroSmall, ClearLinux, ClearOS, Clover, Cobalt, Condres, ContainerLinux, CRUX, CRUXSmall, CrystalLinux, Cucumber, CutefishOS, CuteOS, CyberOS, Dahlia, DarkOS, Debian, DebianSmall, Deepin, DesaOS, Devuan, DevuanSmall, DietPi, DracOS, DragonFly, DragonFlyOld, DragonFlySmall, Drauger, Droidian, Elementary, ElementarySmall, Elive, EncryptOS, Endeavour, Endless, Enso, EuroLinux, EvolutionOS, Exherbo, ExodiaPredator, Fedora, FedoraOld, FedoraSmall, FemboyOS, Feren, Finnix, Floflis, FreeBSD, FreeBSDSmall, FreeMiNT, Frugalware, Funtoo, GalliumOS, Garuda, GarudaDragon, GarudaSmall, Gentoo, GentooSmall, GhostBSD, Glaucus, GNewSense, Gnome, GNU, GoboLinux, GrapheneOS, Grombyang, Guix, GuixSmall, Haiku, HaikuSmall, HamoniKR, HarDClanZ, HardenedBSD, Hash, Huayra, Hybrid, HydroOS, Hyperbola, HyperbolaSmall, Iglunix, InstantOS, IRIX, Itc, Januslinux, Kaisen, Kali, KaliSmall, KaOS, KDENeon, Kibojoe, KISSLinux, Kogaion, Korora, KrassOS, KSLinux, Kubuntu, LangitKetujuh, Laxeros, LEDE, LibreELEC, Linspire, Linux, LinuxLight, LinuxLightSmall, LinuxMint, LinuxMintOld, LinuxMintSmall, LinuxSmall, Live_Raizo, LMDE, Lunar, MacOS, MacOS2, MacOS2Small, MacOSSmall, Mageia, MageiaSmall, MagpieOS, Mandriva, Manjaro, ManjaroSmall, MassOS, MatuusOS, MaUI, Meowix, Mer, Minix, Mint, MintOld, MintSmall, MiracleLinux, MOS, Msys2, MX, MXSmall, Namib, Nekos, Neptune, NetBSD, NetRunner, Nitrux, NixOS, NixOS_small, NixOsOld, NixOsSmall, Nobara, NomadBSD, Nurunner, NuTyX, Obarun, OBRevenge, OmniOS, OpenBSD, OpenBSDSmall, OpenEuler, OpenIndiana, OpenKylin, OpenMamba, OpenMandriva, OpenStage, OpenSuse, OpenSuseLeap, OpenSuseSmall, OpenSuseTumbleweed, OpenWrt, OPNsense, Oracle, Orchid, OrchidSmall, OS_Elbrus, OSMC, OSX, OSXSmall, PacBSD, Panwah, Parabola, ParabolaSmall, Parch, Pardus, Parrot, Parsix, PCBSD, PCLinuxOS, PearOS, Pengwin, Pentoo, Peppermint, PhyOS, Pisi, PNMLinux, Pop, PopSmall, Porteus, PostMarketOS, PostMarketOSSmall, Proxmox, PuffOS, Puppy, PureOS, PureOSSmall, Q4OS, Qubes, Qubyt, Quibian, Radix, Raspbian, RaspbianSmall, RavynOS, Reborn, RebornSmall, RedCore, RedHatEnterpriseLinux, RedHatEnterpriseLinux_old, RedstarOS, Refracted Devuan, Regata, Regolith, RhaymOS, RockyLinux, RockyLinuxSmall, RosaLinux, Sabayon, Sabotage, Sailfish, SalentOS, SalientOS, Salix, SambaBOX, Sasanqua, Scientific, Semc, Septor, Serene, SharkLinux, ShastraOS, Siduction, SkiffOS, Slackel, Slackware, SlackwareSmall, Slitaz, SmartOS, Soda, Solaris, SolarisSmall, Solus, SourceMage, Sparky, Star, SteamOS, StockLinux, Sulin, Suse, SuseSmall, Swagarch, T2, Tails, TeArch, TorizonCore, Trisquel, TuxedoOS, Twister, Ubuntu, Ubuntu2Old, Ubuntu2Small, UbuntuBudgie, UbuntuCinnamon, UbuntuGnome, UbuntuKde, UbuntuKylin, UbuntuMate, UbuntuOld, UbuntuSmall, UbuntuStudio, UbuntuSway, UbuntuTouch, UbuntuUnity, Ultramarine, Univalent, Univention, UOS, UrukOS, Uwuntu, Vanilla, Venom, Vnux, Void, VoidSmall, Vzlinux, WiiLinuxNgx, Windows, Windows11, Windows11Small, Windows8, Windows95, Xferience, YiffOS, Zorin
```

Run `fastfetch --print-logos` to print them

##### Package managers
```
apk, brew, Chocolatey, dpkg, emerge, eopkg, Flatpak, MacPorts, nix, Pacman, paludis, pkg, pkgtool, rpm, scoop, Snap, xbps
```

##### WM themes
```
DWM (Windows), KWin, Marco, Muffin, Mutter, Openbox (LXDE, LXQT & without DE), Quartz Compositor (macOS), XFWM
```

##### DE versions
```
Budgie, Cinnamon, Gnome, KDE Plasma, LXQt, Mate, XFCE4
```

##### Terminal fonts
```
Alacritty, Apple Terminal, ConEmu, Deepin Terminal, foot, Gnome Terminal, iTerm2, Kitty, Konsole, LXTerminal, MATE Terminal, mintty, QTerminal, Tabby, Terminator, Termux, Tilix, TTY, Warp, WezTerm, Windows Terminal, XFCE4 Terminal, Yakuake
```

## Building

fastfetch uses [`cmake`](https://cmake.org/) for building. [`pkg-config`](https://www.freedesktop.org/wiki/Software/pkg-config/) is recommended for better library detection. The simplest steps to build the fastfetch and flashfetch binaries are:
```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target fastfetch --target flashfetch
```

If the build process fails to find the headers for a library listed in [dependencies](#dependencies), fastfetch will simply build without support for that specific feature. This means, it won't look for it at runtime and just act like it isn't available.

### Building on Windows

Currently GCC or clang is required (MSVC is not supported). MSYS2 with CLANG64 subsystem (or CLANGARM64 if needed) is suggested (and tested) to build fastfetch. If you need Windows 7 / 8.x support, using MINGW64 is suggested.

1. Install [MSYS2](https://www.msys2.org/#installation)
1. Open `MSYS2 / CLANG64` (not `MSYS2 / MSYS`, which targets cygwin C runtime)
1. Install dependencies
```bash
pacman -Syu mingw-w64-clang-x86_64-cmake mingw-w64-clang-x86_64-pkgconf mingw-w64-clang-x86_64-clang mingw-w64-clang-x86_64-vulkan-loader mingw-w64-clang-x86_64-opencl-icd
```

Follow the building instructions of Linux next.

## Packaging

### Repositories

[![Packaging status](https://repology.org/badge/vertical-allrepos/fastfetch.svg?header=)](https://repology.org/project/fastfetch/versions)

### Manual

* DEB / RPM package: `cmake --build . --target package`
* Install directly: `cmake --install . --prefix /usr/local`

## FAQ

Q: Why do you need a very performant version of neofetch?
> I like putting neofetch in my ~/.bashrc to have a system overwiew whenever I use the terminal, but the slow speed annoyed me, so I created this. Also neofetch didn't output everything correctly (e.g Font is displayed as "[Plasma], Noto Sans, 10 [GTK2/3]") and writing my own tool gave me the possibility to fine tune it to run perfectly on at least my configuration.

Q: It does not display [*] correctly for me, what can I do?
> This is most likely because your system is not implemented (yet). At the moment I am focusing more on making the core app better, than adding more configurations. Feel free to open a pull request if you want to add support for your configuration
