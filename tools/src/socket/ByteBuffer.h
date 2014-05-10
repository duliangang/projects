#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H
#include "../log/log.h"
#include <vector>
#include <string>
#include <list>
#include <vector>
#include "../shared/pstdint.h"

class ByteBufferException : public std::exception
{
public:
	~ByteBufferException() throw() { }

	char const* what() const throw() { return msg_.c_str(); }

protected:
	std::string & message() throw() { return msg_; }

private:
	std::string msg_;
};

class ByteBufferPositionException : public ByteBufferException
{
public:
	ByteBufferPositionException(bool add, size_t pos, size_t size, size_t valueSize);

	~ByteBufferPositionException() throw() { }
};

class ByteBufferSourceException : public ByteBufferException
{
public:
	ByteBufferSourceException(size_t pos, size_t size, size_t valueSize);

	~ByteBufferSourceException() throw() { }
};
class ByteBuffer {
public:
	const static size_t DEFAULT_SIZE = 0x1000;

	// constructor
	ByteBuffer() :
	_rpos(0), _wpos(0) {
		_storage.reserve(DEFAULT_SIZE);
	}

	// constructor
	ByteBuffer(size_t res) :
	_rpos(0), _wpos(0) {
		_storage.reserve(res);
	}

	// copy constructor
	ByteBuffer(const ByteBuffer &buf) :
	_rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage) {
	}

	void clear() {
		_storage.clear();
		_rpos = _wpos = 0;
	}

	template<typename T> void append(T value) {

		append((UINT8 *) &value, sizeof(value));
	}

	template<typename T> void put(size_t pos, T value) {

		put(pos, (UINT8 *) &value, sizeof(value));
	}

	ByteBuffer &operator<<(UINT8 value) {
		append<UINT8>(value);
		return *this;
	}

	ByteBuffer &operator<<(UINT16 value) {
		append<UINT16>(value);
		return *this;
	}

	ByteBuffer &operator<<(UINT32 value) {
		append<UINT32>(value);
		return *this;
	}

	ByteBuffer &operator<<(UINT64 value) {
		append<UINT64>(value);
		return *this;
	}

	// signed as in 2e complement
	ByteBuffer &operator<<(INT8 value) {
		append<INT8>(value);
		return *this;
	}

	ByteBuffer &operator<<(INT16 value) {
		append<INT16>(value);
		return *this;
	}

	ByteBuffer &operator<<(INT32 value) {
		append<INT32>(value);
		return *this;
	}

	ByteBuffer &operator<<(INT64 value) {
		append<INT64>(value);
		return *this;
	}

	// floating points
	ByteBuffer &operator<<(float value) {
		append<float>(value);
		return *this;
	}

	ByteBuffer &operator<<(double value) {
		append<double>(value);
		return *this;
	}

	ByteBuffer &operator<<(const std::string &value) {
		append((UINT8 const *) value.c_str(), value.length());
		append((UINT8) 0);
		return *this;
	}
	ByteBuffer &operator<<(const std::wstring &value) {
		append((UINT16 const *) value.c_str(), value.length());
		append((UINT16) 0);
		return *this;
	}
	ByteBuffer &operator<<(const char *str) {
		append((UINT8 const *) str, str ? strlen(str) : 0);
		append((UINT8) 0);
		return *this;
	}

	ByteBuffer &operator>>(bool &value) {
		value = read<char>() > 0 ? true : false;
		return *this;
	}

	ByteBuffer &operator>>(UINT8 &value) {
		value = read<UINT8>();
		return *this;
	}

	ByteBuffer &operator>>(UINT16 &value) {
		value = read<UINT16>();
		return *this;
	}

	ByteBuffer &operator>>(UINT32 &value) {
		value = read<UINT32>();
		return *this;
	}

	ByteBuffer &operator>>(UINT64 &value) {
		value = read<UINT64>();
		return *this;
	}

	//signed as in 2e complement
	ByteBuffer &operator>>(INT8 &value) {
		value = read<INT8>();
		return *this;
	}

	ByteBuffer &operator>>(INT16 &value) {
		value = read<INT16>();
		return *this;
	}

	ByteBuffer &operator>>(INT32 &value) {
		value = read<INT32>();
		return *this;
	}

	ByteBuffer &operator>>(INT64 &value) {
		value = read<INT64>();
		return *this;
	}

	ByteBuffer &operator>>(float &value) {
		value = read<float>();
		return *this;
	}

	ByteBuffer &operator>>(double &value) {
		value = read<double>();
		return *this;
	}

	ByteBuffer &operator>>(std::string& value) {
		value.clear();
		while (rpos() < size()) // prevent crash at wrong string format in packet
		{
			char c = read<char>();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}
	ByteBuffer &operator>>(std::wstring& value) {
		value.clear();
		while (rpos() < size()) // prevent crash at wrong string format in packet
		{
			UINT16 c = read<UINT16>();
			if (c == 0)
				break;
			value += (WCHAR)c;
		}
		return *this;
	}

	UINT8 operator[](size_t pos) const {
		if (pos >= size())
			throw ByteBufferPositionException(false, pos, 1, size());
		return read<UINT8>(pos);
	}

	size_t rpos() const {
		return _rpos;
	}

	size_t rpos(size_t rpos_) {
		_rpos = rpos_;
		return _rpos;
	}

	void rfinish() {
		_rpos = wpos();
	}

	size_t wpos() const {
		return _wpos;
	}

	size_t wpos(size_t wpos_) {
		_wpos = wpos_;
		return _wpos;
	}

	template<typename T>
	void read_skip() {
		read_skip(sizeof(T));
	}

	void read_skip(size_t skip) {
		if (_rpos + skip > size())
			   throw ByteBufferPositionException(false, _rpos, skip, size());
		_rpos += skip;
	}

	template<typename T> T read() {
		T r = read<T>(_rpos);
		_rpos += sizeof(T);
		return r;
	}

	template<typename T> T read(size_t pos) const {
		if (pos + sizeof(T) > size())
			 throw ByteBufferPositionException(false, pos, sizeof(T), size());
		T val = *((T const*) &_storage[pos]);

		return val;
	}

	void read(UINT8 *dest, size_t len) {
		if (_rpos + len > size())
			throw ByteBufferPositionException(false, _rpos, len, size());
		memcpy(dest, &_storage[_rpos], len);
		_rpos += len;
	}



	const UINT8 *contents() const {
		return &_storage[0];
	}

	size_t size() const {
		return _storage.size();
	}
	bool empty() const {
		return _storage.empty();
	}

	void resize(size_t newsize) {
		_storage.resize(newsize);
		_rpos = 0;
		_wpos = size();
	}

	void reserve(size_t ressize) {
		if (ressize > size())
			_storage.reserve(ressize);
	}

	void append(const std::string& str) {

		append((UINT8 const*) str.c_str(), str.size() + 1);
	}

	void append(const char *src, size_t cnt) {
		return append((const UINT8 *) src, cnt);
	}

	template<class T> void append(const T *src, size_t cnt) {
		return append((const UINT8 *) src, cnt * sizeof(T));
	}

	void append(const UINT8 *src, size_t cnt) {
		if (!cnt)
			throw ByteBufferSourceException(_wpos, size(), cnt);

		if (!src)
			throw ByteBufferSourceException(_wpos, size(), cnt);

		ASSERT(size() < 10000000);

		if (_storage.size() < _wpos + cnt)
			_storage.resize(_wpos + cnt);
		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;
	}

	void append(const ByteBuffer& buffer) {
		if (buffer.wpos())
			append(buffer.contents(), buffer.wpos());
	}

	// can be used in SMSG_MONSTER_MOVE opcode

	void put(size_t pos, const UINT8 *src, size_t cnt) {
		if (pos + cnt > size())
			throw ByteBufferPositionException(true, pos, cnt, size());

		if (!src)
			throw ByteBufferSourceException(_wpos, size(), cnt);
		memcpy(&_storage[pos], src, cnt);
	}

	void print_storage() const {


		sLog->outDebug(,_T("STORAGE_SIZE: %lu"),
			(unsigned long) size());
		for (UINT32 i = 0; i < size(); ++i)
			//sLog->outDebugInLine(_T("%u - "), read<UINT8>(i));
		sLog->outDebug(_T("\n"));
	}

	/*void textlike() const {


		sLog->outString( _T("STORAGE_SIZE: %lu"),
			(unsigned long) size());
		for (UINT32 i = 0; i < size(); ++i)
			sLog->outDebugInLine(_T("%c"), read<UINT8>(i));
		sLog->outString(_T("\n"));
	}

	void hexlike() const {


		UINT32 j = 1, k = 1;
		sLog->outString(_T("STORAGE_SIZE: %lu"),
			(unsigned long) size());

		for (UINT32 i = 0; i < size(); ++i) {
			if ((i == (j * 8)) && ((i != (k * 16)))) {
				if (read<UINT8>(i) < 0x10) {
					sLog->outDebugInLine(_T("| 0%X "), read<UINT8>(i));
				} else {
					sLog->outDebugInLine(_T("| %X "), read<UINT8>(i));
				}
				++j;
			} else if (i == (k * 16)) {
				if (read<UINT8>(i) < 0x10) {
					sLog->outDebugInLine(_T("\n"));

					sLog->outDebugInLine(_T("0%X "), read<UINT8>(i));
				} else {
					sLog->outDebugInLine(_T("\n"));

					sLog->outDebugInLine(_T("%X "), read<UINT8>(i));
				}

				++k;
				++j;
			} else {
				if (read<UINT8>(i) < 0x10) {
					sLog->outDebugInLine(_T("0%X "), read<UINT8>(i));
				} else {
					sLog->outDebugInLine(_T("%X "), read<UINT8>(i));
				}
			}
		}
		sLog->outDebugInLine(_T("\n"));
	}*/
	char *Buffer(){return (char*)&_storage[0];}
protected:
	size_t _rpos, _wpos;
	std::vector<UINT8> _storage;
};

template<typename T>
inline ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v) {
	b << (UINT32) v.size();
	for (typename std::vector<T>::iterator i = v.begin(); i != v.end(); ++i) {
		b << *i;
	}
	return b;
}

template<typename T>
inline ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v) {
	UINT32 vsize;
	b >> vsize;
	v.clear();
	while (vsize--) {
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template<typename T>
inline ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v) {
	b << (UINT32) v.size();
	for (typename std::list<T>::iterator i = v.begin(); i != v.end(); ++i) {
		b << *i;
	}
	return b;
}

template<typename T>
inline ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v) {
	UINT32 vsize;
	b >> vsize;
	v.clear();
	while (vsize--) {
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template<typename K, typename V>
inline ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m) {
	b << (UINT32) m.size();
	for (typename std::map<K, V>::iterator i = m.begin(); i != m.end(); ++i) {
		b << i->first << i->second;
	}
	return b;
}

template<typename K, typename V>
inline ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m) {
	UINT32 msize;
	b >> msize;
	m.clear();
	while (msize--) {
		K k;
		V v;
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

// TODO: Make a ByteBuffer.cpp and move all this inlining to it.
template<> inline std::string ByteBuffer::read<std::string>() {
	std::string tmp;
	*this >> tmp;
	return tmp;
}

template<>
inline void ByteBuffer::read_skip<char*>() {
	std::string temp;
	*this >> temp;
}

template<>
inline void ByteBuffer::read_skip<char const*>() {
	read_skip<char*>();
}

template<>
inline void ByteBuffer::read_skip<std::string>() {
	read_skip<char*>();
}
#endif
