#ifndef _CONDITION_QUEUE_H_
#define _CONDITION_QUEUE_H_
#include <boost/thread.hpp>
#include <map>
#include <queue>
#include <iostream>
#include <stdio.h>

template<class Type>
class BlockQueue
{
public:
	BlockQueue():m_isDestory(false)
	{

	}
	~BlockQueue()
	{
		destory();
	}
	void destory()
	{
		if(!m_isDestory)
		{
			boost::mutex::scoped_lock _lock(m_mutex);
			if(m_isDestory)return;
			m_isDestory=true;
			m_cond.notify_all();

		}
	}
	bool enqueue(const Type& res)
	{
		if(m_isDestory)return false;
		boost::mutex::scoped_lock _lock(m_mutex);
		m_queue.push_back(res);
		m_cond.notify_one();
		return !m_isDestory;
	}
	bool dequeue(Type& res)
	{
		boost::mutex::scoped_lock _lock(m_mutex);
		while(m_queue.empty()&&!m_isDestory)
		{
			m_cond.wait(m_mutex);
		}
		if(m_isDestory)return false;
		res=m_queue.front();
		m_queue.pop_front();
		return true;
	}
	bool dequeue(Type& res,unsigned int ms_timeout)
	{
		boost::system_time end_time=boost::get_system_time()+boost::posix_time::millisec(ms_timeout);
		if(!m_mutex.timed_lock(end_time))
			return false;
		while(m_queue.empty()&&!m_isDestory)
		{
			if(!m_cond.timed_wait(m_mutex,end_time))
			{
				m_mutex.unlock();
				return false;
			}
		}
		if(m_isDestory)
		{
			m_mutex.unlock();
			return false;
		}
		res=m_queue.front();
		m_queue.pop();
		m_mutex.unlock();
		return true;
	}
private:
	BlockQueue(const BlockQueue<Type>&other);
	BlockQueue<Type>& operator=(const BlockQueue<Type>& other);
	std::deque<Type> m_queue;
	bool m_isDestory;
	boost::mutex  m_mutex;
	boost::condition_variable_any m_cond;
};
#endif