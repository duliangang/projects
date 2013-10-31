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
	:m_queue()
{
	open();
}

LogWorker::~LogWorker()
{
	m_queue.destory();
	join();
}

int LogWorker::enqueue(LogOperation* op)
{
	 m_queue.enqueue(op);
	 return 0;
}

void LogWorker::svc()
{
	while (1)
	{
		LogOperation* request;
		if (m_queue.dequeue(request) !=0)
			break;

		request->call();
		delete request;
	}

	return ;
}