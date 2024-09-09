#pragma once

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H
#include<iostream>
#include<stdlib.h>
#include<pthread.h>
#include<sys/time.h>
#include"../lock/locker.h"

using namespace std;

template<class T>
class block_queue
{
public:
	//��ʼ��˽�г�Ա
	block_queue(int max_size = 1000)
	{
		if (max_size <= 0)
		{
			exit(-1);
		}

		//���캯������ѭ������
		m_max_size = max_size;
		m_array = new T[max_size];
		m_size = 0;
		m_front = -1;
		m_back = -1;

		//��������������������
		m_mutex = new pthread_mutex_t;
		m_cond = new pthread_cond_t;
		pthread_mutex_init(m_mutex, NULL);
		pthread_cond_init(m_cond, NULL);
	}

	//����
	~block_queue()
	{
		pthread_mutex_lock(m_mutex);
		if (m_array != NULL)
		{
			delete[] m_array;
		}

		pthread_mutex_unlock(m_mutex);
	}

	void clear()
	{
		pthread_mutex_lock(m_mutex);
		m_size = 0;
		m_front = -1;
		m_back = -1;
		pthread_mutex_unlock(m_mutex);
	}

	//�ж϶����Ƿ�����
	bool full()
	{
		pthread_mutex_lock(m_mutex);
		if (m_size >= m_max_size)
		{
			pthread_mutex_unlock(m_mutex);
			return true;
		}
		pthread_mutex_unlock(m_mutex);
		return false;
	}

	//�ж϶����Ƿ�Ϊ��
	bool empty()
	{
		pthread_mutex_lock(m_mutex);
		if (0 == m_size)
		{
			pthread_mutex_unlock(m_mutex);
			return true;
		}
		pthread_mutex_unlock(m_mutex);
		return false;
	}

	//���ض���Ԫ��
	bool front(T& value)
	{
		pthread_mutex_lock(m_mutex);
		if (0 == m_size)
		{
			pthread_mutex_unlock(m_mutex);
			return false;
		}
		value = m_array[m_front];
		pthread_mutex_unlock(m_mutex);
		return true;
	}
	
	//���ض�βԪ��
	bool back(T& value)
	{
		pthread_mutex_lock(m_mutex);
		if (0 == m_size)
		{
			pthread_mutex_unlock(m_mutex);
			return false;
		}
		value = m_array[m_back];
		pthread_mutex_unlock(m_mutex);
		return true;
	}

	int size()
	{
		int tmp = 0;

		pthread_mutex_lock(m_mutex);
		tmp = m_size;

		pthread_mutex_unlock(m_mutex);
		return tmp;
	}

	int max_size()
	{
		int tmp = 0;

		pthread_mutex_lock(m_mutex);
		tmp = m_max_size;

		pthread_mutex_unlock(m_mutex);
		return tmp;
	}

	//���������Ԫ�أ���Ҫ������ʹ�ö��е��߳��Ȼ���
	//����pushԪ�ؽ����У��൱��������������һ��Ԫ��
	//����ǰ�߳�û����������������������
	bool push(const T &item)
	{
		pthread_mutex_lock(m_mutex);
		if (m_size >= m_max_size)
		{
			pthread_cond_broadcast(m_cond);
			pthread_mutex_unlock(m_mutex);
			return false;
		}

		//���������ݷ���ѭ������Ķ�Ӧλ��
		m_back = (m_back + 1) % m_max_size;
		m_array[m_back] = item;
		m_size++;

		pthread_cond_broadcast(m_cond);
		pthread_mutex_unlock(m_mutex);

		return true;
	}

	//popʱ�������ǰ����û��Ԫ�أ����ȴ���������
	bool pop(T &item)
	{
		pthread_mutex_lock(m_mutex);
		//��������ߵ�ʱ��������Ҫ��while������if
		while (m_size<=0)
		{
			//������������������pthread_cond_wait����0
			if (0 != pthread_cond_wait(m_cond, m_mutex))
			{
				pthread_mutex_unlock(m_mutex);
				return false;
			}
		}
		//ȡ�����׵�Ԫ��(ģ��ѭ������)
		m_front = (m_front + 1) % m_max_size;
		item = m_array[m_front];
		m_size--;
		pthread_mutex_unlock(m_mutex);
		return true;
	}

	//���ӳ�ʱ����
	//��pthrad_cond_wait�����������˵ȴ���ʱ�䣬ָֻ��ʱ������������������
	//�����߼�����
	bool pop(T &item, int ms_timeout)
	{
		struct timespec t = { 0,0 };
		struct timeval now = { 0,0 };
		gettimeofday(&now, NULL);
		pthread_mutex_lock(m_mutex);
		if (m_size <= 0)
		{
			t.tv_sec = now.tv_sec + ms_timeout / 1000;
			t.tv_nsec = (ms_timeout % 1000) * 1000;
			if (0 != pthread_cond_timedwait(m_cond, m_mutex, &t))
			{
				pthread_mutex_unlock(m_mutex);
				return false;
			}
		}

		if (m_size <= 0)
		{
			pthread_mutex_unlock(m_mutex);
			return false;
		}	

		m_front = (m_front + 1) % m_max_size;
		item = m_array[m_front];
		m_size--;
		pthread_mutex_unlock(m_mutex);
		return true;
	}
private:
	pthread_mutex_t* m_mutex; // ָ�򻥳�����ָ��  
	pthread_cond_t* m_cond;   // ָ������������ָ��

	T* m_array;
	int m_size;
	int m_max_size;
	int m_front;
	int m_back;
};
#endif