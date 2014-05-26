#include <cstdarg>
#include <cstdio>
#include <sstream>
#include "Log.h"
#include "../config/config.h"
#include "../shared/Tokenizer.h"
#include "LogWorker.h"
#include "AppenderFile.h"
#include "AppenderConsole.h"
#ifdef _UNICODE
#define Tokenizer WTokenizer
#else
#define Tokenizer MTokenizer
#endif
Log::Log() : worker(NULL)
{
	m_logsTimestamp = _T("_") + GetTimestampStr();
}

Log::~Log()
{
	Close();
}

uint8_t Log::MakeAppenderId()
{
	return AppenderId++;
}

int32_t Log::GetConfigIntDefault(std::_tstring base, const _tchar* name, int32_t value)
{
	base.append(name);
	return m_config->Read(base.c_str(), value);
}

std::_tstring Log::GetConfigStringDefault(std::_tstring base, const _tchar* name, const _tchar* value)
{
	base.append(name);
	return m_config->Read(base.c_str(), value);
}

// Returns default logger if the requested logger is not found
Logger* Log::GetLoggerByType(uint8_t filter)
{
	LoggerMap::iterator it = loggers.begin();
	while (it != loggers.end() && it->second.getType() != filter)
		++it;

	return it == loggers.end() ? &(loggers[0]) : &(it->second);
}

Appender* Log::GetAppenderByName(std::_tstring const& name)
{
	AppenderMap::iterator it = appenders.begin();
	while (it != appenders.end() && it->second && it->second->getName() != name)
		++it;

	return it == appenders.end() ? NULL : it->second;
}

void Log::CreateAppenderFromConfig(const _tchar* name)
{
	if (!name || *name == '\0')
		return;

	// Format=type,level,flags,optional1,optional2
	// if type = File. optional1 = file and option2 = mode
	// if type = Console. optional1 = Color
	std::_tstring options = _T("Appender.");
	options.append(name);
	options =m_config->Read(options, _T(""));
	Tokenizer tokens(options, ',');
	Tokenizer::const_iterator iter = tokens.begin();

	if (tokens.size() < 2)
	{	
		fprintf(stderr, "Log::CreateAppenderFromConfig: Wrong configuration for appender %s. Config line: %s\n", name, options.c_str());
		return;
	}

	AppenderFlags flags = APPENDER_FLAGS_NONE;
	AppenderType type = AppenderType(_tatoi(*iter));
	++iter;
	LogLevel level = LogLevel(_tatoi(*iter));
	if (level > LOG_LEVEL_FATAL)
	{
		fprintf(stderr, "Log::CreateAppenderFromConfig: Wrong Log Level %d for appender %s\n", level, name);
		return;
	}

	if (++iter != tokens.end())
		flags = AppenderFlags(_tatoi(*iter));

	switch (type)
	{
	case APPENDER_CONSOLE:
		{
			AppenderConsole* appender = new AppenderConsole(MakeAppenderId(), name, level, flags);
			appenders[appender->getId()] = appender;
			if (++iter != tokens.end())
				appender->InitColors(*iter);
			//fprintf(stdout, "Log::CreateAppenderFromConfig: Created Appender %s (%u), Type CONSOLE, Mask %u\n", appender->getName().c_str(), appender->getId(), appender->getLogLevel()); // DEBUG - RemoveMe
			break;
		}
	case APPENDER_FILE:
		{
			std::_tstring filename;
			std::_tstring mode = _T("a");

			if (++iter == tokens.end())
			{
				_tfprintf(stderr, _T("Log::CreateAppenderFromConfig: Missing file name for appender %s\n"), name);
				return;
			}

			filename = *iter;

			if (++iter != tokens.end())
				mode = *iter;

			if (flags & APPENDER_FLAGS_USE_TIMESTAMP)
			{
				size_t dot_pos = filename.find_last_of(_T("."));
				if (dot_pos != filename.npos)
					filename.insert(dot_pos, m_logsTimestamp);
				else
					filename += m_logsTimestamp;
			}

			uint8_t id = MakeAppenderId();
			appenders[id] = new AppenderFile(id, name, level, filename.c_str(), m_logsDir.c_str(), mode.c_str(), flags);
			//fprintf(stdout, "Log::CreateAppenderFromConfig: Created Appender %s (%u), Type FILE, Mask %u, File %s, Mode %s\n", name, id, level, filename.c_str(), mode.c_str()); // DEBUG - RemoveMe
			break;
		}
	
	default:
		fprintf(stderr, "Log::CreateAppenderFromConfig: Unknown type %d for appender %s\n", type, name);
		break;
	}
}


bool Log::ShouldLog(uint8_t type, LogLevel level) const
{
	LoggerMap::const_iterator it = loggers.find(type);
	if (it != loggers.end())
	{
		LogLevel loggerLevel = it->second.getLogLevel();
		return loggerLevel && loggerLevel <= level;
	}

	if (type != LOG_FILTER_GENERAL)
		return ShouldLog(LOG_FILTER_GENERAL, level);

	return false;
}
void Log::setFilterString(const std::map<uint8_t,std::string>& FilterStringList)
{
	Appender::MapLogFilterTypeString=FilterStringList;
	return ;
}
void Log::CreateLoggerFromConfig(const _tchar* name)
{
	if (!name || *name == '\0')
		return;

	LogLevel level = LOG_LEVEL_DISABLED;
	int32_t type = -1;

	std::_tstring options = _T("Logger.");
	options.append(name);
	options =m_config->Read(options.c_str(), _T(""));

	if (options.empty())
	{
		fprintf(stderr, "Log::CreateLoggerFromConfig: Missing config option Logger.%s\n", name);
		return;
	}

	Tokenizer tokens(options, ',');
	Tokenizer::const_iterator iter = tokens.begin();

	if (tokens.size() != 3)
	{
		fprintf(stderr, "Log::CreateLoggerFromConfig: Wrong config option Logger.%s=%s\n", name, options.c_str());
		return;
	}

	type = _tatoi(*iter);
	if (type > MaxLogFilter)
	{
		fprintf(stderr, "Log::CreateLoggerFromConfig: Wrong type %u for logger %s\n", type, name);
		return;
	}

	Logger& logger = loggers[type];
	if (!logger.getName().empty())
	{
		fprintf(stderr, "Error while configuring Logger %s. Already defined\n", name);
		return;
	}

	++iter;
	level = LogLevel(_tatoi(*iter));
	if (level > LOG_LEVEL_FATAL)
	{
		fprintf(stderr, "Log::CreateLoggerFromConfig: Wrong Log Level %u for logger %s\n", type, name);
		return;
	}

	logger.Create(name, uint8_t(type), level);
#ifdef _UNICODE
	std::string tempstr;
	Appender::MapLogFilterTypeString[type]=WStrToConsole(name,tempstr);
#else
	Appender::MapLogFilterTypeString[type]=name;
#endif
	//fprintf(stdout, "Log::CreateLoggerFromConfig: Created Logger %s, Type %u, mask %u\n", name, LogFilterType(type), level); // DEBUG - RemoveMe

	++iter;
	std::_tistringstream ss(*iter);
	std::_tstring str;

	ss >> str;
	while (ss)
	{
		if (Appender* appender = GetAppenderByName(str))
		{
			logger.addAppender(appender->getId(), appender);
			//fprintf(stdout, "Log::CreateLoggerFromConfig: Added Appender %s to Logger %s\n", appender->getName().c_str(), name); // DEBUG - RemoveMe
		}
		else
			fprintf(stderr, "Error while configuring Appender %s in Logger %s. Appender does not exist", str.c_str(), name);
		ss >> str;
	}
}

void Log::ReadAppendersFromConfig()
{
	std::_tistringstream ss(m_config->Read(_T("Appenders"), _T("")));
	std::_tstring name;

	do
	{
		ss >> name;
		CreateAppenderFromConfig(name.c_str());
		name = _T("");
	}
	while (ss);
}

void Log::ReadLoggersFromConfig()
{
	std::_tistringstream ss(m_config->Read(_T("Loggers"), _T("")));

	std::_tstring name;

	do
	{
		ss >> name;
		CreateLoggerFromConfig(name.c_str());
		name.clear();
	}
	while (ss);

	// root logger must exist. Marking as disabled as its not configured
	if (loggers.find(LOG_FILTER_GENERAL) == loggers.end())
		loggers[LOG_FILTER_GENERAL].Create(_T("root"), LOG_FILTER_GENERAL, LOG_LEVEL_DISABLED);
}


void Log::va_log(uint8_t filter, LogLevel level, wchar_t const* str, va_list argptr)
{
	wchar_t text[MAX_QUERY_LEN];
	_vsnwprintf(text, MAX_QUERY_LEN, str, argptr);
	std::string cstr;
	WStrToConsole(text,cstr);
	write(new LogMessage(level, filter, cstr));
}
void Log::va_log(uint8_t filter, LogLevel level, char const* str, va_list argptr)
{
	char text[MAX_QUERY_LEN];
	vsnprintf(text, MAX_QUERY_LEN, str, argptr);
	write(new LogMessage(level, filter, str));
}
void Log::write(LogMessage* msg)
{
	msg->text.append("\n");
	Logger* logger = GetLoggerByType(msg->type);
	if (worker)
	{
		
		worker->enqueue(new LogOperation(logger, msg));
	}
	else
	{
		logger->write(*msg);
	}
}

std::_tstring Log::GetTimestampStr()
{
	time_t t = time(NULL);
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

bool Log::SetLogLevel(std::_tstring const& name, const _tchar* newLevelc, bool isLogger /* = true */)
{
	LogLevel newLevel = LogLevel(_tatoi(newLevelc));
	if (newLevel < 0)
		return false;

	if (isLogger)
	{
		LoggerMap::iterator it = loggers.begin();
		while (it != loggers.end() && it->second.getName() != name)
			++it;

		if (it == loggers.end())
			return false;

		it->second.setLogLevel(newLevel);
	}
	else
	{
		Appender* appender = GetAppenderByName(name);
		if (!appender)
			return false;

		appender->setLogLevel(newLevel);
	}
	return true;
}


void Log::outTrace(uint8_t filter, char const* str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_TRACE))
		return;
	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_TRACE, str, ap);

	va_end(ap);
}
void Log::outTrace(uint8_t filter, const wchar_t * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_TRACE))
		return;
	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_TRACE, str, ap);

	va_end(ap);
}


void Log::outDebug(uint8_t filter, const char * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_DEBUG))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_DEBUG, str, ap);

	va_end(ap);
}
void Log::outDebug(uint8_t filter, const wchar_t * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_DEBUG))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_DEBUG, str, ap);

	va_end(ap);
}

void Log::outInfo(uint8_t filter, const wchar_t * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_INFO))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_INFO, str, ap);

	va_end(ap);
}
void Log::outInfo(uint8_t filter, const char * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_INFO))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_INFO, str, ap);

	va_end(ap);
}
void Log::outWarn(uint8_t filter, const wchar_t * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_WARN))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_WARN, str, ap);

	va_end(ap);
}
void Log::outWarn(uint8_t filter, const char * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_WARN))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_WARN, str, ap);

	va_end(ap);
}
void Log::outError(uint8_t filter, const wchar_t * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_ERROR))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_ERROR, str, ap);

	va_end(ap);
}
void Log::outError(uint8_t filter, const char * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_ERROR))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_ERROR, str, ap);

	va_end(ap);
}
void Log::outFatal(uint8_t filter, const wchar_t * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_FATAL))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_FATAL, str, ap);

	va_end(ap);
}

void Log::outFatal(uint8_t filter, const char * str, ...)
{
	if (!str || !ShouldLog(filter, LOG_LEVEL_FATAL))
		return;

	va_list ap;
	va_start(ap, str);

	va_log(filter, LOG_LEVEL_FATAL, str, ap);

	va_end(ap);
}



void Log::Close()
{
	if(worker)
	{
		delete worker;
		worker = NULL;
	}
	
	m_config=NULL;
	loggers.clear();
	for (AppenderMap::iterator it = appenders.begin(); it != appenders.end(); ++it)
	{
		delete it->second;
		it->second = NULL;
	}
	appenders.clear();
}

void Log::LoadFromConfig(const Config* conf)
{
	if(!conf)
	{
		return ;
	}
	Close();
	worker = new LogWorker();
	m_config=conf;
	AppenderId = 0;
	m_logsDir = m_config->Read(_T("LogsDir"), _T(""));
	if (!m_logsDir.empty())
		if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
			m_logsDir.push_back('/');
	ReadAppendersFromConfig();
	ReadLoggersFromConfig();
}
