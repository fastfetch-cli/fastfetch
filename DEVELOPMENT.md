# Development help

This file is not an official documentation!
Here i just add things that are easy to forget.

## Features i am planning to implement

- [ ] Support for printing images as ascii art logos (using imlib2 and libcaca probably)
- [ ] Support for printing images in terminals that support it
- [ ] Better documentation (especially default config file and the various --help options. Colored output?)
- [ ] Better OS output for all possible combinations of /etc/os-release variables.
- [ ] Fallback OS detection with lsb
- [ ] Android support
- [ ] Using package managers libraries for package count detection instead own. E.g. using libalpm instead of counting files in /var/lib/pacman/local
- [ ] Support for more platforms by quering supported features at runtime (sysconf())
- [ ] Expose temperatures to CPU / GPU format string
- [ ] Fallback GPU detection using GL (multi GPU suport somehow possible?)
- [ ] More presets / logos / terminal fonts etc
- [ ] Better error handling, especially the messages shown when using --show-errors
- [ ] General feature parity with neofetch, to demonstrate supremity of c
- [ ] Find wayland compositor by looking at \${XDG_RUNTIME_DIR}/${WAYLAND_DISPLAY:-wayland-0}
- [ ] Make LocalIP module more configurable
- [ ] Automatic migrate old config files to newer versions
- [ ] ZSH completions
- [ ] fish completions
- [ ] Add an easteregg
- [ ] Make CPU usage detection much faster and more accurate
- [ ] Detect CPU usage in a separate thread and expose it both to the cpuUsage module and the cpu format string
