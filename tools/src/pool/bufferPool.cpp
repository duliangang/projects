#include "BufferPool.h"
#define NOT_CREATENEW_MIN_SPACE 0.3
int BufferPool::Create(apr_uint32_t init_size,apr_pool_t* rootpool)
{
	if(rootpool==NULL)return 0;
	m_root=rootpool;
	m_size=init_size;
	m_rpos=0;
	m_wpos=0;
	m_pool=NULL;
	m_veclist=NULL;
	apr_pool_create(&m_pool,rootpool);
	m_veclist=(char*)apr_palloc(m_pool,init_size);
	return 0;
}
void BufferPool::Free()
{
	apr_pool_destroy(m_pool);
	m_veclist=NULL;
	m_pool=NULL;
	m_size=0;
}
apr_int32_t  BufferPool::append(char* p,apr_uint32_t size)
{
	if(m_size-m_wpos<size&&((m_size-m_wpos+m_rpos)-size)>((double)(double)m_size*NOT_CREATENEW_MIN_SPACE))
	{
		memmove(m_veclist,m_veclist+m_rpos,m_wpos-m_rpos);
		m_wpos-=m_rpos;
		m_rpos=0;
	}
	else if(m_size-m_wpos<size)
	{
		apr_pool_t* pnew;
		apr_pool_create(&pnew,m_root);
		char* newMem=(char*)apr_palloc(pnew,m_size*2);
		memcpy(newMem,m_veclist+m_rpos,m_wpos-m_rpos);
		m_wpos-=m_rpos;
		m_rpos=0;
		m_size=m_size*2;
		apr_pool_destroy(m_pool);
		m_pool=pnew;
		m_veclist=newMem;
	}
	memcpy(m_veclist+m_wpos,p,size);
	m_wpos+=size;
	return size;
}
apr_int32_t BufferPool::read(char* p,apr_uint32_t size)
{
	if(p==NULL)return -1;
	if(m_wpos-m_rpos<size)
	{
		size=m_wpos-m_rpos;
	}
	memcpy(p,m_veclist+m_rpos,size);
	m_rpos+=size;
	return size;
}
int32_t BufferPool::read_skip(apr_uint32_t size)
{
	if(m_wpos-m_rpos<size)
	{
		size=m_wpos-m_rpos;
	}
	m_rpos+=size;
	return size;
}
void BufferPool::clear()
{
	m_wpos=m_rpos=0;
}
apr_uint32_t BufferPool::length()
{
	return m_wpos-m_rpos;
}
apr_uint32_t BufferPool::size()
{
	return m_size;
}
apr_uint32_t BufferPool::space()
{
	return m_size-m_wpos;
}
