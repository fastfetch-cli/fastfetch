#!/usr/bin/env python3

"""
Python script to generate a comprehensive man page for the command `fastfetch`.

The generated man page content will be printed to stdout,
so you will need to pipe it to a file if you want to save it.
Example: python3 gen-man.py > fastfetch.1

The command options are generated using a JSON file.
For the JSON file format, see:
https://github.com/fastfetch-cli/fastfetch/blob/dev/src/data/help.json
"""

from json import load
from datetime import datetime, timezone
from time import time
from re import search
from os import environ, path


###### Text Decorations Tags ######

startUnderline = r"\fI" # start underline text tag
endUnderline = r"\fR" # end underline text tag

startBold = r"\fB" # start bold text tag
endBold = r"\fR" # end bold text tag


###### Parameters ######

# path to the current directory
pathToCurrentDir = path.dirname(__file__)
# path to the JSON option file
pathToHelpFile = path.join(pathToCurrentDir, "../src/data/help.json")
# man page section
manSection = 1
# title (center header)
titlePage = "FASTFETCH"
# date (center footer)
# format : "Month (abbreviation) Day Year"
todayDate = datetime.fromtimestamp(
    int(environ.get("SOURCE_DATE_EPOCH", time())),
    tz=timezone.utc,
).strftime("%b %d %Y")
# file to fastfetch version (left footer)
pathToVersionFile = path.join(pathToCurrentDir, "../CMakeLists.txt")


###### Sections Text ######

# text displayed in the "NAME" section
nameSection = r"fastfetch \- A fast and feature-rich system information tool similar to neofetch"

# text displayed in the "DESCRIPTION" section
descriptionSection = r"""
Fastfetch is a tool for displaying system information in a visually appealing way. Written primarily in C, it focuses on performance and customizability while providing functionality similar to neofetch.
It supports Linux, Android, FreeBSD, macOS, and Windows 7 or newer.
"""

# text displayed at the beginning of the "OPTIONS" section
optionSection = r"""
Options are parsed in a case-insensitive manner. For example, \fB--logo-type\fR and \fB--LOGO-TYPE\fR are treated identically.

Arguments in square brackets are optional. Optional boolean arguments default to 'true' when specified without a value.

For more detailed information about a specific option, use:
\fBfastfetch -h <option_name_without_dashes>\fR

Any combination of options can be made permanent by generating a configuration file:
\fBfastfetch <options> --gen-config\fR
"""

# text displayed in the "CONFIGURATION"
configurationSection = f"""
.SS Fetch Structure

The structure defines which modules to display and in what order. It consists of module names separated by colons (:).
For example: {startBold}title:separator:os:kernel:uptime{endBold}

To list all available modules, use {startBold}--list-modules{endBold}


.SS Config Files

Fastfetch uses JSONC (JSON with Comments) for configuration files. These files must have the .jsonc extension.

You can generate a default config file using {startBold}--gen-config{endBold}. By default, the config file is saved at {startBold}~/.config/fastfetch/config.jsonc{endBold}.

The configuration/preset files are searched in the following locations (in order):

{startBold}1.{endBold} Relative to the current working directory

{startBold}2.{endBold} Relative to ~/.local/share/fastfetch/presets/

{startBold}3.{endBold} Relative to /usr/share/fastfetch/presets/

For detailed information on logo options, module configuration, and formatting, visit:
{startBold}https://github.com/fastfetch-cli/fastfetch/wiki/Configuration{endBold}

Fastfetch provides several built-in presets. List them with {startBold}--list-presets{endBold}.

.SS JSON Schema
A JSON schema is available for editor intelligence when editing the configuration file. Add the following line at the beginning of your config file:

{startBold}"$schema": "https://github.com/fastfetch-cli/fastfetch/raw/dev/doc/json_schema.json"{endBold}
"""

# text displayed in the "EXAMPLE" section
exampleSection = f"""
.SS Basic Usage
{startBold}fastfetch{endBold}

.SS Use a specific logo
{startBold}fastfetch --logo arch{endBold}

.SS Custom structure
{startBold}fastfetch --structure title:os:kernel:uptime:memory{endBold}

.SS Generate a config file
{startBold}fastfetch --gen-config{endBold}

.SS Use a preset
{startBold}fastfetch --config neofetch{endBold}

.SS Config File Example
.nf
// ~/.config/fastfetch/config.jsonc
{{
    "$schema": "https://github.com/fastfetch-cli/fastfetch/raw/dev/doc/json_schema.json",
    "logo": {{
        "type": "auto",
        "source": "arch"
    }},
    "display": {{
        "separator": ": ",
        "color": {{
            "keys": "blue",
            "title": "red"
        }},
        "key": {{
            "width": 12
        }}
    }},
    "modules": [
        "title",
        "separator",
        "os",
        "kernel",
        "uptime",
        {{
            "type": "memory",
            "format": "{{used}}/{{total}} ({{used_percent}}%)"
        }}
    ]
}}
.fi
"""

# text displayed in the "BUGS" section
bugSection = "Please report bugs to: https://github.com/fastfetch-cli/fastfetch/issues"

# text displayed in the "AUTHORS" section
authorsSection = "Fastfetch is developed by a team of contributors on GitHub.\nVisit https://github.com/fastfetch-cli/fastfetch for more information."


###### Argument decoration ######

### optional arguments tags ###

# if an optional argument is displayed as [?optArg] (with "optArg" underlined)
# this value should be f"[?{startUnderline}"
startOptionalArgument = f"[{startUnderline}"
# if an optional argument is displayed as [?optArg] (with "optArg underlined")
# this value should be f"{endUnderline}]"
endOptionalArgument = f"{endUnderline}]"

### mandatory arguments tags ###
startMandatoryArgument = f"{startUnderline}"
endMandatoryArgument = f"{endUnderline}"

def main():

    # importing the JSON file
    with open(pathToHelpFile, 'r') as jsonFile:
        helpFileData = load(jsonFile) # json.load


    ######## Start printing the generated .1 file ########


    ###### header, footer & config #####

    print(f".TH FASTFETCH {manSection} ", end=" ")
    print(f"\"{todayDate}\"", end=" ")

    # version number
    with open(pathToVersionFile, 'r') as versionFile:

        # research version number in file with regex
        for line in versionFile:
            researchVersion = search(r"^\s*VERSION (\d+\.\d+\.\d+)$", line)
            if (researchVersion):
                print(f"\"Fastfetch {researchVersion.group(1)}\"", end=" ")
                break

    print(f"\"{titlePage}\"")


    ###### Name ######

    print(".SH NAME")
    print(nameSection)


    ##### Synopsis ######

    print(".SH SYNOPSIS")
    print(".B fastfetch")
    print(f"[{startUnderline}OPTIONS{endUnderline}...]")


    ###### Description ######

    print(".SH DESCRIPTION")
    print(descriptionSection)


    ###### Configuration ######

    print(".SH CONFIGURATION")
    print(configurationSection)


    ###### Options ######

    print(".SH OPTIONS")
    print(optionSection)
    print()

    # loop through every options sections
    for key, value in helpFileData.items():

        # print new subsection
        print(f".SS {key}")

        # loop through every option in a section
        for option in value:
            # list of existing keys for this option
            keyList = option.keys()

            # start a new "option" entry
            print(".TP")
            print(startBold, end="")

            # short option (-opt)
            if "short" in keyList:
                print(fr"\-{ option['short'] }", end="")
                # if also have a long option, print a comma
                if "long" in keyList:
                    print(", ", end="")

            # long option (--option)
            if "long" in keyList:
                print(fr"\-\-{ option['long'] }", end="")

            print(endBold, end=" ")

            # arguments
            if "arg" in keyList:
                # if argument is optional, print "[arg]"
                if "optional" in option["arg"].keys() and option["arg"]["optional"]:
                    print(startOptionalArgument + option['arg']['type'] + endOptionalArgument, end="")

                # if argument is mandatory, print "arg"
                else:
                    print(startMandatoryArgument + option['arg']['type'] + endMandatoryArgument, end="")

            # description
            print()

            # If desc is a list, join with newlines and proper spacing
            if isinstance(option['desc'], list):
                desc_text = "\n ".join(option['desc'])
                print(f" {desc_text}")
            else:
                print(f" {option['desc']}")

            # Add remarks if available
            if "remark" in keyList:
                print()
                if isinstance(option['remark'], list):
                    for remark in option['remark']:
                        print(f" {remark}")
                else:
                    print(f" {option['remark']}")

            print()


    ###### Examples ######

    print(".SH EXAMPLES")
    print(exampleSection)


    ###### See Also ######

    print(".SH \"SEE ALSO\"")
    print(".BR neofetch (1)")


    ###### Bugs ######

    print(".SH BUGS")
    print(bugSection)


    ###### Authors ######

    print(".SH AUTHORS")
    print(authorsSection)


if __name__ == "__main__":
    main()
