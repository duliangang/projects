#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include "base.h"

template<class TYPE,class MUTEX_TYPE>
class Singleton
{
public:
	static TYPE* GetInstance()
	{
		if(m_t==NULL)
		{
			MUTEX_TYPE mutex_;
			AutoLock<MUTEX_TYPE> autolock(&mutex_);
			if(m_t==NULL)
			{
				m_t=new TYPE();
			}
		}
		return m_t;
	}
private:
	static TYPE* m_t;
};
template<class TYPE,class MUTEX_TYPE>
TYPE* Singleton<TYPE,MUTEX_TYPE>::m_t=NULL;





template<class Type,class MutexType>
struct LockResult
{
	LockResult(Type* pType,MutexType* pmutex):lock(pmutex),type(pType)
	{
	}
	Type* type;
	AutoLock<MutexType> lock;
	~LockResult()
	{
		type=NULL;
	}
};

template<class TYPE,class MUTEX_TYPE>
class LOCKFUN_Singleton
{
public:
	static TYPE* _GetInstance()
	{
		
		if(m_t==NULL||mMutex==NULL)
		{
			static MUTEX_TYPE mutex_;
			AutoLock autolock(&mutex_);
				if(m_t==NULL)
				{
					m_t=new TYPE();
				}
				if(mMutex==NULL)
				{
					mMutex=new MUTEX_TYPE();
				}
		}
		
		return LockResult<TYPE,MUTEX_TYPE>(m_t,mMutex);
	}
private:
	static TYPE* m_t;
	static MUTEX_TYPE *mMutex;
};
template<class TYPE,class MUTEX_TYPE>
TYPE* LOCKFUN_Singleton<TYPE,MUTEX_TYPE>::m_t=NULL;

template<class TYPE,class MUTEX_TYPE>
TYPE* LOCKFUN_Singleton<TYPE,MUTEX_TYPE>::mMutex=NULL;

#define GetLockFunInstance() _GetInstance().type
#endif