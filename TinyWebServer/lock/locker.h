#pragma once
#ifndef LOCKER_H
#define LOCKER_H

#include<semaphore.h>//与信号量相关
#include<exception>
#include<pthread.h>//多线程相关

//信号量类
class sem
{
public:
	//构造
	sem()
	{
		//信号量初始化
		// 第一个参数是sem_t结构体的地址
		// 第二个参数0代表在线程上共享内存(非零则代表在进程上共享内存)
		// 第三个参数代表初始值为零
		if (sem_init(&m_sem, 0, 0) != 0)
		{
			throw std::exception();
		}
	}
	sem(int num)
	{
		if (sem_init(&m_sem, 0, num) != 0)
		{
			throw std::exception();
		}
	}
	//析构
	~sem()
	{
		//信号量销毁
		sem_destroy(&m_sem);
	}

	bool wait()
	{
		return sem_wait(&m_sem) == 0;
	}

	bool post()
	{
		return sem_post(&m_sem) == 0;
	}
private:
	//信号量变量
	sem_t m_sem;
};

class locker
{
public:
	locker()
	{
		if (pthread_mutex_init(&m_mutex, NULL)!=0)
		{
			throw std::exception();
		}
	}
	~locker()
	{
		pthread_mutex_destroy(&m_mutex) == 0;
	}
	bool lock()
	{
		return pthread_mutex_lock(&m_mutex)==0;
	}
	bool unlock()
	{
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
	pthread_mutex_t* get()
	{
		return &m_mutex;
	}
private:
	pthread_mutex_t m_mutex;
};

class cond
{
public:
	//构造
	cond()
	{
		if (pthread_cond_init(&m_cond, NULL) != 0)
		{
			throw std::exception();
		}
	}
	//析构
	~cond()
	{
		pthread_cond_destroy(&m_cond);
	}
	
	bool wait(pthread_mutex_t* m_mutex)
	{
		int ret = 0;
		pthread_mutex_lock(m_mutex);//互斥量加锁
		ret=pthread_cond_wait(&m_cond, m_mutex);//等待条件变量，自动解锁互斥量
		pthread_mutex_unlock(m_mutex);//互斥量解锁
		return ret == 0;//调用成功
	}
	bool timewait(pthread_mutex_t* m_mutex, struct timespec t)
	{
		pthread_mutex_lock(m_mutex);
		int ret = 0;
		ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
		pthread_mutex_lock(m_mutex);
		return  ret == 0;
	}
	bool signal()
	{
		return pthread_cond_signal(&m_cond)==0;//返回条件变量，唤醒线程
	}
	bool broadcase()
	{
		return pthread_cond_broadcast(&m_cond) == 0;
	}
private:
	//pthread_mutex_t m_mutex=PTHREAD_MUTEX_INITIALIZER;//定义互斥量，并初始化
	pthread_cond_t m_cond=PTHREAD_COND_INITIALIZER; //定义条件变量，并初始化
};
#endif