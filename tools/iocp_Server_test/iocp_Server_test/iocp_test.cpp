#include <WinSock2.h>
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <boost/shared_ptr.hpp>
#pragma comment(lib,"Ws2_32.lib")
HANDLE m_hIocp;

typedef struct _OVERLAPPEDPLUS
{
	OVERLAPPED _overlapped;
	unsigned char opcode;
	WSABUF wbuf;
	char buffer[1000];
}OVERLAPPEDPLUS;
typedef struct _SOCKET_HANDLE
{
	SOCKET sClient;
}SOCKET_HANDLE;
DWORD WINAPI WorkProcess(LPVOID lpParam);
BOOL GetSockName(SOCKET s, char* rSocketAddress, UINT& rSocketPort)
{
	SOCKADDR_IN sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	int nSockAddrLen = sizeof(sockAddr);
	int iResult = getsockname(s, (SOCKADDR*)&sockAddr, &nSockAddrLen);
	if (0 == iResult)
	{
		rSocketPort = ntohs(sockAddr.sin_port);
		char* t=inet_ntoa(sockAddr.sin_addr);
		strcpy_s(rSocketAddress,strlen(t)+1,t);
		return TRUE;
	}
	return FALSE;
}
bool Write(SOCKET s,void* buf,int len)
{
	OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
	ol->wbuf.buf=new char[len];
	memcpy(ol->wbuf.buf,buf,len);
	ol->wbuf.len=len;
	ol->opcode=1;
	DWORD dwBytes=0,dwFlags=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
	int nRet = WSASend(s,&(ol->wbuf),len,&dwBytes,dwFlags,&(ol->_overlapped),NULL);
	if(nRet==SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		if(nError!=ERROR_IO_PENDING)
		{
			printf("send error :%d socket %d error\n",s,nError);
			return false;
		}
	}
	return true;
}
void OnRead(SOCKET s,void* buf,int len)
{
	char strAddr[20];
	UINT port;
	GetSockName(s,strAddr,port);
	printf("%s:%u say: %s\nlen=%d\n",strAddr,port,buf,len);
	char time_[100];
	time_t tm_t=time(NULL);
	sprintf(time_,"%s\n",ctime(&(tm_t)));
	Write(s,time_,strlen(time_)+1);
	return ;
}
void OnWrite(SOCKET s,int len)
{
	char strAddr[20];
	UINT port;
	GetSockName(s,strAddr,port);
	printf("my say to %s:%u :%d len\n",strAddr,port,len);
}
void OnClose(SOCKET s)
{
	char strAddr[20];
	UINT port;
	GetSockName(s,strAddr,port);
	printf("%d socket ip:%s:%u close\n",s,strAddr,port);
}
int main( )
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData );
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);

	SOCKET m_Server = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5981);
	bind(m_Server,(SOCKADDR*)&addr,sizeof(SOCKADDR));
	listen(m_Server,5000);
	sockaddr_in sockAddr = {0};
	int len = sizeof(sockAddr);
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	for(int i = 0; i < SystemInfo.dwNumberOfProcessors * 2; i++)
	{
		HANDLE hProcessIO = CreateThread(NULL, 0, WorkProcess, NULL, 0, NULL);
		if(!hProcessIO)
		{
			printf("create process fail\n");
			return 0;
		}

	}
	while(TRUE){
		SOCKET sNew = WSAAccept(m_Server,(sockaddr*)&addr,&len,NULL,NULL);
		// 将客户端加入到IOCP队列中
		SOCKET_HANDLE * hand=new SOCKET_HANDLE();
		
		CreateIoCompletionPort((HANDLE)sNew,m_hIocp,ULONG_PTR(hand),0);
		// 这里要注意一下，对网上一些相关的资料，一些新手在写IOCP时
		// 会发现得不到用户请求及一些事件，哪是因为没有对该用户投递一个接收IO
		// IOCP的工作原理就是，必须对一个连接投递一个投收IO，处理完一个，再投递一个...
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
		ol->wbuf.buf=ol->buffer;
		ol->wbuf.len=sizeof(ol->buffer);
		ol->opcode=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		DWORD dwBytes=0,dwFlags=0;
		int nRet = WSARecv(sNew, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
		
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				break;
			}
		}
	}
	closesocket(m_Server);
	return 0;
}

DWORD WINAPI WorkProcess(LPVOID lpParam)
{
	 DWORD BytesTransferred;
	 SOCKET_HANDLE* dwKey=NULL;

	while(true)
	{
		OVERLAPPED* _ol;
		DWORD err=GetQueuedCompletionStatus(m_hIocp, &BytesTransferred,(PULONG_PTR)&dwKey , (LPOVERLAPPED*)&(_ol), INFINITE);
		if(0 == err||BytesTransferred==0)
		{
			  DWORD nError = GetLastError();
			if( BytesTransferred==0||(nError == WAIT_TIMEOUT) || (nError == ERROR_NETNAME_DELETED) )
			{
				closesocket(dwKey->sClient);
				OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
				delete olplus;
				continue;
			}
			else
			{
				 printf("GetQueuedCompletionStatus failed!");
				 return 0;
			}
		}
		OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(_ol,OVERLAPPEDPLUS,_overlapped);
		//OVERLAPPEDPLUS* olplus=CONTAINING_RECORD(&(_ol),OVERLAPPEDPLUS,ol);
		switch(olplus->opcode)
		{
		case 0:
			OnRead(dwKey->sClient,olplus->buffer,BytesTransferred);
			break;
		case 1:
			if(olplus->wbuf.len>BytesTransferred)
			{
				printf("%d socket 发送没有完成,本应发送%d字节，实际发送%d字节\n",olplus->wbuf.len,BytesTransferred);
				Write(dwKey->sClient,&(olplus->wbuf.buf[BytesTransferred]),olplus->wbuf.len-BytesTransferred);
			}
			else{
			OnWrite(dwKey->sClient,BytesTransferred);
			}
			delete[] olplus->wbuf.buf;
			break;
		case 2:
			OnClose(dwKey->sClient);
			closesocket(dwKey->sClient);
			break;
		}
		delete olplus;
		OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
		ol->wbuf.buf=ol->buffer;
		ol->wbuf.len=sizeof(ol->buffer);
		ol->opcode=0;
		memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		DWORD dwBytes=0,dwFlags=0;
		int nRet = WSARecv(dwKey->sClient, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				OnClose(dwKey->sClient);
				closesocket(dwKey->sClient);
				delete ol;
			}
		}
	}
}