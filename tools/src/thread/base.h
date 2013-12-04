#ifndef _BASE_H_
#define _BASE_H_
#include "../shared/Define.h"
#include "semaphore.h"
#include <queue>
#ifdef WIN32
#include <Windows.h>
#include <errno.h>
typedef CRITICAL_SECTION thread_mutex_t;

void init_lock_opt(thread_mutex_t* lock_);

void acquire_lock_opt(thread_mutex_t* lock_);


int tryacquire_lock_opt(thread_mutex_t* lock_);

int acquire_lock_time_opt(thread_mutex_t* lock,uint32_t dwMillsec);
void release_lock_opt(thread_mutex_t* lock_);
void destory_lock_opt(thread_mutex_t* lock_);
#elif defined _LINUX
#include<pthread.h>
#include <time.h>
typedef pthread_mutex_t  thread_mutex_t;
void init_lock_opt(thread_mutex_t* lock_);
int acquire_lock_time_opt(thread_mutex_t* lock,uint32_t dwMillsec)
int tryacquire_lock_opt(thread_mutex_t* lock_);
void acquire_lock_opt(thread_mutex_t* lock_);
void release_lock_opt(thread_mutex_t* lock_);
void destory_lock_opt(thread_mutex_t* lock_);
#endif
class Thread_Mutex
{
public:
	Thread_Mutex();
	~Thread_Mutex();
	
	int acquire();
	int acquire(uint32_t dwMillSec);
	int tryacquire (void);
	void release();

private:
	thread_mutex_t lock_;
};
class Null_Mutex
{
public:
	Null_Mutex(){}
	~Null_Mutex(){}

	int acquire(){return 0;}
	int acquire(uint32_t wait_time){return 0;}
	int tryacquire (void){return 0;}
	void release(){}
};

template<class LOCK_TYPE>
class AutoLock
{
public:
	AutoLock( LOCK_TYPE* t):m_t(t){if(m_t)m_t->acquire();}
	~AutoLock(){if(m_t)m_t->release();}
private:
	LOCK_TYPE* m_t;
};
class Thread_Semaphore
{
public:

	Thread_Semaphore(unsigned int value,unsigned int max,const char* name=NULL,SECURITY_ATTRIBUTES* sa=NULL):isdestory(false)
	{
		sema_init(&t,name,value,max,sa);
	}
	void destory()
	{
		if(!isdestory)
		{
			int i=sema_destroy(&t);
			isdestory=true;
			
		}
	}
	~Thread_Semaphore()
	{
		if(!isdestory)
			sema_destroy(&t);
	}
	
	int wait()
	{
		return sema_wait(&t);
	}

	int trywait()
	{
		return sema_trywait(&t);
	}

	int post()
	{
		return sema_post(&t);
	}

private:
	sema_t t;
	bool isdestory;
};




#endif
