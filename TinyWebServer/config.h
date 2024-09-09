#pragma once
#ifndef CONFIG_H
#define CONFIG_H

#include"webserver.h"

using namespace std;

class Config
{
public:
	Config();
	~Config();

	//���������к���
	void parse_arg(int argc,char** argv);

	//�˿ں�
	int PORT;

	//��־д�뷽ʽ
	int LOGWrite;

	//������Ϸ�ʽ
	int TRIGMode;

	//Listenfd��ʽ
	int LISTENTrigmode;

	//connfd����ģʽ
	int CONNTrigmode;

	//���Źر�����
	int OPT_LINGER;

	//���ݿ����ӳ�����
	int sql_num;

	//�̳߳��ڵ��߳�����
	int thread_num;

	//�Ƿ�ر���־
	int close_log;

	//����ģ��ѡ��
	int actor_model;
};

#endif