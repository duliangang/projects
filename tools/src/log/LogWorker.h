#ifndef LOGWORKER_H
#define LOGWORKER_H
#include "BlockQueue.h"
class Logger;
struct LogMessage;

class LogOperation
{
public:
	LogOperation(Logger* _logger, LogMessage* _msg)
		: logger(_logger)
		, msg(_msg)
	{ }

	~LogOperation();

	int call();

protected:
	Logger *logger;
	LogMessage *msg;
};




class LogWorker
{
public:
	LogWorker();
	~LogWorker();

	bool enqueue(LogOperation *op);
	virtual void svc();
private:
	
	BlockQueue<LogOperation*> m_queue;
};

#endif
