---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: ''

---

<!-- We will try hard to solve the issue. However since platforms and hardwares vary greatly, it can be hard to find the root cause is. Providing following information may help us greatly. Thanks in advance -->

# General description of bug:

* What happened:
* What should happen:
* Did it work in an older version: <!-- if yes, which version -->
* Where did you get the binary: <!-- Github Release / Github Actions / Installed with a package manager (What package manager) / Built from source yourself -->
* Does this issue still occurs in [the latest dev build](https://github.com/fastfetch-cli/fastfetch/actions/)?

# Often helpful information:

Output of `fastfetch --version`:
```
//paste here
```

The content of the configuration file you use (if any)
```
//paste here
```

Output of `env NO_CONFIG=1 fastfetch --load-config all --show-errors --stat --multithreading false --disable-linewrap false --hide-cursor false`:
<!--
Note that this output will contain you public IP. If it is not relevant for the issue, feel free to remove it before uploading.

If you get the following error: `Error: couldn't find config: [...]`, copy the files in [presets](../../presets/) to `/usr/share/fastfetch/presets/` or `~/.local/share/fastfetch/presets/`.
If this isn't possible (or too much work) for you, post the output of `env NO_CONFIG=1 fastfetch --show-errors --stat --multithreading false --disable-linewrap false --hide-cursor false`. 
-->

```
//paste here
```

Output of `fastfetch --list-features`:
```
//paste here
```

## If fastfatch crashed

Paste the stacktrace here. You may get it with:

```
$ gdb /path/to/fastfetch
$ run
$ bt
```

If you are able to identify which module crashed, the strace can be helpful too

```
$ strace /path/to/fastfetch --multithreading false --structure {MODULE} --pipe
```

If you cannot do the instructions above, please upload the core dump file:

## If my image logo didn't show / work

<!-- Please make sure the terminal does support the image protocol you used. Note Gnome terminal doesn't support any image protocols -->

* The image protocol you used: 
* The terminal you used: 
* Upload the image file here, or paste the image URL: 
* Does it work with `--logo-width {WIDTH} --logo-height {HEIGHT}`? 

## If fastfetch behaves incorrectly on shell starting

<!-- There are known imcompatibility between fastfetch and p10k instant prompt. The p10k doc clearly states that you should NOT print anything to stdout after p10k-instant-prompt is initialized. You should either put `fastfetch` before initialization of p10k-instant-prompt (recommended), remove it or use `fastfetch --pipe false` -->

* The bug is reproduceable with fresh / clean shell configuration (i.e. `fastfetch` is the single line of `.zshrc` or `~/.config/fish/config.fish`): 
* Does `sleep 1` before running `fastfetch` work? 
