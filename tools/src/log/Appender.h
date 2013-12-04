#ifndef _APPENDER_
#define _APPENDER_
#include "../shared/Define.h"
#include <string>
#include <map>
#include <vector>
#include <time.h>
#define LOG_FILTER_GENERAL 255
enum LogLevel
{
	LOG_LEVEL_DISABLED                           = 0,
	LOG_LEVEL_TRACE                              = 1,
	LOG_LEVEL_DEBUG                              = 2,
	LOG_LEVEL_INFO                               = 3,
	LOG_LEVEL_WARN                               = 4,
	LOG_LEVEL_ERROR                              = 5,
	LOG_LEVEL_FATAL                              = 6
};
enum AppenderType
{
	APPENDER_NONE,
	APPENDER_CONSOLE,
	APPENDER_FILE,
	APPENDER_SOCKET,
	APPENDER_DB,
};


enum AppenderFlags
{
	APPENDER_FLAGS_NONE                          = 0x00,
	APPENDER_FLAGS_PREFIX_TIMESTAMP              = 0x01,
	APPENDER_FLAGS_PREFIX_LOGLEVEL               = 0x02,
	APPENDER_FLAGS_PREFIX_LOGFILTERTYPE          = 0x04,
	APPENDER_FLAGS_USE_TIMESTAMP                 = 0x08, // only used by FileAppender
	APPENDER_FLAGS_MAKE_FILE_BACKUP              = 0x10  // only used by FileAppender
};
const uint8_t MaxLogFilter=255;
const uint8_t MaxLogLevels = 6;

struct LogMessage
{
	LogMessage(LogLevel _level, uint8_t _type, std::_tstring _text)
		: level(_level)
		, type(_type)
		, text(_text)
	{
		mtime = time(NULL);
	}

	static std::_tstring getTimeStr(time_t time);
	std::_tstring getTimeStr();

	LogLevel level;
	uint8_t type;
	std::_tstring text;
	std::_tstring prefix;
	std::_tstring param1;
	time_t mtime;
};
class Log;
class Appender
{
public:
	friend class Log;
	Appender(uint8_t _id, std::_tstring const& name, AppenderType type = APPENDER_NONE, LogLevel level = LOG_LEVEL_DISABLED, AppenderFlags flags = APPENDER_FLAGS_NONE);
	virtual ~Appender();

	uint8_t getId() const;
	std::_tstring const& getName() const;
	AppenderType getType() const;
	LogLevel getLogLevel() const;
	AppenderFlags getFlags() const;

	void setLogLevel(LogLevel);
	void write(LogMessage& message);
	static const _tchar* getLogLevelString(LogLevel level);
	static const _tchar* getLogFilterTypeString(uint8_t type);

private:
	virtual void _write(LogMessage& /*message*/) = 0;

	uint8_t id;
	std::_tstring name;
	AppenderType type;
	LogLevel level;
	AppenderFlags flags;
	static std::map<uint8_t,std::_tstring> MapLogFilterTypeString;
};

typedef std::map<uint8_t, Appender*> AppenderMap;

#endif