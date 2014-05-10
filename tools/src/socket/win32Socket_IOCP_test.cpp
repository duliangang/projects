
#include "../thread/base.h"
#include "../shared/Winsocket.h"
#include "../shared/MessageBlock_new.h"
#include <WinSock2.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include "Packet.h"
#include <stdio.h>
#include "file.pb.h"
#include "dispatcher.h"
#pragma comment(lib,"Ws2_32.lib")
class CSock;
Thread_Mutex g_read_mutex;
Thread_Mutex g_write_mutex;
class CWinTest;
using namespace google::protobuf;


Message* createMessage(const std::string& typeName)
{
	Message* message = NULL;
	const Descriptor* descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		const Message* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype)
		{
			message = prototype->New();
		}
	}
	return message;
}
int testfun(void *par,char* data,int data_size);
  CallBack_ callback[]={
	{1,testfun},
};


class CWinTest:public WinHandle
{
public:
	boost::mutex m_rLock;
	boost::mutex m_wLock;
protected:
	virtual BOOL SysReadFile(OVERLAPPEDPLUS* olpuls)
	{
		DWORD dwBytes=0,dwFlags=0;
		int nRet =WSARecv((SOCKET)get_handle(), &(olpuls->wBuf), 1, &(dwBytes), &(dwFlags), &(olpuls->_overlapped), NULL);
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				return FALSE;
			}
		}
		return true;
	}
	virtual BOOL SysWriteFile(OVERLAPPEDPLUS* olpuls)
	{
		DWORD dwBytes=0,dwFlags=0;
		int nRet =WSASend((SOCKET)get_handle(), &(olpuls->wBuf), 1, &(dwBytes), dwFlags, &(olpuls->_overlapped), NULL);
		if(nRet==SOCKET_ERROR)
		{
			int nError = WSAGetLastError();
			if(nError!=ERROR_IO_PENDING)
			{
				return FALSE;
			}
		}
		return true;
	}
public:
	uint32_t alreadyGuid;
	uint32_t lostpacketCount;
	uint32_t outOrderCount;
	uint32_t writeGuid;

	SocketPacket packet;
	CWinTest():packet(NULL,callback,sizeof(callback)/sizeof(callback[0])),alreadyGuid(0),lostpacketCount(0),outOrderCount(0),writeGuid(0)
	{
		packet.SetProcessPar(this);
	}
	virtual bool OnRead(void* buf,int len)
	{
		boost::mutex::scoped_lock(m_wLock);
		if(packet.ProcessData((char*)buf,len)==SocketPacket::PacketStatus::PACKET_STATUS_BROKEN)
		{
			this->close();
		}
		return true;
	}
};
extern int main(int argc, char **argv)
{

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData );
	//m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);

	SOCKET m_Server = WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN addr;
	addr.sin_addr.S_un.S_addr = inet_addr("10.5.220.21");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5981);
	::bind(m_Server,(SOCKADDR*)&addr,sizeof(SOCKADDR));
	listen(m_Server,5000);
	sockaddr_in sockAddr = {0};
	int len = sizeof(sockAddr);

	sWinHandleManage->Step();
	while(TRUE){
		SOCKET sNew = WSAAccept(m_Server,(sockaddr*)&addr,&len,NULL,NULL);
		boost::shared_ptr<CWinTest> testSOCK(new CWinTest());
		testSOCK->open((HANDLE)sNew);
		sWinHandleManage->AddSocket(testSOCK);
	}
	closesocket(m_Server);
	sWinHandleManage->unstep();
	return 0;
}
int testfun(void *par,char* data,int data_size)
{
	CWinTest* Hand=(CWinTest*)par;
	TestMessage t;
	if(!t.ParseFromArray(data,data_size))
	{
		return -1;
	}
	uint32_t curid= t.guid();
	
	if(curid==Hand->alreadyGuid+1)
	{
		Hand->alreadyGuid=curid;
	}
	else
	{
		if(curid > Hand->alreadyGuid+1)
		{
			Hand->lostpacketCount=Hand->lostpacketCount+curid-Hand->alreadyGuid+1;
			Hand->alreadyGuid=curid;
		}
		else
		{
			if(Hand->lostpacketCount!=0)
			{
				--Hand->lostpacketCount;
				++Hand->outOrderCount;
			}
		}
	}
	
	t.set_guid(++Hand->writeGuid);
	SendPacket spkt;
	Hand->packet.makePacket(&t,1,spkt);
	Hand->Write(spkt.data,spkt.size);
	return data_size;
}