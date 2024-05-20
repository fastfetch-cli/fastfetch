---
name: Bug report
about: Create a report to help us improve
title: '[BUG] '
labels: bug
assignees: ''

---

Be sure to read [FAQ](https://github.com/fastfetch-cli/fastfetch?tab=readme-ov-file#faq) before submitting a new issue.

<!-- We will try hard to solve the issue. However since platforms and hardwares vary greatly, it can be hard to find the root cause is. Providing following information may help us greatly. Thanks in advance -->

# General description of bug:

* What happened:
* What should happen:
* Fastfetch version used: <!-- please use the latest version, if possible -->
* Did it work in an older version: <!-- if yes, which version -->
* Where did you get the binary: <!-- Github Release / Github Actions / Installed with a package manager (What package manager) / Built from source yourself -->
* Does this issue still occurs in [the latest dev build](https://github.com/fastfetch-cli/fastfetch/actions/)?

# Often helpful information:

Screenshot:
<!-- Paste the screenshot here -->

The content of the configuration file you use (if any):
```
//paste here
```

Output of `fastfetch -c ci.jsonc --format json`:
<!--
Note that this output will contain you public IP. If it is not relevant for the issue, feel free to remove it before uploading.
-->

```
//paste here
```

Output of `fastfetch --list-features`:
```
//paste here
```

## If fastfatch crashed or freezed

Paste the stacktrace here. You may get it with:

```shell
# You may need Ctrl+C to stop the process if it freezes
gdb -q -ex 'set confirm off' -ex run -ex 'bt full' -ex quit --args /path/to/fastfetch
```

If you are able to identify which module crashed, the strace can be helpful too

```shell
strace /path/to/fastfetch --multithreading false -s {MODULE} --pipe
```

If you cannot do the instructions above, please upload the core dump file:

## If fastfetch is slow

Use `time fastfetch --stat` to show time usage for each module.

## If my image logo didn't show / work

<!-- Please make sure the terminal does support the image protocol you used. Note Gnome terminal doesn't support any image protocols -->

* The image protocol you used: 
* The terminal you used: 
* Upload the image file here, or paste the image URL: 
* Does it work with `--logo-width {WIDTH} --logo-height {HEIGHT}`? 

## If fastfetch behaves incorrectly on shell starting

* The bug is reproduceable with fresh / clean shell configuration (i.e. `fastfetch` is the single line of `.zshrc` or `~/.config/fish/config.fish`): 
* Does `sleep 1` before running `fastfetch` work? 
