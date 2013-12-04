#ifndef _APR_BASE_H
#define _APR_BASE_H
#include "apr_pools.h"
#include "../base.h"
#include "apr_thread_proc.h"
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
#define MAX_THREAD_COUNT 5
class Task_Base
{
	typedef  threadInfo::ThreadStatu Task_Statu;
public:
	Task_Base(int thread_count=1);
	int open();
	void task();
	virtual void svc()=0;
	Task_Statu statu(){return m_info->statu;}
	virtual void close(){};
	void join();
	virtual ~Task_Base();
private:
	static  void*  APR_THREAD_FUNC createThread(apr_thread_t *pthread, void* lpParam)
	{
		Task_Base* task_ba=(Task_Base*)lpParam;
		task_ba->task();
		return pthread;
	}
	threadInfo* m_info;
	Thread_Mutex m_mutex;
	int m_currentThreadCount;
	int m_threadCount;
	apr_thread_t* m_threadList[MAX_THREAD_COUNT];
	//std::vector<apr_thread_t*>* m_threadList;
	apr_pool_t* m_pool;
};
#endif