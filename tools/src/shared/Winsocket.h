#include <WinSock2.h>
#include <Windows.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "MessageBlock_new.h"
#include <set>

#include <boost/thread.hpp>
#include <map>
#include <queue>
#include "../thread/singleton.h"
class WinHandle;
typedef struct _OVERLAPPEDPLUS
{
	OVERLAPPED _overlapped;
	unsigned char opcode;
	boost::shared_ptr<WinHandle> _WinHandle;
	WSABUF wBuf;
	long alreadySendBytes;
}OVERLAPPEDPLUS;
class WinHandle:public boost::noncopyable
{
public:
	friend class WinHandleManage;
	WinHandle();
	HANDLE get_handle();
	bool open(HANDLE _handle);
	bool Write(void* buf,int len);
	virtual bool OnRead(void* buf,int len)=0;
	bool is_close();
	void close();
	~WinHandle();
protected:
	HANDLE m_handle;
	bool m_isClose;
	std::queue<MessageBlock_*> m_SendQueue;
	boost::mutex  m_writeMutex;

	virtual BOOL SysWriteFile(OVERLAPPEDPLUS* olpuls)=0;
	virtual BOOL SysReadFile(OVERLAPPEDPLUS* olpuls)=0;
private:
	void OnWrite(int wr_len);
};	



class WinHandleManage
{
public:
	//不是线程安全的....多个线程同时accept的情况 不予以考虑
	void AddSocket(boost::shared_ptr<WinHandle> newsock);
	void DelSocket(boost::shared_ptr<WinHandle> sock);
	void DelSocket(WinHandle* sock);

	void DelSocket(HANDLE _handle);
	bool Step();
	void unstep();
	DWORD WorkProcess();
	boost::shared_ptr<WinHandle> getSharedHandle(HANDLE _handle){
		return  m_HandleList.find(_handle)==m_HandleList.end() ? NULL: m_HandleList[_handle];
	}
	HANDLE GetIOCPHandle(){return m_hIOCP;}
private:
	std::map<HANDLE,boost::shared_ptr<WinHandle> > m_HandleList;
	HANDLE m_hIOCP;
	boost::recursive_mutex  m_Mutex;
	boost::thread_group  m_ThreadGourp;
};
#define sWinHandleManage Singleton<WinHandleManage,Thread_Mutex>::GetInstance() 