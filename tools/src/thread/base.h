#ifndef _BASE_H_
#define _BASE_H_
#include "../shared/Define.h"
#include <boost/thread/thread.hpp>
#include <boost/function/function0.hpp>  
#include <boost/shared_ptr.hpp>
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

	int acquire(){}
	int acquire(uint32_t wait_time){}
	int tryacquire (void){}
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
struct threadInfo
{
	enum ThreadStatu
	{
		CREATE=0,
		START=1,
		STOP=2,
		END=3,
		DESTORY=4,
	};
	ThreadStatu statu;
};
class Task_Base
{
	typedef  threadInfo::ThreadStatu Task_Statu;
public:
	struct BLOCK
	{
		void* msg;
		int32_t size;
		boost::shared_ptr<threadInfo> thread_info;
	};
	Task_Base(int thread_count=1);
	int open();
	void task(int thread_index);
	virtual void svr()=0;
	 Task_Statu statu(){return m_info->statu;}
	int putmsgto(void* msg,int32_t size,Task_Base* target);
	int putmsg(const BLOCK& block);
	int32_t getmsg(BLOCK& block);
	virtual void close()=0;
	void join();
	~Task_Base();
protected:
    boost::shared_ptr<threadInfo> m_info;
	std::queue<BLOCK> m_msgBlockList;
private:
	
	int m_threadCount;
	int m_currentThreadCount;
	Thread_Mutex m_mutex;
	std::vector<boost::thread*> m_threadList;
};
#endif
