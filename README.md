# SSAV
Code for paper in CVPR2019, 'Shifting More Attention to Video Salient Object Detection', [Deng-Ping Fan](http://dpfan.net), [Wenguan Wang](https://github.com/wenguanwang), [Ming-Ming Cheng](http://mmcheng.net), [Jianbing Shen](http://iitlab.bit.edu.cn/mcislab/~shenjianbing/).

__Contact__:  Deng-Ping Fan, Email: dengpingfan@mail.nankai.edu.cn

![SSAV framework](https://github.com/DengPingFan/DAVSOD/blob/master/figures/framework.png "framework")

__Abstract__
The last decade has witnessed a growing interest in video salient object detection (VSOD). However, the research community long-term lacked a well-established VSOD dataset representative of real dynamic scenes with high-quality annotations. To address this issue, we elaborately collected a visual-attention-consistent Densely Annotated VSOD (DAVSOD) dataset, which contains 226 videos with 23,938 frames that cover diverse realistic-scenes, objects, instances and motions. With corresponding real human eye-fixation data, we obtain precise ground-truths. This is the first work that explicitly emphasizes the challenge of saliency shift, i.e., the video salient object(s) may dynamically change. To further contribute the community a complete benchmark, we systematically assess 17 representative VSOD algorithms over seven existing VSOD datasets and our DAVSOD with totally ~84K frames (largest-scale). Utilizing three famous metrics, we then present a comprehensive and insightful performance analysis. Furthermore, we propose a baseline model. It is equipped with a saliencyshift-aware convLSTM, which can efficiently capture video saliency dynamics through learning human attention-shift behavior. Extensive experiments1 open up promising future directions for model development and comparison.

__pre-computed saliency maps__: [http://dpfan.net/DAVSOD/](http://dpfan.net/DAVSOD/). 


## Usage
1. Clone this repo into your computer
```bash
git clone https://github.com/DengPingFan/DAVSOD.git
```
2. Cd to `DAVSOD/mycaffe-convlstm`, follow the [official instructions](http://caffe.berkeleyvision.org/installation.html) to build caffe. We provide our make file `Makefile.config` in folder `DAVSOD/mycaffe-convlstm`.

The code has been tested successfully on Ubuntu 16.04 with CUDA 8.0 and OpenCV 3.1.0

3. Make 'caffe'
```
make all -j8
```

4. Make 'pycaffe'
```
make pycaffe
```

5. Download pretrained caffemodel from [my homepage](http://dpfan.net/DAVSOD) or directly from [[baidu pan](https://pan.baidu.com/s/1dg_dcgFNOnUubfQev0e4Ag)](Fetch Code: pb0h)/ [[google drive](https://drive.google.com/open?id=1o9PkfgMpUI8McGSCgWG8cdGJF4dFmHrM)] and extract the .zip file under the root directory `DAVSOD/model/`. 
If you want to train the model start from scratch, you can download the basemodel from [[baidu pan](https://pan.baidu.com/s/1qEyXennBYT2yv82bNx5TgA)](Fetch Code:0xk4) or [[google drive]()] 

6. Put the test image in `DAVSOD/Datasets/` and run `generateTestList.py` to get the test list. Then run `SSAV_test.py` to get the saliency maps. 
The results will be saved in `DAVSOD/results/SSAV/`. 

7. You can also evaluate the model performance (S-measure[1], E-measure[2], F-measure and MAE) using our one-key matlab code `main.m` in `DAVSOD/EvaluateTool/` directory.

```
[1]Structure-measure: A New Way to Evaluate the Foregournd Maps, ICCV2017, spotlight.
```

```
[2]Enhanced Alignment Measure for Binary Foreground Map Evaluation, IJCAI2018, Oral.
```

*************************************************************************************************************
Note that: This version only provide the implicit manner for learning attention-shift. 
           The explicit way to train this model will not be released due to the commercial purposes (Hua Wei, IIAI).
*************************************************************************************************************

## Performance Preview
Quantitative comparisons
![table4](https://github.com/DengPingFan/DAVSOD/blob/master/figures/Table4.png "table4")

Quanlitative comparisons
![figure6](https://github.com/DengPingFan/DAVSOD/blob/master/figures/Figure6.png "figure6")


## Related Citations (BibTeX)
If you find this useful, please cite the related works as follows:
SSAV model/DAVSOD dataset
```
@InProceedings{Fan_2019_CVPR,
   author = {Fan, Deng-Ping and Wang, Wenguan and Cheng, Ming-Ming and Shen, Jianbing}, 
   title = {Shifting More Attention to Video Salient Object Detection},
   booktitle = {IEEE CVPR},
   year = {2019}
}
```

Metrics
```
%E-measure
@inproceedings{Fan2018Enhanced,
   author={Fan, Deng-Ping and Gong, Cheng and Cao, Yang and Ren, Bo and Cheng, Ming-Ming and Borji, Ali},
   title={{Enhanced-alignment Measure for Binary Foreground Map Evaluation}},
   booktitle={IJCAI},
   pages={698--704},
   year={2018}
}
```

```
%S-measure
@inproceedings{fan2017structure,
  author    = {Fan, Deng-Ping and Cheng, Ming-Ming and Liu, Yun and Li, Tao and Borji, Ali},
  title     = {{Structure-measure: A New Way to Evaluate Foreground Maps}},
  booktitle = {IEEE ICCV},
  year      = {2017},
  pages     = {4548-4557}
}
```

##License

	Copyright (c) 2019, Deng-Ping Fan
	All rights reserved.
    
	This code is for academic communication only and not for commercial purposes. 
	If you want to use for commercial please contact me.
	
	Redistribution and use in source with or without
	modification, are permitted provided that the following conditions are
	met:
    		* Redistributions of source code must retain the above copyright
      		  notice, this list of conditions and the following disclaimer.
    		* Redistributions in binary form must reproduce the above copyright
      		  notice, this list of conditions and the following disclaimer in
      		  the documentation and/or other materials provided with the distribution

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 	
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


If you find any bugs, please contact Deng-Ping Fan (dengpingfan@mail.nankai.edu.cn).
