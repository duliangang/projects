#include "Win32Socket_IOCP.h"

class CSock:public CWinSocket
{
public:
	CSock(SOCKET s):CWinSocket(s){}
	virtual void OnRead()
	{
		m_pReadBlockMutex.acquire();
		printf("read %d ",m_readBlock.length());
		for (int i=0;i!=m_readBlock.length();i++)
		{
			printf("%c ",m_readBlock[i]);
		}
		printf("\n");
		m_readBlock.read_skip(m_readBlock.length());
		m_readBlock.crunch();
		m_pReadBlockMutex.release();
		Write("收到\n",strlen("收到\n")+1);
	}

};


extern int main(int argc, char **argv)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData );
	//m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);

	SOCKET m_Server = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5981);
	bind(m_Server,(SOCKADDR*)&addr,sizeof(SOCKADDR));
	listen(m_Server,5000);
	sockaddr_in sockAddr = {0};
	int len = sizeof(sockAddr);
	/*SYSTEM_INFO SystemInfo;
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
	HANDLE hTestProcess=CreateThread(NULL, 0, TestProcess, NULL, 0, NULL);
	if(!hTestProcess)
	{
	fprintf(stderr,"create test process fail\n");
	return 0;
	}*/
	sCSocketMgr->Step();
	while(TRUE){
		SOCKET sNew = WSAAccept(m_Server,(sockaddr*)&addr,&len,NULL,NULL);
		CWinSocket* sock=new CSock(sNew);
		sCSocketMgr->AddSocket(sock);
		// 将客户端加入到IOCP队列中
		
		//SOCKET_HANDLE * hand=new SOCKET_HANDLE();
		//hand->sClient=sNew;
		//CreateIoCompletionPort((HANDLE)sNew,m_hIocp,ULONG_PTR(hand),0);

		//Thread_Mutex* mutex_status=new Thread_Mutex();
		//g_MapScoketStatuMutex[sNew]=mutex_status;

		//g_MapScoketStatuMutex[sNew]->acquire();
		//g_MapScoketStatu[sNew]=true;
		//g_MapScoketStatuMutex[sNew]->release();

		//Thread_Mutex* shutdownMutex=new Thread_Mutex();
		//g_MapScoketShutdownStatuMutex[sNew]=shutdownMutex;

		//g_MapScoketShutdownStatuMutex[sNew]->acquire();
		//g_MapSocketShutdownStatus[sNew]=false;
		//g_MapScoketShutdownStatuMutex[sNew]->release();


		//Thread_Mutex* _blockMutex=new Thread_Mutex();
		//g_MapWriteBlockMutex[sNew]=_blockMutex;

		//g_MapWriteBlockMutex[sNew]->acquire();
		//g_MapWriteBlock[sNew].resize(1024*1024);
		//g_MapWriteBlock[sNew].clear();
		//g_MapWriteBlockMutex[sNew]->release();

		//

		//OVERLAPPEDPLUS* ol=new OVERLAPPEDPLUS();// ....对OL结构进行初始化后
		//ol->wbuf.buf=ol->buffer;
		//ol->wbuf.len=sizeof(ol->buffer);
		//ol->opcode=0;
		//memset(&(ol->_overlapped),0,sizeof(ol->_overlapped));
		//DWORD dwBytes=0,dwFlags=0;


		//int nRet = WSARecv(sNew, &(ol->wbuf), 1, &(dwBytes), &(dwFlags), &(ol->_overlapped), NULL);
		//
		//if(nRet==SOCKET_ERROR)
		//{
		//	int nError = WSAGetLastError();
		//	if(nError!=ERROR_IO_PENDING)
		//	{
		//		break;
		//	}
		//}
	}
	closesocket(m_Server);
	sCSocketMgr->unstep();
	return 0;
}
