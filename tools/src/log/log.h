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
	typedef std::map<uint8_t, Logger> LoggerMap;
public:
	void LoadFromConfig(const Config* conf);
	void Close();
	bool SetLogLevel(std::_tstring const& name,_tchar const * level,bool isLogger=true);


	void outTrace(LogFilterType f,  _tchar const* str, ...);
	void outDebug(LogFilterType f, _tchar const* str, ...);
	void outInfo(LogFilterType f, _tchar const* str, ...);
	void outWarn(LogFilterType f, _tchar const* str, ...) ;
	void outError(LogFilterType f, _tchar const* str, ...);
	void outFatal(LogFilterType f, _tchar const* str, ...);
	static std::_tstring GetTimestampStr();

private:
	void va_log(LogFilterType f, LogLevel level, _tchar const* str, va_list argptr);
	void write(LogMessage* msg);
	Logger* GetLoggerByType(LogFilterType filter);
	Appender* GetAppenderByName(std::_tstring const& name);
	uint8_t MakeAppenderId();
	int32_t GetConfigIntDefault(std::_tstring base, const _tchar* name, int32_t value);
	std::_tstring GetConfigStringDefault(std::_tstring base, const _tchar* name, const _tchar* value);


	void CreateAppenderFromConfig(const _tchar* name);
	void CreateLoggerFromConfig(const _tchar* name);
	void ReadAppendersFromConfig();
	void ReadLoggersFromConfig();

	bool ShouldLog(LogFilterType type, LogLevel level) const;
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

