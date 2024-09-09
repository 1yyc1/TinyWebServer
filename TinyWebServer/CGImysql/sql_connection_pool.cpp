#define _CRT_SECURE_NO_WARNINGS 1
#include<mysql/mysql.h>
#include<stdio.h>
#include<string>
#include<stdlib.h>
#include<list>
#include<pthread.h>
#include<iostream>
#include"sql_connection_pool.h"

using namespace std;
connection_pool::connection_pool()
{
	this->m_CurConn = 0;
	this->m_FreeConn = 0;
}

//RAII�����������Ӵ�
connection_pool::~connection_pool()
{
	DestroyPool();
}

connection_pool* connection_pool::GetInstance()
{
	static connection_pool connPool;
	return &connPool;
}


//�����ʼ��
void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
	//��ʼ�����ݿ���Ϣ
	this->m_url = url;
	this->m_Port = Port;
	this->m_User = User;
	this->m_PassWord = PassWord;
	this->m_DatabaseName=DBName;
	this->m_close_log = close_log;

	//����MaxConn�����ݿ�����
	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL* con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			cout << "Error:" << mysql_error(con);
			exit(1);
		}
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			cout << "Error:" << mysql_error(con);
			exit(1);
		}

		//�������ӴʺͿ�����������
		connList.push_back(con);
		++m_FreeConn;
	}

	//���ź�����ʼ��Ϊ������Ӵ���
	reserve = sem(m_FreeConn);

	this->m_MaxConn = m_FreeConn;
}

//��������ʱ�������ݿ����ӳ��з���һ���������ӣ�����ʹ�úͿ���������
MYSQL* connection_pool::GetConnection()
{
	MYSQL* con = NULL;

	if (0 == connList.size())
	{
		return NULL;
	}

	//ȡ�����ӣ��ź���ԭ�Ӽ�1��Ϊ0��ȴ�
	reserve.wait();

	lock.lock();

	con = connList.front();
	connList.pop_front();

	--m_FreeConn;
	++m_CurConn;

	lock.unlock();
	return con;
}

//�ͷŵ�ǰʹ�õ�����
bool connection_pool::ReleaseConnection(MYSQL* con)
{
	if (NULL == con)
	{
		return false;
	}

	lock.lock();

	connList.push_back(con);
	++m_FreeConn;
	--m_CurConn;

	lock.unlock();

	//�ͷ�����ԭ�Ӽ�1
	reserve.post();
	return true;
}

//�������ݿ����ӳ�
void connection_pool::DestroyPool()
{
	lock.lock();
	if (connList.size() > 0)
	{
		//ͨ���������������ر����ݿ�����
		list<MYSQL*>::iterator it;
		for (it = connList.begin(); it != connList.end(); it++)
		{
			MYSQL* con = *it;
			mysql_close(con);
		}
		m_CurConn = 0;
		m_FreeConn = 0;

		//���list
		connList.clear();

		lock.unlock();
	}

	lock.unlock();
}

int connection_pool::GetFreeConn()
{
	return this->m_FreeConn;
}

connectionRAII::connectionRAII(MYSQL** SQL, connection_pool* connPool)
{
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
	poolRAII->ReleaseConnection(conRAII);
}
