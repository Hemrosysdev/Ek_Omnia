#!/usr/bin/python3

#
# Example calls:
# ./<scriptname> -i inOutFilename
# ./<scriptname> inFilename outFilename
#

import sys 
import re

# default settings:
inPlace        = False;
inputFilename  = "SettingsStatemachine.cpp"
outputFilename = "out.cpp"
newRuler       = """
///////////////////////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////////////////////"""


# argument parsing:
if len(sys.argv)>1:
    if sys.argv[1]=="-i":
        inPlace = True
    else:
        inputFilename = sys.argv[1]
if len(sys.argv)>2:
    if inPlace==True:
        inputFilename  = sys.argv[2]
        outputFilename = sys.argv[2]
    else:
        outputFilename = sys.argv[2]

if inPlace==True:
    outputFilename = inputFilename
    
    
# Info:
print("Settings:")
print(" inputFilename:  \""+inputFilename+"\"")
print(" outputFilename: \""+outputFilename+"\"")
print("")

# Action:

inFile = open(inputFilename, 'r')
inFileLines = inFile.readlines();
inFileContent = "".join(inFileLines);

content = inFileContent

# Remove old rule after function signature:
content = re.sub("/[*][+-=#]+[*]/\n{", "{", content)

# Replace old rule before function signature:
content = re.sub("/[*][+-=#]+[*]/\n", "\n", content)

# Add new rulers:
content = re.sub("\n}\n[\s\n]*", "\n}\n"+newRuler+"\n\n", content)

print(content)

with open(outputFilename, "w+") as outFile:
    outFile.write(content)


exit()

