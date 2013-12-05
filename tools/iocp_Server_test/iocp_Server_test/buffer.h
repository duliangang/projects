#include <stdint.h>
class queue_buffer
{
public:
	queue_buffer(uint32_t size);
	uint32_t size()const;
	uint32_t space()const;//size-write
	uint32_t length()const;//write-read
	bool crunch(int n);
	int append(void* buffer,uint32_t len);
	int pop(void* buffer,int32_t len);
	bool empty()const;
	void clear();
	void* wr_ptr();
	const void const* rd_ptr()const;
	void read_skip(uint32_t skip);
	void rfinish();
	int32_t  rpos()const;
	int32_t  wpos()const;
protected:
	void resize(uint32_t size);
private:
	vector<char> m_list;
	int32_t _rpos;
	int32_t _wpos;
};