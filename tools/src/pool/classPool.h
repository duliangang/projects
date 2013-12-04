#include "apr_pools.h"
#include "../shared/Define.h"
#include "../thread/apr/apr_root_pool.h"
#include <assert.h>
template<class T>
class Pool
{
public:
	Pool(size_t number=0);

	virtual T* calloc();

	virtual  void free(T *p);

	virtual  void Debug();

	virtual ~Pool();
private:
	T* CanUse;
	void* pStart;
	size_t m_number;
protected:
	apr_pool_t* this_pool;

};
template<class T>
class Pool_Mutex:public Pool<T>
{
public:
	Pool_Mutex(size_t number=0):Pool<T>(number),m_mutex()
	{

	}

	virtual T* calloc()
	{
		m_mutex.acquire();
		T* p=Pool<T>::calloc();
		m_mutex.release();
		return p;
	}

	virtual  void free(T *p)
	{
		m_mutex.acquire();
		Pool<T>::free(p);
		m_mutex.release();
	}

	virtual  void Debug()
	{
		m_mutex.acquire();
		Pool<T>::Debug();
		m_mutex.release();
	}

	virtual ~Pool_Mutex()
	{

	}
private:
	Thread_Mutex m_mutex;

};







template<class T>  
Pool<T>::Pool(size_t number=0)
{
	number  = (number==0) ? 256 : number;
	m_number=number;
	size_t i;
	apr_pool_create(&this_pool,sRootPool->getRootPool());

	CanUse=(T*)(apr_palloc(this_pool,number*sizeof(T)));

	for (i=0; i<number-1; ++i)
	{
		*(T**)(CanUse + i) = (CanUse + i + 1);
	}
	*(T**)(CanUse+i) = 0;
	pStart=CanUse;
}



template<class T>
T* Pool<T>::calloc()
{
	T* p = CanUse;
	if(CanUse != 0)
	{
		if(*(T**)CanUse!=NULL)
			CanUse = *(T**)CanUse;
		else
		{
			T* temp_CanUse;
			temp_CanUse=(T*)(apr_palloc(this_pool,m_number*sizeof(T)*2));
			//printf("%p\n",temp_CanUse);
			size_t i=0;
			for ( i=0; i<m_number*2-1; ++i)
			{
				*(T**)(temp_CanUse + i) = (temp_CanUse + i + 1);
				//printf("%p\n",temp_CanUse+i);
			}
			*(T**)(temp_CanUse+i) = 0;

			*(T**)CanUse=temp_CanUse;
			CanUse=*(T**)CanUse;
		}

	}
	else 
	{
		return NULL;
	}
	return p;
}
template<class T>
void  Pool<T>::free(T *p)
{
	if(CanUse==p)
	{
		assert(false);
		return ;
	}
	*(T**)p = CanUse;
	CanUse  =p;
}
template<class T>
void Pool<T>::Debug()
{
	int i=0;
	T* temp=CanUse;
	while(temp!=NULL)
	{
		temp=*(T**)temp;
		i++;
		if(i%5!=0)
		{
			printf("%p\t",temp);
		}
		else
		{
			printf("%p\n",temp);
		}
	}
	printf("pool have can user element %d count\n",i);

}
template<class T>
Pool<T>::~Pool()
{
	apr_pool_destroy(this_pool);
}

