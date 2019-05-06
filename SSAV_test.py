import numpy as np
import sys
caffe_root = './mycaffe-convlstm/'
sys.path.insert(0, caffe_root + 'python')
#sys.path.append('./mycaffe-convlstm/python')
import caffe
import os
import cv2
import scipy.misc as misc
import datetime
import math

T = 3
#T = 1

def MaxMinNormalization(x,Max,Min):  
    x = (x - Min) / (Max - Min);  
    return x; 

caffe.set_mode_gpu()
caffe.set_device(0)
print "Load net..."

net = caffe.Net('./test.prototxt','./model/SSAV.caffemodel', caffe.TEST)
#net = caffe.Net('./test.prototxt','./snapshot/SSAV_iter_15800.caffemodel', caffe.TEST)


Dataset = 'DAVIS'

with open('./txt/' + Dataset + '_test.txt') as f:
	lines = f.readlines()

for idx in range(len(lines)/T):
	print "Run net..."
	net.forward()
	for i in xrange(T):
		all = net.blobs['conv7_sm'].data
		out = all[i][0]
		line = lines[idx*T+i].replace(" 0\n", "")
		dir, file = os.path.split(line)
		file = file.replace(".jpg", '.png')
		print(dir)
		video = dir.split("/")[-2]
		img = misc.imread(line)
		out = misc.imresize(out, img.shape)
		out = out.astype('float')
		out = MaxMinNormalization(out, out.max(), out.min())
		savePath = './results/SSAV/SSAV_' + Dataset + '/' + video + '/'

		if not os.path.exists(savePath):
			os.makedirs(savePath)
		misc.imsave(savePath + file, out)
		print 'Save the ' + str(idx*T+i) + ' Image: ' + savePath
