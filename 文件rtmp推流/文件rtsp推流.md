文件rtmp 推流

```c++
void filePush2(JNIEnv *env, jclass jclass1, jstring filename,jstring pushUrlStr){
    const char*  inUrl = env->GetStringUTFChars(filename,NULL);
    //rtmp://1xxxx/live
    const char * outUrl = env->GetStringUTFChars(pushUrlStr,NULL);
//封装格式(读入，写出)（解封装，得到frame）
    AVFormatContext *inFmtCtx = avformat_alloc_context(), *outFmtCtx = NULL;
    AVOutputFormat *ofmt = NULL;
    AVPacket pkt;
    int error = 0;

    //注册组件
    av_register_all();
    //初始化网络
    avformat_network_init();
    int ret = 0;
    LOGE("视频文件地址 '%s'", inUrl);

    //Input
    if ((ret = avformat_open_input(&inFmtCtx, inUrl, 0, 0)) < 0) {
//        av_strerror(ret, buf, 1024);
//        LOGE("Couldn't open file %s:", input_cstr, ret);
//        LOGE("Couldn't open file %s: ", input_cstr, ret);
        error = 2;
        LOGE("Could not open input file.");
    }
    //获取文件信息
    if ((ret = avformat_find_stream_info(inFmtCtx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        error = 3;
//        goto end;
    }
    //获取视频的索引位置
    int videoindex = -1;
    for (int i = 0; i < inFmtCtx->nb_streams; i++)
        if (inFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    //输出封装格式，推送flv封装格式的视频流
    avformat_alloc_output_context2(&outFmtCtx, NULL, "flv", outUrl); //RTMP
    //avformat_alloc_output_context2(&outFmtCtx, NULL, "mpegts", output_cstr);//UDP

    if (!outFmtCtx) {
        LOGE("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        error = 4;
    }


    for (int i = 0; i < inFmtCtx->nb_streams; i++) {
        //解码器，解码上下文保持一致
        AVStream *in_stream = inFmtCtx->streams[i];
        AVStream *out_stream = avformat_new_stream(outFmtCtx, in_stream->codec->codec);
        if (!out_stream) {
            LOGE("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            error = 5;
        }
        //复制解码器上下文的 设置
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            LOGE("Failed to copy context from input to output stream codec context\n");
            error = 6;
//            goto end;
        }
        //全局的header
        out_stream->codec->codec_tag = 0;
        if (outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }


    //打开输出的AVIOContext IO流上下文
    ofmt = outFmtCtx->oformat;
    //Open output URL
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&outFmtCtx->pb, outUrl, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Could not open output URL '%s'", outUrl);
            error = 7;
        }
    }
    //先写一个头
    ret = avformat_write_header(outFmtCtx, NULL);
    if (ret < 0) {
        LOGE("Error occurred when opening output URL\n");
        error = 8;
    }


    int frame_index = 0;
    int64_t start_time = av_gettime();



    while (1) {
       /* if (isPlay != 0) {
            break;
        }*/
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        ret = av_read_frame(inFmtCtx, &pkt);
        if (ret < 0)
            break;
        //FIX：No PTS (Example: Raw H.264)
        //raw stream 裸流
        //PTS:Presentation Time Stamp 解码后视频帧要在什么时候取出来
        //DTS:送入解码器后什么时候标识进行解码
        if (pkt.pts == AV_NOPTS_VALUE) {
            //Write PTS
            AVRational time_base1 = inFmtCtx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration =
                    (int64_t) ((double) AV_TIME_BASE /
                               av_q2d(inFmtCtx->streams[videoindex]->r_frame_rate));
            //Parameters
            pkt.pts = (int64_t) ((double) (frame_index * calc_duration) /
                                 (double) (av_q2d(time_base1) * AV_TIME_BASE));
            pkt.dts = pkt.pts;
            pkt.duration = (int64_t) ((double) calc_duration /
                                      (double) (av_q2d(time_base1) * AV_TIME_BASE));
//            av_rescale_q()

        }
        //读入速度比较快，可以在这里调整读取速度减轻服务器压力
        if (pkt.stream_index == videoindex) {
            AVRational time_base = inFmtCtx->streams[videoindex]->time_base;
            AVRational time_base_q = {1, AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time){
                LOGE("pts_time - now_time %u",(unsigned int)(pts_time - now_time));
                av_usleep((unsigned int) (pts_time - now_time));
            }


        }

        in_stream = inFmtCtx->streams[pkt.stream_index];
        out_stream = outFmtCtx->streams[pkt.stream_index];
        /* copy packet */
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        LOGE("duration :: %llu",(long long)pkt.duration);
        pkt.pos = -1;
        //Print to Screen
        if (pkt.stream_index == videoindex) {
            LOGE("Send %8d video frames to output URL\n", frame_index);
            frame_index++;
        }
        //写数据
        //ret = av_write_frame(outFmtCtx, &pkt);
        ret = av_interleaved_write_frame(outFmtCtx, &pkt);

        if (ret < 0) {
            LOGE("Error muxing packet\n");
            break;
        }
        av_free_packet(&pkt);

    }
//    isPlay = 0;
    //写结尾
    av_write_trailer(outFmtCtx);
//    (*env)->CallVoidMethod(env, obj, isStream, 0);

    end:
    //释放自愿
    avformat_close_input(&inFmtCtx);
    /* 关闭输出流 */
    if (outFmtCtx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(outFmtCtx->pb);
    avformat_free_context(outFmtCtx);
    env->ReleaseStringUTFChars(pushUrlStr,outUrl);
    env->ReleaseStringUTFChars(filename,inUrl);
   /* if (ret < 0 && ret != AVERROR_EOF) {
        LOGE("Error occurred.\n");
        (*env)->CallVoidMethod(env, obj, isStream, error);
        return -1;
    }
    return 0;*/


}
void filePush(JNIEnv *env, jclass jclass1, jstring filename,jstring pushUrlStr){
    const char*  path = env->GetStringUTFChars(filename,NULL);
    //rtmp://1xxxx/live
    const char * pushUrl = env->GetStringUTFChars(pushUrlStr,NULL);
    LOGE("filePush start");
    //1, 初始化封装 和解封装 flv ,mp4 ,mov
    av_register_all();
    //初始化网络库
    avformat_network_init();

    //AVFormatContext 打开的所有操作都基于 上下文
    // url 文件的绝对路径， 也支持rtmp ,rtsp
    //指定 解封器 AVInputFormat 这里传递null, 自己内部跟后缀名判断
    // AVDictionary 参数的设置，打开文件的参数 不用什么设置，主要在打开rtsp 设置延迟时间。 直接传null
    AVFormatContext* ictx = NULL; //输入封装格式上下文 包含所有的格式内容，io 文件为文件io, 网络为网络io
    //打开文件 解封文件头 mp4 都有统一的协议头的，协议头放所有的文件信息
    int re = avformat_open_input(&ictx,path,0,0);
    if(re!=0){
        //failure
        XError(re);
        return ;
    }
    //解封协议头之后，获取流信息 h264，flv 没有头信息的.  mp4 用也没有问题
    re = avformat_find_stream_info(ictx,0);
    if(re!=0){
        XError(re);
        return ;
    }
    //索引
    //AVFormatContext *ic,
    //   int index,  有音频，视频，字幕 流索引 传 0 代表全部 打印
    //  const char *url, 不重要，知识打印的时候显示的。
    // int is_outpu 是否是输出 ，这里是输入 传0
    av_dump_format(ictx,0,path,0);

    //输出流
    AVFormatContext *octx = NULL;
    //AVFormatContext **ctx,
    // AVOutputFormat *oformat, 输出流的格式  根据输入格式设定
    // const char *format_name, 格式的名称flv  对于输出文件这个参数可以不传，通过文件名就可以判断出来 是什么封装格式，对于流媒体来说，判断不出来，需要指定
    // const char *filename
    //创建输出流上下文
    avformat_alloc_output_context2(&octx,0,"flv",pushUrl);
    if(!octx){
        LOGE("avformat_alloc_output_context2 ERROR ");
        return ;
    }

    /**
     * AVFormatContext
     *    AVIOContext *pb; io 上下文
     *    AVStream ** streams; 视频音频字幕流 指针数组
     *    int nb_streams  存的数量
     */

    /**
     * AVStream
     * AVRational time_base ; 时间戳
     * AVCodecParameters * codecpar;// 音视频参数
     * AVCodecContext *codec; //编码器
     */
    //配置输出流
    //对于推流来说，也不用管什么格式  开发过程中 只用 codecpar 有些格式会有未知异常，转的时候还是使用codec
    //输入流是什么格式，输出流也是需要指定一下。把每个AVStream 的格式读出来，在给他写入输出流
    //遍历输入的AVStream
    for(int i=0;i<ictx->nb_streams;i++){
        // 需要创建一个新的流到输出上下文中 。之前创建的输出流上下文 什么信息也没有 得创建 视频，音频 字幕流
//        const AVCodec *c 什么格式的流 编码格式
        AVStream * out= avformat_new_stream(octx,ictx->streams[i]->codec->codec);
        if(!out){
            LOGE("avformat_new_stream error");
            return;
        }
        //把输入的复制到输出 复制配置信息
        //复制配置信息 同于mp4
//        re=avcodec_copy_context(out->codec,ictx->streams[i]->codec);

        re = avcodec_parameters_copy(out->codecpar,ictx->streams[i]->codecpar);
        if(re!=0){
            LOGE("avcodec_parameters_copy error");
            return;
        }
        out->codec->codec_tag = 0; //不需要编码，直接封装的文件写进去。
        out->codecpar->codec_tag = 0;
    }
    LOGE(" 打开io 建立连接  AVIO_FLAG_WRITE 写 %s , %s",pushUrl,path);
   // av_dump_format(octx,0,pushUrl,1);//这里1是输出
    //输出流的参数配置好了
    //rtmp 开始推流
    //打开io 建立连接  AVIO_FLAG_WRITE 写
    re = avio_open(&octx->pb,pushUrl,AVIO_FLAG_WRITE);
    if(re<0){
        LOGE("avio_open error");
//        XError(re);
        return ;
    }
    if(!octx->pb){
        XError(re);
        return ;
    }
    //写入头信息  AVDictionary 配置项
    LOGE(" avio_open success %d",octx);
    re = avformat_write_header(octx,0);
    LOGE(" avformat_write_header success");
    if(re<0){
        //写入失败
        LOGE(" avformat_write_header failure %d",re);
        return;
    }

    //打开io 写入头 都成功 改变octx stream 流的time_base 的信息，都会做调整
    //推流每一帧数据 播放视频文件，
    //播放视频文件，比如flv : 里面不一定有时间戳 怎么写pts 显示时间dts 解码时间 这个时间没有怎么计算
    AVPacket pkt;
    /**
     * AVPacket
     * int64_t pts ; //pts *(num/den ) pts 里面以time_base 1s 钟有多少个单位，单位就是time_base   pts*(num/den) 得到的是秒，
     *              //这一帧视频或者音频，是第几秒显示。 这里其实很小，是0.0 几秒 是一个浮点数， ,需要换算。
     *              //time_base 是存在AV_stream 中，音频和视频的time_base不一样。AV_stream 是一个流 p代表print
     * int64_t dts // 解码时间
     * uint8_t *data //数据
     * int size  //数据的大小
     * int stream_index; //流的索引 根据索引确定是音频还是视频 调用相应的解码器，编码器
     * int flags
     */

    /***
     * 一组图片 ，有一个关键帧  关键帧存放的是完整的信息，P帧存放的是相对于上一帧的变化，B帧存放的是相对于上一帧和下一帧的变化。这样解码顺序和显示顺序不一致
     * B帧 需要把上一帧和下一帧的算出来，就是先解码，但是显示是必须后显示。 所以pts 和 dts 值不一致问题。
     * 如果没有B帧 就不会存在这个问题。但是有了B帧，压缩率会更高。
     */
     //ffmpeg 提供了工具，计算pts,dts 基于时间基数计算
     long long startTime = av_gettime();// 获取当前的时间戳 微秒
    for(;;){
        //读取每一帧
//        LOGE("av_read_frame start");
        re = av_read_frame(ictx,&pkt);
        LOGE("av_read_frame success");
        if(re!=0){
//            XError(re);
            LOGE("av_read_frame success ERROR");
            break;
        }
        //读到每一帧 信息。
//        LOGE("pkt.pts ",pkt.pos)
        //如果打开摄像头，录制usb 摄像的时候，pts 必须要计算，这里读取文件只需要换算
        //把输入的time_base 和 输出的time_base 对应起来就可
        //不需要解码 直接把视频帧发出去
        //计算转换时间戳 pst dts  为什么不直接取？ 输入的time_base 和 输出的time_base 不一致，特别是avformat_write_header实时，time_base 会变化，所以两边的不一致。
        AVRational itime = ictx->streams[pkt.stream_index]->time_base;
        AVRational otime = octx->streams[pkt.stream_index]->time_base;
        //ffmpeg 提供的函数运算
        //int64_t a, 源pts
        // AVRational bq,  输入timebase
        // AVRational cq, 转换 输出timeBase
        //enum AVRounding rnd
        //转换后，赋值给 pkt.pts
        pkt.pts = av_rescale_q_rnd(pkt.pts,itime,otime,(AVRounding)(AV_ROUND_NEAR_INF| AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts,itime,otime,(AVRounding)(AV_ROUND_NEAR_INF| AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q_rnd(pkt.duration,itime,otime,(AVRounding)(AV_ROUND_NEAR_INF| AV_ROUND_PASS_MINMAX));// 一帧视频 经过的时间
        pkt.pos = -1;// pkt 发出后，是有文件的位置的，发流媒体不存在这个内容，指定-1
        //准备好 发送的pkt 发送这一帧数据

        //视频帧的推送速度
        if(ictx->streams[pkt.stream_index] ->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            //视频帧的时候
            AVRational tb = ictx->streams[pkt.stream_index]->time_base;
            long long now = av_gettime()-startTime; //当前已经过去时间
            //时间基数 1秒钟 有多少个时间单位，这个时间单位本身的运算时由 num,den //
            //dts = pkt.dts;// 是有时间基数的 den 是单位，也就是说 1秒钟有 tb.num/tb.den 个单位,比如 num是1，den 是1000,就是千分之一， pkt.dts 为1，就是千分之一秒，
            //得到秒数，要转换成微秒数
            long long dts = pkt.dts*(1000*1000*r2d(tb));//微妙
//            dts = pkt.dts;// 是有时间基数的
            // 用这个时间
            if(dts>now){
                LOGE("SELLP TIME %lld",(dts-now));
                av_usleep(dts-now);
            }
        }
        re = av_interleaved_write_frame(octx,&pkt);//会根据pts dts 排序发送。也会把Pkt 的空间释放掉。所以不需要 av_packet_unref(&pkt);
        LOGE("av_interleaved_write_frame return %d",re);
        if(re<0){
            //XError(re);
        }


    }




    LOGE("filePush SUCCESS");
    env->ReleaseStringUTFChars(pushUrlStr,pushUrl);
    env->ReleaseStringUTFChars(filename,path);
}

```

