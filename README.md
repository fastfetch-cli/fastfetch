# Fastfetch

[![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/fastfetch-cli/fastfetch/ci.yml)](https://github.com/fastfetch-cli/fastfetch/actions)
[![GitHub license](https://img.shields.io/github/license/fastfetch-cli/fastfetch)](https://github.com/fastfetch-cli/fastfetch/blob/dev/LICENSE)
[![GitHub contributors](https://img.shields.io/github/contributors/fastfetch-cli/fastfetch)](https://github.com/fastfetch-cli/fastfetch/graphs/contributors)
[![GitHub top language](https://img.shields.io/github/languages/top/fastfetch-cli/fastfetch?logo=c&label=)](https://github.com/fastfetch-cli/fastfetch/blob/dev/CMakeLists.txt#L5)
[![GitHub commit activity (branch)](https://img.shields.io/github/commit-activity/m/fastfetch-cli/fastfetch)](https://github.com/fastfetch-cli/fastfetch/commits)  
[![homebrew downloads](https://img.shields.io/homebrew/installs/dm/fastfetch?logo=homebrew)](https://formulae.brew.sh/formula/fastfetch#default)
[![GitHub all releases](https://img.shields.io/github/downloads/fastfetch-cli/fastfetch/total?logo=github)](https://github.com/fastfetch-cli/fastfetch/releases)  
[![GitHub release (with filter)](https://img.shields.io/github/v/release/fastfetch-cli/fastfetch?logo=github)](https://github.com/fastfetch-cli/fastfetch/releases)
[![latest packaged version(s)](https://repology.org/badge/latest-versions/fastfetch.svg)](https://repology.org/project/fastfetch/versions)
[![Packaging status](https://repology.org/badge/tiny-repos/fastfetch.svg)](https://repology.org/project/fastfetch/versions)

Fastfetch is a [neofetch](https://github.com/dylanaraps/neofetch)-like tool for fetching system information and displaying it prettily. It is written mainly in C, with performance and customizability in mind. Currently, Linux, Android, FreeBSD, macOS, SunOS and Windows 7+ are supported.

<img src="screenshots/example1.png" width="49%" align="left" />
<img src="https://upload.wikimedia.org/wikipedia/commons/2/24/Transparent_Square_Tiles_Texture.png" width="49%" height="16px" align="left" />
<img src="screenshots/example4.png" width="49%" align="left" />
<img src="https://upload.wikimedia.org/wikipedia/commons/2/24/Transparent_Square_Tiles_Texture.png" width="49%" height="16px" align="left" />
<img src="screenshots/example2.png" width="48%" align="top" />
<img src="screenshots/example3.png" width="48%" align="top" />
<img src="screenshots/example5.png" height="15%" align="top" />

There are [screenshots on different platforms](https://github.com/fastfetch-cli/fastfetch/wiki).

## Installation

### Linux

Some distros package an outdated fastfetch version. Older versions receive no support, so please try always to use the latest version.

* Ubuntu: [`ppa:zhangsongcui3371/fastfetch`](https://launchpad.net/~zhangsongcui3371/+archive/ubuntu/fastfetch) (for Ubuntu 22.04 or newer)
* Debian: `apt install fastfetch` (for Debian 13 or newer)
* Debian / Ubuntu: Download `fastfetch-linux-<proper architecture>.deb` from [Github release page](https://github.com/fastfetch-cli/fastfetch/releases/latest) and double-click it (for Ubuntu 20.04 or newer and Debian 11 or newer).
* Arch Linux: `pacman -S fastfetch`
* Fedora: `dnf install fastfetch`
* Gentoo: `emerge --ask app-misc/fastfetch`
* Alpine: `apk add --upgrade fastfetch`
* NixOS: `nix-shell -p fastfetch`
* openSUSE: `zypper install fastfetch`
* ALT Linux: `apt-get install fastfetch`
* Exherbo: `cave resolve --execute app-misc/fastfetch`
* GNU Guix: `guix install fastfetch`
* Solus: `eopkg install fastfetch`
* Slackware: `sbopkg -i fastfetch`
* Void Linux: `xbps-install fastfetch`
* Venom Linux: `scratch install fastfetch`

You may need `sudo`, `doas` or `sup` to run these commands.

[See also if fastfetch has been packaged for your favorite Linux distro](#Packaging).

If fastfetch is not packaged for your distro or an outdated version is packaged, [linuxbrew](https://brew.sh/) is a good alternative: `brew install fastfetch`

### macOS

* [HomeBrew](https://formulae.brew.sh/formula/fastfetch#default): `brew install fastfetch`
* [MacPorts](https://ports.macports.org/port/fastfetch/): `sudo port install fastfetch`

### Windows

* [scoop](https://scoop.sh/#/apps?q=fastfetch): `scoop install fastfetch`
* [Chocolatey](https://community.chocolatey.org/packages/fastfetch): `choco install fastfetch`
* [winget](https://github.com/microsoft/winget-pkgs/tree/master/manifests/f/Fastfetch-cli/Fastfetch): `winget install fastfetch`
* [MSYS2 MinGW](https://packages.msys2.org/base/mingw-w64-fastfetch): `pacman -S mingw-w64-<subsystem>-<arch>-fastfetch`

You may also download the program directly from [the GitHub releases page](https://github.com/fastfetch-cli/fastfetch/releases/latest) in the form of an archive file.

### FreeBSD

* `pkg install fastfetch`

### Android (Termux)

* `pkg install fastfetch`

## Build from source

See Wiki: https://github.com/fastfetch-cli/fastfetch/wiki/Building

## Usage

* Run it with default configuration: `fastfetch`
* Run it with [all supported modules](https://github.com/fastfetch-cli/fastfetch/wiki/Support+Status#available-modules) and find what interests you: `fastfetch -c all.jsonc`
* Find all data that fastfetch detects: `fastfetch -s <module> --format json`
* Display help messages: `fastfetch --help`
* Generate config file based on command line arguments: `fastfetch --arg1 --arg2 --gen-config`

## Customization

Fastfetch uses the JSONC (or JSON with comments) for configuration. [See Wiki for detail](https://github.com/fastfetch-cli/fastfetch/wiki/Configuration). There are some premade config files in [`presets`](presets), including the ones used for the screenshots above. You can load them using `-c <filename>`. Those files can serve as examples of the configuration syntax.

Logos can also be heavily customized; see the [logo documentation](https://github.com/fastfetch-cli/fastfetch/wiki/Logo-options) for more information.


## Packaging

### Repositories

[![Packaging status](https://repology.org/badge/vertical-allrepos/fastfetch.svg?header=)](https://repology.org/project/fastfetch/versions)

### Manual

* DEB / RPM package: `cmake --build . --target package`
* Install directly: `cmake --install . --prefix /usr/local`

## FAQ

### Q: Neofetch is good enough. Why do I need fastfetch?

1. Fastfetch is actively maintained.
2. Fastfetch is faster. As the name suggests.
3. Fastfetch has a greater number of features, though by default fastfetch only has a few modules enabled; use `fastfetch -c all` to find what you want.
4. Fastfetch is more configurable. You can find more information in the Wiki: <https://github.com/fastfetch-cli/fastfetch/wiki/Configuration>.
5. Fastfetch is more polished. For example, neofetch prints `555 MiB` in the Memory module and `23 G` in the Disk module, whereas fastfetch prints `555.00 MiB` and `22.97 GiB` respectively.
6. Fastfetch is more accurate. For example, [neofetch never actually supports the Wayland protocol](https://github.com/dylanaraps/neofetch/pull/2395).

### Q: Fastfetch shows my local IP address. It leaks my privacy!

A local IP (10.x.x.x, 172.x.x.x, 192.168.x.x) has nothing to do with privacy. It only makes sense if you are on the same network, for example, if you connect to the same Wi-Fi network.

Actually the `Local IP` module is the most useful module for me personally. I (@CarterLi) have several VMs installed to test fastfetch and often need to SSH into them. I have fastfetch running on shell startup and I never need to type `ip addr` manually.

If you really don't like it, you can disable the `Local IP` module in `config.jsonc`.

### Q: Where is the config file? I can't find it.

`Fastfetch` does not generate config file automatically. You can use `fastfetch --gen-config` to generate one. The config file will be saved in `~/.config/fastfetch/config.jsonc` by default. See [Wiki for detail](https://github.com/fastfetch-cli/fastfetch/wiki/Configuration).

### Q: The configuration is so complex. Where is the documentation?

Fastfetch uses JSON (with comments) for configuration. I suggest you use an IDE with JSON schema support (like VSCode) to edit it.

Alternatively, you can refer to the presets in [`presets` directory](https://github.com/fastfetch-cli/fastfetch/tree/dev/presets).

The **correct** way to edit the configuration:

This is an example that [changes size prefix from MiB / GiB to MB / GB](https://github.com/fastfetch-cli/fastfetch/discussions/1014). Editor used: [helix](https://github.com/helix-editor/helix)

[![asciicast](https://asciinema.org/a/1uF6sTPGKrHKI1MVaFcikINSQ.svg)](https://asciinema.org/a/1uF6sTPGKrHKI1MVaFcikINSQ)

### Q: I WANT THE DOCUMENTATION!

[Here is the documentation](https://github.com/fastfetch-cli/fastfetch/wiki/Json-Schema). It is generated from [JSON schema](https://github.com/fastfetch-cli/fastfetch/blob/dev/doc/json_schema.json) but you won't like it.

### Q: How can I customize the module output?

Fastfetch uses `format` to generate output. For example, to make the `GPU` module show only the GPU name (leaving other information undisplayed), you can use

```jsonc
{
    "modules": [
        {
            "type": "gpu",
            "format": "{2}" // See `fastfetch -h gpu-format` for detail
        }
    ]
}
```

. . which is equivalent to `fastfetch -s gpu --gpu-format '{2}'`

See `fastfetch -h format` for information on basic usage. For module specific formattion, see `fastfetch -h <module>-format`

### Q: I have my own ascii-art / image file. How can I show it with fastfetch?

Try `fastfetch -l /path/to/logo`. See [logo documentation](https://github.com/fastfetch-cli/fastfetch/wiki/Logo-options) for detail.

If you just want to display distro name in [FIGlet text](https://github.com/pwaller/pyfiglet):

```bash
# install pyfiglet and jq first
pyfiglet -s -f small_slant $(fastfetch -s os --format json | jq -r '.[0].result.name') && fastfetch -l none
```

![image](https://github.com/fastfetch-cli/fastfetch/assets/6134068/6466524e-ab8c-484f-848d-eec7ddeb7df2)

### Q: Fastfetch runs in white and black on shell startup. Why?

This issue usually happens when using fastfetch with `p10k`. There are known incompatibility between fastfetch and p10k instant prompt.
The p10k doc clearly states that you should NOT print anything to stdout after `p10k-instant-prompt` is initialized. You should either put `fastfetch` before initialization of `p10k-instant-prompt` (recommended)

You can always use `fastfetch --pipe false` to force fastfetch running in colorful mode.

### Q: Why do fastfetch and neofetch show different memory usage result?

See [#1096](https://github.com/fastfetch-cli/fastfetch/issues/1096).

### Q: I want feature A / B / C. Will fastfetch support it?

Fastfetch is a system information tool. We only accept hardware or system level software feature requests. For most personal uses, I recommend using `Command` module to detect it yourself, which can be used to grab output from a custom shell script:

```jsonc
// This module shows the default editor
{
    "modules": [
        {
            "type": "command",
            "text": "$EDITOR --version | head -1",
            "key": "Editor"
        }
    ]
}
```

Otherwise, open a feature request in [GitHub Issues](https://github.com/fastfetch-cli/fastfetch/issues).

### Q: I have questions. Where can I get help?

* For usage questions, please start a discussion in [GitHub Discussions](https://github.com/fastfetch-cli/fastfetch/discussions).
* For possible bugs, please open an issue in [GitHub Issues](https://github.com/fastfetch-cli/fastfetch/issues). Be sure to fill the bug-report template carefully for developers to investigate.

## Star History

Give it a star to support us!

<a href="https://star-history.com/#fastfetch-cli/fastfetch&Date">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=fastfetch-cli/fastfetch&type=Date&theme=dark" />
    <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=fastfetch-cli/fastfetch&type=Date" />
    <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=fastfetch-cli/fastfetch&type=Date" />
  </picture>
</a>
