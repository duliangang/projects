#include "apr_pools.h"
#include "classPool.h"
#include <assert.h>
class BufferPool
{
private:
	BufferPool();
	~BufferPool();
protected:
	friend class BufferPoolManage;
	int  Create(apr_uint32_t init_size,apr_pool_t* rootpool);
	void Free();
public:
	apr_int32_t append(char* p,apr_uint32_t size);
	apr_int32_t read(char* p,apr_uint32_t size);
	int32_t read_skip(apr_uint32_t len);
	void clear();
	apr_uint32_t length();
	apr_uint32_t size();
	apr_uint32_t space();
protected:
	apr_pool_t* m_root;
	apr_pool_t* m_pool;
	char* m_veclist;
	apr_uint32_t m_size;
	apr_uint32_t m_rpos;
	apr_uint32_t m_wpos;
};

class BufferPoolManage:public Pool_Mutex<BufferPool>
{
public:
	BufferPool* calloc(apr_uint32_t size)
	{
		BufferPool* buf=Pool_Mutex<BufferPool>::calloc();
		assert(buf!=NULL);
		buf->Create(size,this_pool);
		return buf;
	}
	virtual void Free(BufferPool* pBufferPool)
	{
		pBufferPool->Free();
		Pool_Mutex<BufferPool>::free(pBufferPool);
	}
};
#define  sBufferManage Singleton<BufferPoolManage,Thread_Mutex>::GetInstance()