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
	//初始化私有成员
	block_queue(int max_size = 1000)
	{
		if (max_size <= 0)
		{
			exit(-1);
		}

		//构造函数创建循环数组
		m_max_size = max_size;
		m_array = new T[max_size];
		m_size = 0;
		m_front = -1;
		m_back = -1;

		//创建互斥锁和条件变量
		m_mutex = new pthread_mutex_t;
		m_cond = new pthread_cond_t;
		pthread_mutex_init(m_mutex, NULL);
		pthread_cond_init(m_cond, NULL);
	}

	//析构
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

	//判断队列是否已满
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

	//判断队列是否为空
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

	//返回队首元素
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
	
	//返回队尾元素
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

	//往队列添加元素，需要将所有使用队列的线程先唤醒
	//当有push元素进队列，相当于生产者生产了一个元素
	//若当前线程没有条件变量，则唤醒无意义
	bool push(const T &item)
	{
		pthread_mutex_lock(m_mutex);
		if (m_size >= m_max_size)
		{
			pthread_cond_broadcast(m_cond);
			pthread_mutex_unlock(m_mutex);
			return false;
		}

		//将新增数据放在循环数组的对应位置
		m_back = (m_back + 1) % m_max_size;
		m_array[m_back] = item;
		m_size++;

		pthread_cond_broadcast(m_cond);
		pthread_mutex_unlock(m_mutex);

		return true;
	}

	//pop时，如果当前队列没有元素，则会等待条件变量
	bool pop(T &item)
	{
		pthread_mutex_lock(m_mutex);
		//多个消费者的时候，这里是要用while而不是if
		while (m_size<=0)
		{
			//当重新抢到互斥锁，pthread_cond_wait返回0
			if (0 != pthread_cond_wait(m_cond, m_mutex))
			{
				pthread_mutex_unlock(m_mutex);
				return false;
			}
		}
		//取出队首的元素(模拟循环队列)
		m_front = (m_front + 1) % m_max_size;
		item = m_array[m_front];
		m_size--;
		pthread_mutex_unlock(m_mutex);
		return true;
	}

	//增加超时处理
	//在pthrad_cond_wait基础上增加了等待的时间，只指定时间能抢到互斥锁即可
	//其他逻辑不变
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
	pthread_mutex_t* m_mutex; // 指向互斥量的指针  
	pthread_cond_t* m_cond;   // 指向条件变量的指针

	T* m_array;
	int m_size;
	int m_max_size;
	int m_front;
	int m_back;
};
#endif