#ifndef LOGWORKER_H
#define LOGWORKER_H
#include "../thread/base.h"
#include "../thread/Msg_Queue_Ex.h"
#include "../thread/apr/apr_base.h"
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




class LogWorker: protected Task_Base
{
public:
	LogWorker();
	~LogWorker();

	int enqueue(LogOperation *op);

private:
	virtual void svc();
	Msg_Queue<LogOperation,Null_Mutex> m_queue;
};

#endif
