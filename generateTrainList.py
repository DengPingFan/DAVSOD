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




rootpath = './Training-Datasets/'

dataset = 'DUTS-train'

dataPath = rootpath + dataset


seqList = os.listdir(dataPath+'/img')

outfile = "./txt/DUTS_train.txt"

with open(outfile,"w") as file:
    for l in seqList:
        img_s = dataPath + '/img/'+ l + '/'
        gt_s  = dataPath + '/gt/'+ l + '/'
        file.write(img_s + ' ' + gt_s + "\n")


    
