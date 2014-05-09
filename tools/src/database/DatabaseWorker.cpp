#include "DatabaseWorker.h"
#include "SQLOperation.h"
DatabaseWorker::DatabaseWorker(BlockQueue<SQLOperation*>* new_queue,MySQLConnection* con) :
m_queue(new_queue),
m_conn(con),
m_pThread(NULL)
{
    /// Assign thread to task
	if(m_queue)m_pThread=new boost::thread(boost::bind(&DatabaseWorker::svc,this));
}
int DatabaseWorker::wait()
{
	if(m_pThread){m_pThread->join();}
	return 0;
}
int DatabaseWorker::svc()
{
    if (!m_queue)
        return -1;

    SQLOperation *request = NULL;
    while (1)
    {
		
        if (!m_queue->dequeue(request)||!request)
            break;

        request->SetConnection(m_conn);
        request->call();

        delete request;
		request = NULL;
    }

    return 0;
}
