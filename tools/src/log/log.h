#ifndef _LOG_H_
#define _LOG_H_
#include "../shared/Define.h"
#include "../thread/singleton.h"
#include "Logger.h"
#include <string>

class Config;
class LogWorker;
class Log
{
	friend class Singleton<Log,Thread_Mutex>;
	typedef std::map<int32_t, Logger> LoggerMap;
public:
	void LoadFromConfig(const Config* conf);
	void Close();
	bool SetLogLevel(std::_tstring const& name,_tchar const * level,bool isLogger=true);
	

	void outTrace(uint8_t f,  char const* str, ...);
	void outDebug(uint8_t f, char const* str, ...);
	void outInfo(uint8_t f, char const* str, ...);
	void outWarn(uint8_t f, char const* str, ...) ;
	void outError(uint8_t f, char const* str, ...);
	void outFatal(uint8_t f, char const* str, ...);

	void outTrace(uint8_t f,  wchar_t const* str, ...);
	void outDebug(uint8_t f, wchar_t const* str, ...);
	void outInfo(uint8_t f, wchar_t const* str, ...);
	void outWarn(uint8_t f, wchar_t const* str, ...) ;
	void outError(uint8_t f, wchar_t const* str, ...);
	void outFatal(uint8_t f, wchar_t const* str, ...);
	static std::_tstring GetTimestampStr();
	void setFilterString(const std::map<uint8_t,std::_tstring>& FilterStringList);

private:
	void va_log(uint8_t f, LogLevel level, wchar_t const* str, va_list argptr);
	void va_log(uint8_t f, LogLevel level, char const* str, va_list argptr);
	void write(LogMessage* msg);
	Logger* GetLoggerByType(uint8_t filter);
	Appender* GetAppenderByName(std::_tstring const& name);
	uint8_t MakeAppenderId();
	int32_t GetConfigIntDefault(std::_tstring base, const _tchar* name, int32_t value);
	std::_tstring GetConfigStringDefault(std::_tstring base, const _tchar* name, const _tchar* value);


	void CreateAppenderFromConfig(const _tchar* name);
	void CreateLoggerFromConfig(const _tchar* name);
	void ReadAppendersFromConfig();
	void ReadLoggersFromConfig();

	bool ShouldLog(uint8_t type, LogLevel level) const;
private:
	Log();
	~Log();
	AppenderMap appenders;
	LoggerMap loggers;
	uint8_t AppenderId;
	 LogWorker* worker;
	const Config* m_config;
	std::_tstring m_logsDir;
	std::_tstring m_logsTimestamp;
};
#define sLog Singleton<Log,Thread_Mutex>::GetInstance()
#endif

