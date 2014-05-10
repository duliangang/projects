
#ifndef _MYSQLCONNECTION_H
#define _MYSQLCONNECTION_H
#include "com.h"
#include "../shared/Util.h"
#include <map>
#include "BlockQueue.h"

#include "QueryResult.h"
#include "Transaction.h"

class DatabaseWorker;
class PreparedStatement;
class MySQLPreparedStatement;
class PingOperation;

enum ConnectionFlags
{
    CONNECTION_ASYNC = 0x1,
    CONNECTION_SYNCH = 0x2,
    CONNECTION_BOTH = CONNECTION_ASYNC | CONNECTION_SYNCH
};

struct MySQLConnectionInfo
{
    MySQLConnectionInfo() {}
    MySQLConnectionInfo(const std::string& infoString)
    {
        MTokenizer tokens(infoString, ';');

        if (tokens.size() != 5)
            return;

        uint8 i = 0;

        host.assign(tokens[i++]);
        port_or_socket.assign(tokens[i++]);
        user.assign(tokens[i++]);
        password.assign(tokens[i++]);
        database.assign(tokens[i++]);
    }

    std::string user;
    std::string password;
    std::string database;
    std::string host;
    std::string port_or_socket;
};

typedef std::map<uint32 /*index*/, std::pair<std::string /*query*/, ConnectionFlags /*sync/async*/> > PreparedStatementMap;

class MySQLConnection
{
    template <class T> friend class DatabaseWorkerPool;
    friend class PingOperation;

    public:
        MySQLConnection(MySQLConnectionInfo& connInfo);                               //! Constructor for synchronous connections.
        MySQLConnection(BlockQueue<SQLOperation*>* queue, MySQLConnectionInfo& connInfo);  //! Constructor for asynchronous connections.
        virtual ~MySQLConnection();

        virtual bool Open();
        void Close();

    public:
        bool Execute(const char* sql);
        bool Execute(PreparedStatement* stmt);
        ResultSet* Query(const char* sql);
        PreparedResultSet* Query(PreparedStatement* stmt);
        bool _Query(const char *sql, MYSQL_RES **pResult, MYSQL_FIELD **pFields, uint64_t* pRowCount, uint32_t* pFieldCount);
        bool _Query(PreparedStatement* stmt, MYSQL_RES **pResult, uint64_t* pRowCount, uint32_t* pFieldCount);

        void BeginTransaction();
        void RollbackTransaction();
        void CommitTransaction();
        bool ExecuteTransaction(SQLTransaction& transaction);

        operator bool () const { return m_Mysql != NULL; }
        void Ping() { mysql_ping(m_Mysql); }

        uint32 GetLastError() { return mysql_errno(m_Mysql); }

    protected:
        bool LockIfReady()
        {
            /// Tries to acquire lock. If lock is acquired by another thread
            /// the calling parent will just try another connection
            return m_Mutex.try_lock();
        }

        void Unlock()
        {
            /// Called by parent databasepool. Will let other threads access this connection
            m_Mutex.unlock();
        }

        MYSQL* GetHandle()  { return m_Mysql; }
        MySQLPreparedStatement* GetPreparedStatement(uint32 index);
        void PrepareStatement(uint32 index, const char* sql, ConnectionFlags flags);

        bool PrepareStatements();
        virtual void DoPrepareStatements() = 0;

    protected:
        std::vector<MySQLPreparedStatement*> m_stmts;         //! PreparedStatements storage
        PreparedStatementMap                 m_queries;       //! Query storage
        bool                                 m_reconnecting;  //! Are we reconnecting?
        bool                                 m_prepareError;  //! Was there any error while preparing statements?

    private:
        bool _HandleMySQLErrno(uint32 errNo);

    private:
        BlockQueue<SQLOperation*>* m_queue;                      //! Queue shared with other asynchronous connections.
        DatabaseWorker*       m_worker;                     //! Core worker task.
        MYSQL *               m_Mysql;                      //! MySQL Handle.
        MySQLConnectionInfo&  m_connectionInfo;             //! Connection info (used for logging)
        ConnectionFlags       m_connectionFlags;            //! Connection flags (for preparing relevant statements)
        boost::mutex m_Mutex;
};

#endif
