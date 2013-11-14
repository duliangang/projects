#include "Win32Socket_IOCP.h"


#include <WinSock2.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include "file.pb.h"
#include "dispatcher.h"
#pragma comment(lib,"Ws2_32.lib")
class CSock;
Thread_Mutex g_read_mutex;
Thread_Mutex g_write_mutex;
int canwrite(SOCKET sock,CSock* s);
int canread(SOCKET sock,CSock* s);
class CSock:public CWinSocket
{
public:
	CSock(SOCKET s):CWinSocket(s){}
	int recv(void* buffer,uint32_t size)
	{
		m_pReadBlockMutex.acquire();
		if(is_shutdown())
		{
			m_pReadBlockMutex.release();
			return  0;
		}
		if(m_readBlock.space()<size)
		{
			size=m_readBlock.space();
		}
		memcpy(buffer,m_readBlock.rd_ptr(),size);
		m_readBlock.read_skip(size);
		m_readBlock.crunch();
		m_pReadBlockMutex.release();
		return size;

	}
	virtual void OnRead()
	{
		canread(this->handle(),this);
		canwrite(this->handle(),this);
	}

};






ProtobufDispatcher dispatcher;

uint32_t alreadyGuid=0;
uint32_t lostpacketCount=0;
uint32_t outOrderCount=0;


//static union {   
//	char c[4];   
//	uint32_t mylong;   
//} endian_test = {{ 'l', '?', '?', 'b' } };  
#define ENDIANNESS ((char)endian_test.mylong)

template<typename T> inline T& MyEndianConvert(T& val) 
{
	if(ENDIANNESS=='l')
	{
		ByteConverter::apply<T>(&val); 
	}
	return val;
}


void OnTestMessage(TestMessage* query)
{
	uint32_t curid= query->guid();
	if(curid==alreadyGuid+1)
	{
		alreadyGuid=curid;
		printf("%u\n",alreadyGuid);
		return ;
	}
	else
	{
		if(curid>alreadyGuid+1)
		{
			lostpacketCount=lostpacketCount+curid-alreadyGuid+1;
			alreadyGuid=curid;
		}
		else
		{
			if(lostpacketCount!=0)
			{
				--lostpacketCount;
				++outOrderCount;
			}
			return ;
		}
	}
}
 google::protobuf::Message* createMessage(const std::string& typeName)
{
	 google::protobuf::Message* message = NULL;
	const  google::protobuf::Descriptor* descriptor =google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		const google::protobuf::Message* prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype)
		{
			message = prototype->New();
		}
	}
	return message;
}


char buffer[1000];
#pragma pack(1) 
struct Data
{
	Data():head(0xffffffff){}
	uint32_t head;
	uint32_t  length;
	char* type;
	unsigned char *bufferstream;
};
struct tempData
{
	char buffer[sizeof(uint32_t)*2];
	char* data;
};
#pragma pop()

tempData tempbuffer;
bool m_complete=true;
int32_t recvsize=0;
uint32_t tempData_len=0;
int canread(SOCKET sock,CSock* s)
{
	AutoLock<Thread_Mutex> __lock(&g_read_mutex);
	if(m_complete)
	{
		memset (tempbuffer.buffer,0,sizeof(tempbuffer.buffer)/sizeof(tempbuffer.buffer[0]));
		delete[] tempbuffer.data;
		tempbuffer.data=NULL;
		tempData_len=0;
		recvsize=0;
		int val=s->recv(tempbuffer.buffer,8);
		if(val<0)
		{
			if(val!=0)
				closesocket(sock);
			return -1;
		}
		else if(val==0)
		{
			return 0;
		}
		else if(val<8)
		{
			m_complete=false;
			recvsize=val;
			return recvsize;
		}

		m_complete=false;
		uint32_t * head=(uint32_t *)&(tempbuffer.buffer[0]);
		if((*head)!=0xffffffff)
		{
			closesocket(sock);
			printf("error packet");
			return -1;
		}
		uint32_t * len=(uint32_t *)&(tempbuffer.buffer[4]);
		tempData_len= *len;
		MyEndianConvert(tempData_len);
		tempbuffer.data=new char[tempData_len];
		recvsize=8;
		val=s->recv(tempbuffer.data,	tempData_len);
		if(val<0)
		{
			closesocket(sock);
			return -1;
		}
		recvsize+=val;
	}
	else
	{
		if(recvsize<8)
		{
			int len=s->recv(&(tempbuffer.buffer[recvsize]),8-recvsize);
			recvsize+=len;
			if(recvsize<8)
			{
				return len;
			}
			uint32_t * plen=(uint32_t *)&(tempbuffer.buffer[4]);
			tempData_len= *plen;
			MyEndianConvert(tempData_len);
			tempbuffer.data=new char[tempData_len];
		}
		int val=s->recv(&(tempbuffer.data[recvsize-8]),tempData_len-(recvsize-8));
		if(val<0)
		{
			closesocket(sock);
			return -1;
		}
		recvsize+=val;
	}
	if(recvsize==tempData_len+8)
	{
		m_complete=true;
		Data* _data=new Data();
		_data->head=*(uint32_t*)tempbuffer.buffer;
		_data->length=*(uint32_t*)(&(tempbuffer.buffer[4]));
		_data->type=tempbuffer.data;
		_data->bufferstream=(unsigned char*)&(tempbuffer.data[strlen(_data->type)+1]);
		(Data*) &tempbuffer;
		google::protobuf::Message* message=createMessage(_data->type);
		if(!message)
		{
			printf("recv error");
			return 0;
		}
		if(!message->ParsePartialFromArray(_data->bufferstream,MyEndianConvert(_data->length)-strlen(_data->type)-1))
		{
			printf("error packet");
			return -1;
		}
		dispatcher.onMessage(message);
		delete _data;
		return 0;
	}
	return recvsize;
}
char* writeBuffer=0;
int length=0;
int sendlenth=0;
uint32_t writeGuid=0;
int canwrite(SOCKET sock,CSock* s)
{
	AutoLock<Thread_Mutex> __lock(&g_write_mutex);
	if(writeBuffer==NULL||length==0)
	{
		length=rand();
		length=(length%100+100);
		TestMessage message;
		uint32_t setlenth=length;
		uint32_t setwriteguid=++writeGuid;
		message.set_guid(setwriteguid);
		message.set_length(setlenth);
		char* Data_byte=new char[length];
		for (int i=0;i!=length;i++)
		{
			int tmp=rand();
			tmp=tmp%256-128;
			Data_byte[i]=tmp;
		}
		message.set_data(Data_byte,length);
		delete[] Data_byte;
		std::string str;
		message.SerializeToString(&str);
		Data* _writeBuffer=new Data();
		_writeBuffer->type=new char[strlen("TestMessage")+1];
		strcpy(_writeBuffer->type,"TestMessage");
		_writeBuffer->bufferstream=new unsigned char[str.size()];
		memcpy(_writeBuffer->bufferstream,&(str[0]),str.size());
		uint32_t alllength=strlen("TestMessage")+1+str.size();
		length=alllength+sizeof(uint32_t)*2;
		_writeBuffer->length=MyEndianConvert(alllength);
		writeBuffer=new char[length];
		uint32_t curp=sizeof(_writeBuffer->head);
		memcpy(writeBuffer,&(_writeBuffer->head),curp);
		uint32_t netp=sizeof(_writeBuffer->length);
		memcpy(writeBuffer+curp,&(_writeBuffer->length),netp);
		strcpy(writeBuffer+curp+netp,_writeBuffer->type);
		curp=curp+netp+strlen("TestMessage")+1;
		memcpy(writeBuffer+curp,_writeBuffer->bufferstream,str.size());
		delete[] _writeBuffer->bufferstream;
		delete[] _writeBuffer->type;
		delete _writeBuffer;
		s->Write(writeBuffer,length);
		delete[] writeBuffer;
		writeBuffer=0;
		length=0;
		sendlenth=0;
		return 0;
	}
	return 0;
}


extern int main(int argc, char **argv)
{
	dispatcher.registerMessageCallback<TestMessage>(OnTestMessage);
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

	sCSocketMgr->Step();
	while(TRUE){
		SOCKET sNew = WSAAccept(m_Server,(sockaddr*)&addr,&len,NULL,NULL);
		CWinSocket* sock=new CSock(sNew);
		sCSocketMgr->AddSocket(sock);

	}
	closesocket(m_Server);
	sCSocketMgr->unstep();
	return 0;
}
