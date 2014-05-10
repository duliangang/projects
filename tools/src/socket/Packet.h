#ifndef _PACKET_H_
#define _PACKET_H_
#include <boost/function.hpp>
#include "file.pb.h"
#include "../shared/MessageBlock_new.h"
#include "../shared/Define.h"
#ifndef _STDINT
#define _STDINT
#include "../shared/pstdint.h"
#endif
#pragma pack(push,1)
struct PktHeader
{
	uint16_t size;
	uint16_t cmd;
};
#pragma pack(pop)

struct SendPacket
{
	SendPacket():data(NULL),size(0){}
	~SendPacket()
	{
		delete[] data;
		data=NULL;
	}
	char* data;
	int size;
};
enum PktParam
{
	PACKET_MAX_SIZE=10240,
	PACKET_MAX_CMD_SIZE=1000,
};

struct CallBack_
{
	uint16_t cmd;
	int (*pAPacketCallBackFun)(void* Par,char* data,int data_size);
};
class SocketPacket
{

public:
	enum PacketStatus
	{
		PACKET_STATUS_SUCCESS,
		PACKET_STATUS_BROKEN,
		PACKET_STATUS_NOT_COMPLETE,
	};
	SocketPacket(void* ProcessPar, CallBack_ * callBackFun,int callbackfun_size):m_HeaderPkt(new char[sizeof(PktHeader)],sizeof(PktHeader),true),
		m_RecvPct(new char[PACKET_MAX_SIZE],PACKET_MAX_SIZE,true)
	{
		for (int i=0;i!=callbackfun_size;i++)
		{
			m_CallBackFun[callBackFun[i].cmd]=callBackFun[i];
		}
		m_ProcessPar=ProcessPar;
	}
	void SetProcessPar(void* ProcessPar){m_ProcessPar=ProcessPar;}
	void* GetProcessPar(){return m_ProcessPar;}
	bool makePacket(const google::protobuf::Message* message,const uint16_t cmd,SendPacket& sendPacket)
	{
		assert(sendPacket.data==NULL&&sendPacket.size==0);
		uint16_t messageSize=sizeof(PktHeader);
		if(message!=NULL)
		{
			messageSize+=message->ByteSize();
		}
		sendPacket.data=new char[messageSize];
		sendPacket.size=messageSize;
		PktHeader newPktHeader;
		newPktHeader.cmd=EndianConvert(cmd);
		newPktHeader.size=EndianConvert(messageSize);
		memcpy(sendPacket.data,&newPktHeader,sizeof(PktHeader));
		if(message!=NULL)
		{
			message->SerializeToArray(sendPacket.data+sizeof(PktHeader),message->ByteSize());
		}
		return true;
	}
	PacketStatus ProcessData(char* data,int data_size)
	{
		MessageBlock_ message_block(data,data_size,false);
		message_block.skip_wr(data_size);
		while(message_block.length()>0)
		{
			if(m_HeaderPkt.space()>0)
			{
				const size_t to_header = (message_block.length() > m_HeaderPkt.space() ? m_HeaderPkt.space() : message_block.length());
				m_HeaderPkt.append(message_block.rd_ptr(),to_header);
				message_block.skip_rd(to_header);

				if(m_HeaderPkt.space()>0)
				{
					assert(message_block.length()==0);
					return PACKET_STATUS_SUCCESS;
				}
				assert(m_HeaderPkt.length() == sizeof(PktHeader));
				m_header=((PktHeader*)m_HeaderPkt.rd_ptr());
				PktHeader& _header = *m_header;
				EndianConvertReverse(_header.size);
				EndianConvertReverse(_header.cmd);

				if ((_header.size < 4) || (_header.size > PACKET_MAX_SIZE) || (_header.cmd > PACKET_MAX_CMD_SIZE))
				{
					return PACKET_STATUS_BROKEN;

				}
				_header.size -= 4;
				m_RecvPct.reset();
			}
			int pktspace=m_header->size - m_RecvPct.length();
			if(pktspace>0)
			{
				const size_t to_data = (message_block.length() > pktspace ? pktspace : message_block.length());
				m_RecvPct.append(message_block.rd_ptr(), to_data);
				message_block.skip_rd(to_data);
				if(m_RecvPct.length()!=m_header->size)
				{
					return PACKET_STATUS_SUCCESS;
				}
			}
			const int ret=m_CallBackFun[m_header->cmd].pAPacketCallBackFun(m_ProcessPar,m_RecvPct.rd_ptr(),m_RecvPct.length());
			m_RecvPct.reset();
			m_HeaderPkt.reset();
			m_header=NULL;
			if(ret==-1)
			{
				return PACKET_STATUS_BROKEN;
			}
		}
		return PACKET_STATUS_SUCCESS;
	}
private:
	MessageBlock_ m_RecvPct;
	MessageBlock_ m_HeaderPkt;
	PktHeader* m_header;
	void* m_ProcessPar;
	std::map<int, CallBack_> m_CallBackFun;
};
#endif