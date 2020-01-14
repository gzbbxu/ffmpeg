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

#include "XFilter.h"
#include "XBilateralFilter.h"
#include <iostream>
using namespace std;
bool XFilter::Set(std::string key, double value)
{
	if (paras.find(key) == paras.end())
	{
		cout << "para " << key << " is not support!" << endl;
		return false;
	}
	paras[key] = value;
	return true;
}
XFilter * XFilter::Get(XFilterType t)
{
	static XBilateralFilter xbf;
	switch (t)
	{
	case XBILATERAL: //˫���˲�
		return &xbf;
		break;
	default:
		break;
	}
	return 0;

}
XFilter::XFilter()
{
}


XFilter::~XFilter()
{
}
