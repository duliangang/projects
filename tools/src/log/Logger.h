#ifndef LOGGER_H
#define LOGGER_H

#include "Appender.h"

class Logger
{
public:
	Logger();
	~Logger();

	void Create(std::_tstring const& name, uint8_t type, LogLevel level);
	void addAppender(uint8_t type, Appender *);
	void delAppender(uint8_t type);

	std::_tstring const& getName() const;
	uint8_t getType() const;
	LogLevel getLogLevel() const;
	void setLogLevel(LogLevel level);
	void write(LogMessage& message);

private:
	std::_tstring name;
	uint8_t type;
	LogLevel level;
	AppenderMap appenders;
};

#endif
