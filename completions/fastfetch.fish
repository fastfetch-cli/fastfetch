if not type -q fastfetch
    exit
end

command -q python3
if test $status -ne 0
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


def escape_fish_dq(s: str) -> str:
    """Escape special characters for Fish double quotes to prevent unexpected expansions. One slash in output needs four slashes in script"""
    return str(s).replace("\\\\", "\\\\\\\\").replace("\\"", "\\\\\\"").replace("$", "\\\\$")


def main():
    data: dict[str, list[dict]] = json.loads(subprocess.check_output(["fastfetch", "--help-raw"]))

    enum_functions = []
    complete_commands = []

    for key in data:
        for flag in data[key]:
            long_name = flag.get("long", "")

            # Handle pseudo flag logo-color-[1-9]
            if long_name == "logo-color-[1-9]":
                safe_desc_global = escape_fish_dq(flag.get("desc", ""))
                for i in range(1, 10):
                    complete_commands.append(f"complete -c fastfetch -d \"{safe_desc_global}\" -l \"logo-color-{i}\" -x -a \"(__fastfetch_complete_color)\"")
                continue

            if flag.get("pseudo", False):
                continue

            safe_desc_global = escape_fish_dq(flag.get("desc", ""))
            command_prefix = f"complete -c fastfetch -d \"{safe_desc_global}\" -l \"{long_name}\""

            if "short" in flag:
                short_name = flag["short"]
                command_prefix += f" -o {short_name}"

            if "arg" in flag:
                arg_type: str = flag["arg"]["type"]
                if arg_type == "bool":
                    complete_commands.append(f"{command_prefix} -x -a \"(__fastfetch_complete_bool)\"")
                elif arg_type == "color":
                    complete_commands.append(f"{command_prefix} -x -a \"(__fastfetch_complete_color)\"")
                elif arg_type == "command":
                    complete_commands.append(f"{command_prefix} -x -a \"(__fastfetch_complete_command)\"")
                elif arg_type == "config":
                    complete_commands.append(f"{command_prefix} -x -a \"(__fastfetch_complete_config)\"")
                elif arg_type == "enum":
                    # Sanitize flag name to create a valid Fish function name
                    func_name = long_name.replace("-", "_").replace("[", "_").replace("]", "_")
                    func_name = f"__fastfetch_complete_enum_{func_name}"

                    # Generate the enum function
                    func_lines = [f"function {func_name}"]
                    for enum_val, enum_desc in flag["arg"]["enum"].items():
                        safe_val = escape_fish_dq(enum_val)
                        safe_desc = escape_fish_dq(enum_desc)
                        func_lines.append(f"    echo -e \"{safe_val}\\t{safe_desc}\"")
                    func_lines.append("end\n")
                    enum_functions.append("\n".join(func_lines))

                    # Append complete command pointing to the new function
                    complete_commands.append(f"{command_prefix} -x -a \"({func_name})\"")
                elif arg_type == "logo":
                    complete_commands.append(f"{command_prefix} -x -a \"(__fastfetch_complete_logo)\"")
                elif arg_type == "structure":
                    complete_commands.append(f"{command_prefix} -x -a \"(__fish_complete_list : __fastfetch_complete_structure)\"")
                elif arg_type == "path":
                    complete_commands.append(f"{command_prefix} -r -F")
                else:
                    complete_commands.append(f"{command_prefix} -x")
            else:
                complete_commands.append(f"{command_prefix} -f")

    # Print generated enum functions first, then complete rules
    print("\n".join(enum_functions))
    print("\n".join(complete_commands))


if __name__ == "__main__":
    try:
        main()
    except:
        sys.exit(1)
' | python3 | source
