#include "Msg_Queue_Ex.h"
#include "apr/apr_base.h"
FILE* fp=0;
std::_tstring GetTimestampStr()
{
	time_t t = time(NULL);
	tm* aTm = localtime(&t);
	//       YYYY   year
	//       MM     month (2 digits 01-12)
	//       DD     day (2 digits 01-31)
	//       HH     hour (2 digits 00-23)
	//       MM     minutes (2 digits 00-59)
	//       SS     seconds (2 digits 00-59)
	_tchar buf[20];
	_tsnprintf(buf, 20, _T("%04d-%02d-%02d_%02d-%02d-%02d"), aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	return std::_tstring(buf);
}

struct content
{
	content(std::_tstring  str,int number):m_str(str),m_number(number)
	{

	}
	~content()
	{

	}
	std::_tstring m_str;
	int m_number;
};
class  _task_process:public Task_Base
{
public:
	_task_process(int thread_count):Task_Base(thread_count)
	{
		m_bStop=false;
	}
	void stop()
	{
		m_bStop=true;
		m_queue.destory();
	}
	virtual void close()
	{
		_tfprintf(fp,TEXT("task_process-- log:not process msg size %d\n"),m_queue.size());
		return ;
	}
	virtual void svc()
	{
		while(!m_bStop)
		{
			content* con;

			if(m_queue.dequeue(con)!=0)
			{
				break;
			}
			_tfprintf(fp,TEXT("task_process-- log:recv packet number %d, send time is %s\n"),con->m_number,con->m_str.c_str());
			delete con;
		}
	}
	int enqueue(content* _content)
	{
		m_queue.enqueue(_content);
		return 0;
	}
private:
	Msg_Queue<content,Thread_Mutex> m_queue;
	bool m_bStop;
};
class  _task_Create:public Task_Base
{
public:
	_task_Create(int thread_count,_task_process* target):m_target(target),Task_Base(thread_count)
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
			_tsprintf(msg,TEXT("%s"),GetTimestampStr().c_str());
			m_target->enqueue(new content(GetTimestampStr(),packetsize++));
#ifdef WIN32
			Sleep(1);
#elif defined _LINUX
			delay(1);
#endif

		}
	}
	virtual void close()
	{
		_tfprintf(fp,TEXT("task_Create-- log:total send msg count:  %d\n"),packetsize);
		return ;
	}
private:
	int packetsize;
	bool m_bStop;
	_task_process* m_target;
};


//class Msg_Queue_Ex_text:public Msg_Queue_Ex<content>
//{
//public:
//	virtual void DataFunc(content& con)
//	{
//		_tfprintf(fp,TEXT("Msg_Queue_Ex_text-- log:recv packet number %d, send time is %s\n"),con.m_number,con.m_str.c_str());
//	}
//};


//class  task_Create_ToMsg_Queue_Ex:public Task_Base
//{
//public:
//	task_Create_ToMsg_Queue_Ex(int thread_count,Msg_Queue_Ex_text* target):m_target(target),Task_Base(thread_count)
//	{
//		packetsize=0;
//		m_bStop=false;
//	}
//	void stop()
//	{
//		m_bStop=true;
//	}
//	virtual void svc()
//	{
//		while(!m_bStop)
//		{
//			_tchar msg[100];
//			_tsprintf(msg,TEXT("%s"),GetTimestampStr().c_str());
//			content t(GetTimestampStr(),packetsize++);
//			m_target->Put(t);
//#ifdef WIN32
//			Sleep(10);
//#elif defined _LINUX
//			delay(10);
//#endif
//
//		}
//	}
//	virtual void close()
//	{
//		_tfprintf(fp,TEXT("task_Create_ToMsg_Queue_Ex log:total send msg count:  %d\n"),packetsize);
//		return ;
//	}
//private:
//	int packetsize;
//	bool m_bStop;
//	Msg_Queue_Ex_text* m_target;
//};
//Thread_Semaphore *sam;
//
//HANDLE SemapleHandle;
//
//
//
//
//
//class  _task_Test_Sam:public Task_Base
//{
//public:
//	_task_Test_Sam(int thread_count):Task_Base(thread_count)
//	{
//		packetsize=0;
//		m_bStop=false;
//	}
//	void stop()
//	{
//		m_bStop=true;
//	}
//	virtual void svr()
//	{
//		if(WaitForSingleObject(SemapleHandle,INFINITE)!=WAIT_OBJECT_0)
//		/*if(sam->wait()==-1)*/
//		 return ;
//		printf("thread get the sam\n");
//		return ;
//	}
//	virtual void close()
//	{
//		_tprintf(TEXT("task_Create-- log:total send msg count:  %d\n"),packetsize);
//		return ;
//	}
//private:
//	int packetsize;
//	bool m_bStop;
//	_task_process* m_target;
//};
int main()
{

	fp=_tfopen(_T("print.txt"),_T("w"));
	_task_process process(10);
	_task_Create creator(30,&process);
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
	fclose(fp);
	return 0;

}


