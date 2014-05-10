#include "MessageBlock_new.h"
#include <assert.h>

MessageBlock_::MessageBlock_(char* buffer,int buffer_size,bool auto_delete):m_buffer(buffer),m_buffer_size(buffer_size),m_auto_delete(auto_delete),m_rpos(0),m_wpos(0)
{
	
}
MessageBlock_::~MessageBlock_()
{
	if(m_auto_delete)
	{
		delete[] m_buffer;
		m_buffer=NULL;
	}
}

 int MessageBlock_::append(const char* data,int len)
 {
	 int can_append_len= m_buffer_size-m_wpos > len ?  len:m_buffer_size-m_wpos;
	 assert(can_append_len>=0);
	 if(can_append_len<=0)return 0;
	memcpy(m_buffer+m_wpos,data,can_append_len);
	m_wpos+=can_append_len;
	return can_append_len;
}
 void MessageBlock_::skip_wr(int write_len)
 {
	 m_wpos+=write_len;
 }
 void MessageBlock_::skip_rd(int write_len)
 {
	 assert(m_rpos+write_len<=m_wpos);
	 m_rpos+=write_len;
	if(m_rpos>m_wpos)m_rpos=m_wpos;
 }
 void MessageBlock_::reset()
 {
	 m_rpos=m_wpos=0;
 }

 void MessageBlock_::crunch()
 {
	 assert(m_wpos-m_rpos>=0);
	 if(m_rpos==0)return;
	 memmove(m_buffer,m_buffer+m_rpos,m_wpos-m_rpos);
 }
 void MessageBlock_::rfinish()
 {
	 m_rpos=m_wpos;
 }