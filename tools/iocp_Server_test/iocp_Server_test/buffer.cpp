#include "buffer.h"
#include <stdio.h>
#include <string>
queue_buffer::queue_buffer(uint32_t size)
{
	m_list.resize(size,0);
	_wpos=_rpos=0;
}
	uint32_t queue_buffer::size()const
	{
		return m_list.size();
	}
	uint32_t queue_buffer::space()const//size-write
	{
		if(_wpos>=_rpos)
		return(m_list.size()-_wpos)+_rpos;
		else
		return (_rpos-_wpos-1);
	}
	uint32_t queue_buffer::length()const//write-read
	{
		if(_wpos>_rpos)
		return (_wpos-_rpos);
		else
			return (m_list.size()-_rpos)+_wpos
	}
	bool queue_buffer::crunch(int n)
	{
		if(_wpos-_rpos<n)
		{
			_rpos=_wpos;
			return false;
		}
		else
		{
			_rpos+=n;
		}
		return  true;
	}
	int queue_buffer::append(void* buffer,uint32_t len)
	{
		if(len>space())
			len=space();
		int b=0;
		if(_wpos>=_rpos)
		{
			 b=m_list.size()-_wpos;
		}
		else
		{
			b=_rpos-_wpos;
		}
		if(len>b)
		{
			memcpy(&(m_list[_wpos]),buffer,b);
			memcpy(&(m_list[0]),(char*)buffer+b,len-b);
			_wpos=len-b;
		}
		else
		{
			memcpy(&(m_list[_wpos]),buffer,len);
			_wpos+=len;
		}
		return len;
	}
int queue_buffer::pop(void* buffer,int32_t len)
{
	
}
//	bool empty()const;
//	void clear();
//	void* wr_ptr();
//	const void const* rd_ptr()const;
//	void read_skip(uint32_t skip);
//	void rfinish();
//	int32_t  rpos()const;
//	int32_t  wpos()const;
//protected:
//	void resize(uint32_t size);
//private:
//	vector<char> m_list;
//	int32_t _rpos;
//	int32_t _wpos;
//};