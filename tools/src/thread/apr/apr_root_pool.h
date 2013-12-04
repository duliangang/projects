#ifndef _APR_ROOT_POOL_H_
#define _APR_ROOT_POOL_H_

#include "../singleton.h"
#include "apr_pools.h"
#include "../base.h"
class RootPool
{
public:
	friend class Singleton<RootPool,Thread_Mutex>;
	apr_pool_t* getRootPool(){return m_rootpool;}
private:
	RootPool()
	{
		apr_pool_initialize();
		apr_pool_create(&m_rootpool,NULL);
		apr_allocator_create(&m_rootalloc);
		apr_thread_mutex_create(&m_mutex,0,m_rootpool);


		apr_allocator_max_free_set(m_rootalloc,0);

		apr_allocator_mutex_set(m_rootalloc,m_mutex);

		apr_allocator_owner_set(m_rootalloc,m_rootpool);

	}
	~RootPool()
	{
		//apr_thread_mutex_destroy(m_mutex);
		//apr_allocator_destroy(m_rootalloc);
		//apr_pool_destroy(m_rootpool);
		//apr_pool_terminate();
	}
private:
	apr_pool_t* m_rootpool;
	apr_allocator_t* m_rootalloc;
	apr_thread_mutex_t* m_mutex;
};
#define sRootPool Singleton<RootPool,Thread_Mutex>::GetInstance()
#endif