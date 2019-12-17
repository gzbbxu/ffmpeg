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
#include <QThread>
#include "XData.h"
#include <list>
class XDataThread:public QThread
{
public:
	//�������б��С���б����ֵ������ɾ����ɵ�����
	int maxList = 100;

	//���б��β����
	virtual void Push(XData d);

	//��ȡ�б������������,����������Ҫ����XData.Drop����
	virtual XData Pop();

	//�����߳�
	virtual bool Start();

	//�˳��̣߳����ȴ��߳��˳���������
	virtual void Stop();

	virtual void Clear();
	XDataThread();
	virtual ~XDataThread();
protected:
	//��Ž������� ������� �Ƚ��ȳ�
	std::list<XData> datas;

	//���������б��С
	int dataCount = 0;
	//������� datas;
	QMutex mutex;

	//�����߳��˳�
	bool isExit = false;
};

