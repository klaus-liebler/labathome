import glob
import os

source = glob.glob("*.kicad_pcb")[0]
source_renamed=source+"-bak_move";
sourceHandle=open(source, mode="r")
sourceLines = sourceHandle.read().splitlines()
sourceHandle.close()
targetLines=[]

for line in sourceLines:
    if(line.find("fp_text reference")!=-1 and line.find("hide")==-1):
        line+= " hide"
    if(line.find("fp_text value")!=-1 and line.find("hide")!=-1):
        line=line.replace("hide", "")
    if(line.find("fp_text value")!=-1):
        line=line.replace("layer F.Fab", "layer F.SilkS")
        line=line.replace("layer B.Fab", "layer B.SilkS")
    targetLines.append(line)

try:
    os.remove(source_renamed);
except:
    print("")
os.rename(source, source_renamed)
targetHandle = open(source, "w")
targetHandle.write("\n".join(targetLines))
targetHandle.close()