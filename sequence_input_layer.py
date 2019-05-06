#-*- coding: utf-8 -*-
#!/usr/bin/env python

#Data layer for video.  Change RGB_frames to be the path to the RGB frames.

import sys
sys.path.append('./mycaffe-convlstm/python')
import caffe
import io
from PIL import Image
#import matplotlib.pyplot as plt
import numpy as np
import scipy.misc as misc
import time
import pdb
import glob
import pickle as pkl
import random
import h5py
from multiprocessing import Pool
from threading import Thread
import skimage.io
import copy
import matplotlib
matplotlib.use('Agg')


BASE_DIR = './Training-Datasets/DUTS-train'
test_frames = 3  #5 
train_frames = 3  #5
test_buffer = 1
train_buffer = 1 
# train_buffer = 1 


def processImageCrop(im_info, transformer, flow):
  im_path = im_info[0]
  im_reshape = im_info[1]
  data_in = caffe.io.load_image(im_path)
  if (data_in.shape[0] < im_reshape[0]) | (data_in.shape[1] < im_reshape[1]):
    data_in = caffe.io.resize_image(data_in, im_reshape)
  processed_image = transformer.preprocess('data_in',data_in)
  return processed_image

class ImageProcessorCrop(object):
  def __init__(self, transformer, flow):
    self.transformer = transformer
    self.flow = flow
  def __call__(self, im_info):
    return processImageCrop(im_info, self.transformer, self.flow)

class sequenceGeneratorVideo(object):
  
  def __init__(self, buffer_size, clip_length, num_videos, video_dict, video_order):
    self.buffer_size = buffer_size
    self.clip_length = clip_length
    self.N = self.buffer_size * self.clip_length
    self.num_videos = num_videos
    self.video_dict = video_dict
    self.video_order = video_order
    self.idx = 0

  def __call__(self):
    label_r = []
    im_paths = []
    im_reshape = []  
    label_paths = []
    att_paths = []
 
    if self.idx + self.buffer_size >= self.num_videos:
      idx_list = range(self.idx, self.num_videos)
      idx_list.extend(range(0, self.buffer_size-(self.num_videos-self.idx)))
    else:
      idx_list = range(self.idx, self.idx+self.buffer_size)

    for i in idx_list:
      key = self.video_order[i]

      video_reshape = self.video_dict[key]['reshape']

      im_reshape.extend([(video_reshape)]*self.clip_length)
      x = 4 #6
      k = random.randint(1,x)
      top = self.video_dict[key]['num_frames']-self.clip_length*k-1
      while top <= 1:
        x -= 1
        k = random.randint(1,x)
        top = self.video_dict[key]['num_frames']-self.clip_length*k-1

      rand_frame = random.randint(0, top)
      frames = []
      labels = []
      atts = []

      for i in range(rand_frame,rand_frame+k*self.clip_length, k):
        frames.append(self.video_dict[key]['frames'][i])
        labels.append(self.video_dict[key]['labels'][i])
        atts.append(self.video_dict[key]['atts'][i])
      
      im_paths.extend(frames) 
      label_paths.extend(labels)
      att_paths.extend(atts)
    
    im_info = zip(im_paths, im_reshape)
    label_info = zip(label_paths, im_reshape)
    att_info = zip(att_paths,im_reshape)


    self.idx += self.buffer_size
    if self.idx >= self.num_videos:
      self.idx = self.idx - self.num_videos

    return att_info, label_info, im_info

def advance_batch(result, sequence_generator, image_processor, label_processor, attlabel_processor, pool):
  label_info, att_info, im_info = sequence_generator()
  result['label'] = pool.map(label_processor, label_info)
  result['attlabel'] = pool.map(attlabel_processor,att_info)

  for i in range(0, len(label_info)):
      result['label'][i] = result['label'][i][0]/255.
      result['attlabel'][i] = result['label'][i][0]/255.
      
  result['data'] = pool.map(image_processor, im_info)
  cm = np.ones(len(label_info))
  #cm[0::5] = 0
  cm[0::3] = 0
  result['clip_markers'] = cm

class BatchAdvancer():
    def __init__(self, result, sequence_generator, image_processor, label_processor, attlabel_processor, pool):
      self.result = result
      self.sequence_generator = sequence_generator
      self.image_processor = image_processor
      self.label_processor = label_processor
      self.attlabel_processor = attlabel_processor

      self.pool = pool
 
    def __call__(self):
      #multi-threading
      return advance_batch(self.result, self.sequence_generator, self.image_processor, self.label_processor, self.attlabel_processor, self.pool)


class videoRead(caffe.Layer):

  def initialize(self):
    self.train_or_test = 'train'
    self.flow = False
    self.buffer_size = test_buffer  #num videos processed per batch
    self.frames = test_frames   #length of processed clip
    self.N = self.buffer_size*self.frames
    self.idx = 0
    self.channels = 3
    self.channel = 1
    self.height = 473
    self.width = 473
    self.path_to_images = BASE_DIR 
    
    self.video_list  = './txt/DUTS_train.txt'

  def setup(self, bottom, top):
    # random.seed(10)
    self.initialize()
    f = open(self.video_list, 'r')
    f_lines = f.read().splitlines()
    f.close()

    video_dict = {}
    current_line = 0
    self.video_order = []

    for ix, line in enumerate(f_lines):
      video = line.split()[0].split('/')[-2]
      img = line.split()[0]
      gt = line.split()[1]
      # att = line.split()[2]
      att = gt # video without attention or pure static image
      frames = glob.glob('%s/*.*' % img)
      frames.sort()
      labels = glob.glob('%s/*.png' % gt)
      labels.sort()
      atts = glob.glob('%s/*.png' % att)
      atts.sort()
      num_frames = len(frames)
      video_dict[video] = {}
      video_dict[video]['frames'] = frames
      video_dict[video]['reshape'] = (473, 473)
      video_dict[video]['num_frames'] = num_frames
      video_dict[video]['labels'] = labels
      video_dict[video]['atts'] = atts
      self.video_order.append(video)

    self.video_dict = video_dict
    self.num_videos = len(video_dict.keys())

    #set up data transformer
    shape = (self.N, self.channels, self.height, self.width)
    shape1 = (self.N, self.channel, self.height, self.width)

    shape2 = (self.N, self.channel, self.height, self.width)
        
    self.transformer = caffe.io.Transformer({'data_in': shape})
    self.transformer.set_raw_scale('data_in', 255)
    self.transformer1 = caffe.io.Transformer({'data_in': shape1})
    self.transformer1.set_raw_scale('data_in', 255)

    self.transformer2 = caffe.io.Transformer({'data_in': shape2})
    self.transformer2.set_raw_scale('data_in', 255)

    if self.flow:
      image_mean = [128, 128, 128]
      self.transformer.set_is_flow('data_in', True)
    else:
      image_mean = [104.00698793, 116.66876762, 122.67891434]
      self.transformer.set_is_flow('data_in', False)

    channel_mean = np.zeros((3,473,473))
    for channel_index, mean_val in enumerate(image_mean):
      channel_mean[channel_index, ...] = mean_val

    self.transformer.set_mean('data_in', channel_mean)
    self.transformer.set_channel_swap('data_in', (2, 1, 0)) #RGB to BGR

    self.transformer.set_transpose('data_in', (2, 0, 1))
    self.transformer1.set_transpose('data_in', (2, 0, 1))
    self.transformer2.set_transpose('data_in', (2, 0, 1))

    self.thread_result = {}
    self.thread = None
    pool_size = 1 

    self.image_processor = ImageProcessorCrop(self.transformer, self.flow)
    self.label_processor = ImageProcessorCrop(self.transformer1, self.flow)
    self.attlabel_processor = ImageProcessorCrop(self.transformer2,self.flow)
    
    self.sequence_generator = sequenceGeneratorVideo(self.buffer_size, self.frames, self.num_videos, self.video_dict, self.video_order)

    self.pool = Pool(processes=pool_size)
    self.batch_advancer = BatchAdvancer(self.thread_result, self.sequence_generator, self.image_processor, self.label_processor, self.attlabel_processor, self.pool)
    
    ########################################
    self.dispatch_worker()
    ########################################
    
    self.top_names = ['data', 'label', 'attlabel', 'clip_markers']
    if len(top) != len(self.top_names):
      raise Exception('Incorrect number of outputs (expected %d, got %d)' %
                      (len(self.top_names), len(top)))

    ########################################
    self.join_worker()
    ########################################

    for top_index, name in enumerate(self.top_names):
      if name == 'data':
        shape = (self.N, self.channels, self.height, self.width)
      elif name == 'label':
        shape = (self.N, self.channel, self.height, self.width)
      elif name == 'attlabel':
        shape = (self.N, self.channel, self.height, self.width)
      elif name == 'clip_markers':
        shape = (self.N,)
      top[top_index].reshape(*shape)

  def reshape(self, bottom, top):
    pass

  def forward(self, bottom, top):  
    if self.thread is not None:
      ########################################
      self.join_worker() 
      ########################################

    #rearrange the data: 
    #The LSTM takes inputs as [video0_frame0, video1_frame0,...] 
    #but the data is currently arranged as [video0_frame0, video0_frame1, ...]
    new_result_data = [None]*len(self.thread_result['data']) 
    new_result_label = [None]*len(self.thread_result['label']) 
    new_result_att = [None]*len(self.thread_result['attlabel']) 
    new_result_cm = [None]*len(self.thread_result['clip_markers'])
    for i in range(self.frames):
      for ii in range(self.buffer_size):
        old_idx = ii*self.frames + i
        new_idx = i*self.buffer_size + ii
        new_result_data[new_idx] = self.thread_result['data'][old_idx]
        new_result_label[new_idx] = self.thread_result['label'][old_idx]
        new_result_att[new_idx] = self.thread_result['attlabel'][old_idx]
        new_result_cm[new_idx] = self.thread_result['clip_markers'][old_idx]

    for top_index, name in zip(range(len(top)), self.top_names):
      if name == 'data':
        for i in range(self.N):
          top[top_index].data[i, ...] = new_result_data[i] 
      elif name == 'label':
        for i in range(self.N):
          top[top_index].data[i, ...] = new_result_label[i]
      elif name == 'attlabel':
        for i in range(self.N):
          top[top_index].data[i, ...] = new_result_att[i]
      elif name == 'clip_markers':
        top[top_index].data[...] = new_result_cm

    ########################################
    self.dispatch_worker()
    ######################################## 

  def dispatch_worker(self):
    assert self.thread is None
    self.thread = Thread(target=self.batch_advancer)
    self.thread.start()

  def join_worker(self):
    assert self.thread is not None
    self.thread.join()
    self.thread = None

  def backward(self, top, propagate_down, bottom):
    print "backward!"
    pass

class videoReadTrain_RGB(videoRead):

  def initialize(self):
    self.train_or_test = 'train'
    self.flow = False
    self.buffer_size = train_buffer  #num videos processed per batch
    self.frames = train_frames   #length of processed clip
    self.N = self.buffer_size*self.frames
    self.idx = 0
    self.channels = 3
    self.channel = 1
    self.height = 473
    self.width = 473
    self.path_to_images = BASE_DIR
    
    self.video_list = './txt/DUTS_train.txt'
    #Formate of the DUTS_train.txt:
    #./Training-Datasets/DUTS-train/img/part_000/ ./Training-Datasets/DUTS-train/gt/part_000/

class videoReadTest_RGB(videoRead):

  def initialize(self):
    self.train_or_test = 'test'
    self.flow = False
    self.buffer_size = test_buffer  #num videos processed per batch
    self.frames = test_frames   #length of processed clip
    self.N = self.buffer_size*self.frames
    self.idx = 0
    self.channels = 3
    self.channel = 1
    self.height = 473
    self.width = 473
    self.path_to_images = BASE_DIR
    self.video_list = './txt/FBMS_test.txt' 
