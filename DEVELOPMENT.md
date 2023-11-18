# Development help

This file is not an official documentation!
Here i just add things that are easy to forget.

## Features i am planning to implement

### Specific

- [ ] Split the project in different libs / executables, all linking static by default:
  - `libffdetect`: contains all the detection stuff. To be used by anything that needs system information
  - `libffprint`: contains the printing functions, logos, format etc
  - `fastfetch` and `flashfetch`: Executables, that initialize the config of libffprint. Fist one at runtime, second one at compile time
- [ ] Make LocalIP module more configurable
- [ ] Automatic migrate old config files to newer versions
- [ ] ZSH completions
- [ ] Fish completions
- [ ] Make CPU usage detection much faster and more accurate
- [ ] Detect CPU usage in a common detection method and expose it both to the cpuUsage module and the cpu format string
- [ ] Per session caching

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
