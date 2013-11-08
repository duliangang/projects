//#include <WinSock2.h>
//#include <Windows.h>
//#include <time.h>
//#include <stdio.h>
//#include <map>
//#include "../shared/Message_Block.h"
//#include "../thread/base.h"
//#pragma comment(lib,"Ws2_32.lib")
//HANDLE m_hIocp;
//
//
//typedef std::map<UINT64,Message_Block> MessageBlockMap;
//typedef std::map<UINT64,Thread_Mutex*>  ThreadMutexMap;
//
//typedef std::map<UINT64,bool> SocketStatusMap;
//
//MessageBlockMap g_MapWriteBlock;
//
////在任何情况下 都不应该锁着block锁去请求另外一个锁....
////单纯负责buffer更改的锁
//ThreadMutexMap g_MapWriteBlockMutex;
//
//SocketStatusMap g_MapScoketStatu;
//ThreadMutexMap g_MapScoketStatuMutex;
//
//SocketStatusMap g_MapSocketShutdownStatus;
//
//ThreadMutexMap  g_MapScoketShutdownStatuMutex;
//
//typedef struct _OVERLAPPEDPLUS
//{
//	OVERLAPPED _overlapped;
//	unsigned char opcode;
//	WSABUF wbuf;
//	char buffer[1000];
//}OVERLAPPEDPLUS;
//typedef struct _SOCKET_HANDLE
//{
//	SOCKET sClient;
//}SOCKET_HANDLE;
//bool g_teststop=false;
//DWORD WINAPI WorkProcess(LPVOID lpParam);
//
//bool Write(SOCKET s,void* buf,int len);
//
//
//
//DWORD WINAPI TestProcess(LPVOID lpParam)
//{
//	while(!g_teststop)
//	{
//		MessageBlockMap::const_iterator itr=g_MapWriteBlock.begin();
//		while(itr!=g_MapWriteBlock.end())
//		{
//			bool flag=false;
//			g_MapScoketShutdownStatuMutex[itr->first]->acquire();
//			flag=g_MapSocketShutdownStatus[itr->first];
//			g_MapScoketShutdownStatuMutex[itr->first]->release();
//			if(!flag)
//				Write(itr->first,"test send buffer\n",strlen("test send buffer\n")+1);
//			++itr;
//		}
//		Sleep(100);
//	}
//	return 0;
//}
//
//BOOL GetSockName(SOCKET s, char* rSocketAddress, UINT& rSocketPort)
//{
//	SOCKADDR_IN sockAddr;
//	memset(&sockAddr, 0, sizeof(sockAddr));
//
//	int nSockAddrLen = sizeof(sockAddr);
//	int iResult = getsockname(s, (SOCKADDR*)&sockAddr, &nSockAddrLen);
//	if (0 == iResult)
//	{
//		rSocketPort = ntohs(sockAddr.sin_port);
//		char* t=inet_ntoa(sockAddr.sin_addr);
//		strcpy_s(rSocketAddress,strlen(t)+1,t);
//		return TRUE;
//	}
//	return FALSE;
//}
////socket send函数
//bool Write(SOCKET s,void* buf,int len)
//{
//
//	g_MapWriteBlockMutex[s]->acquire();
//	if(0==g_MapWriteBlock[s].space())
//	{
//		g_MapWriteBlockMutex[s]->release();
//		return false;
//	}
//
//	if(g_MapWriteBlock[s].space()<len)
//	{
//		len=g_MapWriteBlock[s].space();
//	}
//	g_MapWriteBlock[s].append(buf,len);
//
//	g_MapWriteBlockMutex[s]->release();
//
//	AutoLock<Thread_Mutex> _auto((g_MapScoketStatuMutex[s]));
//
//
//	if(!g_MapScoketStatu[s])//端口未完成
//	{	
//		return true;
//	}
//	
//	g_MapScoketStatu[s]=false;
//	
//
//
//	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
//
//
//	g_MapWriteBlockMutex[s]->acquire();
//
//	len=g_MapWriteBlock[s].length();
//	buf=(void*)g_MapWriteBlock[s].rd_ptr();
//
//	ol->wbuf.buf=new char[len];
//	memcpy(ol->wbuf.buf,buf,len);
//	ol->wbuf.len=len;
//
//	g_MapWriteBlockMutex[s]->release();
//
//
//	ol->opcode=1;
//	DWORD dwBytes=0,dwFlags=0;
//	memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
//
//	g_MapScoketShutdownStatuMutex[s]->acquire();
//	if(g_MapSocketShutdownStatus[s])
//	{
//		g_MapScoketShutdownStatuMutex[s]->release();
//		return false;
//	}
//	int nRet = WSASend(s,&(ol->wbuf),len,&dwBytes,dwFlags,&(ol->_overlapped),NULL);
//	g_MapScoketShutdownStatuMutex[s]->release();
//	if(nRet==SOCKET_ERROR)
//	{
//		int nError = WSAGetLastError();
//		if(nError!=ERROR_IO_PENDING)
//		{
//			printf("send error :%d socket %d error\n",s,nError);
//			delete ol;
//			return false;
//		}
//	}
//	return true;
//}
////用户处理读取事件
//void OnRead(SOCKET s,void* buf,int len)
//{
//
//	char strAddr[20];
//	UINT port;
//	GetSockName(s,strAddr,port);
//	printf("%s:%u say: %s\nlen=%d\n",strAddr,port,buf,len);
//	char time_[100];
//	time_t tm_t=time(NULL);
//	sprintf(time_,"%s\n",ctime(&(tm_t)));
//	return ;
//}
////内部处理成功写入数据
//void _OnWrite(SOCKET s,int len)
//{
//	if(s==0||len==0){return ;}
//	g_MapWriteBlockMutex[s]->acquire();
//	g_MapWriteBlock[s].read_skip(len);
//	g_MapWriteBlockMutex[s]->release();
//	AutoLock<Thread_Mutex> _auto((g_MapScoketStatuMutex[s]));
//	if(g_MapWriteBlock[s].length()==0)
//	{
//		g_MapScoketStatu[s]=true;
//		return ;
//	}
//	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
//
//
//	g_MapWriteBlockMutex[s]->acquire();
//
//	len=g_MapWriteBlock[s].length();
//	void *buf=(void*)g_MapWriteBlock[s].rd_ptr();
//
//	ol->wbuf.buf=new char[len];
//	memcpy(ol->wbuf.buf,buf,len);
//	ol->wbuf.len=len;
//
//	g_MapWriteBlockMutex[s]->release();
//
//
//	ol->opcode=1;
//	DWORD dwBytes=0,dwFlags=0;
//	memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
//	int nRet = WSASend(s,&(ol->wbuf),len,&dwBytes,dwFlags,&(ol->_overlapped),NULL);
//	if(nRet==SOCKET_ERROR)
//	{
//		int nError = WSAGetLastError();
//		if(nError!=ERROR_IO_PENDING)
//		{
//			//printf("send error :%d socket %d error\n",s,nError);
//			delete ol;
//			return ;
//		}
//	}
//	//char strAddr[20];
//	//UINT port;
//	//GetSockName(s,strAddr,port);
//	//printf("my say to %s:%u :%d len\n",strAddr,port,len);
//}
//
//
//void shutdown(SOCKET s)
//{
//	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
//	ol->wbuf.buf=ol->buffer;
//	ol->wbuf.len=0;
//	ol->opcode=2;
//	memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
//	DWORD dwBytes=0,dwFlags=0;
//	int nRet = WSARecv(s, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
//	if(nRet==SOCKET_ERROR)
//	{
//		int nError = WSAGetLastError();
//		if(nError!=ERROR_IO_PENDING)
//		{
//			delete ol;
//		}
//	}
//}
//
////用户处理关闭socket 事件
//
//void OnClose(SOCKET s)
//{
//	char strAddr[20];
//	UINT port;
//	GetSockName(s,strAddr,port);
//	printf("%d socket ip:%s:%u close\n",s,strAddr,port);
//}
//extern int main(int argc, char **argv)
//{
//	WSADATA wsaData;
//	WSAStartup(MAKEWORD(2,2), &wsaData );
//	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
//
//	SOCKET m_Server = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
//
//	SOCKADDR_IN addr;
//	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
//	addr.sin_family = AF_INET;
//	addr.sin_port = htons(5981);
//	bind(m_Server,(SOCKADDR*)&addr,sizeof(SOCKADDR));
//	listen(m_Server,5000);
//	sockaddr_in sockAddr = {0};
//	int len = sizeof(sockAddr);
//	SYSTEM_INFO SystemInfo;
//	GetSystemInfo(&SystemInfo);
//
//	for(int i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
//	{
//		HANDLE hProcessIO = CreateThread(NULL, 0, WorkProcess, NULL, 0, NULL);
//		if(!hProcessIO)
//		{
//			printf("create process fail\n");
//			return 0;
//		}
//
//	}
//	HANDLE hTestProcess=CreateThread(NULL, 0, TestProcess, NULL, 0, NULL);
//	if(!hTestProcess)
//	{
//		fprintf(stderr,"create test process fail\n");
//		return 0;
//	}
//	while(TRUE){
//		SOCKET sNew = WSAAccept(m_Server,(sockaddr*)&addr,&len,NULL,NULL);
//		// 将客户端加入到IOCP队列中
//		SOCKET_HANDLE * hand=new SOCKET_HANDLE();
//		hand->sClient=sNew;
//		CreateIoCompletionPort((HANDLE)sNew,m_hIocp,ULONG_PTR(hand),0);
//
//		Thread_Mutex* mutex_status=new Thread_Mutex();
//		g_MapScoketStatuMutex[sNew]=mutex_status;
//
//		g_MapScoketStatuMutex[sNew]->acquire();
//		g_MapScoketStatu[sNew]=true;
//		g_MapScoketStatuMutex[sNew]->release();
//
//		Thread_Mutex* shutdownMutex=new Thread_Mutex();
//		g_MapScoketShutdownStatuMutex[sNew]=shutdownMutex;
//
//		g_MapScoketShutdownStatuMutex[sNew]->acquire();
//		g_MapSocketShutdownStatus[sNew]=false;
//		g_MapScoketShutdownStatuMutex[sNew]->release();
//
//
//		Thread_Mutex* _blockMutex=new Thread_Mutex();
//		g_MapWriteBlockMutex[sNew]=_blockMutex;
//
//		g_MapWriteBlockMutex[sNew]->acquire();
//		g_MapWriteBlock[sNew].resize(1024*1024);
//		g_MapWriteBlock[sNew].clear();
//		g_MapWriteBlockMutex[sNew]->release();
//
//		
//
//		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
//		ol->wbuf.buf=ol->buffer;
//		ol->wbuf.len=sizeof(ol->buffer);
//		ol->opcode=0;
//		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
//		DWORD dwBytes=0,dwFlags=0;
//
//
//		int nRet = WSARecv(sNew, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
//		
//		if(nRet==SOCKET_ERROR)
//		{
//			int nError = WSAGetLastError();
//			if(nError!=ERROR_IO_PENDING)
//			{
//				break;
//			}
//		}
//	}
//	closesocket(m_Server);
//	g_teststop=true;
//	return 0;
//}
//
//DWORD WINAPI WorkProcess(LPVOID lpParam)
//{
//	 DWORD BytesTransferred;
//	 SOCKET_HANDLE* dwKey=NULL;
//
//	while(true)
//	{
//		OVERLAPPED* _ol;
//		DWORD err=GetQueuedCompletionStatus(m_hIocp, &BytesTransferred,(PULONG_PTR)&dwKey , (LPOVERLAPPED*)&(_ol), INFINITE);
//		if(0 == err||BytesTransferred==0)
//		{
//			  DWORD nError = GetLastError();
//			  OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
//			  int op=olplus->opcode;
//			  //delete olplus;
//			  //主动请求关闭
//			if(op==2)
//			{
//				bool flag=false;
//				g_MapScoketShutdownStatuMutex[dwKey->sClient]->acquire();
//				if(!g_MapSocketShutdownStatus[dwKey->sClient])
//				{
//					flag=true;
//					g_MapSocketShutdownStatus[dwKey->sClient]=true;
//				}
//				g_MapScoketShutdownStatuMutex[dwKey->sClient]->release();
//			
//				if(flag)
//				{
//					OnClose(dwKey->sClient);
//					closesocket(dwKey->sClient);
//				}
//					
//				continue;
//			}
//			 
//			else if( BytesTransferred==0||(nError == WAIT_TIMEOUT) || (nError == ERROR_NETNAME_DELETED) )
//			{
//				bool flag=false;
//				g_MapScoketShutdownStatuMutex[dwKey->sClient]->acquire();
//				if(!g_MapSocketShutdownStatus[dwKey->sClient])
//				{
//					flag=true;
//					g_MapSocketShutdownStatus[dwKey->sClient]=true;
//				}
//				g_MapScoketShutdownStatuMutex[dwKey->sClient]->release();
//			
//				if(flag)
//				{
//					OnClose(dwKey->sClient);
//					closesocket(dwKey->sClient);
//				}
//				
//				continue;
//			}
//			else
//			{
//				 printf("GetQueuedCompletionStatus failed!");
//				 return 0;
//			}
//		}
//		OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
//		//OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(&(_ol),OVERLAPPEDPLUS,ol);
//		switch(olplus->opcode)
//		{
//		case 0:
//			OnRead(dwKey->sClient,olplus->buffer,BytesTransferred);
//			break;
//		case 1:
//			
//			if(olplus->wbuf.len>BytesTransferred)
//			{
//				printf("%d socket 发送没有完成,本应发送%d字节，实际发送%d字节\n",olplus->wbuf.len,BytesTransferred);
//			}
//			else{
//			_OnWrite(dwKey->sClient,BytesTransferred);
//			}
//			delete[] olplus->wbuf.buf;
//			break;
//		case 2:
//			{
//				bool flag=false;
//				g_MapScoketShutdownStatuMutex[dwKey->sClient]->acquire();
//				if(!g_MapSocketShutdownStatus[dwKey->sClient])
//				{
//					flag=true;
//					g_MapSocketShutdownStatus[dwKey->sClient]=true;
//				}
//				g_MapScoketShutdownStatuMutex[dwKey->sClient]->release();
//				
//				if(flag)
//				{
//					OnClose(dwKey->sClient);
//					closesocket(dwKey->sClient);
//				}
//				
//				continue;
//			}
//			
//		}
//		delete olplus;
//		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
//		ol->wbuf.buf=ol->buffer;
//		ol->wbuf.len=sizeof(ol->buffer);
//		ol->opcode=0;
//		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
//		DWORD dwBytes=0,dwFlags=0;
//		int nRet = WSARecv(dwKey->sClient, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
//		if(nRet==SOCKET_ERROR)
//		{
//			int nError = WSAGetLastError();
//			if(nError!=ERROR_IO_PENDING)
//			{
//				delete ol;
//			}
//		}
//	}
//}