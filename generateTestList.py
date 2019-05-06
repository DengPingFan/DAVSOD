#!/usr/bin/python
import os
import glob



def listFilesToTxt(dir,txtFile,extension,recursion):
    exts = extension.split(";")
    files = os.listdir(dir)

    for name in files:
        fullname = os.path.join(dir,name)
        if(os.path.isdir(fullname) & recursion):
            listFilesToTxt(fullname,txtFile,extension,recursion)
        else:
            for ext in exts:
                if (name.endswith(ext)):
                    txtFile.write(name+"\n")
                    break


rootpath = './Datasets/'

dataset = ['DAVIS']

#datasets  = ['DAVIS', 'DAVSOD', 'FBMS', 'MCL' , 'SegTrack-V1', 'SegTrack-V2', 'UVSD', 'ViSal', 'VOS']

for dataset in datasets:
    
    dataPath = rootpath + dataset + '/'


    seqList = os.listdir(dataPath)

    outfile = './txt/' + dataset + '_test.txt'

    with open(outfile,"w") as file:
        for l in seqList:
            s = dataPath + l + '/Imgs/'
            imgFiles = glob.glob(os.path.join(s,'*.jpg'))
            imgFiles.sort()
            for f in imgFiles:
                file.write(f+ ' 0'+"\n")


    
