#include "../shared/Define.h"

class Base_Task
{
public:
	virtual void svr()=0;
	virtual int put(void *);
	virtual void start();
	virtual void wait();
	virtual void suspend();
	virtual void resume();
	static int svr_run(void*){}
};
#ifdef WIN32
#include <Windows.h>
typedef CRITICAL_SECTION ACE_thread_mutex_t;
#else
typedef int  thread_mutex_t;
#endif
class Thread_Mutex
{
public:
	Thread_Mutex();
	~Thread_Mutex();

	void acquire(uint32_t wait_time=INFINITE);
	int tryacquire (void);
	void release();

private:
	thread_mutex_t lock_;
};
class Null_Mutex
{
public:
	Null_Mutex();
	~Null_Mutex();

	void acquire(uint32_t wait_time=INFINITE);
	int tryacquire (void);
	void release();
};

template<class LOCK_TYPE>
class AutoLock
{
public:
	AutoLock( LOCKTYPE* t):m_t(t){m_t->acquire();}
	~AutoLock(){m_t->release();}
private:
	LOCKTYPE* m_t;
};

