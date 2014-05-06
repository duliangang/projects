#ifndef _WORKERTHREAD_H
#define _WORKERTHREAD_H

#include <BlockQueue.h>

class MySQLConnection;
class SQLOperation;

class DatabaseWorker 
{
    public:
        DatabaseWorker(BlockQueue<SQLOperation*>* new_queue,MySQLConnection* con);


        ///- Inherited from ACE_Task_Base
        int svc();
        int wait() ;

    private:
        DatabaseWorker()   {}
        BlockQueue<SQLOperation*>* m_queue;
        MySQLConnection* m_conn;
		boost::thread* m_pThread;
};

#endif
