
#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

#include "../log/log.h"

class MsgBlockException
{
public:
	MsgBlockException(size_t pos, size_t size, size_t valueSize)
		: Pos(pos), Size(size), ValueSize(valueSize)
	{
	}

protected:
	size_t Pos;
	size_t Size;
	size_t ValueSize;
};

class MsgBlockPositionException : public MsgBlockException
{
public:
	MsgBlockPositionException(bool add, size_t pos, size_t size, size_t valueSize)
		: MsgBlockException(pos, size, valueSize), _add(add)
	{
		PrintError();
	}

protected:
	void PrintError() const
	{

		sLog->outError(LOG_FILTER_NETWORK_IO, _T("Attempted to %s value with size: %d in ByteBuffer (pos: %d size: %d)\n"),
			(_add ? _T("put"): _T("get")), ValueSize, Pos, Size);
	}

private:
	bool _add;
};

class MsgBlockSourceException : public MsgBlockException
{
public:
	MsgBlockSourceException(size_t pos, size_t size, size_t valueSize)
		: MsgBlockException(pos, size, valueSize)
	{
		PrintError();
	}

protected:
	void PrintError() const
	{

		sLog->outError(LOG_FILTER_NETWORK_IO, _T("Attempted to put a %s in ByteBuffer (pos: %d size: %d)\n"),
			(ValueSize > 0 ? _T("NULL-pointer") :_T("zero-sized value")), Pos, Size);
	}
};

class Message_Block
{
public:
	const static size_t DEFAULT_SIZE = 0x1000;

	// constructor
	Message_Block(): _rpos(0), _wpos(0)
	{
		_storage.reserve(DEFAULT_SIZE);
	}

	// constructor
	Message_Block(size_t res): _rpos(0), _wpos(0)
	{
		_storage.resize(res);
		_storage.reserve(DEFAULT_SIZE);
	}
	Message_Block(size_t size,size_t reverse_size): _rpos(0), _wpos(0)
	{
		_storage.reserve(reverse_size);
		_storage.resize(size);
	}
	// copy constructor
	Message_Block(const Message_Block &buf): _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) { }

	void clear()
	{
		_rpos = _wpos = 0;
	}

	template <typename T> void append(T value)
	{
		EndianConvert(value);
		append((uint8_t *)&value, sizeof(value));
	}

	template <typename T> void put(size_t pos, T value)
	{
		EndianConvert(value);
		put(pos, (uint8_t *)&value, sizeof(value));
	}

	Message_Block &operator<<(uint8_t value)
	{
		append<uint8_t>(value);
		return *this;
	}

	Message_Block &operator<<(uint16_t value)
	{
		append<uint16_t>(value);
		return *this;
	}

	Message_Block &operator<<(uint32_t value)
	{
		append<uint32_t>(value);
		return *this;
	}

	Message_Block &operator<<(uint64_t value)
	{
		append<uint64_t>(value);
		return *this;
	}

	// signed as in 2e complement
	Message_Block &operator<<(int8_t value)
	{
		append<int8_t>(value);
		return *this;
	}

	Message_Block &operator<<(int16_t value)
	{
		append<int16_t>(value);
		return *this;
	}

	Message_Block &operator<<(int32_t value)
	{
		append<int32_t>(value);
		return *this;
	}

	Message_Block &operator<<(int64_t value)
	{
		append<int64_t>(value);
		return *this;
	}

	// floating points
	Message_Block &operator<<(float value)
	{
		append<float>(value);
		return *this;
	}

	Message_Block &operator<<(double value)
	{
		append<double>(value);
		return *this;
	}

	Message_Block &operator<<(const std::string &value)
	{
		if (size_t len = value.length())
			append((uint8_t const*)value.c_str(), len);
		append((uint8_t)0);
		return *this;
	}

	Message_Block &operator<<(const char *str)
	{
		if (size_t len = (str ? strlen(str) : 0))
			append((uint8_t const*)str, len);
		append((uint8_t)0);
		return *this;
	}
	Message_Block &operator<<(const std::wstring &value)
	{
		if (size_t len = value.length())
			append((uint8_t const*)value.c_str(), len*2);
		append((uint16_t)0);
		return *this;
	}

	Message_Block &operator<<(const wchar_t *str)
	{
		if (size_t len = (str ? wcslen(str) : 0))
			append((uint8_t const*)str, len*2);
		append((uint16_t)0);
		return *this;
	}
	Message_Block &operator>>(bool &value)
	{
		value = read<char>() > 0 ? true : false;
		return *this;
	}

	Message_Block &operator>>(uint8_t &value)
	{
		value = read<uint8_t>();
		return *this;
	}

	Message_Block &operator>>(uint16_t &value)
	{
		value = read<uint16_t>();
		return *this;
	}

	Message_Block &operator>>(uint32_t &value)
	{
		value = read<uint32_t>();
		return *this;
	}

	Message_Block &operator>>(uint64_t &value)
	{
		value = read<uint64_t>();
		return *this;
	}

	//signed as in 2e complement
	Message_Block &operator>>(int8_t &value)
	{
		value = read<int8_t>();
		return *this;
	}

	Message_Block &operator>>(int16_t &value)
	{
		value = read<int16_t>();
		return *this;
	}

	Message_Block &operator>>(int32_t &value)
	{
		value = read<int32_t>();
		return *this;
	}

	Message_Block &operator>>(int64_t &value)
	{
		value = read<int64_t>();
		return *this;
	}

	Message_Block &operator>>(float &value)
	{
		value = read<float>();
		return *this;
	}

	Message_Block &operator>>(double &value)
	{
		value = read<double>();
		return *this;
	}

	Message_Block &operator>>(std::string& value)
	{
		value.clear();
		while (rpos() < size())                         // prevent crash at wrong string format in packet
		{
			char c = read<char>();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}
	Message_Block &operator>>(std::wstring& value)
	{
		value.clear();
		while (rpos() < size())                         // prevent crash at wrong string format in packet
		{
			wchar_t c=  read<wchar_t>();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}
	uint8_t operator[](size_t pos) const
	{
		return read<uint8_t>(pos);
	}

	size_t rpos() const { return _rpos; }

	size_t rpos(size_t rpos_)
	{
		_rpos = rpos_;
		return _rpos;
	}
	size_t rd_size()const {return _wpos-_rpos;}
	void rfinish()
	{
		_rpos = wpos();
	}
	void crunch()
	{
		if(_wpos<=_rpos)
		{
			clear();
			return ;
		}
		size_t i=_wpos;
		_wpos=0;
		append(rd_ptr(),i-_rpos);
		_rpos=0;
	}
	size_t wpos() const { return _wpos; }

	size_t wpos(size_t wpos_)
	{
		_wpos = wpos_;
		return _wpos;
	}

	template<typename T>
	void read_skip() { read_skip(sizeof(T)); }

	void read_skip(size_t skip)
	{
		if (_rpos + skip > size())
			throw MsgBlockPositionException(false, _rpos, skip, size());
		_rpos += skip;
	}

	template <typename T> T read()
	{
		T r = read<T>(_rpos);
		_rpos += sizeof(T);
		return r;
	}

	template <typename T> T read(size_t pos) const
	{
		if (pos + sizeof(T) > size())
			throw MsgBlockPositionException(false, pos, sizeof(T), size());
		T val = *((T const*)&_storage[pos]);
		EndianConvert(val);
		return val;
	}

	void read(uint8_t *dest, size_t len)
	{
		if (_rpos  + len > size())
			throw MsgBlockPositionException(false, _rpos, len, size());
		memcpy(dest, &_storage[_rpos], len);
		_rpos += len;
	}

	void readPackGUID(uint64_t& guid)
	{
		if (rpos() + 1 > size())
			throw MsgBlockPositionException(false, _rpos, 1, size());

		guid = 0;

		uint8_t guidmark = 0;
		(*this) >> guidmark;

		for (int i = 0; i < 8; ++i)
		{
			if (guidmark & (uint8_t(1) << i))
			{
				if (rpos() + 1 > size())
					throw MsgBlockPositionException(false, _rpos, 1, size());

				uint8_t bit;
				(*this) >> bit;
				guid |= (uint64_t(bit) << (i * 8));
			}
		}
	}

	uint32_t ReadPackedTime()
	{
		uint32_t packedDate = read<uint32_t>();
		tm lt;
		memset(&lt, 0, sizeof(lt));

		lt.tm_min = packedDate & 0x3F;
		lt.tm_hour = (packedDate >> 6) & 0x1F;
		//lt.tm_wday = (packedDate >> 11) & 7;
		lt.tm_mday = ((packedDate >> 14) & 0x3F) + 1;
		lt.tm_mon = (packedDate >> 20) & 0xF;
		lt.tm_year = ((packedDate >> 24) & 0x1F) + 100;

		return uint32_t(mktime(&lt) + timezone);
	}

	Message_Block& ReadPackedTime(uint32_t& time)
	{
		time = ReadPackedTime();
		return *this;
	}

	uint8_t * contents() { return &_storage[0]; }

	const uint8_t *contents() const { return &_storage[0]; }

	const char* rd_ptr()const
	{
		if(_rpos==_storage.size())return NULL;
		return (const char*)&(_storage[_rpos]);
	}
	const char* rd_ptr()
	{
		if(_rpos==_storage.size())return NULL;
		return ( const char*)&(_storage[_rpos]);
	}
	char*   wr_ptr()
	{
		if(_wpos==_storage.size())return NULL;
		return (char*)&(_storage[_wpos]);
	}
	uint32_t space()const {return _storage.size()-_wpos > 0 ?  _storage.size()-_wpos :0;}
	size_t size() const { return _storage.size(); }
	size_t length()const {return _wpos-_rpos;}
	bool empty() const { return _storage.empty(); }

	void resize(size_t newsize)
	{
		_storage.resize(newsize, 0);
		_rpos = 0;
		_wpos = size();
	}

	void reserve(size_t ressize)
	{
		if (ressize > size())
			_storage.reserve(ressize);
	}

	void append(const char *src, size_t cnt)
	{
		return append((const uint8_t *)src, cnt);
	}

	void append(const wchar_t *src, size_t cnt)
	{
		return append((const uint8_t *)src, cnt*2);
	}

	template<class T> void append(const T *src, size_t cnt)
	{
		return append((const uint8_t *)src, cnt * sizeof(T));
	}

	void append(const uint8_t *src, size_t cnt)
	{
		if (!cnt)
			throw MsgBlockSourceException(_wpos, size(), cnt);

		if (!src)
			throw MsgBlockSourceException(_wpos, size(), cnt);


		if (_storage.size() < _wpos + cnt)
			_storage.resize(_wpos + cnt);
		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;
	}

	void append(const Message_Block& buffer)
	{
		if (buffer.wpos())
			append(buffer.contents(), buffer.wpos());
	}

	// can be used in SMSG_MONSTER_MOVE opcode


	void appendPackGUID(uint64_t guid)
	{
		uint8_t packGUID[8+1];
		packGUID[0] = 0;
		size_t size = 1;
		for (uint8_t i = 0;guid != 0;++i)
		{
			if (guid & 0xFF)
			{
				packGUID[0] |= uint8_t(1 << i);
				packGUID[size] =  uint8_t(guid & 0xFF);
				++size;
			}

			guid >>= 8;
		}
		append(packGUID, size);
	}

	void AppendPackedTime(time_t time)
	{
		tm* lt = localtime(&time);
		append<uint32_t>((lt->tm_year - 100) << 24 | lt->tm_mon  << 20 | (lt->tm_mday - 1) << 14 | lt->tm_wday << 11 | lt->tm_hour << 6 | lt->tm_min);
	}

	void put(size_t pos, const uint8_t *src, size_t cnt)
	{
		if (pos + cnt > size())
			throw MsgBlockPositionException(true, pos, cnt, size());

		if (!src)
			throw MsgBlockSourceException(_wpos, size(), cnt);

		memcpy(&_storage[pos], src, cnt);
	}

	void print_storage() const
	{

		std::_tostringstream o;
		o << "STORAGE_SIZE: " << size();
		for (uint32_t i = 0; i < size(); ++i)
			o << read<uint8_t>(i) << " - ";
		o << " ";

		sLog->outTrace(LOG_FILTER_NETWORK_IO, _T("%s"), o.str().c_str());
	}

	void textlike() const
	{
		std::_tostringstream o;
		o << "STORAGE_SIZE: " << size();
		for (uint32_t i = 0; i < size(); ++i)
		{
			_tchar buf[1];
			_tsnprintf(buf, 1, _T("%c"), read<uint8_t>(i));
			o << buf;
		}
		o << _T(" ");
		sLog->outTrace(LOG_FILTER_NETWORK_IO, _T("%s"), o.str().c_str());
	}

	void hexlike() const
	{

		uint32_t j = 1, k = 1;

		std::_tostringstream o;
		o << _T("STORAGE_SIZE: ") << size();

		for (uint32_t i = 0; i < size(); ++i)
		{
			_tchar buf[3];
			_tsnprintf(buf, 1, _T("%2X "), read<uint8_t>(i));
			if ((i == (j * 8)) && ((i != (k * 16))))
			{
				o <<_T( "| ");
				++j;
			}
			else if (i == (k * 16))
			{
				o << _T("\n");
				++k;
				++j;
			}

			o << buf;
		}
		o << _T(" ");
		sLog->outTrace(LOG_FILTER_NETWORK_IO, _T("%s"), o.str().c_str());
	}

protected:
	size_t _rpos, _wpos;
	std::vector<uint8_t> _storage;
};

template <typename T>
inline Message_Block &operator<<(Message_Block &b, std::vector<T> v)
{
	b << (uint32_t)v.size();
	for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i)
	{
		b << *i;
	}
	return b;
}

template <typename T>
inline Message_Block &operator>>(Message_Block &b, std::vector<T> &v)
{
	uint32_t vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename T>
inline Message_Block &operator<<(Message_Block &b, std::list<T> v)
{
	b << (uint32_t)v.size();
	for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i)
	{
		b << *i;
	}
	return b;
}

template <typename T>
inline Message_Block &operator>>(Message_Block &b, std::list<T> &v)
{
	uint32_t vsize;
	b >> vsize;
	v.clear();
	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename K, typename V>
inline Message_Block &operator<<(Message_Block &b, std::map<K, V> &m)
{
	b << (uint32_t)m.size();
	for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i)
	{
		b << i->first << i->second;
	}
	return b;
}

template <typename K, typename V>
inline Message_Block &operator>>(Message_Block &b, std::map<K, V> &m)
{
	uint32_t msize;
	b >> msize;
	m.clear();
	while (msize--)
	{
		K k;
		V v;
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

/// @todo Make a ByteBuffer.cpp and move all this inlining to it.
template<> inline std::string Message_Block::read<std::string>()
{
	std::string tmp;
	*this >> tmp;
	return tmp;
}
template<> inline std::wstring Message_Block::read<std::wstring>()
{
	std::wstring tmp;
	*this >> tmp;
	return tmp;
}
template<>
inline void Message_Block::read_skip<char*>()
{
	std::string temp;
	*this >> temp;
}
template<>
inline void Message_Block::read_skip<wchar_t*>()
{
	std::wstring temp;
	*this >> temp;
}
template<>
inline void Message_Block::read_skip<char const*>()
{
	read_skip<char*>();
}
template<>
inline void Message_Block::read_skip<wchar_t const*>()
{
	read_skip<wchar_t*>();
}
template<>
inline void Message_Block::read_skip<std::string>()
{
	read_skip<char*>();
}
template<>
inline void Message_Block::read_skip<std::wstring>()
{
	read_skip<wchar_t*>();
}
#endif
