#include <WinSock2.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include "file.pb.h"
#include "dispatcher.h"
#pragma comment(lib,"Ws2_32.lib")

using namespace google::protobuf;
ProtobufDispatcher dispatcher;

uint32_t alreadyGuid=0;
uint32_t lostpacketCount=0;
uint32_t outOrderCount=0;

namespace ByteConverter
{
	template<size_t T>
	inline void convert(char *val)
	{
		std::swap(*val, *(val + T - 1));
		convert<T - 2>(val + 1);
	}
	template<> inline void convert<0>(char *) {}
	template<> inline void convert<1>(char *) {}            // ignore central byte

	template<typename T> inline void apply(T *val)
	{
		convert<sizeof(T)>((char *)(val));
	}
}
static union {   
	char c[4];   
	uint32_t mylong;   
} endian_test = {{ 'l', '?', '?', 'b' } };  
#define ENDIANNESS ((char)endian_test.mylong)

template<typename T> inline T& EndianConvert(T& val) 
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
int canread(SOCKET sock)
{
	if(m_complete)
	{
		memset (tempbuffer.buffer,0,sizeof(tempbuffer.buffer)/sizeof(tempbuffer.buffer[0]));
		delete[] tempbuffer.data;
		tempbuffer.data=NULL;
		tempData_len=0;
		recvsize=0;
		int val=recv(sock,tempbuffer.buffer,8,0);
		if(val<=0)
		{
			if(val!=0)
			closesocket(sock);
			return -1;
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
			EndianConvert(tempData_len);
			tempbuffer.data=new char[tempData_len];
			recvsize=8;
			val=recv(sock,tempbuffer.data,	tempData_len,0);
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
			int len=recv(sock,&(tempbuffer.buffer[recvsize]),8-recvsize,0);
			recvsize+=len;
			if(recvsize<8)
			{
				return len;
			}
			uint32_t * plen=(uint32_t *)&(tempbuffer.buffer[4]);
			tempData_len= *plen;
			EndianConvert(tempData_len);
			tempbuffer.data=new char[tempData_len];
		}
		int val=recv(sock,&(tempbuffer.data[recvsize-8]),tempData_len-(recvsize-8),0);
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
		Message* message=createMessage(_data->type);
		if(!message->ParsePartialFromArray(_data->bufferstream,EndianConvert(_data->length)-strlen(_data->type)-1))
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
int canwrite(SOCKET sock)
{
	if(writeBuffer==NULL||length==0)
	{
	   length=rand();
	   length=(length%100+100);
	   TestMessage message;
	   uint32_t setlenth=strlen("i am a man")+1;
	   uint32_t setwriteguid=++writeGuid;
	   message.set_guid(setwriteguid);
	   message.set_length(setlenth);
	   char* Data_byte=new char[length];
	   strcpy(Data_byte,"i am a man");
	   /* for (int i=0;i!=length;i++)
	   {
	   int tmp=rand();
	   tmp=tmp%256-128;
	   Data_byte[i]=tmp;
	   }*/
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
	   _writeBuffer->length=EndianConvert(alllength);
	   writeBuffer=new char[length];
	   uint32_t curp=sizeof(_writeBuffer->head);
	   memcpy(writeBuffer,&(_writeBuffer->head),curp);
	   uint32 netp=sizeof(_writeBuffer->length);
	   memcpy(writeBuffer+curp,&(_writeBuffer->length),netp);
	   strcpy(writeBuffer+curp+netp,_writeBuffer->type);
	   curp=curp+netp+strlen("TestMessage")+1;
	   memcpy(writeBuffer+curp,_writeBuffer->bufferstream,str.size());
	   delete[] _writeBuffer->bufferstream;
	   delete[] _writeBuffer->type;
	   delete _writeBuffer;
	   int ij=send(sock,writeBuffer,length,0);
	  //printf("already send %d length, max size %d length\n",ij,length);
	   if(ij<=0)
	   {
		   closesocket(sock);
		   return -1;
	   }
	   else if(ij>=length)
	   {
		   delete[] writeBuffer;
		   writeBuffer=0;
		   length=0;
		   sendlenth=0;
		   return ij;
	   }
	   else
	   {
		   sendlenth=ij;
	   }
	}
	else
	{
		if(sendlenth>=length)
		{
			delete[] writeBuffer;
			writeBuffer=0;
			length=0;
			sendlenth=0;
			return 0;
		}
		int ij=send(sock,writeBuffer+sendlenth,length-sendlenth,0);

		//printf("already send %d length, max size %d length\n",sendlenth,length);
		sendlenth+=ij;
		if(sendlenth>=length)
		{
			delete[] writeBuffer;
			writeBuffer=0;
			length=0;
			sendlenth=0;
			return 0;
		}
	}
}
int main()
{
	srand(time(NULL));
	dispatcher.registerMessageCallback<TestMessage>(OnTestMessage);
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData );

	SOCKET client=socket(AF_INET,SOCK_STREAM,0);
	SOCKADDR_IN addrClient;
	 memset(&addrClient,   0,   sizeof(addrClient));
	 addrClient.sin_family=AF_INET;
	addrClient.sin_addr.S_un.S_addr =inet_addr("127.0.0.1");
	addrClient.sin_port = htons(5981);
	if(connect(client,(SOCKADDR*)&addrClient,sizeof(SOCKADDR_IN))!=0)
	{
		printf("socket connect error  %d ",WSAGetLastError());
		return  0;
	}
	bool m_stop=false;
	fd_set  readfd,writefd,exfd;
	memset(&readfd,0,sizeof(readfd));
	
	memset(&writefd,0,sizeof(writefd));
	memset(&exfd,0,sizeof(exfd));
	unsigned long ul = 1; 
	ioctlsocket(client,FIONBIO,&ul);

	timeval timer_val;
	timer_val.tv_sec=0;
	timer_val.tv_usec=100;
	int _count=0;
	while(!m_stop)
	{
		FD_SET(client,&readfd);
		FD_SET(client,&writefd);
		FD_SET(client,&exfd);

		switch(select(client,&readfd,&writefd,&exfd,&timer_val))
		{
		case  -1:
			exit(-1);
		case 0:
			continue;
		default:
			{
				if(FD_ISSET(client,&readfd))
				{
					if(-1==canread(client))
					{
							m_stop=true;
							break;
					}
				}
				if(FD_ISSET(client,&writefd))
				{
					if(-1==canwrite(client))
					{
						m_stop=true;
						break;
					}
				}
			}
		}
		if(++_count%1000==0)
		{
			printf("总发送包数%d个,接收包数%d个,乱序包%d个，丢失包%d个",writeGuid,alreadyGuid,outOrderCount,lostpacketCount);
		}
	}
	return 0;
}