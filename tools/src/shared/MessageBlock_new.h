#ifndef _MESSAGE_BLOCK_NEW_H_
#define _MESSAGE_BLOCK_NEW_H_
#include <boost/noncopyable.hpp>
class MessageBlock_:boost::noncopyable
{
public:
	MessageBlock_( char* buffer,int buffer_size,bool auto_delete);
	~MessageBlock_();
	int MessageBlock_::size()const
	{
		return m_buffer_size;
	}
	int MessageBlock_::length()const 
	{
		return m_wpos-m_rpos;
	}
	int  MessageBlock_::space()const
	{
		return m_buffer_size-m_wpos;
	}
	char* rd_ptr()
	{
		if(m_rpos>=m_wpos||m_rpos>=m_buffer_size)
		{
			return NULL;
		}
		return m_buffer+m_rpos;
	}
	//inline const char* rd_ptr()const;
	 char* wr_prt()
	 {
		 if(m_wpos>=m_buffer_size||m_wpos<m_rpos)
		 {
			 return NULL;
		 }
		 return m_buffer+m_wpos;
	 }
	/*inline const char*  wr_ptr()const;*/
	int append(const char* data,int len);
	void skip_wr(int write_len);
	void skip_rd(int read_len);
	void reset();
	void crunch();
	void rfinish();
	void resize(int val);
protected:
	char* m_buffer;
	int m_buffer_size;
	bool m_auto_delete;
	int m_rpos;
	int m_wpos;
};
#endif