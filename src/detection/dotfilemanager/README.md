# Dotfile Manager

This module attempts to detect the user's dotfile manager. Many [popular dotfile
managers](https://dotfiles.github.io/utilities/) are supported.

## Supported dotfile managers

* [Bare git repository](https://www.atlassian.com/git/tutorials/dotfiles)
* [chezmoi](https://chezmoi.io)
* [YADM](https://yadm.io)

## Unsupported dotfile managers

### Dotbot

As [Dotbot](https://github.com/anishathalye/dotbot) is usually imported as a
submodule inside the user's dotfile repository, there is no reliable way to
detect if the user is using it.

### GNU Stow

[GNU Stow](https://www.gnu.org/software/stow/) has no standard configuration, so
there is no reliable way to detect if the user is using it.

### Home manager

[Home manager](https://github.com/nix-community/home-manager) has no standard
configuration. 

### Mackup

[Mackup](https://github.com/lra/mackup) does not work on macOS since version 14
(Sonoma, released September 2023).

### rcm

[rcm](https://thoughtbot.github.io/rcm/rcm.7.html) has no standard
configuration.
