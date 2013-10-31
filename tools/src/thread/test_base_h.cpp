#include "base.h"
#include <stdio.h>
#ifdef _LINUX
#include <linux/delay.h>
#endif
class task_process:public Task_Base
{
public:
	task_process(int thread_count):Task_Base(thread_count)
	{
		m_bStop=false;
	}
	void stop()
	{
		m_bStop=true;
	}
	virtual void close()
	{
		_tprintf(TEXT("not process msg size %d\n"),m_msgBlockList.size());
		return ;
	}
	virtual void svc()
	{
		while(!m_bStop)
		{
			BLOCK block;
			if(getmsg(block)==-1)
			{
				continue;
			}
			_tprintf(TEXT("get msg:%s ,size =%d,send thread status:%d\n"),(_tchar*)block.msg,block.size,block.thread_info->statu);
			delete block.msg;
#ifdef WIN32
			Sleep(10);
#elif defined _LINUX
			delay(10);
#endif

		}
	}
private:
	bool m_bStop;
};
class  task_Create:public Task_Base
{
public:
	task_Create(int thread_count,Task_Base* target):m_target(target),Task_Base(thread_count)
	{
		packetsize=0;
		m_bStop=false;
	}
	void stop()
	{
		m_bStop=true;
	}
	virtual void svc()
	{
		while(!m_bStop)
		{
			_tchar msg[100];
			_tsprintf(msg,TEXT("this msg id is %d"),packetsize++);
			putmsgto(msg,sizeof(msg),m_target);
#ifdef WIN32
			Sleep(10);
#elif defined _LINUX
			delay(10);
#endif

		}
	}
	virtual void close()
	{
		_tprintf(TEXT("total send msg count:  %d\n"),packetsize);
		return ;
	}
private:
	int packetsize;
	bool m_bStop;
	Task_Base* m_target;
};


int test_Base_H()
{
	task_process process(10);
	task_Create creator(30,&process);
	process.open();
	creator.open();
	for(int i=0;i!=1000;i++)
	{
#ifdef WIN32
		Sleep(10);
#elif defined _LINUX
		delay(10);
#endif
	}
	creator.stop();
	creator.join();
	process.stop();
	
	process.join();
	std::_tstring str;
	std::_tcin>>str;
	return 0;
}