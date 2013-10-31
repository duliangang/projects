#include "CMySocket.h"
#pragma comment(lib,"ws2_32.lib")
HandleFun OpertionList[EVENT_MASK_MAX]=
{
	{TIMEOUT_MASK,on_timeout},
	{READ_MASK,on_read},
	{WRITE_MASK,on_write},
	{SIGNAL_MASK,NULL},
	{PERSIST_MASK,NULL},
	{TIMER_MASK,on_timer},
};
inline int logn2(int N)
{
	long result;
	__asm
	{
		bsf eax, N
			bsr edx, N
			cmp eax, edx
			jz ok
			or eax, -1
ok:
		mov result, eax
	}
	return result;
}

event_callback_fn GetCallFun(EVENT_HANDLE _event_type)
{
	if(logn2(_event_type)>=EVENT_MASK_MAX)
	{
		return NULL;
	}
	return OpertionList[logn2(_event_type)].fun;
}

SVC_Handler::SVC_Handler()
{
	m_peer=0;
	m_closing_=false;
}
int SVC_Handler::create(evutil_socket_t fd,const sockaddr* addr_)
{
	m_peer=fd;
	memcpy(&m_addr,addr_,sizeof(m_addr));
	Reactor_()->schedule_wakeup(EVENT_HANDLE::READ_MASK,this);
	return 0;
}

int SVC_Handler::shutdown()
{
	 int flag=closefd();
	 m_peer=NULL;
	 return flag;
	//Reactor_()->cancel_wakeup(this,READ_MASK);
}
int SVC_Handler::sendfd(const void* msg,uint32_t size)
{
	return send(m_peer,(const char*)msg,size,0);
}
int SVC_Handler::recvfd(void* buffer,uint32_t size)
{
	return recv(m_peer,(char*)buffer,size,0);
}
int SVC_Handler::closefd()
{
	return evutil_closesocket(m_peer);
}
int32_t SVC_Handler::putmsg(Message_Block& block)
{
	m_mutex.acquire();
	m_msgBlockList.push(block);
	m_mutex.release();
	return 0;
}
int32_t SVC_Handler::getmsg(Message_Block& block)
{
	m_mutex.acquire();
	block=m_msgBlockList.front();
	m_msgBlockList.pop();
	m_mutex.release();
	return 0;
}
bool SVC_Handler::isEmptymsg()
{
	return m_msgBlockList.empty();
}
bool  SVC_Handler::clearmsg()
{
	m_mutex.acquire();
	while(!m_msgBlockList.empty())
	{	
		m_msgBlockList.pop();
	}
	m_mutex.release();
	return 0;
}

Reactor::Reactor()
{
	base=event_base_new();
	m_event_msgqueue=msgqueue_new(base,10000,on_append_start,this);
}
Reactor::~Reactor()
{

	std::map<Event_Handle_Base*,Reactor_Data>::iterator itr=m_SvcHandleList.begin();
	msgqueue_destroy(m_event_msgqueue);
	while(itr!=m_SvcHandleList.end())
	{
			if(itr->second._reactor_event)
			{
				itr->second.flag=0;
				for (int i=0;i!=EVENT_MASK_MAX;i++)
				{

					event_del(itr->second._reactor_event[i]);
					event_free(itr->second._reactor_event[i]);
				}
			}
		
	}
	event_base_free(base);
}


int Reactor::schedule_wakeup(EVENT_HANDLE eventHandle,Event_Handle_Base* _handle)
{
	if(!_handle)return 0;
	Reactor_temp_data* d =new Reactor_temp_data();
	d->_handle=_handle;
	d->flag=eventHandle;
	d->apped=true;
	msgqueue_push(m_event_msgqueue,d);
	return 0;
}

int Reactor::_schedule_wakeup(EVENT_HANDLE eventHandle,Event_Handle_Base* _handle)
{
	event_callback_fn fun=GetCallFun(eventHandle);
	if(NULL==fun)return 0;
	if(m_SvcHandleList.find(_handle)==m_SvcHandleList.end())
	{
		m_SvcHandleList[_handle].flag=eventHandle|PERSIST_MASK;
		for (int i=0;i!=EVENT_MASK_MAX;i++)
		{
			m_SvcHandleList[_handle]._reactor_event[i]=0;
		}
		
	}
	else
	{
		if(!(m_SvcHandleList[_handle].flag&eventHandle))return 0;
		m_SvcHandleList[_handle].flag|=eventHandle;
	}
	m_SvcHandleList[_handle]._reactor_event[logn2(eventHandle)]=event_new(base,_handle->m_peer,eventHandle|PERSIST_MASK,fun,_handle);
	event_add(m_SvcHandleList[_handle]._reactor_event[logn2(eventHandle)],NULL);
	return 0;
}
int Reactor::schedule_wakeup(timeval _timeval,Event_Handle_Base* _handle)
{
	if(!_handle)return 0;
	Reactor_temp_data* d=new Reactor_temp_data();
	d->_handle=_handle;
	d->flag=TIMER_MASK;
	d->_timeval=_timeval;
	d->apped=true;
	msgqueue_push(m_event_msgqueue,d);
	return 0;
}
int Reactor::cancel_wakeup(Event_Handle_Base* _handle,EVENT_HANDLE eventhandle)
{
	if(!_handle)return 0;
	Reactor_temp_data* d=new Reactor_temp_data();
	d->_handle=_handle;
	d->flag=eventhandle;
	d->apped=false;
	msgqueue_push(m_event_msgqueue,d);
	return 0;
}
int Reactor::_cancel_wakeup(Event_Handle_Base* _handle,EVENT_HANDLE eventhandle)
{

	if(m_SvcHandleList.find(_handle)==m_SvcHandleList.end())
	{
		return 0;
	}
	
	if(m_SvcHandleList[_handle]._reactor_event[logn2(eventhandle)]==NULL||!(m_SvcHandleList[_handle].flag&eventhandle))
	{
		return 0;
	}
	event_del(m_SvcHandleList[_handle]._reactor_event[logn2(eventhandle)]);
	m_SvcHandleList[_handle].flag&= ~(1<<logn2(eventhandle));
	return 0;
}
void Reactor::run()
{
	if(!base)return ;
	event_base_dispatch(base);
	return ;

}
int Reactor::clear_wakeup(Event_Handle_Base* handle)
{
	for(int i=0;i!=EVENT_MASK_MAX;i++)
	{
		cancel_wakeup(handle,(EVENT_HANDLE)(1<<i));
	}
	return 0;
}
int Reactor::_schedule_wakeup(timeval _timeval,Event_Handle_Base* _handle)
{
	EVENT_HANDLE eventHandle=TIMER_MASK;
	event_callback_fn fun=GetCallFun(eventHandle);
	if(NULL==fun)return 0;
	if(m_SvcHandleList.find(_handle)==m_SvcHandleList.end())
	{
		m_SvcHandleList[_handle].flag=PERSIST_MASK;
		for (int i=0;i!=EVENT_MASK_MAX;i++)
		{
			m_SvcHandleList[_handle]._reactor_event[i]=0;
		}
		
	}
	else
	{
		if(!(m_SvcHandleList[_handle].flag&eventHandle))return 0;
	}
	m_SvcHandleList[_handle]._reactor_event[logn2(eventHandle)]=event_new(base,_handle->m_peer,eventHandle|PERSIST_MASK,fun,_handle);
	event_add(m_SvcHandleList[_handle]._reactor_event[logn2(eventHandle)],&_timeval);
    m_SvcHandleList[_handle].flag|=PERSIST_MASK;
	m_SvcHandleList[_handle]._time = _timeval;
	return 0;
}
void Reactor::on_append_start(void* vp_data,void* _pthis)
{
	    Reactor_temp_data* pdata=(Reactor_temp_data*)vp_data;
		Reactor* _this=(Reactor*)_pthis;
	   if(pdata->apped)
	   {
		   if(pdata->flag&TIMER_MASK)
		   {
			   _this->_schedule_wakeup(pdata->_timeval,pdata->_handle);
		   }
		   else
		   {
			   _this->_schedule_wakeup((EVENT_HANDLE)pdata->flag,pdata->_handle);
		   }
	   }
	   else
	   {
		   _this->_cancel_wakeup(pdata->_handle,(EVENT_HANDLE)pdata->flag);
	   }
	   delete pdata;
	   return ;
}


void on_read(int fd, short ev, void *arg)
{
	Event_Handle_Base* _handle=(Event_Handle_Base*)arg;
	if(_handle->handle_input()==-1)
	{
		_handle->handle_close();
	}
	return;
}
void on_write(int fd, short ev, void *arg)
{
	Event_Handle_Base* _handle=(Event_Handle_Base*)arg;
	if(_handle->handle_output()==-1)
	{
		_handle->handle_close();
	}
	return;
}
void on_timeout(int fd, short ev, void *arg)
{
	Event_Handle_Base* _handle=(Event_Handle_Base*)arg;
	if(_handle->handle_timeout()==-1)
	{
		_handle->handle_close();
	}
	return;
}
void on_timer(int fd, short ev, void *arg)
{
	Event_Handle_Base* _handle=(Event_Handle_Base*)arg;
	if(_handle->handle_timeout()==-1)
	{
		_handle->handle_close();
	}
	return;
}