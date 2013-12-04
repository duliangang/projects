#ifndef _DEFINE_H_
#define _DEFINE_H_

#define WIN32_LEAN_AND_MEAN
#include <sstream>
#define PLATFORM_WINDOWS 0
#ifdef WIN32
#define PLATFORM PLATFORM_WINDOWS
#include <Windows.h>
#else
#define PLATFORM
#endif
#ifdef WIN32
#ifndef _STDINT
#include "pstdint.h"
#define _STDINT
#else
#include "stdint.h"
#endif
#endif


#if PLATFORM == PLATFORM_WINDOWS
#  define DEF_PATH_MAX MAX_PATH
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#else //PLATFORM != PLATFORM_WINDOWS
#  define DEF_PATH_MAX PATH_MAX
#  define DECLSPEC_NORETURN
#endif //PLATFORM

#include <algorithm>

namespace ByteConverter
{
	template<size_t T>
	inline void convert(char *val)
	{
		std::swap(*val, *(val + T - 1));
		convert<T - 2>(val + 1);
	}

	template<> inline void convert<0>(char *) {}
	template<> inline void convert<1>(char *) {}            // ignore central byte

	template<typename T> inline void apply(T *val)
	{
		convert<sizeof(T)>((char *)(val));
	}
}
static union { char c[4]; unsigned long mylong; } endian_test = {{ 'l', '?', '?', 'b' } };

#define ENDIANNESS ((char)endian_test.mylong)

template<typename T> inline T& EndianConvert(T& val) 
{
	if(ENDIANNESS=='l')
	{
		ByteConverter::apply<T>(&val); 
	}
	return val;
}

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(uint8_t&) { }
inline void EndianConvert( int8_t&) { }
inline void EndianConvertReverse(uint8_t&) { }
inline void EndianConvertReverse( int8_t&) { }



const int MAX_QUERY_LEN = 4096;
#include <iostream>
#include <tchar.h>
#ifdef _UNICODE
#define _tchar TCHAR
#define _tsprintf _stprintf
#define _tfprintf _ftprintf
#define _tsnprintf _sntprintf
#define _tatoi     _ttoi
#define _tvsnprintf _vsnwprintf
#define rename      _wrename

#define _tifstream      wifstream
#define _tistream		wistream
#define _tostream       wostream
#define _tistringstream wistringstream
#define _tostringstream wostringstream
#define _tstring wstring
#define _tcout  wcout
#define _tcin   wcin

#else
#define _tchar CHAR
#define _tsprintf _stprintf
#define _tfprintf _ftprintf
#define _tsnprintf _sntprintf
#define _tvsnprintf _vsnprintf
#define _tatoi     _ttoi
#define rename      rename

#define _tifstream      ifstream
#define _tistream		istream
#define _tostream       ostream
#define _tistringstream istringstream
#define _tostringstream ostringstream
#define _tstring string
#define _tcout  cout
#define _tcin   cin
#endif




#endif