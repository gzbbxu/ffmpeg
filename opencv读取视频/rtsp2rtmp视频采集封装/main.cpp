/*******************************************************************************
**                                                                            **
**                     Jiedi(China nanjing)Ltd.                               **
**	               �������Ĳܿ����˴��������Ϊѧϰ�ο�                       **
*******************************************************************************/

/*****************************FILE INFOMATION***********************************
**
**  Project       : FFmpegSDKʵս�γ�
**  Description   : ����FFMpegSDK��ʵս�γ�
**  Contact       : xiacaojun@qq.com
**  ����   : http://blog.csdn.net/jiedichina
**  ��Ƶ�γ�
**  http://edu.csdn.net/lecturer/lecturer_detail?lecturer_id=961
**  http://edu.51cto.com/lecturer/12016059.html
**  http://study.163.com/u/xiacaojun
**  https://jiedi.ke.qq.com/
**  ����γ̺���Լ�Ⱥ��ѯ�γ�ѧϰ����
**  Ⱥ�� 132323693 fmpeg��ֱ�������γ�
**  ΢�Ź��ں�  : jiedi2007
**	ͷ����	 : �Ĳܿ�
**
*******************************************************************************/
#include <opencv2/highgui.hpp>
#include <iostream>

#include "XMediaEncode.h"
#include "XRtmp.h"

#pragma comment(lib,"opencv_world320.lib")
using namespace std;
using namespace cv;
int main(int argc, char *argv[])
{
	//���������rtsp url
	char *inUrl = "rtsp://test:test123456@192.168.1.109";
	//nginx-rtmp ֱ��������rtmp����URL
	char *outUrl = "rtmp://192.168.1.44/live";


	//�����������ظ�ʽת��
	XMediaEncode *me = XMediaEncode::Get(0);

	//��װ����������
	XRtmp *xr = XRtmp::Get(0);

	VideoCapture cam;
	Mat frame;
	namedWindow("video");




	int ret = 0;
	try
	{	////////////////////////////////////////////////////////////////
		/// 1 ʹ��opencv��rtsp���
		cam.open(inUrl);
		if (!cam.isOpened())
		{
			throw exception("cam open failed!");
		}
		cout << inUrl << " cam open success" << endl;
		int inWidth = cam.get(CAP_PROP_FRAME_WIDTH);
		int inHeight = cam.get(CAP_PROP_FRAME_HEIGHT);
		int fps = cam.get(CAP_PROP_FPS);

		///2 ��ʼ����ʽת��������
		///3 ��ʼ����������ݽṹ
		me->inWidth = inWidth;
		me->inHeight = inHeight;
		me->outWidth = inWidth;
		me->outHeight = inHeight;
		me->InitScale();

		///4 ��ʼ������������
		//a �ҵ�������
		if (!me->InitVideoCodec())
		{
			throw exception("InitVideoCodec failed!");
		}

		///5 �����װ������Ƶ������
		xr->Init(outUrl);

		//�����Ƶ�� 
		xr->AddStream(me->vc);
		xr->SendHead();

		for (;;)
		{
			///��ȡrtsp��Ƶ֡��������Ƶ֡
			if (!cam.grab())
			{
				continue;
			}
			///yuvת��Ϊrgb
			if (!cam.retrieve(frame))
			{
				continue;
			}
			//imshow("video", frame);
			//waitKey(1);


			///rgb to yuv
			me->inPixSize = frame.elemSize();
			AVFrame *yuv = me->RGBToYUV((char*)frame.data);
			if (!yuv) continue;

			///h264����
			AVPacket *pack = me->EncodeVideo(yuv);
			if (!pack) continue;

			xr->SendFrame(pack);


		}

	}
	catch (exception &ex)
	{
		if (cam.isOpened())
			cam.release();
		cerr << ex.what() << endl;
	}
	getchar();
	return 0;
}