#ifdef WIN32
#include "pstdint.h"
#include <tchar.h>
#ifdef _UNICODE
#define _tchar TCHAR
#define _tsprintf _stprintf
#define _tfprintf _ftprintf

#define _tstring wstring
#define _tcout  wcout
#define _tcin   wcin
#else
#define _tchar CHAR
#define _tsprintf _stprintf
#define _tfprintf _ftprintf

#define _tstring string
#define _tcout  cout
#define _tcin   cin
#endif


#else if linux
#include <stdint.h>

#endif