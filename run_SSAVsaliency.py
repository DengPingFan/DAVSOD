import numpy as np
import sys

caffe_root = './mycaffe-convlstm/'
sys.path.insert(0, caffe_root + 'python')

import caffe
import os
import cv2
import scipy.misc as misc
import datetime
import math


def MaxMinNormalization(x,Max,Min):  
    x = (x - Min) / (Max - Min);  
    return x; 

def upsample_filt(size):
    factor = (size + 1) // 2
    if size % 2 == 1:
        center = factor - 1
    else:
        center = factor - 0.5
    og = np.ogrid[:size, :size]
    return (1 - abs(og[0] - center) / factor) * \
           (1 - abs(og[1] - center) / factor)

# set parameters s.t. deconvolutional layers compute bilinear interpolation
# N.B. this is for deconvolution without groups
def interp_surgery(net, layers):
    for l in layers:
        m, k, h, w = net.params[l][0].data.shape
        if m != k:
            print 'input + output channels need to be the same'
            raise
        if h != w:
            print 'filters need to be square'
            raise
        filt = upsample_filt(h)
        net.params[l][0].data[range(m), range(k), :, :] = filt

base_weights = './model/pdb-convlstm.caffemodel' # the vgg16 model

# init
caffe.set_mode_gpu()
caffe.set_device(1)

print "Load net..."


solver = caffe.SGDSolver('solver.prototxt')

# do net surgery to set the deconvolution weights for bilinear interpolation
#interp_layers = [k for k in solver.net.params.keys() if 'up' in k]
#interp_surgery(solver.net, interp_layers)

# copy base weights for fine-tuning
#solver.restore('./snapshot/SSAV_iter_100.solverstate')
solver.net.copy_from(base_weights)

solver.step(50000)
