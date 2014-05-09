#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include <boost/thread.hpp>
template<class TYPE>
class Singleton
{
public:

	template<class _TYPE>
	class Garbo
	{
	public:
		~Garbo()
		{
			if(Singleton<_TYPE>::m_t)
			{
				delete Singleton<_TYPE>::m_t;
				Singleton<_TYPE>::m_t=NULL;
			}
		}
	};
	static TYPE* GetInstance()
	{
		if(m_t==NULL)
		{
		
			static Garbo<TYPE> _garbo; 
			mutex_->lock();
			if(m_t==NULL)
			{
				m_t=new TYPE();
			}
			mutex_->unlock();
		}
		return m_t;
	}
	~Singleton()
	{
		delete Singleton<TYPE>::m_t;
		Singleton<TYPE>::m_t=NULL;
	}
	
	
private:
	static TYPE* m_t;
	static boost::mutex* mutex_;
};
template<class TYPE>
TYPE* Singleton<TYPE>::m_t=NULL;

template<class TYPE>
boost::mutex* Singleton<TYPE>::mutex_=new boost::mutex();



#define GetLockFunInstance() _GetInstance().type
#endif