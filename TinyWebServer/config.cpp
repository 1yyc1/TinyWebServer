#define _CRT_SECURE_NO_WARNINGS 1
#include"config.h"

Config::Config()
{
	//�˿ںţ�Ĭ��9006
	PORT = 9006;

	//��־д�뷽ʽ��Ĭ��ͬ��
	LOGWrite = 0;

	//�������ģʽ��Ĭ��listenfd LT  +  connfd LT
	TRIGMode = 0;

	//listen����ģʽ��Ĭ��LT
	LISTENTrigmode = 0;

	//connfd����ģʽ,Ĭ��LT
	CONNTrigmode = 0;

	//���Źر����ӣ�Ĭ�ϲ�ʹ��
	OPT_LINGER = 0;

	//�������ӳ�������Ĭ��8
	sql_num = 8;
	
	//�̳߳��ڵ����ӳ�������Ĭ��8
	thread_num = 8;

	//�ر���־,Ĭ�ϲ��ر�
	close_log = 0;

	//����ģ��,Ĭ����proactor
	actor_model = 0;
}


Config::~Config()
{
}
void Config::parse_arg(int argc, char** argv)
{
	int opt;
	const char* str = "p:l:m:o:s:t:c:a:";
	while ((opt = getopt(argc, argv, str)))
	{
		switch (opt)
		{
		case 'p':
		{
			PORT = atoi(optarg);
			break;
		}
		case 'l':
		{
			LOGWrite = atoi(optarg);
			break;
		}
		case 'm':
		{
			TRIGMode = atoi(optarg);
			break;
		}
		case 'o':
		{
			OPT_LINGER = atoi(optarg);
			break;
		}
		case 's':
		{
			sql_num = atoi(optarg);
			break;
		}
		case 't':
		{
			thread_num = atoi(optarg);
			break;
		}
		case 'c':
		{
			close_log = atoi(optarg);
			break;
		}
		case 'a':
		{
			actor_model = atoi(optarg);
			break;
		}
		default:
			break;
		}
	}
}