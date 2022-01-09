# Development help

This file is not an official documentation!
Here i just add things that are easy to forget.

## Features i am planning to implement

### Specific

- [ ] Support for printing images as ascii art logos (using imlib2 and libcaca probably)
- [ ] Support for printing images in terminals that support it
- [ ] Better OS output for all possible combinations of /etc/os-release variables.
- [ ] Fallback OS detection with lsb
- [ ] Android support
- [ ] Expose temperatures to CPU format string
- [ ] Expose temperatures to GPU format string
- [ ] Create a GPU driver module
- [ ] Expose GPU driver to GPU format string
- [ ] Fallback GPU detection using GL (multi GPU suport somehow possible?)
- [ ] Add a public IP module
- [ ] Find wayland compositor by looking at \${XDG_RUNTIME_DIR}/${WAYLAND_DISPLAY:-wayland-0}
- [ ] Make LocalIP module more configurable
- [ ] Automatic migrate old config files to newer versions
- [ ] ZSH completions
- [ ] Fish completions
- [ ] Make CPU usage detection much faster and more accurate
- [ ] Detect CPU usage in a common detection methode and expose it both to the cpuUsage module and the cpu format string

### General
- More presets
- More logos
- More package managers
- More DE detections
- More WM theme detections
- More terminal font detections
- Performance optimizations
- More module specific options
- Better error handling, especially the messages shown when using --show-errors
- Using package managers libraries for package count detection instead own. E.g. using libalpm instead of counting files in /var/lib/pacman/local
- Better documentation (especially default config file and the various --help options. Colored output?)
- General feature parity with neofetch, to demonstrate supremity of c
