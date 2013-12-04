
#include "Logger.h"

Logger::Logger(): name(_T("")), type(LOG_FILTER_GENERAL), level(LOG_LEVEL_DISABLED)
{
}

void Logger::Create(std::_tstring const& _name, uint8_t _type, LogLevel _level)
{
	name = _name;
	type = _type;
	level = _level;
}

Logger::~Logger()
{
	for (AppenderMap::iterator it = appenders.begin(); it != appenders.end(); ++it)
		it->second = NULL;
	appenders.clear();
}

std::_tstring const& Logger::getName() const
{
	return name;
}

uint8_t Logger::getType() const
{
	return type;
}

LogLevel Logger::getLogLevel() const
{
	return level;
}

void Logger::addAppender(uint8_t id, Appender* appender)
{
	appenders[id] = appender;
}

void Logger::delAppender(uint8_t id)
{
	AppenderMap::iterator it = appenders.find(id);
	if (it != appenders.end())
	{
		it->second = NULL;//多对多关系 不应当在此处删除 而应当在创建位置删除
		appenders.erase(it);
	}
}

void Logger::setLogLevel(LogLevel _level)
{
	level = _level;
}

void Logger::write(LogMessage& message)
{
	if (!level || level > message.level || message.text.empty())
	{
		return;
	}

	for (AppenderMap::iterator it = appenders.begin(); it != appenders.end(); ++it)
		if (it->second)
			it->second->write(message);
}
