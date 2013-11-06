#include "socket_test.h"

Learn_RealmSocket::Learn_RealmSocket():_session(0),input_buffer(1024*1024)
{
}
Learn_RealmSocket::~Learn_RealmSocket()
{
	m_closing_=true;
	if (_session)
	{
		delete _session;
	}
	closefd();
}

bool Learn_RealmSocket::recv(char* buf,size_t len)
{
	if(input_buffer.size()<len)
	{
		return false;
	}
	memcpy(buf,input_buffer.rd_ptr(),len);	
	return true;
}
size_t Learn_RealmSocket::recv_len()
{
	return input_buffer.length();
}
void Learn_RealmSocket::recv_skip(size_t len)
{
	input_buffer.rpos(len);
	return ;
}
bool Learn_RealmSocket::send(const char* buf,size_t len)
{
	if(buf==NULL||len==0)
		return true;
	Message_Block message_block(len);
	message_block.append(buf,len);
	if(isEmptymsg())
	{
		int n=noblk_send(message_block);
		if (n<0)
		{
			return false;
		}
		size_t un=size_t(n);
		if (un==len)
		{
			return true;
		}
		message_block.rpos(un);
	}
	Message_Block new_message_block;
	new_message_block.append(message_block.rd_ptr(),message_block.rd_size());
	putmsg(new_message_block);
	if (Reactor_()->schedule_wakeup(EVENT_HANDLE::WRITE_MASK,this)==-1)
	{
		return false;
	}
	return true;
}
int Learn_RealmSocket::handle_output()
{
	if(m_closing_)
		return 0;
	Message_Block block;
	if (isEmptymsg())
	{
		Reactor_()->cancel_wakeup(this,EVENT_HANDLE::WRITE_MASK);
		return 0;
	}
	getmsg(block);
	int32_t n=noblk_send(block);
	if (n<0)
	{
		return -1;
	}
	else if (size_t(n)==block.size())
	{
		return 1;
	}
	else
	{
		block.rpos(n);
		putmsg(block);
		return 0;
	}
	return -1;
}
int Learn_RealmSocket::handle_input()
{
	if (m_closing_)
	{
		return 0;
	}
	const uint32_t space=input_buffer.space();
	//printf("apace =%d,",space);
	int32_t n=recvfd(input_buffer.wr_ptr(),space);
	if (n<0)
	{
		return errno == EWOULDBLOCK ? 0 : -1;
	}
	else if (n==0)
	{
		return -1;
	}
	input_buffer.wpos(n);
	if (_session)
	{
		_session->OnRead();
		input_buffer.crunch();
	/*	input_buffer.base(NULL,0);
		input_buffer.reset();*/
		//printf("apace =%d \n",space);
	}
	return n == space ? 1 : 0 ;
}
void Learn_RealmSocket::set_session(Session* session)
{
	if (_session)
	{
		delete _session;
	}
	_session=session;
}
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

int Learn_RealmSocket::noblk_send(Message_Block &message_block)
{
	size_t length=message_block.size();
	if (length==0)
	{
		return -1;
	}
	int32_t n=sendfd(message_block.rd_ptr(),length);
	if (n<0)
	{
		if (n==EWOULDBLOCK)
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if (n==0)
	{
		return -1;
	}
	return n;
}

int Learn_RealmSocket::open()
{
	
	if (_session)
	{
		_session->OnAccept();
	}
	return 0;
}
int Learn_RealmSocket::close()
{
	shutdown();
	m_closing_=true;
	return 0;
}
int Learn_RealmSocket::handle_close()
{
	m_closing_=true;
	
	if (_session)
	{
		_session->OnClose();
	}
	//Reactor_()->clear_wakeup(this);
	shutdown();
	return 0;
}
DecodeSocket::DecodeSocket(Learn_RealmSocket* sh):_sh(sh)
{
		
}

void DecodeSocket::OnAccept()
{
	printf("start accept\n");
}

void DecodeSocket::OnRead()
{
	if (!_sh)
	{
		return ;
	}
	size_t n=_sh->recv_len();
	if (n<=0)
	{
		return ;
	}
	Message_Block block(n);
	int32_t len=_sh->recv((char*)block.wr_ptr(),n);
	if (len<=0)
	{
		return ; 
	} 
	_sh->recv_skip(n);
	std::string sendret;
	const char* ptr=block.rd_ptr();
	for (int i=0;i!=n;i++)
	{
		char rev[10];
		itoa(ptr[i],rev,10);
		sendret+=rev;
		sendret+=",";
	}
	_sh->send(sendret.c_str(),sendret.size());
	printf("recv : %s\n",sendret.c_str());
}
void DecodeSocket::OnClose()
{
	printf("%s close\n","clent");
}


extern int main(int argc, char **argv)
{

	//Sleep(10000);
#ifdef WIN32
	WSADATA wsaData;

	int Ret;

	// Initialize Winsock version 2.2

	if ((Ret = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0)
	{
		// NOTE: Since Winsock failed to load we cannot use 
		// WSAGetLastError to determine the specific error for
		// why it failed. Instead we can rely on the return 
		// status of WSAStartup.

		printf("WSAStartup failed with error %d\n", Ret);
		return 0;
	}

	// Setup Winsock communication code here 

	// When your application is finished call WSACleanup
	

#endif
	Reactor* r=new Reactor();
	struct addrinfo hints;

	
	//hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	//hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	//hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	//hints.ai_protocol = 0;          /* Any protocol */
	//hints.ai_canonname = NULL;
	//hints.ai_addr = NULL;
	//hints.ai_next = NULL;

	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0,sizeof( sockaddr_in));
	sock_addr.sin_port=htons(5981);
	sock_addr.sin_family=AF_INET;
	sock_addr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);
	
	ServerReactor acceptor;
	//addrinfo* bind_addr;
	//int err=getaddrinfo(NULL,"0",&hints,&bind_addr);
	/*if(0!=err)
	{
		printf("cannot get addrinfo:error %s",WSAGetLastError ());
		return 0;
	}*/
	
	if (acceptor.open(&sock_addr,r) == -1)
	{
		fprintf(stderr,"bind error");
	}
	acceptor.run_reactor_event_loop();
#ifdef WIN32
	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("WSACleanup failed with error %d\n", WSAGetLastError());
	}
#endif
	return 0;

};	