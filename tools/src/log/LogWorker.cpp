#include "LogWorker.h"
#include "Logger.h"
LogOperation::~LogOperation()
{
	delete msg;
}

int LogOperation::call()
{
	if (logger && msg)
		logger->write(*msg);
	return 0;
}

LogWorker::LogWorker()
{
	
}

LogWorker::~LogWorker()
{
	m_queue.destory();
}

bool LogWorker::enqueue(LogOperation* op)
{
	 m_queue.enqueue(op);
	 return true;
}

void LogWorker::svc()
{
	while (1)
	{
		LogOperation* request;
		if (!m_queue.dequeue(request))
			break;

		request->call();
		delete request;
	}
	return ;
}