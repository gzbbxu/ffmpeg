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
#pragma once
#include "XDataThread.h"
enum XAUDIOTYPE
{
	X_AUDIO_QT
};

class XAudioRecord:public XDataThread
{
public:
	int channels = 2;		//������
	int sampleRate = 44100;	//������
	int sampleByte = 2;		//�����ֽڴ�С
	int nbSamples = 1024;	//һ֡��Ƶÿ��ͨ������������
	//��ʼ¼��
	virtual bool Init() = 0;

	//ֹͣ¼��
	virtual void Stop() = 0;

	static XAudioRecord *Get(XAUDIOTYPE type = X_AUDIO_QT, unsigned char index = 0);
	virtual ~XAudioRecord();
protected:
	XAudioRecord();
};

