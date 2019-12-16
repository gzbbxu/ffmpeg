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

#include <QtCore/QCoreApplication>
#include <QAudioInput>
#include <iostream>
#include <QThread>
#include "XMediaEncode.h"
#include "XRtmp.h"
using namespace std;
int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	

	char *outUrl = "rtmp://192.168.1.44/live";
	int ret = 0;
	int sampleRate = 44100;
	int channels = 2;
	int sampleByte = 2;

	///1 qt��Ƶ��ʼ¼��
	QAudioFormat fmt;
	fmt.setSampleRate(sampleRate);
	fmt.setChannelCount(channels);
	fmt.setSampleSize(sampleByte *8);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
	if (!info.isFormatSupported(fmt))
	{
		cout << "Audio format not support!" << endl;
		fmt = info.nearestFormat(fmt);
	}
	QAudioInput *input = new QAudioInput(fmt);
	//��ʼ¼����Ƶ
	QIODevice *io = input->start();


	///2 ��Ƶ�ز��� �����ĳ�ʼ��
	XMediaEncode *xe = XMediaEncode::Get();
	xe->channels = channels;
	xe->nbSample = 1024;
	xe->sampleRate = sampleRate;
	xe->inSampleFmt = XSampleFMT::X_S16;
	xe->outSampleFmt = XSampleFMT::X_FLATP;
	if (!xe->InitResample())
	{
		getchar();
		return -1;
	}
	///4 ��ʼ����Ƶ������
	if (!xe->InitAudioCode())
	{
		getchar();
		return -1;
	}
	


	///5 �����װ������Ƶ������
	//a ���������װ��������
	XRtmp *xr = XRtmp::Get(0);
	if (!xr->Init(outUrl))
	{
		getchar();
		return -1;
	}
	//b �����Ƶ�� 
	if (!xr->AddStream(xe->ac))
	{
		getchar();
		return -1;
	}

	///��rtmp ���������IO
	//д���װͷ
	if (!xr->SendHead())
	{
		getchar();
		return -1;
	}
	//һ�ζ�ȡһ֡��Ƶ���ֽ���
	int readSize = xe->nbSample*channels*sampleByte;
	char *buf = new char[readSize];
	for (;;)
	{
		//һ�ζ�ȡһ֡��Ƶ
		if (input->bytesReady() < readSize)
		{
			QThread::msleep(1);
			continue;
		}
		int size = 0;
		while (size != readSize)
		{
			int len = io->read(buf + size, readSize - size);
			if (len < 0)break;
			size += len;
		}

		if (size != readSize)continue;

		//�Ѿ���һ֡Դ����
		//cout << size << " ";
		//�ز���Դ����
		AVFrame *pcm = xe->Resample(buf);

		//pts ����
		//  nb_sample/sample_rate  = һ֡��Ƶ������sec
		// timebase  pts = sec * timebase.den
		AVPacket *pkt = xe->EncodeAudio(pcm);
		if (!pkt)continue;
		////����
		xr->SendFrame(pkt);
		if (ret == 0)
		{
			cout << "#" << flush;
		}

	}
	delete buf;

	getchar();
	return a.exec();
}
