# SSAV
Code for paper in CVPR2019, 'Shifting More Attention to Video Salient Object Detection', [Deng-Ping Fan](http://dpfan.net), [Wenguan Wang](https://github.com/wenguanwang), [Ming-Ming Cheng](http://mmcheng.net), [Jianbing Shen](http://iitlab.bit.edu.cn/mcislab/~shenjianbing/).

__Contact__:  Deng-Ping Fan, Email: dengpingfan@mail.nankai.edu.cn

![SSAV framework](https://github.com/DengPingFan/DAVSOD/blob/master/figures/framework.png "framework")

__pre-computed saliency maps__: [line](http://dpfan.net/VSOD/). 


## Usage
1. Cd to `SSAV/mycaffe-convlstm`, follow the [official instructions](http://caffe.berkeleyvision.org/installation.html) to build caffe. We provide our make file `Makefile.config` in folder `SSAV/mycaffe-convlstm`.

The code has been tested successfully on Ubuntu 16.04 with CUDA 8.0 and OpenCV 3.1.0

3. Make caffe

4. Download pretrained caffemodel from [my homepage](http://dpfan.net/VSOD) and extract the .zip file under the root directory `SSAV/model/`. 

5. Put the test image in `SSAV/Datasets/` and run `generateTestList.py` to get the test list. Then run `SSAV_test.py` to get the saliency maps. 
The results will be saved in `SSAV/results/SSAV/`. You can also evaluate the model performance (S-measure[1], E-measure[2], F-measure and MAE) 
using our matlab code `main.m` in `SSAV/EvaluateTool/` directory.

[1]Structure-measure: A New Way to Evaluate the Foregournd Maps, ICCV2017, spotlight.
[2]Enhanced Alignment Measure for Binary Foreground Map Evaluation, IJCAI2018, Oral.

## Citation
```
@InProceedings{Fan_2019_CVPR,
   author = {Fan, Deng-Ping and Wang, Wenguan and Cheng, Ming-Ming and Shen, Jianbing}, 
   title = {Shifting More Attention to Video Salient Object Detection},
   booktitle = {IEEE CVPR},
   year = {2019}
}
```
