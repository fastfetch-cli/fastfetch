# fastfetch

fastfetch is a [neofetch](https://github.com/dylanaraps/neofetch) like tool for fetching system information and displaying them in a pretty way. It is written in c to achieve much better performance, in return only linux is supported. It also uses mechanisms like multithreading and caching to finish as fast as possible.  
  
The output on my personal machine with default configurations looks like this:  
![example output](screenshots/example.png)

## Customization

With customization and speed being two competing goals, this project actually builds two executables.  
The main one being `fastfetch`, which can be very greatly configured via flags. These flags can be made persistent in `~/.config/fastfetch/config.conf`. To view the available options run `fastfetch --help`.  
The second executable being buildt is called `flashfetch`, which is configured at compile time to eliminate any possible overhead. Configuration of it can be very easily done in [`src/flashfetch.c`](src/flashfetch.c).  
At the moment the performance difference is measurable, but too small to be human recognizable. But the leap will get bigger with more and more options coming, and on slow machines this might actually make a difference.  
  
There are some presets defined for fastfech in [`presets`](presets), you can can load them with `--load-config`. They may also serve as a good example for format arguments.

## Dependencies

In order to run properly on every machine, fastfetch dynamically loads needed libraries if they are available. Therefore its only hard dependency is [`glibc`](https://www.gnu.org/software/libc/) (`libc`, `libdl` and `libpthread` are actually used) which is automatically shipped with every linux system.  
The following libraries are used if present:
* [`libpci`](https://github.com/pciutils/pciutils): Needed for GPU output. _Should_ be available on every linux system.
* [`libX11`](https://gitlab.freedesktop.org/xorg/lib/libx11): Needed for resolution output
* [`libXrandr`](https://gitlab.freedesktop.org/xorg/lib/libxrandr): Needed for appending refresh rate to resolution output.
* [`libwayland-client`](https://wayland.freedesktop.org/): Better resolution performance + support for monitors with different refresh rates in wayland sessions.  
* [`libGIO`](https://developer.gnome.org/gio/unstable/): Needed for values that are only stored GSettings.
* [`libDConf`](https://developer.gnome.org/dconf/unstable/): Needed for values that are only stored in DConf + Fallback for GSettings.

## Support status
All categories not listed here should work without needing a specific implementation.

### Logos:  
```
Arch, Artix, Manjaro, Garuda, Debian, Ubuntu, Void
```
Unknown/unsupported logos will be replaced with a question mark when running fastfetch.

### Package managers:
```
Pacman, xbps, dpkg, Flatpak, Snap
```

### Window managers:
```
KWin, Mutter, Muffin, Openbox, XFWM, Sway, Wayfire, Weston
```

### Window manager themes:
```
KWin, Mutter, Muffin, XFWM, Openbox (LXDE, LXQT & without DE)
```

### Terminal fonts:
```
konsole, xfce4-terminal, tilix, lxterminal, TTY
```

## Building

fastfetch uses [`cmake`](https://cmake.org/) for building. The simplest steps to build the entire project are:  
```bash
mkdir -p build && \
cd build && \
cmake .. && \
cmake --build .
```
  
This will produce `build/fastfetch` and `build/flashfetch`, both standalone executables.  
Command line completions for bash can be found in [`completions/bash`](completions/bash).  

## Packaging

At the moment, i only package for the [AUR](https://aur.archlinux.org/packages/fastfetch-git/). This package will install both the fastfetch and the flashfetch binary (with default configuration), as well as the bash completion.  
There is also a package in the [Manjaro Repositories](https://gitlab.manjaro.org/packages/community/fastfetch), not packaged by me, but usually very up-to-date.

## FAQ

Q: Why do you need a very performant version of neofetch?
> I like putting neofetch in my ~/.bashrc to have a system overwiew whenever i use the terminal, but the slow speed annoyed me, so i created this. Also neofetch didn't output everything correctly (e.g Font is displayed as "[Plasma], Noto Sans, 10 [GTK2/3]") and writing my own tool gave me the possibility to fine tune it to run perfectly on at least my configuration.

Q: It does not display [*] correctly for me, what can i do?
> This is most likely because your system is not implemented (yet). At the moment i am focusing more on making the core app better, than adding more configurations. Feel free to open a pull request if you want to add support for your configuration
