#!/usr/bin/env python3
"""
Python script to generate a man page for the command `fastfetch`.
The man content will be printed to stdout so you will need to 
pipe it to a file if you want to save it.
The command options will be generated using a JSON file.
For the format of the JSON file, see https://github.com/fastfetch-cli/fastfetch/blob/dev/src/data/help.json
"""

from json import load
from datetime import date
from re import search

###### Parameters ######

# path to the JSON option file
pathToHelpFile = "../src/data/help.json"
# man page section
manSection = 1
# title (center header)
titlePage = "Fastfetch man page" 
# date (center footer)
todayDate = date.today().strftime("%b %d %Y") # format : "Month (abbreviation) Day Year"
# file to fastfetch version (left footer)
pathToVersionFile = "../CMakeLists.txt"


# text displayed in the "NAME" section
nameSection = "\
fastfetch - a neofetch-like tool for fetching system \
information and displaying them in a pretty way"    

# text displayed in the "DESCRIPTION" section
descriptionSection = "\
A maintained, feature-rich and performance \
oriented, neofetch like system information tool."   

# text displayed at the beginning of the "OPTIONS" section
optionSection = "\
Parsing is not case sensitive. E.g. \\fB--lib-PCI\\fR \
is equal to \\fB--Lib-Pci\\fR. \
    \n\n\
If a value is between square brakets, it is optional. \
An optional boolean value defaults to true if not \
specified. \
    \n\n\
More detailed help messages for each options can be \
printed with \\fB-h <option_without_dash_prefix>\\fR. \
    \n\n\
All options can be made permanent with command \
\\fBfastfetch <options> --gen-config\\fR. \
"

# text displayed in the "BUGS" section
bugSection = "Please report bugs to : \
https://github.com/fastfetch-cli/fastfetch/issues"


###### Text Decorations Tags ######

startUnderline = "\\fI" # start underline text tag
endUnderline = "\\fR" # end underline text tag

startBold = "\\fB" # start bold text tag
endBold = "\\fR" # end bold text tag


###### Argument decoration ######

### optional arguments tags ###

# if an optional argument is displayed as [?optArg] (with "optArg" underlined)
# this value should be f"[?{startUnderline}"
startOptionalArgument = f"[{startUnderline}?"
# if an optional argument is displayed as [?optArg] (with "optArg underlined")
# this value should be f"{endUnderline}]"
endOptionalArgument = f"{endUnderline}]" 

### mandatory arguments tags ###
startMandatoryArgument = f"{startUnderline}" 
endMandatoryArgument = f"{endUnderline}"

def generateManPage():

    # importing the JSON file
    try:
        with open(pathToHelpFile, 'r') as jsonFile:
            helpFileData = load(jsonFile) # json.load
            jsonFile.close()
    except IOError as error:
        print("Error with file", pathToHelpFile, ":", error)
        return
    

    ######## Start printing the generated .1 file ########


    ###### header, footer & config #####

    print(f".TH man {manSection}", end=" ")
    print(f"\"{todayDate}\"", end=" ")

    # version number
    try:
        with open(pathToVersionFile, 'r') as versionFile:

            # research version number in file with regex
            for line in versionFile:
                researchVersion = search("^\s*VERSION (\d+\.\d+\.\d+)$", line) # re.search()
                if (researchVersion != None):
                    versionNumber = line[researchVersion.start():researchVersion.end()]
                    break

            versionFile.close()
    except IOError as error:
        print("Error with file", pathToHelpFile, ":", error)
        return
    
    print(f"\"{versionNumber}\"", end=" ")     
    print(f"\"{titlePage}\"")


    ###### Name ######

    print(".SH NAME")
    print(nameSection)


    ##### Synopsis ######

    print(".SH SYNOPSIS")
    print(".B fastfetch")
    print(f"[{startUnderline}OPTIONS{endUnderline}]")


    ##### Description #####

    print(".SH DESCRIPTION")
    print(descriptionSection)


    ###### Options ######

    print(".SH OPTIONS")
    print(optionSection)
    print()

    # loop through every options sections
    for key, value in helpFileData.items():

        # print new subsection
        print(f".SS {key}:")

        # loop through every option in a section
        for option in value:
            # list of existing keys for this option
            keyList = option.keys()

            # start a new "option" entry
            print(".TP")
            print(startBold, end="")

            # short option (-opt)
            if "short" in keyList:
                print(f"\\-{ option['short'] }", end="")
                # if also have a long option, print a comma
                if "long" in keyList:
                    print(", ", end="")

            # long option (--option)
            if "long" in keyList:
                print(f"\\-\\-{ option['long'] }", end="")

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
            print(f"\n {option['desc']} \n")

    ###### Bugs ######

    print(".SH BUGS")
    print(bugSection)




if __name__ == "__main__":
    generateManPage()