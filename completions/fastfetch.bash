#!/usr/bin/env bash

_fastfetch() {
  # Use Bash built-in variables directly
  local cur="${COMP_WORDS[COMP_CWORD]}"
  local prev="${COMP_WORDS[COMP_CWORD-1]}"

  # Check if Python is available
  if ! command -v python3 &>/dev/null; then
    return
  fi

  # Handle standard completion cases
  case "$prev" in
    --color|--color-keys|--color-title|--color-output|--color-separator|--*-color|--*-key-color|--*-output-color|--logo-color-[1-9]|--percent-color-*|--temp-color-*)
      local -a colors=("black" "red" "green" "yellow" "blue" "magenta" "cyan" "white" "default")
      COMPREPLY=($(compgen -W "${colors[*]}" -- "$cur"))
      return
      ;;
    --logo|-l)
      local -a logos
      readarray -t logos < <(fastfetch --list-logos autocompletion 2>/dev/null)
      logos+=("none" "small")
      COMPREPLY=($(compgen -W "${logos[*]}" -- "$cur"))
      return
      ;;
    --config|-c)
      local -a presets
      readarray -t presets < <(fastfetch --list-presets autocompletion 2>/dev/null)
      presets+=("none")
      COMPREPLY=($(compgen -W "${presets[*]}" -- "$cur"))
      # Also allow file path completion
      if type _filedir &>/dev/null; then
        _filedir
      elif type compgen &>/dev/null; then
        COMPREPLY+=($(compgen -f -- "$cur"))
      fi
      return
      ;;
    --structure|-s)
      # Get all module names in lowercase only
      local -a structures
      readarray -t structures < <(fastfetch --list-modules autocompletion 2>/dev/null | cut -d':' -f1 | tr '[:upper:]' '[:lower:]')
      COMPREPLY=($(compgen -W "${structures[*]}" -- "$cur"))
      return
      ;;
    --help|-h)
      local -a modules
      readarray -t modules < <(fastfetch --list-modules autocompletion 2>/dev/null)
      # Convert to lowercase and keep only module names
      local -a module_names=()
      for module in "${modules[@]}"; do
        module_names+=($(echo "$module" | cut -d':' -f1 | tr '[:upper:]' '[:lower:]')-format)
      done
      module_names+=("format" "color")
      COMPREPLY=($(compgen -W "${module_names[*]}" -- "$cur"))
      return
      ;;
    --format)
      COMPREPLY=($(compgen -W "json default" -- "$cur"))
      return
      ;;
    --*-format)
      # Format string completion, handle spaces
      return
      ;;
    --*path*|--*file*|--gen-config*|--*data*)
      # File path completion
      if type _filedir &>/dev/null; then
        _filedir
      elif type compgen &>/dev/null; then
        COMPREPLY=($(compgen -f -- "$cur"))
      fi
      return
      ;;
  esac

  # If not a special option, generate all possible options
  if [[ "$cur" == -* ]]; then
    local -a opts
    readarray -t opts < <(python3 - "$cur" <<'EOF'
import json
import sys
import subprocess

def main(current):
    try:
        # Use fastfetch --help-raw to get option data
        output = subprocess.check_output(['fastfetch', '--help-raw'], stderr=subprocess.DEVNULL)
        data = json.loads(output)

        for category in data.values():
            for flag in category:
                if flag.get("pseudo", False):
                    continue

                if "short" in flag:
                    print(f"-{flag['short']}")

                if "long" in flag:
                    if flag["long"] == "logo-color-[1-9]":
                        for i in range(1, 10):
                            print(f"--logo-color-{i}")
                    else:
                        print(f"--{flag['long']}")
    except Exception:
        # If error occurs, return no options
        pass

if __name__ == "__main__":
    main(sys.argv[1])
EOF
)
    COMPREPLY=($(compgen -W "${opts[*]}" -- "$cur"))
  fi

  return 0
}

# Register completion
complete -F _fastfetch fastfetch
