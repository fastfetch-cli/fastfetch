# Logo options

## General

| Option | Default | Description |
|--------|---------|-------------|
| `--logo`, `-l` | `""` | The logo source, interpreted depending on type |
| `--logo-type` | `auto` | The logo type. See the following categories for possible values |
| `--<type>` | `""` | Short for `--logo-type <type> --logo <value>` |
| `--logo-padding` | `""` | Sets the padding left and right of the logo |
| `--logo-padding-left` | `0` | Sets the padding left of the logo |
| `--logo-padding-right` | `4` | Sets the padding right of the logo |
| `--logo-padding-top` | `0` | Sets the padding top of the logo |

All filesytem paths can be:
* Absolute
* Relative to the current working directory
* Relative to any of the directories listed by `fastfetch --list-data-paths` + `/logos`  
  For example, the file `~/.local/share/fastfetch/logos/shrek` can simply be referenced as `shrek`.

## Text

| Option | Default | Description |
|--------|---------|-------------|
| `--logo-color-[1-9]` | `""` | Overrides a color for logos that support it |
| `--logo-print-remaining` | `true` | Print the remaining logo, if it is higher than the keys |

* Color placeholders are in the form `$[1-9]`. To print a `$`, use `$$`. 
* If a color placeholder value is not set, the placeholder is simply discarded.

## Image

| Option | Default | Description |
|--------|---------|-------------|
| `--logo-width` | `""` | Sets the width of the logo for logos that support it |
| `--logo-height` | `""` | Sets the height of the logo for logos that support it |

* If one of the height / width options is set, but the other not, the aspect ratio is preserved.
* If neither height nor width is set, the original size is used.

# Logo types

## auto

Trys to detect the logo type depending on the value of `--logo`, in the following order:
* If the value is empty / not set, an autodected builtin logo is displayed.
* If the value is the name of a builtin logo, it is displayed.
* If the value is the path to an image file, and the terminal emulator is known to support an image protocol, the image is displayed.
* If the value is the path to a text file, the content is displayed.
* The autodetected builtin logo is displayed.

## builtin

The value of `--logo` is interpreted as the name of a builtin logo.
* If the value is empty / not set, it is autodetected.
* Use `--list-logos` to get a list of all available logos.
* Use `--print-logos` to see all available logos.
* All builtin logos set default values for `--logo-color-[1-9]`.
* Use `none` to disable the logo, but keep key colors from the detected logo.

## file

The value of `--logo` is interpreted as the path to a file. The content of the file is displayed.
* If the file can't be read, the autodetected builtin logo is displayed.

## file-raw

The value of `--logo` is interpreted as the path to a file. The content of the file is displayed.
* No color placeholder replacement is done.
* If the file can't be read, the autodetected builtin logo is displayed.

## data

The value of `--logo` is interpreted as the logo and directly displayed.
* If the value is empty / not set, the autodetected builtin logo is displayed.

## data-raw

The value of `--logo` is interpreted as the logo and directly displayed.
* No color placeholder replacement is done.
* If the value is empty / not set, the autodetected builtin logo is displayed.

## sixel

The value of `--logo` is interpreted as the path to an image file. It is displayed using the sixel graphics protocol.
* `fastfetch` must be compiled with the `imagemagick6` or `imagemagick7` feature.

## kitty

The value of `--logo` is interpreted as the path to an image file. It is displayed using the kitty graphics protocol.
* If the file is a png file, and both `--logo-width` and `--logo-height` are specified, the image is directly send to the terminal emulator.
* Otherwise `fastfetch` must be compiled with the `imagemagick6` or `imagemagick7` feature.

## iterm

The value of `--logo` is interpreted as the path to an image file. It is displayed using the iTerm graphics protocol.
* Both `--logo-width` and `--logo-height` must be specified.

## chafa

The value of `--logo` is interpreted as the path to an image file. It is converted to an ascii logo using `libchafa`.
* `fastfetch` must be compiled with the `chafa` feature.
* `fastfetch` must be compiled with the `imagemagick6` or `imagemagick7` feature.
* Use `--chafa-fg-only` to set weather to render foreground only.
* Use `--chafa-symbols` to set the symbols.
* Use `--chafa-canvas-mode` to set the canvas mode.
* Use `--chafa-color-space` to set the color space.
* Use `--chafa-dither-mode` to set the dither mode.
* See the [chafa documentation](https://hpjansson.org/chafa/man/) for more information.

## raw

The value of `--logo` is interpreted as the path to a binary file. It is printed as-is.
* Both `--logo-width` and `--logo-height` must be specified.
* Use this to display a pre converted image file.
* If the file can't be read, the autodetected builtin logo is displayed.
