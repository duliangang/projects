#include "CMySocket.h"

//单线程 接受消息
class Learn_RealmSocket:public SVC_Handler
{

public:

	Learn_RealmSocket();
	virtual ~Learn_RealmSocket();

	class Session
	{
	public:
		Session(){};
		~Session(){};
		virtual void OnRead(){};
		virtual void OnAccept(){};
		virtual void OnClose(){};
	};

	bool recv(char* buf,size_t len);
	size_t recv_len();
	void recv_skip(size_t len);
	bool send(const char* buf,size_t len);
	virtual int open(); 
	virtual int close();
	virtual int handle_input();
	virtual int handle_output();
	virtual int handle_close();
	void set_session(Session* session);
private:
	int noblk_send(Message_Block &message_block);
	Session* _session;
	Message_Block input_buffer;
};
class DecodeSocket:public Learn_RealmSocket::Session
{
public:
	DecodeSocket(Learn_RealmSocket* sh);
	~DecodeSocket(){};
protected:
	virtual void OnRead();
	virtual void OnAccept();
	virtual void OnClose();
	virtual std::string decode(std::string str){return str;}
	Learn_RealmSocket* _sh;
};
class ServerReactor:public svc_accept<Learn_RealmSocket>
{
public:
	ServerReactor():svc_accept<Learn_RealmSocket>(100)
	{

	}
	~ServerReactor()
	{

	}

protected:
	virtual int make_svc_handler(Learn_RealmSocket* &sh)
	{
		if (sh==NULL)
		{
			sh=new Learn_RealmSocket();
		}
	
		sh->Reactor_(Reactor_());
		sh->set_session(new DecodeSocket(sh));
		return 0;
	}

};