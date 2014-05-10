//#include "../shared/MessageBlock_new.h"
//#include "../thread/singleton.h"
//#include <boost/shared_ptr.hpp>
//#include <WinSock2.h>
//#include <Windows.h>
//#include <set>
//
//#define READ_SIZE 1024
//enum ServerOpCode
//{
//	OP_WRITE=0,
//	OP_READ=1,
//	OP_SHUTDOWN=2,
//	OP_END=3,
//};
//struct SOCKET_HANDLE
//{
//	SOCKET s;
//};
//
//typedef struct _OVERLAPPEDPLUS
//{
//	OVERLAPPED _overlapped;
//	unsigned char opcode;
//	WSABUF wbuf;
//}OVERLAPPEDPLUS;
//
//class CWinSocket;
//
//class CSocketMgr
//{
//public:
//	//不是线程安全的....多个线程同时accept的情况 不予以考虑
//	void AddSocket(boost::shared_ptr<CWinSocket> newsock);
//	void DelSocket(boost::shared_ptr<CWinSocket> sock);
//	bool Step(int procee_count=0);
//	void unstep();
//	
//	
//private:
//	static DWORD WINAPI WorkProcess(LPVOID lpParam);
//	static void shutdown(boost::shared_ptr<CWinSocket> sock);
//	static void OnWrite(boost::shared_ptr<CWinSocket>  sock,int len);
//	HANDLE* m_handleList;
//	static HANDLE m_hIOCP;
//	int m_nCount;
//	std::set<boost::shared_ptr<CWinSocket> > m_WinSocketList;
//};
//#define sCSocketMgr Singleton<CSocketMgr,Thread_Mutex>::GetInstance() 
//class CWinSocket
//{
//public:
//	friend class CSocketMgr;
//	CWinSocket(SOCKET s):m_writeBlock(1024*1024),m_readBlock(1024*1024),m_sock(s),m_writeStatus(true),m_shutdown(false)
//	{
//
//	}
//	SOCKET handle()
//	{
//		return m_sock;
//	}
//	virtual void OnClose()
//	{
//		return ;
//	}
//	virtual void OnRead()
//	{
//		return ;
//	}
//	bool Write(void* buf,int len);
//	void shutdown()
//	{
//		sCSocketMgr->DelSocket(this);
//	}
//	bool is_shutdown()
//	{
//		return m_shutdown;
//	}
//protected:
//	bool pushReadEvent();
//	MessageBlock_ m_writeBlock;
//	MessageBlock_ m_readBlock;
//	Thread_Mutex m_pWriteBlockMutex;
//	Thread_Mutex m_pReadBlockMutex;
//private:
//	CWinSocket(const CWinSocket&);
//	CWinSocket & operator=(const CWinSocket&);
//	
//	
//	bool m_writeStatus;
//	Thread_Mutex m_WriteStatusMutex;
//
//	uint32_t    m_shutdown;
//
//	SOCKET m_sock;
//};