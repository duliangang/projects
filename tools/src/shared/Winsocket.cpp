#include "Winsocket.h"
#include <WinSock2.h>
#include <boost/pool/singleton_pool.hpp>
enum ServerOpCode
{
	OP_WRITE=0,
	OP_READ=1,
	OP_SHUTDOWN=2,
	OP_END=3,
};
enum
{
	READ_MAX_SIZE=1024*10,
};

static long currentSocketSize=0;
//typedef boost::singleton_pool<WSABUF,READ_MAX_SIZE> ReadBufferPool;


void WinHandleManage::DelSocket(HANDLE _handle)
{
	int val=InterlockedExchangeAdd(&currentSocketSize,-1);
	printf("%d socket inline\n",val);
boost::recursive_mutex::scoped_lock _lock(m_Mutex);
	if(m_HandleList.find(_handle)==m_HandleList.end())
	{
		return ;
	}
	m_HandleList[_handle]->close();
	m_HandleList.erase(_handle);
	return ;
}
void WinHandleManage::DelSocket(boost::shared_ptr<WinHandle> sock)
{
		return DelSocket(sock->get_handle());
}
void WinHandleManage::DelSocket(WinHandle* sock)
{
	return DelSocket(sock->get_handle());
}
void WinHandleManage::unstep()
{
	boost::recursive_mutex::scoped_lock _lock(m_Mutex);
	for(int i=0;i!=m_ThreadGourp.size();i++)
	{
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();
		ol->opcode=OP_END;
		DWORD dwBytes=0,dwFlags=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		if(PostQueuedCompletionStatus(m_hIOCP,0,(ULONG_PTR)NULL,&(ol->_overlapped))==false)
		{
			m_ThreadGourp.interrupt_all();
			break;
		}
	}
	m_ThreadGourp.join_all();
	CloseHandle(m_hIOCP);
	std::map<HANDLE,boost::shared_ptr<WinHandle> >::iterator itr=m_HandleList.begin();
	while(itr!=m_HandleList.end())
	{
		itr->second->close();
		++itr;
	}
	m_HandleList.clear();
	return ;
}
bool WinHandleManage::Step()
{
	boost::recursive_mutex::scoped_lock _lock(m_Mutex);
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData );
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	int ThreadCount=SystemInfo.dwNumberOfProcessors * 2;
	for(int i = 0; i < ThreadCount; i++)
	{
		m_ThreadGourp.add_thread(new boost::thread(boost::bind(&WinHandleManage::WorkProcess,this)));
	}
	return true;
}
void WinHandleManage::AddSocket(boost::shared_ptr<WinHandle> newsock)
{
	InterlockedExchangeAdd(&currentSocketSize,1);
	boost::recursive_mutex::scoped_lock _lock(m_Mutex);
	if(m_HandleList.find(newsock->get_handle())!=m_HandleList.end())
	{
		return ;
	}
	m_HandleList[newsock->get_handle()]=newsock;
	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
	
	ol->opcode=OP_READ;
	memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
	ol->_WinHandle=newsock;
	//change -test
	//ol->wBuf.buf=(char*)(ReadBufferPool::malloc());
	ol->wBuf.buf=new char[READ_MAX_SIZE];
	ol->wBuf.len=READ_MAX_SIZE;
	DWORD dwBytes=0,dwFlags=0;


	CreateIoCompletionPort((HANDLE)newsock->get_handle(),m_hIOCP,ULONG_PTR(newsock.get()),0);

	if(!newsock->SysReadFile(ol))
	{
		//change -test
		//ReadBufferPool::free(ol->wBuf.buf);
		delete[] ol->wBuf.buf;
		delete ol;
		ol=NULL;
		DelSocket(newsock);
	}
	/*int nRet = WSARecv((SOCKET)newsock->get_handle(), &(ol->wBuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
	if(nRet==SOCKET_ERROR)
	{
	int nError = WSAGetLastError();
	if(nError!=ERROR_IO_PENDING)
	{
	ReadBufferPool::free(ol->wBuf.buf);
	delete ol;
	ol=NULL;
	DelSocket(newsock);
	}
	}*/
}



DWORD  WinHandleManage::WorkProcess()
{
	DWORD BytesTransferred;
	WinHandle* dwKey;

	while(true)
	{
		OVERLAPPED* _ol;
		DWORD err=GetQueuedCompletionStatus(m_hIOCP, &BytesTransferred,(PULONG_PTR)&dwKey , (LPOVERLAPPED*)&(_ol), INFINITE);
		if(dwKey==NULL){break;}
		if(err==FALSE)
		{
			DWORD nError = GetLastError();
			if(_ol==NULL)
			{
				return nError;
			}
				
			OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
			int op=olplus->opcode;
			if(op!=OP_END&&op!=OP_SHUTDOWN)
			{
				//change -test
				//ReadBufferPool::free(olplus->wBuf.buf);
				delete[] olplus->wBuf.buf;
			}
			DelSocket(olplus->_WinHandle);
			delete olplus;
			continue;
		}
		OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);

		if(olplus->_WinHandle->is_close())
		{
			//change -test
			//ReadBufferPool::free(olplus->wBuf.buf);
			delete[] olplus->wBuf.buf;
			delete olplus;
			continue;
		}
		switch(olplus->opcode)
		{
		case OP_READ:
			{
				if(BytesTransferred==0)
				{
					delete[] olplus->wBuf.buf;
					DelSocket(olplus->_WinHandle);
					delete olplus;
					olplus=NULL;
					continue;
				}
				olplus->_WinHandle->OnRead(olplus->wBuf.buf,BytesTransferred);
				memset(&(olplus->_overlapped),0,sizeof(olplus->_overlapped));
				DWORD dwBytes=0,dwFlags=0;
				if(!olplus->_WinHandle->SysReadFile(olplus))
				{
					//change -test
					//ReadBufferPool::free(olplus->wBuf.buf);
					delete[] olplus->wBuf.buf;
					DelSocket(olplus->_WinHandle);
					delete olplus;
					olplus=NULL;
					
				}
				/*
				int nRet = WSARecv((SOCKET)olplus->_WinHandle->get_handle(), &(olplus->wBuf), 1, &(dwBytes), &(dwFlags), &(olplus->_overlapped), NULL);
				if(nRet==SOCKET_ERROR)
				{
					int nError = WSAGetLastError();
					if(nError!=ERROR_IO_PENDING)
					{
						ReadBufferPool::free(olplus->wBuf.buf);
						delete olplus;
						olplus=NULL;
					}
				}
				*/
				continue;
			}
			break;
		case OP_WRITE:
			{
				long alreadyRead=InterlockedExchangeAdd(&olplus->alreadySendBytes,BytesTransferred);
				if(alreadyRead+BytesTransferred!=olplus->wBuf.len)
				{
					continue;
				}
				olplus->_WinHandle->OnWrite(BytesTransferred);
			}

			break;
		case OP_SHUTDOWN:
			olplus->_WinHandle->close();
			break;
		default:
			break;
		}
		if(olplus)
		{
			delete olplus;
			olplus=NULL;
		}
	}
	return 0;
}
enum{
 MAX_WRITE_BUFFER=1024*1024,
};
WinHandle::WinHandle():m_isClose(true),m_handle(NULL)
{
	
}
bool WinHandle::open(HANDLE _handle)
{	
	assert(m_handle==NULL);
	boost::mutex::scoped_lock _lock(m_writeMutex);
	m_handle=_handle;
	m_isClose=false;
	while(!m_SendQueue.empty())
	{
		delete m_SendQueue.front();
		m_SendQueue.pop();
	}
	return true;
}
bool WinHandle::Write(void* buf,int len)
{
	if(m_isClose)return false;
	
	
	boost::shared_ptr<WinHandle> ptr=sWinHandleManage->getSharedHandle(m_handle);
	if(ptr==NULL)
	{
		return false;
	}
	boost::mutex::scoped_lock _lock(m_writeMutex);
	if(!m_SendQueue.empty())
	{
		MessageBlock_* lastBlock=m_SendQueue.back();
		if(lastBlock->space()>len)
		{
			lastBlock->append((char*)buf,len);
		}
		else
		{
			int templen=len > 10240 ? len : 10240;
			char* _block=new char[templen];
			MessageBlock_* block=new MessageBlock_(_block, templen ,true);
			block->append((char*)buf,len);
			m_SendQueue.push(block);
		}
		
		return true;
	}
	else
	{
		int templen=len > 10240 ? len : 10240;
		char* _block=new char[templen];
		MessageBlock_* block=new MessageBlock_(_block, templen ,true);
		block->append((char*)buf,len);
		m_SendQueue.push(block);
	}
	
	DWORD dwBytes=0,dwFlags=0;
	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();
	ol->opcode=OP_WRITE;
	memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
	ol->_WinHandle=ptr;
	ol->wBuf.buf=NULL;
	ol->alreadySendBytes=0;
	ol->wBuf.len=0;
	if(FALSE==PostQueuedCompletionStatus(sWinHandleManage->GetIOCPHandle(),dwBytes,(ULONG_PTR)m_handle,&(ol->_overlapped)))
	{
		int nError = WSAGetLastError();
		CloseHandle(m_handle);
		m_isClose=true;
		delete ol;
		return false;
	}
	return true;
}
void WinHandle::close()
{
	boost::mutex::scoped_lock _lock(m_writeMutex);
	CloseHandle(m_handle);
	m_isClose=true;
}
HANDLE WinHandle::get_handle()
{
	return  m_handle;
}
bool WinHandle::is_close()
{
	return m_isClose;
}


void  WinHandle::OnWrite(int wr_len)
{
	//assert(wr_len!=0);
	boost::shared_ptr<WinHandle> ptr=sWinHandleManage->getSharedHandle(m_handle);
	if(ptr==NULL)return;
	boost::mutex::scoped_lock _lock(m_writeMutex);
	
	if(m_isClose)
	{
		return;
	}
	assert(!m_SendQueue.empty());
	MessageBlock_* block=m_SendQueue.front();
	if(wr_len<block->length())
	{
		block->skip_rd(wr_len);
	}
	else
	{
		m_SendQueue.pop();
		delete block;
	}
	block=NULL;
	if(m_SendQueue.empty())
	{
		return;
	}
	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();
	ol->opcode=OP_WRITE;
	 block=m_SendQueue.front();
	ol->wBuf.buf=block->rd_ptr();
	ol->wBuf.len=block->length();
	memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
	ol->_WinHandle=ptr;
	ol->alreadySendBytes=0;
	DWORD dwBytes=0,dwFlags=0;
	if(!SysWriteFile(ol))
	{
		CloseHandle(m_handle);
		delete ol;
		return ;
	}
	//DWORD nRet=WSASend((SOCKET)m_handle,&(ol->wBuf),1,&dwBytes,dwFlags,&(ol->_overlapped),NULL);
	//if(nRet==SOCKET_ERROR)
	//{
	//	int nError = WSAGetLastError();
	//	if(nError!=ERROR_IO_PENDING)
	//	{
	//		CloseHandle(m_handle);
	//		delete ol;
	//		m_handle=NULL;
	//		m_isClose=true;
	//		return ;
	//	}
	//}
	return;
}
WinHandle::~WinHandle()
{
	
	boost::mutex::scoped_lock _lock(m_writeMutex);
	if(!m_isClose)CloseHandle(m_handle);
	m_handle=NULL;
	m_isClose=true;
	while(!m_SendQueue.empty())
	{
		delete m_SendQueue.front();
		m_SendQueue.pop();
	}
};