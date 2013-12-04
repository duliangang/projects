#include "apr_base.h"
#include "apr_root_pool.h"

Task_Base::Task_Base(int thread_count):m_info(new threadInfo)
{
	if (thread_count>MAX_THREAD_COUNT)
	{
		thread_count=MAX_THREAD_COUNT;
	}
	m_threadCount=thread_count;
	m_currentThreadCount=0;
	m_info->statu=threadInfo::ThreadStatu::CREATE;
	m_pool=NULL;
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
		if(m_pool)
		{
			apr_pool_destroy(m_pool);
			m_pool=NULL;
		}
		apr_pool_create(&m_pool,sRootPool->getRootPool());
		apr_threadattr_t* pattr_t;
		apr_threadattr_create(&pattr_t,m_pool);
		apr_threadattr_detach_set(pattr_t,0);
		apr_thread_t * tmpT=NULL;
		apr_thread_create(&tmpT,pattr_t,Task_Base::createThread,this,m_pool);
		m_threadList[i]=tmpT;
	}
	m_info->statu=threadInfo::ThreadStatu::START;
	return m_threadCount;
}
void Task_Base::task()
{
	m_mutex.acquire();
	++m_currentThreadCount;
	m_mutex.release();
	svc();
	m_mutex.acquire();
	--m_currentThreadCount;
	if (m_currentThreadCount==0)
	{
		m_info->statu=threadInfo::ThreadStatu::END;
		m_mutex.release();
		close();
		m_info->statu=threadInfo::ThreadStatu::DESTORY;
		return ;
	}
	m_mutex.release();
}
Task_Base::~Task_Base()
{
	m_mutex.acquire();
	join();
	if(m_pool)
	{
		apr_pool_destroy(m_pool);
		m_pool=NULL;
	}
	m_mutex.release();
}
void Task_Base::join()
{

	for(int i=0;i!=m_threadCount;i++)
	{
		if(m_threadList[i]!=NULL)
		{
			apr_status_t retval;
			apr_thread_join(&retval,m_threadList[i]);
		}
	}
}