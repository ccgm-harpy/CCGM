import re
import pyperclip

input("Copy line from il2cpp-functions.h-> ")
inputStr = pyperclip.paste()
print(f"Using input:\n{inputStr}\n")

def get_args(argStr):
    arguments = re.findall(r"\b([\w \*]+)\s+([\w \*]+)(?=[,\)])", argStr)
    return arguments

address, returnType, functionName, arguments = re.findall("(0x[\d\w]*), ([\d\w]*), ([\d\w]*), \((.*)\)\);", inputStr)[0]
arguments = get_args(f"{arguments})")
argumentsString = ""
varTypes = []
varNames = []

for varType, name in arguments[:-1]:
    varTypes.append(varType)
    varNames.append(name)
    argumentsString += f"{varType} {name}, "

argumentsString += f"{arguments[-1][0]} {arguments[-1][1]}"
varTypes.append(arguments[-1][0])
varNames.append(arguments[-1][1])

cppFormat = f"""auto HF_{functionName} = new HookFunction<{returnType}, {', '.join(varTypes)}>({address});
void Template_{functionName}({argumentsString}) """ + "{\n" + f"    HF_{functionName}->original({', '.join(varNames)});   \n" + "}"

pyperclip.copy(cppFormat)
print(f"Copied:\n{cppFormat}")

input("Paste into your templates-> ")

injectStr = f"""Injector::Inject(HF_{functionName}, {functionName}, &Template_{functionName});"""
pyperclip.copy(injectStr)
print(f"Copied\n{injectStr}\n\nPaste into your injects")