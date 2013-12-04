#include "Util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#if PLATFORM==PLATFORM_WINDOWS
#include <Windows.h>
#endif
static const int MINUTE=60;
static const int HOUR=60*MINUTE;
static const int DAY = 24*HOUR;


Tokenizer::Tokenizer(const std::_tstring &src, const _tchar sep, uint32_t vectorReserve)
{
	m_str = new _tchar[src.length() + 1];
	memcpy(m_str, src.c_str(), (src.length()+1)*sizeof(_tchar));

	if (vectorReserve)
		m_storage.reserve(vectorReserve);

	_tchar* posold = m_str;
	_tchar* posnew = m_str;

	for (;;)
	{
		if (*posnew == sep)
		{
			m_storage.push_back(posold);
			posold = posnew + 1;

			*posnew = '\0';
		}
		else if (*posnew == '\0')
		{
			if (posold != posnew)
				m_storage.push_back(posold);

			break;
		}

		++posnew;
	}
}


std::_tstring secsToTimeString(uint64_t timeInSecs, bool shortText, bool hoursOnly)
{
    uint64_t secs    = timeInSecs % MINUTE;
    uint64_t minutes = timeInSecs % HOUR / MINUTE;
    uint64_t hours   = timeInSecs % DAY  / HOUR;
    uint64_t days    = timeInSecs / DAY;

    std::_tostringstream ss;
    if (days)
        ss << days << (shortText ? _T("d") : _T(" Day(s) "));
    if (hours || hoursOnly)
        ss << hours << (shortText ? _T("h") : _T(" Hour(s) "));
    if (!hoursOnly)
    {
        if (minutes)
            ss << minutes << (shortText ? "m" : " Minute(s) ");
        if (secs || (!days && !hours && !minutes) )
            ss << secs << (shortText ? "s" : " Second(s).");
    }

    return ss.str();
}

int32_t MoneyStringToMoney(const std::_tstring& moneyString)
{
    int32_t money = 0;

    if (!(std::count(moneyString.begin(), moneyString.end(), 'g') == 1 ||
        std::count(moneyString.begin(), moneyString.end(), 's') == 1 ||
        std::count(moneyString.begin(), moneyString.end(), 'c') == 1))
        return 0; // Bad format

    Tokenizer tokens(moneyString, ' ');
    for (Tokenizer::const_iterator itr = tokens.begin(); itr != tokens.end(); ++itr)
    {
        std::_tstring tokenString(*itr);
        uint32_t gCount = std::count(tokenString.begin(), tokenString.end(), 'g');
        uint32_t sCount = std::count(tokenString.begin(), tokenString.end(), 's');
        uint32_t cCount = std::count(tokenString.begin(), tokenString.end(), 'c');
        if (gCount + sCount + cCount != 1)
            return 0;

        uint32_t amount = _tatoi(*itr);
        if (gCount == 1)
            money += amount * 100 * 100;
        else if (sCount == 1)
            money += amount * 100;
        else if (cCount == 1)
            money += amount;
    }

    return money;
}

uint32_t TimeStringToSecs(const std::_tstring& timestring)
{
    uint32_t secs       = 0;
    uint32_t buffer     = 0;
    uint32_t multiplier = 0;

    for (std::_tstring::const_iterator itr = timestring.begin(); itr != timestring.end(); ++itr)
    {
        if (isdigit(*itr))
        {
            buffer*=10;
            buffer+= (*itr)-'0';
        }
        else
        {
            switch (*itr)
            {
                case 'd': multiplier = DAY;     break;
                case 'h': multiplier = HOUR;    break;
                case 'm': multiplier = MINUTE;  break;
                case 's': multiplier = 1;       break;
                default : return 0;                         //bad format
            }
            buffer*=multiplier;
            secs+=buffer;
            buffer=0;
        }
    }

    return secs;
}

std::_tstring TimeToTimestampStr(time_t t)
{
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    _tchar buf[20];
    _tsnprintf(buf, 20, _T("%04d-%02d-%02d_%02d-%02d-%02d"), aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::_tstring(buf);
}

/// Check if the string is a valid ip address representation

/// create PID file


size_t utf8length(std::string& utf8str)
{
    try
    {
        return utf8::distance(utf8str.c_str(), utf8str.c_str()+utf8str.size());
    }
    catch(std::exception)
    {
        utf8str = "";
        return 0;
    }
}

void utf8truncate(std::string& utf8str, size_t len)
{
    try
    {
        size_t wlen = utf8::distance(utf8str.c_str(), utf8str.c_str()+utf8str.size());
        if (wlen <= len)
            return;

        std::wstring wstr;
        wstr.resize(wlen,0);
        utf8::utf8to16(utf8str.c_str(), utf8str.c_str()+utf8str.size(), &wstr[0]);
        wstr.resize(len,0);
        char* oend = utf8::utf16to8(wstr.c_str(), wstr.c_str()+wstr.size(), &utf8str[0]);
        utf8str.resize(oend-(&utf8str[0]),0);                 // remove unused tail
    }
    catch(std::exception)
    {
        utf8str = "";
    }
}

bool Utf8toWStr(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize)
{
    try
    {
        size_t len = utf8::distance(utf8str, utf8str+csize);
        if (len > wsize)
        {
            if (wsize > 0)
                wstr[0] = L'\0';
            wsize = 0;
            return false;
        }

        wsize = len;
        utf8::utf8to16(utf8str, utf8str+csize, wstr);
        wstr[len] = L'\0';
    }
    catch(std::exception)
    {
        if (wsize > 0)
            wstr[0] = L'\0';
        wsize = 0;
        return false;
    }

    return true;
}

bool Utf8toWStr(const std::string& utf8str, std::wstring& wstr)
{
    try
    {
        if (size_t len = utf8::distance(utf8str.c_str(), utf8str.c_str()+utf8str.size()))
        {
            wstr.resize(len,0);
            utf8::utf8to16(utf8str.c_str(), utf8str.c_str()+utf8str.size(), &wstr[0]);
        }
    }
    catch(std::exception)
    {
        wstr = L"";
        return false;
    }

    return true;
}

bool WStrToUtf8(wchar_t* wstr, size_t size, std::string& utf8str)
{
    try
    {
        std::string utf8str2;
        utf8str2.resize(size*4,0);                            // allocate for most long case

        if (size)
        {
            char* oend = utf8::utf16to8(wstr, wstr+size, &utf8str2[0]);
            utf8str2.resize(oend-(&utf8str2[0]),0);               // remove unused tail
        }
        utf8str = utf8str2;
    }
    catch(std::exception)
    {
        utf8str = "";
        return false;
    }

    return true;
}

bool WStrToUtf8(std::wstring wstr, std::string& utf8str)
{
    try
    {
        std::string utf8str2;
        utf8str2.resize(wstr.size()*4,0);                     // allocate for most long case

        if (wstr.size())
        {
            char* oend = utf8::utf16to8(wstr.c_str(), wstr.c_str()+wstr.size(), &utf8str2[0]);
            utf8str2.resize(oend-(&utf8str2[0]),0);                // remove unused tail
        }
        utf8str = utf8str2;
    }
    catch(std::exception)
    {
        utf8str = "";
        return false;
    }

    return true;
}

typedef wchar_t const* const* wstrlist;

std::wstring GetMainPartOfName(std::wstring wname, uint32_t declension)
{
    // supported only Cyrillic cases
    if (wname.size() < 1 || !isCyrillicCharacter(wname[0]) || declension > 5)
        return wname;

    // Important: end length must be <= MAX_INTERNAL_PLAYER_NAME-MAX_PLAYER_NAME (3 currently)

    static wchar_t const a_End[]    = { wchar_t(1), wchar_t(0x0430), wchar_t(0x0000)};
    static wchar_t const o_End[]    = { wchar_t(1), wchar_t(0x043E), wchar_t(0x0000)};
    static wchar_t const ya_End[]   = { wchar_t(1), wchar_t(0x044F), wchar_t(0x0000)};
    static wchar_t const ie_End[]   = { wchar_t(1), wchar_t(0x0435), wchar_t(0x0000)};
    static wchar_t const i_End[]    = { wchar_t(1), wchar_t(0x0438), wchar_t(0x0000)};
    static wchar_t const yeru_End[] = { wchar_t(1), wchar_t(0x044B), wchar_t(0x0000)};
    static wchar_t const u_End[]    = { wchar_t(1), wchar_t(0x0443), wchar_t(0x0000)};
    static wchar_t const yu_End[]   = { wchar_t(1), wchar_t(0x044E), wchar_t(0x0000)};
    static wchar_t const oj_End[]   = { wchar_t(2), wchar_t(0x043E), wchar_t(0x0439), wchar_t(0x0000)};
    static wchar_t const ie_j_End[] = { wchar_t(2), wchar_t(0x0435), wchar_t(0x0439), wchar_t(0x0000)};
    static wchar_t const io_j_End[] = { wchar_t(2), wchar_t(0x0451), wchar_t(0x0439), wchar_t(0x0000)};
    static wchar_t const o_m_End[]  = { wchar_t(2), wchar_t(0x043E), wchar_t(0x043C), wchar_t(0x0000)};
    static wchar_t const io_m_End[] = { wchar_t(2), wchar_t(0x0451), wchar_t(0x043C), wchar_t(0x0000)};
    static wchar_t const ie_m_End[] = { wchar_t(2), wchar_t(0x0435), wchar_t(0x043C), wchar_t(0x0000)};
    static wchar_t const soft_End[] = { wchar_t(1), wchar_t(0x044C), wchar_t(0x0000)};
    static wchar_t const j_End[]    = { wchar_t(1), wchar_t(0x0439), wchar_t(0x0000)};

    static wchar_t const* const dropEnds[6][8] = {
        { &a_End[1],  &o_End[1],    &ya_End[1],   &ie_End[1],  &soft_End[1], &j_End[1],    NULL,       NULL },
        { &a_End[1],  &ya_End[1],   &yeru_End[1], &i_End[1],   NULL,         NULL,         NULL,       NULL },
        { &ie_End[1], &u_End[1],    &yu_End[1],   &i_End[1],   NULL,         NULL,         NULL,       NULL },
        { &u_End[1],  &yu_End[1],   &o_End[1],    &ie_End[1],  &soft_End[1], &ya_End[1],   &a_End[1],  NULL },
        { &oj_End[1], &io_j_End[1], &ie_j_End[1], &o_m_End[1], &io_m_End[1], &ie_m_End[1], &yu_End[1], NULL },
        { &ie_End[1], &i_End[1],    NULL,         NULL,        NULL,         NULL,         NULL,       NULL }
    };

    for (wchar_t const* const* itr = &dropEnds[declension][0]; *itr; ++itr)
    {
        size_t len = size_t((*itr)[-1]);                    // get length from string size field

        if (wname.substr(wname.size()-len, len)==*itr)
            return wname.substr(0, wname.size()-len);
    }

    return wname;
}
std::wstring ConsoleToWStr(const std::string& constr,std::wstring& wstr)
{
#if PLATFORM == PLATFORM_WINDOWS
	int len=MultiByteToWideChar(CP_ACP,0,constr.c_str(),constr.length(),NULL,0); 
	wchar_t* wcstr=new wchar_t[len+1];
	memset(wcstr,0,(len+1)*sizeof(wchar_t));
	len=MultiByteToWideChar(CP_ACP,0,constr.c_str(),constr.length(),wcstr,len);
	wstr=wcstr;
	delete[] wcstr;
	return wstr;



#else

	int len=mbstowcs(NULL,constr.c_str(),0);
	wchar_t wcstr=new wchar_t[len+1];
	memset(wcstr,0,(len+1)*sizeof(wchar_t));
	len=mbstowcs(wcstr,constr.c_str(),len+1);
	wstr=wcstr;
	delete[] wcstr;
	return wstr;
#endif
}
std::string WStrToConsole(const std::wstring& wstr,std::string& constr)
{
#if PLATFORM == PLATFORM_WINDOWS
	
	int len=WideCharToMultiByte(CP_ACP,0,wstr.c_str(),wstr.length(),NULL,0,NULL,NULL); 
	char* cstr=new char[len+1];
	memset(cstr,0,(len+1)*sizeof(char));
	len=WideCharToMultiByte(CP_ACP,0,wstr.c_str(),wstr.length(),cstr,len,NULL,NULL);
	constr=cstr;
	delete[] cstr;
	return constr;
#else
	int len =wcstombs(NULL,wstr.c_str(),0);
	char cstr=new char[len+1];
	memset(cstr,0,(len+1)*sizeof(char));
	//constr.resize(len+1);
	len=wcstombs(cstr,wstr.c_str(),len+1);
	constr=cstr;
	delete[] cstr;
	return constr;
#endif
}
bool utf8ToConsole(const std::string& utf8str, std::string& conStr)
{
#if PLATFORM == PLATFORM_WINDOWS
	if(utf8str.empty())return true;
    std::wstring wstr;
    if (!Utf8toWStr(utf8str, wstr))
        return false;

    WStrToConsole(wstr,conStr);
	return true;
   
#else
    // not implemented yet
    conStr = utf8str;
#endif

    return true;
}

bool consoleToUtf8(const std::string& conStr, std::string& utf8str)
{
#if PLATFORM == PLATFORM_WINDOWS
    std::wstring wstr;
    ConsoleToWStr(conStr,wstr);

    return WStrToUtf8(wstr, utf8str);
#else
    // not implemented yet
    utf8str = conStr;
    return true;
#endif
}

bool Utf8FitTo(const std::string& str, std::wstring search)
{
	if(str.empty())return true;
    std::wstring temp;

    if (!Utf8toWStr(str, temp))
        return false;

    // converting to lower case
    wstrToLower( temp );

    if (temp.find(search) == std::wstring::npos)
        return false;

    return true;
}
void utf8printf(FILE* out, const wchar_t *str, ...)
{
	va_list ap;
	va_start(ap, str);
	vutf8printf(out,str,&ap);
	 va_end(ap);
}
void utf8printf(FILE* out, const char *str, ...)
{
    va_list ap;
    va_start(ap, str);
    vutf8printf(out, str, &ap);
    va_end(ap);
}
void vutf8printf(FILE* out, const wchar_t *str, va_list* ap)
{
	 wchar_t wtemp_buf[32*1024];
	 size_t wtemp_len =vswprintf(wtemp_buf,str,*ap);
	 std::string utfstr;
	 WStrToUtf8(wtemp_buf,wtemp_len,utfstr);
	 fprintf(out, "%s", utfstr.c_str());
	 return ;
}
void vutf8printf(FILE* out, const char *str, va_list* ap)
{
#if PLATFORM == PLATFORM_WINDOWS
    char temp_buf[32*1024];
    wchar_t wtemp_buf[32*1024];

    size_t temp_len = vsnprintf(temp_buf, 32*1024, str, *ap);

    size_t wtemp_len = 32*1024-1;
    Utf8toWStr(temp_buf, temp_len, wtemp_buf, wtemp_len);

    CharToOemBuffW(&wtemp_buf[0], &temp_buf[0], wtemp_len+1);
    fprintf(out, "%s", temp_buf);
#else
    vfprintf(out, str, *ap);
#endif
}

std::string ByteArrayToHexStr(uint8_t const* bytes, uint32_t arrayLen, bool reverse /* = false */)
{
    int32_t init = 0;
    int32_t end = arrayLen;
    int8_t op = 1;

    if (reverse)
    {
        init = arrayLen - 1;
        end = -1;
        op = -1;
    }

    std::ostringstream ss;
    for (int32_t i = init; i != end; i += op)
    {
        char buffer[4];
        sprintf(buffer, "%02X", bytes[i]);
        ss << buffer;
    }

    return ss.str();
}
