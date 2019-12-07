### ffmpeg 参数

```
AVCodecContext * pCodecCtxEnc;  
AVCodec *codec;//编码器
codec = avcodec_find_encoder(AV_CODEC_ID_H264);//h.264编码器查找  
/*AVCodecContext 相当于虚基类，需要用具体的编码器实现来给他赋值*/  
pCodecCtxEnc=avcodec_alloc_context3(codec);  
```

```c++
  
//编码器的ID号，这里我们自行指定为264编码器，实际上也可以根据video_st里的codecID 参数赋值  
pCodecCtxEnc->codec_id = AV_CODEC_ID_H264;  
  
//编码器编码的数据类型  
pCodecCtxEnc->codec_type = AVMEDIA_TYPE_VIDEO;  
  
//目标的码率，即采样的码率；显然，采样码率越大，视频大小越大  
pCodecCtxEnc->bit_rate = 200000;  
  
//固定允许的码率误差，数值越大，视频越小  
pCodecCtxEnc->bit_rate_tolerance = 4000000;  
  
//编码目标的视频帧大小，以像素为单位  
pCodecCtxEnc->width = 640;  
pCodecCtxEnc->height = 480;  
  
//帧率的基本单位，我们用分数来表示，  
//用分数来表示的原因是，有很多视频的帧率是带小数的eg：NTSC 使用的帧率是29.97  
pCodecCtxEnc->time_base.den = 30;  
pCodecCtxEnc->time_base = (AVRational){1,25};  
pCodecCtxEnc->time_base.num = 1;  
  
//像素的格式，也就是说采用什么样的色彩空间来表明一个像素点  
pCodecCtxEnc->pix_fmt = PIX_FMT_YUV420P;  
  
//每250帧插入1个I帧，I帧越少，视频越小  
[cpp] view plaincopy

pCodecCtxEnc->gop_size = 250;  
  
//两个非B帧之间允许出现多少个B帧数  
//设置0表示不使用B帧  
//b 帧越多，图片越小  
pCodecCtxEnc->max_b_frames = 0;  
  
//运动估计  
pCodecCtxEnc->pre_me = 2;  
  
//设置最小和最大拉格朗日乘数  
//拉格朗日乘数 是统计学用来检测瞬间平均值的一种方法  
pCodecCtxEnc->lmin = 1;  
pCodecCtxEnc->lmax = 5;  
  
//最大和最小量化系数  
pCodecCtxEnc->qmin = 10;  
pCodecCtxEnc->qmax = 50;  
  
//因为我们的量化系数q是在qmin和qmax之间浮动的，  
//qblur表示这种浮动变化的变化程度，取值范围0.0～1.0，取0表示不削减  
pCodecCtxEnc->qblur = 0.0;  
  
//空间复杂度的masking力度，取值范围 0.0-1.0  
pCodecCtxEnc->spatial_cplx_masking = 0.3;  
  
//运动场景预判功能的力度，数值越大编码时间越长  
pCodecCtxEnc->me_pre_cmp = 2;  
  
//采用（qmin/qmax的比值来控制码率，1表示局部采用此方法，）  
pCodecCtxEnc->rc_qsquish = 1;  
  
//设置 i帧、p帧与B帧之间的量化系数q比例因子，这个值越大，B帧越不清楚  
//B帧量化系数 = 前一个P帧的量化系数q * b_quant_factor + b_quant_offset  
pCodecCtxEnc->b_quant_factor = 1.25;  
  
//i帧、p帧与B帧的量化系数便宜量，便宜越大，B帧越不清楚  
pCodecCtxEnc->b_quant_offset = 1.25;  
  
//p和i的量化系数比例因子，越接近1，P帧越清楚  
//p的量化系数 = I帧的量化系数 * i_quant_factor + i_quant_offset  
pCodecCtxEnc->i_quant_factor = 0.8;  
pCodecCtxEnc->i_quant_offset = 0.0;  
  
//码率控制测率，宏定义，查API  
pCodecCtxEnc->rc_strategy = 2;  
  
//b帧的生成策略  
pCodecCtxEnc->b_frame_strategy = 0;  
  
//消除亮度和色度门限  
pCodecCtxEnc->luma_elim_threshold = 0;  
pCodecCtxEnc->chroma_elim_threshold = 0;  
  
//DCT变换算法的设置，有7种设置，这个算法的设置是根据不同的CPU指令集来优化的取值范围在0-7之间  
pCodecCtxEnc->dct_algo = 0;  
  
//这两个参数表示对过亮或过暗的场景作masking的力度，0表示不作  
pCodecCtxEnc->lumi_masking = 0.0;  
pCodecCtxEnc->dark_masking = 0.0;  
```

### H264画质级别

H.264有四种画质级别,分别是baseline, extended, main, high： 
　　1、**Baseline Profile**：基本画质。支持I/P 帧，只支持无交错（Progressive）和CAVLC； 
　　2、**Extended profile**：进阶画质。支持I/P/B/SP/SI 帧，只支持无交错（Progressive）和CAVLC；(用的少) 
　　3、**Main profile**：主流画质。提供I/P/B 帧，支持无交错（Progressive）和交错（Interlaced）， 
　　　 也支持CAVLC 和CABAC 的支持； 
　　4、**High profile**：高级画质。在main Profile 的基础上增加了8x8内部预测、自定义量化、 无损视频编码和更多的YUV 格式； 
H.264 Baseline profile、Extended profile和Main profile都是针对8位样本数据、4:2:0格式(YUV)的视频序列。在相同配置情况下，High profile（HP）可以比Main profile（MP）降低10%的码率。 
根据应用领域的不同，Baseline profile多应用于实时通信领域，Main profile多应用于流媒体领域，High profile则多应用于广电和存储领域。

下图清楚的给出不同的profile&level的性能区别。 

### sws_scale 函数

对每一帧数据进行转换。

c,

这两个值是**成对出现**的。

**srcSlice**           源数据，是一个数组。默认提供的大小是8，**指针数组**，**这个数组根据不同的数据类型，不一定全用，**

比方说RGB24,32 的，交错存放(BGR,BGR.....)，那么这个数组，只用它第一个下标。

而YUV420P，这个数据是分开存得，y 存在下标为0的位置，U存在1，V存在2的位置。怎么知道每个下标存得字节多大呢？ srcStride 

**srcStride 每一行的字节数**，这个数据和srcSlice 大小是一样的。也是8，并且是一一对应的。它是知道你的数据一行存多大。一行传多大为什么要传呢？不是已经知道图片的宽度了吗？主要是存在一些对其策略的问题。比如是以4对齐的，16对齐，32对齐。如果不是16，32 的倍数，**会在每一行数据后面补空。还是为了统一计算效率的问题。**

srcSliceY 从哪个位置开始计算，一般不用。从0开始。

srcSliceH 视频高度。

dst 

dstStride  和上面的一样

返回值：返回输出数据的高度。





### 时间基数

1s内有多少个时间单位 （1s 内有num/den 个时间单位）

time_base : 后面的pts,以什么数来进行计算。时间基数最终换算成秒。

AVRational {int num,int den}  其实是一个浮点数，在ffmpeg 中，为了精确，把浮点数数字位和浮点位分开存放。如果直接用float 存，有波动问题。用2个整数存浮点数。

vc->time_base = {1,25}  每一帧数据加1，1秒钟有25帧。**每个pts 乘以 time_base 得到的就是秒数**。转换毫秒 * 1000，微秒再乘以1000



gop_size: 画面组的大小，多少帧一个关键帧。同等质量的画面，gop 越大，压缩率越高。就压缩的越小。负面影响，中间丢一帧，这50帧大小有可能都不正确。太大 播放跳转的时候，解码的帧数会很多。比如直播要立刻响应，比如当前画面正好是50帧，要把前50帧画面全部解码，才能显示当前帧。性能和效果权衡。

一个画面，一组数据，**第一个一定是关键帧，成为I帧。**



max_b_frames : **如果是0 ，dts,pts 时间一致。**





## PTS/DTS问题

`PTS`：主要用于度量解码后的视频帧什么时候被显示出来
`DTS`：主要是标识读入内存中的字节流在什么时候开始送入解码器中进行解码
通常谈论到`PTS和DTS`的时候，一般都是跟`time_base相关联的`，`time_base`使用来度量时间概念的，如果把1秒分为25等份，你可以理解就是一把尺，那么每一格表示的就是1/25秒。此时的`time_base={1，25}`
如果你是把1秒分成90000份，每一个刻度就是1/90000秒，此时的`time_base={1，90000}`，`time_base`表示的**就是每个刻度是多少秒**

#### av_rescale_q

```
int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)
{
    return av_rescale_q_rnd(a, bq, cq, AV_ROUND_NEAR_INF);
}
```

#### av_rescale_q_rnd

```
int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
                         enum AVRounding rnd)
{
    int64_t b = bq.num * (int64_t)cq.den;
    int64_t c = cq.num * (int64_t)bq.den;
    return av_rescale_rnd(a, b, c, rnd);
}
```

我们转换时间基的时候用到av_rescale_q，av_rescale_q_rnd，av_compare_ts这些函数。

函数av_rescale_rnd其实就是返回(a * b)/c，只是加了一些舍入方案。

av_rescale_q(pts, timebase1, timebase2)，其实就是按照下面的公式计算了一下， 公式如下：  

```
x = pts * (timebase1.num / timebase1.den )*(timebase2.den / timebase2.num);
```

这个x就是转换后的时间戳。



    int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,                     enum AVRounding) av_const;

### 为什么要转换

如果由某个解码器产生固定帧率的码流

**AVCodecContext中的AVRational根据帧率来设定**，如25帧，那么num = 1，den=25

**AVStream中的time_base一般根据其采样频率设定**，如（1，90000）

在某些场景下涉及到PTS的计算时，就涉及到两个Time的转换，以及到底取哪里的time_base进行转换：


场景1：编码器产生的帧，直接存入某个容器的AVStream中，那么此时packet的Time要从AVCodecContext的time转换成目标AVStream的time

场景2：从一种容器中demux出来的源AVStream的frame，存入另一个容器中某个目的AVStream。

此时的时间刻度应该从源AVStream的time，转换成目的AVStream timebase下的时间。

其实，问题的关键还是要理解，**不同的场景下取到的数据帧的time是相对哪个时间体系的**。

demux出来的帧的time：**是相对于源AVStream的timebase**

编码器出来的帧的time：**是相对于源AVCodecContext的timebase**

mux存入文件等容器的time：**是相对于目的AVStream的timebase**



```c++
在解码之前需要进行如下转换:

packet.pts = av_rescale_q_rnd(packet.pts,  

                    ic->streams[videoindex]->time_base,  

                    ic->streams[videoindex]->codec->time_base,  

                    (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));  


在编码后写到file之前需要进行如下转换:

pkt.pts = av_rescale_q_rnd(pkt.pts,  

                            oc->streams[videoindex]->codec->time_base,  

                            oc->streams[videoindex]->time_base,  

                            (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));  


```



#### av_image_alloc

```c++
  pointers[4]：保存图像通道的地址。如果是RGB，则前三个指针分别指向R,G,B的内存地址。第四个指针保留不用
 
  linesizes[4]：保存图像每个通道的内存对齐的步长，即一行的对齐内存的宽度，此值大小等于图像宽度。
 
  w:                 要申请内存的图像宽度。
 
  h:                  要申请内存的图像高度。
 
  pix_fmt:        要申请内存的图像的像素格式。
 
  align:            用于内存对齐的值。
 
  返回值：所申请的内存空间的总大小。如果是负值，表示申请失败。
 
/**
 * Allocate an image with size w and h and pixel format pix_fmt, and
 * fill pointers and linesizes accordingly.
 * The allocated image buffer has to be freed by using
 * av_freep(&pointers[0]).
 *
 * @param align the value to use for buffer size alignment
 * @return the size in bytes required for the image buffer, a negative
 * error code in case of failure
 */
int av_image_alloc(uint8_t *pointers[4], int linesizes[4],
 
                   int w, int h, enum AVPixelFormat pix_fmt, int align);
```



