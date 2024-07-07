if not type -q fastfetch
    exit
end

complete -c fastfetch -f

function __fastfetch_complete_bool
    echo -e "true\tBool"
    echo -e "false\tBool"
end

function __fastfetch_complete_color
    echo -e "black\tColor"
    echo -e "red\tColor"
    echo -e "green\tColor"
    echo -e "yellow\tColor"
    echo -e "blue\tColor"
    echo -e "magenta\tColor"
    echo -e "cyan\tColor"
    echo -e "white\tColor"
    echo -e "default\tColor"
end

function __fastfetch_complete_command
    for line in (fastfetch --list-modules autocompletion)
        set -l pair (string split -m 2 : $line)
        set -l module (string lower $pair[1])

        echo -e "$module-format\tModule format"
    end
    echo -e "format\tCustom format"
    echo -e "color\tColor format"
end

function __fastfetch_complete_config
    for line in (fastfetch --list-presets autocompletion)
        echo -e "$line\tPreset"
    end
    echo -e "none\tDisable loading config file"
end

function __fastfetch_complete_logo
    for line in (fastfetch --list-logos autocompletion)
        echo -e "$line\tBuiltin logo"
    end
    echo -e "none\tDon't print logo"
    echo -e "small\tPrint small ascii logo if available"
end

function __fastfetch_complete_structure
    for line in (fastfetch --list-modules autocompletion)
        set -l pair (string split -m 2 : $line)
        echo -e "$pair[1]\t$pair[2]"
    end
end

echo '
import json, subprocess, sys

def main():
    data: dict[str, list[dict]] = json.loads(subprocess.check_output(["fastfetch", "--help-raw"]))

    for key in data:
        for flag in data[key]:
            if flag.get("pseudo", False):
                continue

            command_prefix = f"""complete -c fastfetch -d "{flag["desc"]}" -l "{flag["long"]}\""""
            if "short" in flag:
                command_prefix += f""" -o {flag["short"]}"""

            if "arg" in flag:
                type: str = flag["arg"]["type"];
                if type == "bool":
                    print(f"{command_prefix} -x -a \"(__fastfetch_complete_bool)\"")
                elif type == "color":
                    print(f"{command_prefix} -x -a \"(__fastfetch_complete_color)\"")
                elif type == "command":
                    print(f"{command_prefix} -x -a \"(__fastfetch_complete_command)\"")
                elif type == "config":
                    print(f"{command_prefix} -x -a \"(__fastfetch_complete_config)\"")
                elif type == "enum":
                    temp: str = " ".join(flag["arg"]["enum"])
                    print(f"{command_prefix} -x -a \"{temp}\"")
                elif type == "logo":
                    print(f"{command_prefix} -x -a \"(__fastfetch_complete_logo)\"")
                elif type == "structure":
                    print(f"{command_prefix} -x -a \"(__fish_complete_list : __fastfetch_complete_structure)\"")
                elif type == "path":
                    print(f"{command_prefix} -r -F")
                else:
                    print(f"{command_prefix} -x")
            else:
                print(f"{command_prefix} -f")

if __name__ == "__main__":
    try:
        main()
    except:
        sys.exit(1)
' | python3 | source
