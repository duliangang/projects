#include "Win32Socket_IOCP.h"

HANDLE CSocketMgr::m_hIOCP=0;
	void CSocketMgr::AddSocket(CWinSocket* newsock)
	{
		if(m_WinSocketList.find(newsock)!=m_WinSocketList.end())
		{
			return ;
		}
		if(newsock->handle()==0||newsock->is_shutdown())
		{
			delete newsock;
			return ;
		}
		CreateIoCompletionPort((HANDLE)newsock->m_sock,m_hIOCP,ULONG_PTR(newsock),0);
		newsock->pushReadEvent();
		m_WinSocketList.insert(newsock);
	}
	void CSocketMgr::shutdown(CWinSocket* sock)
	{
		bool _shutdown=InterlockedCompareExchange(&(sock->m_shutdown),1,0);
		if(!_shutdown)
		{
			sock->OnClose();
			closesocket(sock->handle());
		}
		return ;
	}

	void CSocketMgr::OnWrite(CWinSocket* sock,int len)
	{

		if(sock==NULL||len==0){return ;}
		sock->m_pWriteBlockMutex.acquire();
		sock->m_writeBlock.read_skip(len);
		sock->m_pWriteBlockMutex.release();
		

		AutoLock<Thread_Mutex> _auto(&(sock->m_WriteStatusMutex));
		if(sock->m_writeBlock.length()==0)
		{
			sock->m_writeStatus=true;
			return ;
		}
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后


		sock->m_pWriteBlockMutex.acquire();

		len=sock->m_writeBlock.length();
		void *buf=(void*)sock->m_writeBlock.rd_ptr();

		ol->wbuf.buf=new char[len];
		ol->wbuf.len=len;
		memcpy(ol->wbuf.buf,buf,len);
		sock->m_pWriteBlockMutex.release();


		ol->opcode=OP_WRITE;
		DWORD dwBytes=0,dwFlags=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		int nRet = WSASend(sock->handle(),&(ol->wbuf),1,&dwBytes,dwFlags,&(ol->_overlapped),NULL);
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				//printf("send error :%d socket %d error\n",s,nError);
				ol->opcode=OP_SHUTDOWN;
				PostQueuedCompletionStatus(m_hIOCP,0,(ULONG_PTR)sock,&(ol->_overlapped));
				return ;
			}
		}
	}
	DWORD WINAPI CSocketMgr::WorkProcess(LPVOID lpParam)
	 {
		 DWORD BytesTransferred;
		 CWinSocket* dwKey;

		 while(true)
		 {
			 OVERLAPPED* _ol;
			 DWORD err=GetQueuedCompletionStatus(m_hIOCP, &BytesTransferred,(PULONG_PTR)&dwKey , (LPOVERLAPPED*)&(_ol), INFINITE);
			 if(dwKey==NULL){break;}
			 if(0 == err||BytesTransferred==0)
			 {
				 DWORD nError = GetLastError();
				 OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
				 int op=olplus->opcode;
				 delete[] olplus->wbuf.buf;
				 delete olplus;
				 //主动请求关闭
				 if(op==OP_SHUTDOWN)
				 {
					shutdown(dwKey);
				 }
				 else if(op==OP_END)
				 {
					 break ;
				 }
				 //网络错误，客户端离线 等原因导致
				 else if( BytesTransferred==0||(nError == WAIT_TIMEOUT) || (nError == ERROR_NETNAME_DELETED) )
				 {
					 shutdown(dwKey);
					 continue;
				 }
				 
				 else
				 {
					 return 0;
				 }
			 }
			 OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
			 switch(olplus->opcode)
			 {
			 case OP_READ:
				 {
					 dwKey->m_pReadBlockMutex.acquire();
					 dwKey->m_readBlock.append(olplus->wbuf.buf,BytesTransferred);
					 dwKey->m_pReadBlockMutex.release();
				 }
				 dwKey->OnRead();
				 break;
			 case OP_WRITE:
				OnWrite(dwKey,BytesTransferred);
				
				 break;
			 case 2:
				shutdown(dwKey);
				break;
			 default:
				 break;
			 }
			 delete[] olplus->wbuf.buf;
			 delete olplus;
			 dwKey->pushReadEvent();
			 //OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
			 //ol->wbuf.buf=ol->buffer;
			 //ol->wbuf.len=sizeof(ol->buffer);
			 //ol->opcode=0;
			 //memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
			 //DWORD dwBytes=0,dwFlags=0;
			 //int nRet = WSARecv(dwKey->sClient, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
			 //if(nRet==SOCKET_ERROR)
			 //{
				// int nError = WSAGetLastError();
				// if(nError!=ERROR_IO_PENDING)
				// {
				//	 delete ol;
				// }
			 //}
		 }
		 return 0;
	 }
	void CSocketMgr::DelSocket(CWinSocket* sock)
	{
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();
		ol->opcode=OP_END;
		DWORD dwBytes=0,dwFlags=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		PostQueuedCompletionStatus(m_hIOCP,0,(ULONG_PTR)sock,&(ol->_overlapped));
		m_delList.insert(sock);
		m_WinSocketList.erase(sock);
		return ;
	}
	bool CSocketMgr::Step(int procee_count)
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2,2), &wsaData );
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
		SYSTEM_INFO SystemInfo;
		GetSystemInfo(&SystemInfo);
		m_handleList=new HANDLE[SystemInfo.dwNumberOfProcessors * 2];
		if(procee_count==0)
		{
			procee_count=SystemInfo.dwNumberOfProcessors * 2;
		}
		m_nCount=procee_count;
		for(int i = 0; i < procee_count; i++)
		{
			m_handleList[i] = CreateThread(NULL, 0, CSocketMgr::WorkProcess, this, 0, NULL);
			if(!m_handleList[i])
			{
				delete[] m_handleList;
				m_handleList=NULL;
				return false;
			}
		}
		return true;
	}
	void CSocketMgr::unstep()
	{
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();
		ol->opcode=OP_END;
		DWORD dwBytes=0,dwFlags=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		PostQueuedCompletionStatus(m_hIOCP,0,(ULONG_PTR)NULL,&(ol->_overlapped));
		WaitForMultipleObjects(m_nCount,m_handleList,TRUE,INFINITE);
		std::set<CWinSocket*>::iterator itr=m_delList.begin();
		while(itr!=m_delList.end())
		{
			delete (*itr);
			++itr;
		}
		itr=m_WinSocketList.begin();
		while(itr!=m_WinSocketList.end())
		{
			delete (*itr);
			++itr;
		}
		return ;
	}
	bool CWinSocket::Write(void* buf,int len)
	{
		if(is_shutdown())
		return false;
	   m_pWriteBlockMutex.acquire();
	   if(0==m_writeBlock.space())
	   {
		   m_pWriteBlockMutex.release();
		   return false;
	   }
		m_writeBlock.append(buf,len);

		m_pWriteBlockMutex.release();



		AutoLock<Thread_Mutex> _auto(&(m_WriteStatusMutex));


		if(!m_writeStatus)//端口未完成
		{	
			return true;
		}

		m_writeStatus=false;



		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后


		m_pWriteBlockMutex.acquire();

		len=m_writeBlock.length();
		buf=(void*)m_writeBlock.rd_ptr();

		ol->wbuf.buf=new char[len];
		memcpy(ol->wbuf.buf,buf,len);
		ol->wbuf.len=len;

		m_pWriteBlockMutex.release();


		ol->opcode=OP_WRITE;
		DWORD dwBytes=0,dwFlags=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));

	
		int nRet = WSASend(m_sock,&(ol->wbuf),1,&dwBytes,dwFlags,&(ol->_overlapped),NULL);
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				printf("send error :%d socket %d error\n",m_sock,nError);
				delete ol;
				shutdown();
				return false;
			}
		}
		return true;
	}

	bool CWinSocket::pushReadEvent()
	{
		if(is_shutdown())
			return false;
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
		ol->wbuf.buf=new char[READ_SIZE];
		ol->wbuf.len=READ_SIZE;
		ol->opcode=OP_READ;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		DWORD dwBytes=0,dwFlags=0;
		int nRet = WSARecv(m_sock, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				delete ol;
				shutdown();
				return false;
			}
		}
		return true;
	}