#compdef fastfetch

function _fastfetch() {

  whence python3 &> /dev/null
  if [ $? -ne 0 ]
  then
    return
  fi

  local state

  local -a opts=("${(f)$(
        python3 <<EOF
import json
import subprocess
import sys


def main():
    data: dict[str, list[dict]] = json.loads(
        subprocess.check_output(["fastfetch", "--help-raw"])
    )

    for key in data:
        for flag in data[key]:
            if flag["long"] == "logo-color-[1-9]":
                for i in range(1, 10):
                    command_prefix = f"--logo-color-{i}[{flag['desc']} ({i})]"
                    print_command(command_prefix, flag)
                continue

            if flag.get("pseudo", False):
                continue

            if "short" in flag:
                command_prefix = f"-{flag['short']}[{flag['desc']}]"
                print_command(command_prefix, flag)

            if "long" in flag:
                command_prefix = f"--{flag['long']}[{flag['desc']}]"
                print_command(command_prefix, flag)


def print_command(command_prefix: str, flag: dict):
    if "arg" in flag:
        type: str = flag["arg"]["type"]
        if type == "bool":
            print(f"{command_prefix}::bool:(true false)")
        elif type == "color":
            print(f"{command_prefix}:color:->colors")
        elif type == "command":
            print(f"{command_prefix}::module:->modules")
        elif type == "config":
            print(f"{command_prefix}:preset:->presets")
        elif type == "enum":
            temp: str = " ".join(flag["arg"]["enum"])
            print(f'{command_prefix}:type:({temp})')
        elif type == "logo":
            print(f"{command_prefix}:logo:->logos")
        elif type == "structure":
            print(f"{command_prefix}:structure:->structures")
        elif type == "path":
            print(f"{command_prefix}::path:_files")
        else:
            print(f"{command_prefix}:")
    else:
        print(f"{command_prefix}")


if __name__ == "__main__":
    try:
        main()
    except Exception:
        sys.exit(1)
EOF
  )}")

  _arguments "$opts[@]"

  case $state in
    colors)
      local -a colors=(black red green yellow blue magenta cyan white default)
      _describe 'color' colors
      ;;
    modules)
      local -a modules=("${(f)$(fastfetch --list-modules autocompletion)}")
      modules=(${(L)^modules[@]%%:*}-format format color)
      _describe 'module' modules
      ;;
    presets)
      local -a presets=(
        "${(f)$(fastfetch --list-presets autocompletion)}"
        "none:Disable loading config file"
      )
      _describe 'preset' presets || _files
      ;;
    structures)
      local -a structures=("${(f)$(fastfetch --list-modules autocompletion)}")
      _describe 'structure' structures
      ;;
    logos)
      local -a logos=(
        "${(f)$(fastfetch --list-logos autocompletion)}"
        "none:Don't print logo"
        "small:Print small ascii logo if available"
      )
      _describe 'logo' logos
      ;;
  esac
}

_fastfetch "$@"
