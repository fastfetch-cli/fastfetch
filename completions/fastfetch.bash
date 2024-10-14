__fastfetch_complete_help()
{
    local __ff_helps=(
        "color"
        "format"
        "config"
        "os-format"
        "host-format"
        "kernel-format"
        "uptime-format"
        "processes-format"
        "packages-format"
        "shell-format"
        "display-format"
        "de-format"
        "wm-format"
        "wmtheme-format"
        "theme-format"
        "icons-format"
        "font-format"
        "cursor-format"
        "terminal-format"
        "terminalfont-format"
        "cpu-format"
        "cpuusage-format"
        "gpu-format"
        "memory-format"
        "swap-format"
        "disk-format"
        "battery-format"
        "poweradapter-format"
        "locale-format"
        "localip-format"
        "publicip-format"
        "player-format"
        "media-format"
        "datetime-format"
        "date-format"
        "time-format"
        "vulkan-format"
        "opengl-format"
        "opencl-format"
        "bluetooth-format"
    )
    COMPREPLY=($(compgen -W "${__ff_helps[*]}" -- "$CURRENT_WORD"))
}

__fastfetch_complete_bool()
{
    COMPREPLY=($(compgen -W "true false" -- "$CURRENT_WORD"))
}

__fastfetch_complete_string()
{
    if [[ $CURRENT_WORD != "" ]]; then
        COMPREPLY=("$CURRENT_WORD")
    fi
}

__fastfetch_complete_path()
{
    COMPREPLY=($(compgen -A file -- "$CURRENT_WORD"))
}

__fastfetch_complete_logo()
{
    COMPREPLY=($(compgen -W "$(fastfetch --list-logos autocompletion)" -- "$CURRENT_WORD"))
}

__fastfetch_complete_logo_type()
{
    local __ff_logo_types=(
        "auto"
        "builtin"
        "file"
        "raw"
        "sixel"
        "kitty"
        "chafa"
    )
    COMPREPLY=($(compgen -W "${__ff_logo_types[*]}" -- "$CURRENT_WORD"))
}

__fastfetch_complete_binary_prefix()
{
    local __ff_size_binary_prefixes=(
        "iec"
        "si"
        "jedec"
    )
    COMPREPLY=($(compgen -W "${__ff_size_binary_prefixes[*]}" -- "$CURRENT_WORD"))
}

__fastfetch_complete_gl()
{
    local __ff_gl_types=(
        "auto"
        "egl"
        "glx"
        "osmesa"
    )
    COMPREPLY=($(compgen -W "${__ff_gl_types[*]}" -- "$CURRENT_WORD"))
}

__fastfetch_complete_option()
{
    local FF_OPTIONS_ALL=(
        "${FF_OPTIONS_BOOL[@]}"
        "${FF_OPTIONS_STRING[@]}"
        "${FF_OPTIONS_PATH[@]}"
        "${FF_OPTIONS_LOGO[@]}"
        "${FF_OPTIONS_LOGO_TYPE[@]}"
        "${FF_OPTIONS_BINARY_PREFIX[@]}"
        "${FF_OPTIONS_OPENGL[@]}"
    )

    if [[ $WORD_COUND -lt 3 ]]; then
        FF_OPTIONS_ALL+=(
            "${FF_OPTIONS_SINGLE[@]}"
            "${FF_OPTIONS_HELP[@]}"
        )
    fi

    for ff_word in ${COMP_WORDS[@]}; do
        if [[ $ff_word == $CURRENT_WORD ]]; then
            break
        fi

        FF_OPTIONS_ALL=("${FF_OPTIONS_ALL[@]/$ff_word}")
    done

    COMPREPLY=($(compgen -W "${FF_OPTIONS_ALL[*]}" -- "$CURRENT_WORD"))
}

__fastfetch_previous_matches()
{
    for ff_option in "$@"; do
        if [[ $ff_option == "$PREVIOUS_WORD" ]]; then
            return 0
        fi
    done
    return 1
}

__fastfetch_completion()
{
    local CURRENT_WORD="${COMP_WORDS[$COMP_CWORD]}"
    local PREVIOUS_WORD="${COMP_WORDS[$COMP_CWORD - 1]}"
    local WORD_COUND="${#COMP_WORDS[@]}"

    local FF_OPTIONS_SINGLE=(
        "-v"
        "--version"
        "--list-logos"
        "--list-modules"
        "--list-presets"
        "--list-features"
        "--print-logos"
        "--print-config-system"
        "--print-config-user"
        "--print-structure"
        "--gen-config"
        "--gen-config-force"
    )

    local FF_OPTIONS_HELP=(
        "-h"
        "--help"
    )

    local FF_OPTIONS_BOOL=(
        "-r"
        "--show-errors"
        "--logo-print-remaining"
        "--multithreading"
        "--stat"
        "--allow-slow-operations"
        "--disable-linewrap"
        "--hide-cursor"
        "--cpu-temp"
        "--gpu-temp"
        "--battery-temp"
        "--display-precise-refresh-rate"
        "--localip-show-ipv4"
        "--localip-show-ipv6"
        "--localip-show-flags"
        "--localip-show-loop"
        "--localip-show-mtu"
        "--localip-show-speed"
        "--localip-name-prefix"
        "--localip-compact-type"
        "--escape-bedrock"
        "--pipe"
        "--title-fqdn"
        "--disk-folders"
        "--disk-show-external"
        "--disk-show-hidden"
        "--disk-show-subvolumes"
        "--gpu-hide-integrated"
        "--gpu-hide-discrete"
        "--gpu-force-method"
        "--disk-show-unknown"
        "--bluetooth-show-disconnected"
    )

    local FF_OPTIONS_STRING=(
        "--logo-type"
        "--logo-padding"
        "--logo-padding-left"
        "--logo-padding-right"
        "--logo-padding-top"
        "--logo-color-1"
        "--logo-color-2"
        "--logo-color-3"
        "--logo-color-4"
        "--logo-color-5"
        "--logo-color-6"
        "--logo-color-7"
        "--logo-color-8"
        "--logo-color-9"
        "--logo-width"
        "--logo-height"
        "--color"
        "--color-keys"
        "--color-title"
        "--display-compact-type"
        "--separator"
        "-s"
        "--structure"
        "--player-name"
        "--percent-type"
        "--publicip-url"
        "--publicip-timeout"
        "--weather-output-format"
        "--weather-timeout"
        "--os-key"
        "--os-format"
        "--os-key-color"
        "--host-key"
        "--host-format"
        "--host-key-color"
        "--kernel-key"
        "--kernel-format"
        "--kernel-key-color"
        "--uptime-key"
        "--uptime-format"
        "--uptime-key-color"
        "--processes-key"
        "--processes-format"
        "--processes-key-color"
        "--packages-key"
        "--packages-format"
        "--packages-key-color"
        "--shell-key"
        "--shell-format"
        "--shell-key-color"
        "--display-key"
        "--display-format"
        "--display-key-color"
        "--de-key"
        "--de-format"
        "--de-key-color"
        "--wm-key"
        "--wm-format"
        "--wm-key-color"
        "--wmtheme-key"
        "--wmtheme-format"
        "--wmtheme-key-color"
        "--theme-key"
        "--theme-format"
        "--theme-key-color"
        "--icons-key"
        "--icons-format"
        "--icons-key-color"
        "--font-key"
        "--font-format"
        "--font-key-color"
        "--cursor-key"
        "--cursor-format"
        "--cursor-key-color"
        "--terminal-key"
        "--terminal-format"
        "--terminal-key-color"
        "--terminalfont-key"
        "--terminalfont-format"
        "--terminalfont-key-color"
        "--cpu-key"
        "--cpu-format"
        "--cpu-key-color"
        "--cpu-useage-key"
        "--cpu-useage-format"
        "--cpu-useage-key-color"
        "--gpu-key"
        "--gpu-format"
        "--gpu-key-color"
        "--memory-key"
        "--memory-format"
        "--memory-key-color"
        "--swap-key"
        "--swap-format"
        "--swap-key-color"
        "--disk-key"
        "--disk-format"
        "--disk-key-color"
        "--battery-key"
        "--battery-format"
        "--battery-key-color"
        "--poweradapter-key"
        "--poweradapter-format"
        "--poweradapter-key-color"
        "--locale-key"
        "--locale-format"
        "--locale-key-color"
        "--localip-key"
        "--localip-format"
        "--localip-key-color"
        "--publicip-key"
        "--publicip-format"
        "--publicip-key-color"
        "--wifi-key"
        "--wifi-format"
        "--wifi-key-color"
        "--weather-key"
        "--weather-format"
        "--weather-key-color"
        "--player-key"
        "--player-format"
        "--player-key-color"
        "--media-key"
        "--media-format"
        "--media-key-color"
        "--datetime-key"
        "--datetime-format"
        "--datetime-key-color"
        "--date-key"
        "--date-format"
        "--date-key-color"
        "--time-key"
        "--time-format"
        "--time-key-color"
        "--vulkan-key"
        "--vulkan-format"
        "--vulkan-key-color"
        "--opengl-key"
        "--opengl-format"
        "--opengl-key-color"
        "--opencl-key"
        "--opencl-format"
        "--opencl-key-color"
        "--users-key"
        "--users-format"
        "--users-key-color"
        "--users-myself-only"
        "--bluetooth-key"
        "--bluetooth-format"
        "--bluetooth-key-color"
    )

    local FF_OPTIONS_PATH=(
        "-c"
        "--config"
        "--lib-vulkan"
        "--lib-wayland"
        "--lib-xcb-randr"
        "--lib-xcb"
        "--lib-xrandr"
        "--lib-X11"
        "--lib-gio"
        "--lib-dconf"
        "--lib-dbus"
        "--lib-xfconf"
        "--lib-sqlite3"
        "--lib-rpm"
        "--lib-imagemagick"
        "--lib-z"
        "--lib-chafa"
        "--lib-egl"
        "--lib-glx"
        "--lib-osmesa"
        "--lib-opencl"
        "--lib-pulse"
        "--lib-ddcutil"
        "--lib-nm"
    )

    local FF_OPTIONS_LOGO=(
        "-l"
        "--logo"
    )

    local FF_OPTIONS_LOGO_TYPE=(
        "--logo-type"
    )

    local FF_OPTIONS_BINARY_PREFIX=(
        "--binary-prefix"
    )

    local FF_OPTIONS_OPENGL=(
        "--opengl-type"
    )

    if __fastfetch_previous_matches "${FF_OPTIONS_SINGLE[@]}"; then
        return
    elif [[ $WORD_COUND -gt 3 && ( ${COMP_WORDS[$COMP_CWORD - 2]} == "--help" || ${COMP_WORDS[$COMP_CWORD - 2]} == "-h" ) ]]; then
        return
    elif [[ $CURRENT_WORD == "-"* ]]; then
        __fastfetch_complete_option
    elif __fastfetch_previous_matches "${FF_OPTIONS_HELP[@]}"; then
        __fastfetch_complete_help
    elif __fastfetch_previous_matches "${FF_OPTIONS_BOOL[@]}"; then
        __fastfetch_complete_bool
    elif __fastfetch_previous_matches "${FF_OPTIONS_STRING[@]}"; then
        __fastfetch_complete_string
    elif __fastfetch_previous_matches "${FF_OPTIONS_PATH[@]}"; then
        __fastfetch_complete_path
    elif __fastfetch_previous_matches "${FF_OPTIONS_LOGO[@]}"; then
        __fastfetch_complete_logo
    elif __fastfetch_previous_matches "${FF_OPTIONS_LOGO_TYPE[@]}"; then
        __fastfetch_complete_logo_type
    elif __fastfetch_previous_matches "${FF_OPTIONS_BINARY_PREFIX[@]}"; then
        __fastfetch_complete_binary_prefix
    elif __fastfetch_previous_matches "${FF_OPTIONS_OPENGL[@]}"; then
        __fastfetch_complete_gl
    else
        __fastfetch_complete_option
    fi
}

complete -F __fastfetch_completion fastfetch
