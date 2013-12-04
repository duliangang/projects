#include "Appender.h"
std::_tstring LogMessage::getTimeStr(time_t time)
{
	tm* aTm = localtime(&time);
	_tchar buf[20];
	_tsnprintf(buf, 20, _T("%04d-%02d-%02d_%02d:%02d:%02d"), aTm->tm_year+1900, aTm->tm_mon+1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	return std::_tstring(buf);
}
std::_tstring LogMessage::getTimeStr()
{
	return getTimeStr(mtime);
}
Appender::Appender(uint8_t _id, std::_tstring const& _name, AppenderType _type /* = APPENDER_NONE*/, LogLevel _level /* = LOG_LEVEL_DISABLED */, AppenderFlags _flags /* = APPENDER_FLAGS_NONE */):
id(_id), name(_name), type(_type), level(_level), flags(_flags)
{
}
Appender::~Appender()
{
}


uint8_t Appender::getId() const
{
	return id;
}

std::_tstring const& Appender::getName() const
{
	return name;
}

AppenderType Appender::getType() const
{
	return type;
}

LogLevel Appender::getLogLevel() const
{
	return level;
}

AppenderFlags Appender::getFlags() const
{
	return flags;
}

void Appender::setLogLevel(LogLevel _level)
{
	level = _level;
}


void Appender::write(LogMessage& message)
{
	if (!level || level > message.level)
	{
		//fprintf(stderr, "Appender::write: Appender %s, Level %s. Msg %s Level %s Type %s WRONG LEVEL MASK\n", getName().c_str(), getLogLevelString(level), message.text.c_str(), getLogLevelString(message.level), getLogFilterTypeString(message.type)); // DEBUG - RemoveMe
		return;
	}

	message.prefix.clear();
	if (flags & APPENDER_FLAGS_PREFIX_TIMESTAMP)
		message.prefix.append(message.getTimeStr().c_str());

	if (flags & APPENDER_FLAGS_PREFIX_LOGLEVEL)
	{
		if (!message.prefix.empty())
			message.prefix.push_back(' ');

		_tchar text[MAX_QUERY_LEN];
		_tsnprintf(text, MAX_QUERY_LEN, _T("%-5s"), Appender::getLogLevelString(message.level));
		message.prefix.append(text);
	}

	if (flags & APPENDER_FLAGS_PREFIX_LOGFILTERTYPE)
	{
		if (!message.prefix.empty())
			message.prefix.push_back(' ');

		_tchar text[MAX_QUERY_LEN];
		_tsnprintf(text, MAX_QUERY_LEN, _T("[%s]"), Appender::getLogFilterTypeString(message.type));
		message.prefix.append(text);
	}

	if (!message.prefix.empty())
		message.prefix.push_back(' ');

	_write(message);
}


const _tchar* Appender::getLogLevelString(LogLevel level)
{
	switch (level)
	{
	case LOG_LEVEL_FATAL:
		return _T("FATAL");
	case LOG_LEVEL_ERROR:
		return _T("ERROR");
	case LOG_LEVEL_WARN:
		return _T("WARN");
	case LOG_LEVEL_INFO:
		return _T("INFO");
	case LOG_LEVEL_DEBUG:
		return _T("DEBUG");
	case LOG_LEVEL_TRACE:
		return _T("TRACE");
	default:
		return _T("DISABLED");
	}
}
std::map<uint8_t,std::_tstring> Appender::MapLogFilterTypeString;
_tchar const* Appender::getLogFilterTypeString(uint8_t type)
{
	if(MapLogFilterTypeString.find(type)!=MapLogFilterTypeString.end())
	{
		return MapLogFilterTypeString[type].c_str();
	}
	return _T("???");
}