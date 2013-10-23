#include "base.h"
#ifdef WIN32


void init_lock_opt(thread_mutex_t* lock_)
{
	InitializeCriticalSection(lock_);
}
void acquire_lock_opt(thread_mutex_t* lock_)
{
	EnterCriticalSection(lock_);
}

int tryacquire_lock_opt(thread_mutex_t* lock_)
{
	if(WAIT_TIMEOUT==WaitForSingleObject(lock_,0))
	{
		errno=ETIME;
		return -1;
	}
	return 0;
}
int acquire_lock_time_opt(thread_mutex_t* lock,uint32_t dwMillsec)
{
	if(WAIT_TIMEOUT==WaitForSingleObject(lock,dwMillsec))
	{
		errno=ETIME;
		return -1;
	}
	return 0;
}
void release_lock_opt(thread_mutex_t* lock_)
{
	LeaveCriticalSection(lock_);
}
void destory_lock_opt(thread_mutex_t* lock_)
{
	DeleteCriticalSection(lock_);
}
#elif defined _LINUX

void init_lock_opt(thread_mutex_t* lock_)
{
	pthread_mutex_init(lock_,0);
}
int acquire_lock_time_opt(thread_mutex_t* lock,uint32_t dwMillsec)
{
	timespec _t;
	_t.tv_sec=dwMillsec/1000;
	_t.tv_nsec=(dwMillsec%1000)*1000;
	int err=pthread_mutex_timedlock(lock,&_t)
		if(err==0)
			return 0;
	errno=err;
	return -1;
}
int tryacquire_lock_opt(thread_mutex_t* lock_)
{
	timespec _t;
	_t.tv_sec=0;
	_t.tv_nsec=0;
	int err=pthread_mutex_timedlock(lock,&_t)
		if(err==0)
			return 0;
	errno=err;
	return -1;
}
void acquire_lock_opt(thread_mutex_t* lock_)
{
	pthread_mutex_lock(lock_);
}
void release_lock_opt(thread_mutex_t* lock_)
{
	pthread_mutex_unlock(lock_);
}
void destory_lock_opt(thread_mutex_t* lock_)
{
	pthread_mutex_destroy(lock_);
}
#endif
Thread_Mutex::Thread_Mutex()
{
	init_lock_opt(&lock_);
}
Thread_Mutex::~Thread_Mutex()
{
	destory_lock_opt(&lock_);
}
int Thread_Mutex::acquire()
{
	acquire_lock_opt(&lock_);
	return 0;
}
int Thread_Mutex::acquire(uint32_t wait_time)
{
	return acquire_lock_time_opt(&lock_,wait_time);
}
int Thread_Mutex::tryacquire()
{
	return tryacquire_lock_opt(&lock_);
}
void Thread_Mutex::release()
{
	release_lock_opt(&lock_);
}
Task_Base::Task_Base(int thread_count):m_info(new threadInfo)
{
	m_threadCount=thread_count;
	m_currentThreadCount=0;
	m_info->statu=threadInfo::ThreadStatu::CREATE;
}
int Task_Base::open()
{
	if(m_info->statu!=threadInfo::ThreadStatu::CREATE)
	{
		return -1;
	}
	AutoLock<Thread_Mutex> autolock_(&(m_mutex));
	if(m_info->statu!=threadInfo::ThreadStatu::CREATE)
	{
		return -1;
	}
	for (int i=0;i!=m_threadCount;i++)
	{
		boost::thread *_thread=new boost::thread(boost::bind(&Task_Base::task,this,i));
		m_threadList.push_back(_thread);
	}
	m_info->statu=threadInfo::ThreadStatu::START;
	return m_threadList.size();
}
void Task_Base::task(int index)
{
	m_mutex.acquire();
	++m_currentThreadCount;
	m_mutex.release();
	svr();
	m_mutex.acquire();
	--m_currentThreadCount;
	if (m_currentThreadCount==0)
	{
		m_info->statu=threadInfo::ThreadStatu::END;
		m_mutex.release();
		close();
		m_info->statu=threadInfo::ThreadStatu::DESTORY;
		while(!m_msgBlockList.empty())
		{
			BLOCK block=m_msgBlockList.front();
			m_msgBlockList.pop();
			delete block.msg;
		}
		return ;
	}
	m_mutex.release();
}
int Task_Base::putmsgto(void* msg,int32_t size,Task_Base* target)
{
	BLOCK block;
	block.msg=msg;
	block.size=size;
	block.thread_info=m_info;
	return target->putmsg(block);
}
int Task_Base::putmsg(const BLOCK& block)
{
	m_mutex.acquire();
	BLOCK newBlock;
	newBlock.msg =new char[block.size];
	memcpy(newBlock.msg,block.msg,block.size);
	newBlock.size=block.size;
	newBlock.thread_info=block.thread_info;
	m_msgBlockList.push(newBlock);
	m_mutex.release();
	return 0;
}
int32_t Task_Base::getmsg(BLOCK& block)
{
   if(m_msgBlockList.empty())return -1;
   AutoLock<Thread_Mutex> autoLock(&m_mutex);
   if(m_msgBlockList.empty())return -1;
   block=m_msgBlockList.front();
   m_msgBlockList.pop();
   return 0;
}
Task_Base::~Task_Base()
{
	for (int i=0;i!=m_threadList.size();i++)
	{
		if(m_threadList[i])delete m_threadList[i];
		m_threadList[i]=NULL;
	}
}
void Task_Base::join()
{
	for(int i=0;i!=m_threadList.size();i++)
	{
		if(m_threadList[i]!=NULL)
		m_threadList[i]->join();
	}
}