#ifndef _CMYSOCKET_H
#define  _CMYSOCKET_H
#include <event2/bufferevent.h>
#include <map>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <queue>
#include "../thread/base.h"
#include "../shared/Message_Block.h"
#include "event_msgqueue.h"
enum EVENT_HANDLE
{
	TIMEOUT_MASK=EV_TIMEOUT,
	READ_MASK =EV_READ,
	WRITE_MASK=EV_WRITE,
	SIGNAL_MASK=EV_SIGNAL,//not use
	PERSIST_MASK =EV_PERSIST,//not use
	TIMER_MASK=0x20,
};
struct HandleFun
{
	EVENT_HANDLE handle;
	event_callback_fn fun;
};
#define EVENT_MASK_MAX 6
extern HandleFun OpertionList[EVENT_MASK_MAX];
event_callback_fn GetCallFun(EVENT_HANDLE _event_type);


void on_read(int fd, short ev, void *arg);
void on_write(int fd, short ev, void *arg);
void on_timeout(int fd, short ev, void *arg);
void on_timer(int fd, short ev, void *arg);

class Reactor;


class Event_Handle_Base
{
	friend void on_read(int fd, short ev, void *arg);
	friend void on_write(int fd, short ev, void *arg);
	friend void on_timer(int fd, short ev, void *arg);
	friend void on_timeout(int fd, short ev, void *arg);
	friend class Reactor;
public:
	virtual ~Event_Handle_Base(){};

	
protected:
	virtual int handle_input(){return 0;}
	virtual int handle_output(){return 0;}
	virtual int handle_close(){return 0;}
	virtual int handle_timeout(){return 0;}
	virtual int handle_timer(){return 0;}
public:
	Reactor* Reactor_(){return m_Reactor;}
	void Reactor_(Reactor* reactor){m_Reactor=reactor;}
	evutil_socket_t m_peer;
private:
	
	Reactor* m_Reactor;
};
class Reactor
{
public:
	struct Reactor_Data
	{
		int flag;
		timeval _time;
		struct event* _reactor_event[EVENT_MASK_MAX];
	};
	Reactor();
	virtual ~Reactor();

	virtual int schedule_wakeup(EVENT_HANDLE eventHandle,Event_Handle_Base* _handle);
	virtual int cancel_wakeup(Event_Handle_Base* handle_,EVENT_HANDLE eventhandle);
	virtual int clear_wakeup(Event_Handle_Base* handle);
	virtual int schedule_wakeup(timeval _timeval,Event_Handle_Base* _handle);
	int _schedule_wakeup(EVENT_HANDLE eventHandle,Event_Handle_Base* _handle);
	int _schedule_wakeup(timeval _timeval,Event_Handle_Base* _handle);
	int _cancel_wakeup(Event_Handle_Base* handle_,EVENT_HANDLE eventhandle);
	void run();
private:
	
	static void on_append_start(void*,void*);
	struct Reactor_temp_data
	{
		bool apped;
		int flag;
		Event_Handle_Base* _handle;
		timeval _timeval;
	};
	std::map<Event_Handle_Base*,Reactor_Data> m_SvcHandleList;
	struct event_base *base;
	struct event_msgqueue* m_event_msgqueue;
	Thread_Mutex m_mutex;
};

class SVC_Handler:public Event_Handle_Base
{
public:
	SVC_Handler();
	~SVC_Handler(){};
	int create(evutil_socket_t,const sockaddr* );
	int shutdown();
	virtual int open(){return 0;};
	virtual int close(){return 0;};
	virtual int handle_input(){return 0;}
	virtual int handle_output(){return 0;}
protected:
	
	sockaddr m_addr;
	bool m_closing_;
protected:
	int sendfd(const void* msg,uint32_t size);
	int recvfd(void* buffer,uint32_t size);
	int closefd();
	int32_t putmsg(Message_Block& block);
	int32_t getmsg(Message_Block& block);
	bool isEmptymsg();
	bool  clearmsg();
private:
	std::queue<Message_Block> m_msgBlockList;
	Thread_Mutex m_mutex;
};


template<class SVC_SOCK>
class svc_accept:public Event_Handle_Base
{
public:
	enum State
	{
		CREATE,
		ACCEPTED,
		STOPED,
		STOP,
		DESTORY,
	};
	svc_accept(uint32_t maxsize);
	~svc_accept();
	int open(SOCKADDR_IN * addr,Reactor* );
	int run_reactor_event_loop();
	void stop(){m_bStop=true;m_stat=STOPED;}
protected:
	virtual int handle_accept_error(void){return 0;};
	virtual int make_svc_handler(SVC_SOCK* &sh)
	{
		sh = new SVC_SOCK();
		return 0;
	};
private:
	int shutdown();
	std::vector<SVC_SOCK* >m_shList;
	uint32_t m_maxsize;
	virtual int handle_input();
	virtual int handle_timeout();
	virtual int handle_close();
	Thread_Mutex mutex;
	bool m_bStop;
	State m_stat;
};
template<class SVC_SOCK>
int svc_accept<SVC_SOCK>::run_reactor_event_loop()
{
	if(Reactor_())
		Reactor_()->run();
	return 0;
}
template<class SVC_SOCK>
svc_accept<SVC_SOCK>::svc_accept(uint32_t maxsize):m_maxsize(maxsize),m_bStop(false)
{
	m_stat=CREATE;
	m_peer=NULL;

}

template<class SVC_SOCK>
int svc_accept<SVC_SOCK>::handle_close()
{
	return shutdown();
}
template<class SVC_SOCK>
int svc_accept<SVC_SOCK>::handle_timeout()
{
	if(m_bStop)
	{
		shutdown();
	}
	return 0;
}

template<class SVC_SOCK>
int svc_accept<SVC_SOCK>::shutdown()
{

	AutoLock<Thread_Mutex> _mu(&mutex);
	if(m_stat==DESTORY){return 0;}
	evutil_closesocket(m_peer);
	std::vector<SVC_SOCK* >::iterator itr=m_shList.begin();
	while(itr!=m_shList.end())
	{
		(*itr)->close();
		++itr;
	}
	m_stat=DESTORY;
	return 0;
}
template<class SVC_SOCK>
svc_accept<SVC_SOCK>::~svc_accept()
{
	shutdown();
	std::vector<SVC_SOCK*>::iterator itr=m_shList.begin();
	while(itr!=m_shList.end())
	{
		delete *itr;
		++itr;
	}
	delete Reactor_();
}
template<class SVC_SOCK>
int svc_accept<SVC_SOCK>::handle_input()
{
	struct sockaddr_in addr_;
	socklen_t addrlen = sizeof(addr_);
	int yes = 1;
	evutil_socket_t cfd= accept(m_peer, (struct sockaddr *)&addr_, &addrlen);
	if (cfd == -1) {
		handle_accept_error();
		return -1;
	}
	evutil_make_socket_nonblocking(cfd);
	//evutil_make_socket_closeonexec(cfd);
	SVC_SOCK* sh=NULL;

	make_svc_handler(sh);
	if(sh==NULL)return 0;
	sh->create(cfd,(sockaddr*)&addr_);
	sh->open();
	
	AutoLock<Thread_Mutex> _mu(&mutex);
	m_shList.push_back(sh);
	
	return 0;
}

template<class SVC_SOCK>
int svc_accept<SVC_SOCK>::open(SOCKADDR_IN * addr,Reactor* reactor)
{
	int re;
	m_peer= socket(addr->sin_family,SOCK_STREAM,0);
	/*for (rp = &addr; rp != NULL; rp = addr.ai_next) {
		 m_peer= socket(rp->ai_family, rp->ai_socktype,
			rp->ai_protocol);
		if (m_peer == -1)
			continue;*/
	re=bind(m_peer, (struct sockaddr *)addr, sizeof(SOCKADDR_IN));
	if (re == 0)
	{              /* Success */
		if ( re< 0)fprintf(stderr,"bind error");
		re=listen(m_peer,m_maxsize);
		if(re<0)fprintf(stderr,"listen error%d",EVUTIL_SOCKET_ERROR());
		int length=sizeof(SOCKADDR_IN);
		SOCKADDR_IN sockAddr;
		memset(&sockAddr,0,sizeof(sockAddr));
		int _iLen=sizeof(SOCKADDR_IN);
		if(getsockname(m_peer,(struct sockaddr *)&sockAddr,&_iLen)!=0)//得到远程IP地址和端口号
		{
			fprintf(stderr,"%d\n",EVUTIL_SOCKET_ERROR());
		}

		std::string str = inet_ntoa(sockAddr.sin_addr);//IP
		int port = sockAddr.sin_port;
	}
	evutil_make_socket_nonblocking(m_peer);
	evutil_make_socket_closeonexec(m_peer);
	
	Reactor_(reactor);
	if(Reactor_())
	{
		Reactor_()->schedule_wakeup(READ_MASK,this);
		timeval val;
		val.tv_sec=30;
		val.tv_usec=0;
		Reactor_()->schedule_wakeup(val,this);
	}

	m_stat=ACCEPTED;
	return 0;
}

#endif
