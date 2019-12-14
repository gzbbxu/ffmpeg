### 基于opencv 采集推流

opencv 采集，ffmpeg 推流

- opencv 采集rtsp 解码

  - 只处理视频，音频被抛弃掉了

  - rtsp 有可能包含音频，相机自带的。但是，直播也不可能用相机自带的。用外接的麦克风。

  - opencv 采集 两种，usb 相机，rtsp 相机。内部处理是由区别的，usb相机，调用系统驱动，rtsp 相机，调用ffmpeg 接口。等同于ffmpeg 打开rtsp, 除了打开之外，还做了转码 + 解码，接口返回的是解码好的rgb数据。

    需要考虑的问题？

    1. rtsp 的源帧率是多少？

       grab 采集，解码 每一帧都要做

       retrieve 跳着做。

```c++
int main(int argc, char *argv[])
{
	//海康相机的rtsp url
	char *inUrl = "rtsp://test:test123456@192.168.1.64";
	VideoCapture cam;
	namedWindow("video");

	//if(cam.open(inUrl))
	if (cam.open(0))
	{
		cout << "open camera success!" << endl;
	}
	else
	{
		cout << "open camera failed!" << endl;

		waitKey(1);
		return -1;
	}
	Mat frame;
	for (;;)
	{
		cam.read(frame);
		imshow("video", frame);
		waitKey(1);
	}

	return 0;
}
```











- ffmpeg 缩放转像素格式。

  - ffmgeg 进行h264编码的时候，是得用到yuv 的数据，还得进行转换。

  - 一开始解码rtsp 的时候，就是yuv ，为什么要转换成rgb ?

    - 因为这两个步骤之间，还要做视频的处理（ 美颜） ，视频的处理，只能在采集端来做，因为不可能放在服务端来做，服务端负载是非常大的，也不可能放在播放端，播放端都是基于flash 的，没有那么高德性能。针对rgb 的视频，做完美颜处理之后，再交给ffmpeg,这个时候，ffmpeg 就需要进行像素的转换。缩放的话，可以用opencv 先做完，肯定是先缩放，再美颜，同等像素，美颜的开销是非常大的。

    - sw_getCachedContext :是从缓冲读取Context ,还提供另外一个sw_getContext ,不通过cached,为什么出现两个呢？分别什么时候用呢？视频帧的处理，有时候是一帧帧的处理，如果中途转换格式发生了变化 ，播放视频的时候，窗口突然放大缩小，你就需要改变转换格式，**getCached 的好处，可以在视频转换之前调用，当重新创建的时候，会返回一个新的，把之前的清理掉。可以放在主循环中做**。如果是sw_getContext, 就不能放在主循环中做，肯定会内存溢出，空间不断的创建

      - ```
        参数
        sws_getCacheContext()
        struct SwsContext * context //传入的参数和配置项是否一致，如果不一致，清理掉，然后再返回上下文。
        int srcW,int srcH,enum AVPixelFormat srcFormat; //源
  int dstW,int dstH //目标
        enum AVPixelFormat stFormat;//目标
        int flags // 图像缩放的算法 SWS_BICUBIC
        srcFilter,dstFilter ,param
        ```
        
        ```c++
        int main(int argc, char *argv[])
        {
        	//海康相机的rtsp url
        	char *inUrl = "rtsp://test:test123456@192.168.1.64";
        	VideoCapture cam;
        	namedWindow("video");
        
        	//像素格式转换上下文
        	SwsContext *vsc = NULL;
        	
        	try
        	{
        		////////////////////////////////////////////////////////////////
        		/// 1 使用opencv打开rtsp相机
        		cam.open(inUrl);
        		if (!cam.isOpened())
        		{
        			throw exception("cam open failed!");
        		}
        		cout << inUrl << " cam open success" << endl;
        		int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
        		int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
        		int fps = cam.get(CAP_PROP_FPS);
        
        		///2 初始化格式转换上下文
        		vsc = sws_getCachedContext(vsc,
        			inWidth, inHeight, AV_PIX_FMT_BGR24,	 //源宽、高、像素格式
        			inWidth, inHeight, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
        			SWS_BICUBIC,  // 尺寸变化使用算法
        			0, 0, 0
        			);
        		if (!vsc)
        		{
        			throw exception("sws_getCachedContext failed!");
        		}
        		Mat frame;
        		for (;;)
        		{
        			///读取rtsp视频帧，解码视频帧
        			if (!cam.grab())
        			{
        				continue;
        			}
        			///yuv转换为rgb
        			if (!cam.retrieve(frame))
        			{
        				continue;
        			}
        			imshow("video", frame);
        			waitKey(1);
        		}
        	}
        	catch (exception &ex)
        	{
        		if (cam.isOpened())
        			cam.release();
        		if (vsc)
        		{
        			sws_freeContext(vsc);
        			vsc = NULL;
        		}
        		cerr << ex.what() << endl;
        	}
        	getchar();
        
        	return 0;
        }
        ```
        
        **sws_scale** **对每一帧数据进行转换**
        
        

- **ffmpeg 编码成h264  avpacket 包。**

  - **avcodec_find_encoder**  找到编码器，指定格式。指定h264  **AV_CODEC_ID_H264**
    - 另外一个接口是通过名字，来找到编码器。 **avcodec_find_encoder_by_name**
  - **编码器是AVCodec** 的一个结构体。**解码器也是通过AVCodec**   来存储的。
  - **avregiserAll 注册的时候，就把这个 对象创建好了**。
  - **avcodec_alloc_context3** 创建**编码器的上下文**  
  - int **avcodec_open2** **编码器打开**。 
    - 参数 AVCodecContext * avctx 编码器上下文
    - const AVCodec * codec,  编码器
    - AVDicionary ** options 对应的设置，比如ffmpege 调用的x264 ,第三方有很多的参数属性，要实现兼容，**就通过options  来设置key,value 中**。**AVCodecContext  大部分参数都可以设置的。**
  - 配置编码器参数
    - vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; 流媒体来说，这一步不配置也没有关系，而如果是写入到文件中，这一步是不得不配。流媒体是没有全局的头文件的，而普通的mp4是有的，而这样配置也有好处，一打开的时候，就知道格式，所以要加进来。
    - vc->gop_size = 50; 画面组的大小，多少帧一个关键帧。值越大，同等质量压缩越小，负面影响，中间丢一帧，这50帧画面都不正确。跳转的时候，需要解码的帧越多。一组画面的第一帧是关键帧，I帧。B帧是上一帧和下一帧的区别。如果是设置 vc->max_b_frames =0 ; 有一个好处就是pts 和 dts 一致。b帧 是前一帧和后一帧，必须要有后一帧。后一帧要先解码。ffmpeg 解码的时候，根据b帧情况，内部会缓冲帧。

- ffmpeg 推流rtmp

   

  ```c++
  int main(int argc, char *argv[])
  {
  	//海康相机的rtsp url
  	char *inUrl = "rtsp://test:test123456@192.168.1.64";
  	//nginx-rtmp 直播服务器rtmp推流URL
  	char *outUrl = "rtmp://192.168.1.44/live";
  
  	//注册所有的编解码器
  	avcodec_register_all();
  
  	//注册所有的封装器
  	av_register_all();
  
  	//注册所有网络协议
  	avformat_network_init();
  
  
  	VideoCapture cam;
  	Mat frame;
  	namedWindow("video");
  
  	//像素格式转换上下文
  	SwsContext *vsc = NULL;
  
  	//输出的数据结构
  	AVFrame *yuv = NULL;
  
  	//编码器上下文
  	AVCodecContext *vc = NULL;
  
  	//rtmp flv 封装器
  	AVFormatContext *ic = NULL;
  
  
  	try
  	{	////////////////////////////////////////////////////////////////
  		/// 1 使用opencv打开rtsp相机
  		cam.open(inUrl);
  		if (!cam.isOpened())
  		{
  			throw exception("cam open failed!");
  		}
  		cout << inUrl << " cam open success" << endl;
  		int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
  		int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
  		int fps = cam.get(CAP_PROP_FPS);
  
  		///2 初始化格式转换上下文
  		vsc = sws_getCachedContext(vsc,
  			inWidth, inHeight, AV_PIX_FMT_BGR24,	 //源宽、高、像素格式
  			inWidth, inHeight, AV_PIX_FMT_YUV420P,//目标宽、高、像素格式
  			SWS_BICUBIC,  // 尺寸变化使用算法
  			0, 0, 0
  			);
  		if (!vsc)
  		{
  			throw exception("sws_getCachedContext failed!");
  		}
  		///3 初始化输出的数据结构
  		yuv = av_frame_alloc();
  		yuv->format = AV_PIX_FMT_YUV420P;
  		yuv->width = inWidth;
  		yuv->height = inHeight;
  		yuv->pts = 0;
  		//分配yuv空间
  		int ret = av_frame_get_buffer(yuv, 32);
  		if (ret != 0)
  		{
  			char buf[1024] = { 0 };
  			av_strerror(ret, buf, sizeof(buf) - 1);
  			throw exception(buf);
  		}
  
  		///4 初始化编码上下文
  		//a 找到编码器
  		AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  		if (!codec)
  		{
  			throw exception("Can`t find h264 encoder!");
  		}
  		//b 创建编码器上下文
  		vc = avcodec_alloc_context3(codec);
  		if (!vc)
  		{
  			throw exception("avcodec_alloc_context3 failed!");
  		}
  		//c 配置编码器参数
  		vc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; //全局参数
  		vc->codec_id = codec->id;
  		vc->thread_count = 8;
  
  		vc->bit_rate = 50 * 1024 * 8;//压缩后每秒视频的bit位大小 50kB
  		vc->width = inWidth;
  		vc->height = inHeight;
  		vc->time_base = { 1,fps };
  		vc->framerate = { fps,1 };
  
  		//画面组的大小，多少帧一个关键帧
  		vc->gop_size = 50;
  		vc->max_b_frames = 0;
  		vc->pix_fmt = AV_PIX_FMT_YUV420P;
  		//d 打开编码器上下文
  		ret = avcodec_open2(vc, 0, 0);
  		if (ret != 0)
  		{
  			char buf[1024] = { 0 };
  			av_strerror(ret, buf, sizeof(buf) - 1);
  			throw exception(buf);
  		}
  		cout << "avcodec_open2 success!" << endl;
  
  		///5 输出封装器和视频流配置
  		//a 创建输出封装器上下文
  		ret = avformat_alloc_output_context2(&ic, 0, "flv", outUrl);
  		if (ret != 0)
  		{
  			char buf[1024] = { 0 };
  			av_strerror(ret, buf, sizeof(buf) - 1);
  			throw exception(buf);
  		}
  		//b 添加视频流 
  		AVStream *vs = avformat_new_stream(ic, NULL);
  		if (!vs)
  		{
  			throw exception("avformat_new_stream failed");
  		}
  		vs->codecpar->codec_tag = 0;
  		//从编码器复制参数
  		avcodec_parameters_from_context(vs->codecpar, vc);
  		av_dump_format(ic, 0, outUrl, 1);
  
  
  		///打开rtmp 的网络输出IO
  		ret = avio_open(&ic->pb, outUrl, AVIO_FLAG_WRITE);
  		if (ret != 0)
  		{
  			char buf[1024] = { 0 };
  			av_strerror(ret, buf, sizeof(buf) - 1);
  			throw exception(buf);
  		}
  
  		//写入封装头
  		ret = avformat_write_header(ic, NULL);
  		if (ret != 0)
  		{
  			char buf[1024] = { 0 };
  			av_strerror(ret, buf, sizeof(buf) - 1);
  			throw exception(buf);
  		}
  
  		AVPacket pack;
  		memset(&pack, 0, sizeof(pack));
  		int vpts = 0;
  		for (;;)
  		{
  			///读取rtsp视频帧，解码视频帧
  			if (!cam.grab())
  			{
  				continue;
  			}
  			///yuv转换为rgb
  			if (!cam.retrieve(frame))
  			{
  				continue;
  			}
  			imshow("video", frame);
  			waitKey(1);
  
  
  			///rgb to yuv
  			//输入的数据结构
  			uint8_t *indata[AV_NUM_DATA_POINTERS] = { 0 };
  			//indata[0] bgrbgrbgr
  			//plane indata[0] bbbbb indata[1]ggggg indata[2]rrrrr 
  			indata[0] = frame.data;
  			int insize[AV_NUM_DATA_POINTERS] = { 0 };
  			//一行（宽）数据的字节数
  			insize[0] = frame.cols * frame.elemSize();
  			int h = sws_scale(vsc, indata, insize, 0, frame.rows, //源数据
  				yuv->data, yuv->linesize);
  			if (h <= 0)
  			{
  				continue;
  			}
  			//cout << h << " " << flush;
  			///h264编码
  			yuv->pts = vpts;
  			vpts++;
  			ret = avcodec_send_frame(vc, yuv);
  			if (ret != 0)
  				continue;
  
  			ret = avcodec_receive_packet(vc, &pack);
  			if (ret != 0 || pack.size > 0)
  			{
  				//cout << "*" << pack.size << flush;
  			}
  			else
  			{
  				continue;
  			}
  			//推流
  			pack.pts = av_rescale_q(pack.pts, vc->time_base, vs->time_base);
  			pack.dts = av_rescale_q(pack.dts, vc->time_base, vs->time_base);
  			pack.duration = av_rescale_q(pack.duration, vc->time_base, vs->time_base);
  			ret = av_interleaved_write_frame(ic, &pack);
  			if (ret == 0)
  			{
  				cout << "#" << flush;
  			}
  		}
  
  	}
  	catch (exception &ex)
  	{
  		if (cam.isOpened())
  			cam.release();
  		if (vsc)
  		{
  			sws_freeContext(vsc);
  			vsc = NULL;
  		}
  
  		if (vc)
  		{
  			avio_closep(&ic->pb);
  			avcodec_free_context(&vc);
  		}
  
  		cerr << ex.what() << endl;
  	}
  	getchar();
  	return 0;
  }
  ```

  