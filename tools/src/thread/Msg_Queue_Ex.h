#ifndef _MSG_QUEUE_EX_H
#define  _MSG_QUEUE_EX_H
#include "base.h"
#include "../shared/Define.h"
#include <queue>
#include <deque>
template<class T,class MUTEX>
class Msg_Queue
{
public:
	Msg_Queue(uint32_t maxsize=1024*1024,MUTEX* _mutex=NULL):m_sama(0,maxsize),isdestory(false)
	{
		if(_mutex)m_mutex_read=_mutex;
		else  m_mutex_read=new MUTEX;
		m_mutex_write=new Thread_Mutex();
	}
	~Msg_Queue()
	{
		  AutoLock<Thread_Mutex> lock(m_mutex_write);
			if(!isdestory)
			{
				isdestory=true;
				m_sama.post();
				m_sama.destory();
			}
			if(m_mutex_read)delete m_mutex_read;
			delete m_mutex_write;
			while(!m_Queue.empty())
			{
				delete m_Queue.front();
				m_Queue.pop();
			}
	}
	void destory()
	{
	   if(isdestory)return ;
	  AutoLock<Thread_Mutex> lock(m_mutex_write);
	  if(isdestory)return ;
	  isdestory=true;
	  m_sama.post();
	}
	void enqueue(T* t)
	{
		AutoLock<Thread_Mutex> lock(m_mutex_write);
		m_Queue.push(t);
		if(isdestory)return ;
		m_sama.post();
	}
	uint32_t size()
	{
		return m_Queue.size();
	}
	int dequeue(T*&ret)
	{
		AutoLock<MUTEX> _auto(m_mutex_read);
		int err=m_sama.wait();
		{

			if(0!=err)
				return err; 
		}
		AutoLock<Thread_Mutex> lock(m_mutex_write);
		if(m_Queue.empty())
			{
				m_sama.destory();
				return -1;
			}
		ret=m_Queue.front();
		m_Queue.pop();
		return 0;
	}
private:
	MUTEX* m_mutex_read;
	Thread_Mutex* m_mutex_write;
	Thread_Semaphore m_sama;
	std::queue<T*> m_Queue;
	bool isdestory;
};




// a pure virtual function
template <class T> class Msg_Queue_Ex
{
public:
	Msg_Queue_Ex(void):m_pThread(0){m_bstop=false;Start()}
	virtual ~Msg_Queue_Ex() {}

public:
	// ����֪ͨ�ӿڣ�һ�㲻����ã�
	void NotifyOne() {m_event.notify_one();}

	// ȫ��֪ͨ�ӿڣ�һ�㲻����ã�
	void NotifyAll() {m_event.notify_all();}

	// ����������
	
	void Put(T& t)
	{
		AutoLock<Thread_Mutex> autoLock(&m_mutex);
		m_record_set.push(t);

		// ����֪ͨ�ź�
		m_event.post();
	}

	// ��ȡ������buffer size�Ľӿ�
	int BufferSize() { return m_record_set.size(); }
	/*void stop()
	{
		m_bstop=true;
	}
	void join()
	{
		m_pThread->join();
	}*/
protected:
	// �������ݵ��̣߳���������ʱ��
	virtual void DataThread()
	{
		while(true)
		{
			if(m_event.wait()==-1)
				break;
			while(!m_record_set.empty())
			{
			
				m_mutex.acquire();
				T& t=m_record_set.front();
				m_record_set.pop();
				m_mutex.release();
				DataFunc(t);
				
				
			}
		}
	}


	// �������ݵĺ�������������ʱ��
	virtual void DataFunc(T& t)=0;

	// ����Ϊ�ڲ�����
protected:
	// ��ʼ����
	int Start()
	{
		if(m_pThread==NULL)
		   m_pThread=new boost::thread(&Msg_Queue_Ex::DataThread, this);
		else
			return -1;
		return 0;
	}

protected:
	// �����ݼ�
	Thread_Mutex m_mutex;
	std::queue<T> m_record_set;
	boost::thread *m_pThread;
	// �ź�
	Thread_Semaphore m_event;
};

//// a pure virtual function
//template <class T> class Msg_Queue_Ex<T*>
//{
//public:
//	Msg_Queue_Ex(void):m_pThread(0){m_bstop=false;Start()}
//	virtual ~Msg_Queue_Ex() {
//			while(!m_record_set.empty())
//	}
//
//public:
//	// ����֪ͨ�ӿڣ�һ�㲻����ã�
//	void NotifyOne() {m_event.notify_one();}
//
//	// ȫ��֪ͨ�ӿڣ�һ�㲻����ã�
//	void NotifyAll() {m_event.notify_all();}
//
//	// ����������
//	
//	void Put(T* t)
//	{
//		AutoLock<Thread_Mutex> autoLock(&m_mutex);
//			m_record_set.push(t);
//
//		// ����֪ͨ�ź�
//		m_event.post();
//	}
//
//	// ��ȡ������buffer size�Ľӿ�
//	int BufferSize() { return m_record_set.size(); }
//	void stop()
//	{
//		m_bstop=true;
//	}
//	void join()
//	{
//		m_pThread->join();
//	}
//protected:
//	// �������ݵ��̣߳���������ʱ��
//	virtual void DataThread()
//	{
//		while(!m_bstop)
//		{
//			if(m_event.wait()==-1)
//				break;
//			while(!m_record_set.empty())
//			{
//				T* t;
//				m_mutex.acquire();
//				   t=m_record_set.front();
//				   m_record_set.pop();
//					m_mutex.release();
//					DataFunc(t);
//				m_mutex.release();
//			}
//		}
//	}
//
//
//	// �������ݵĺ�������������ʱ��
//	virtual void DataFunc(T* t)=0;
//
//	// ����Ϊ�ڲ�����
//protected:
//	// ��ʼ����
//	int Start()
//	{
//		if(m_pThread==NULL)
//			m_pThread=new boost::thread(&Msg_Queue_Ex::DataThread, this);
//		else
//			return -1;
//		return 0;
//	}
//
//protected:
//	// �����ݼ�
//	Thread_Mutex m_mutex;
//	std::queue<T*> m_record_set;
//	boost::thread *m_pThread;
//	// �ź�
//	Thread_Semaphore m_event;
//	bool m_bstop;
//};
#endif