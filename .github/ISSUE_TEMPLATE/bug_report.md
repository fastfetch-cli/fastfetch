---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: ''

---

# General description of bug:


# Often helpful questions:
* Does the issue occur across multiple terminal emulators? [Y/N]
* Does the issue occur across multiple shells? (bash, zsh, fish, etc) [Y/N]

Output of `fastfetch --version`:
```
//paste here
```

Output of `fastfetch --load-config devinfo`:
```
//paste here
```

Output of `fastfetch --load-config devinfo-verbose`:
```
//paste here
```

Output of `fastfetch --list-features`:
```
//paste here
```

<!--
If you get the following error: `Error: couldn't find config: [...]`, copy the files in [presets](../../presets/) to `/usr/share/fastfetch/presets/` or `~/.local/share/fastfetch/presets/`.
If this isn't possible (or too much work) for you, post the output of `fastfetch --show-errors --recache --multithreading false --disable-linewrap false`. 
-->

