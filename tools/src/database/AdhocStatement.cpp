#include "AdhocStatement.h"
#include "MySQLConnection.h"
/*! Basic, ad-hoc queries. */
BasicStatementTask::BasicStatementTask(const char* sql) :
m_has_result(false)
{
    m_sql = strdup(sql);
}

BasicStatementTask::BasicStatementTask(const char* sql,BasicStatementCallbackFunc callback) :
m_has_result(true),
m_callback(callback)
{
    m_sql = strdup(sql);
}

BasicStatementTask::~BasicStatementTask()
{
    free((void*)m_sql);
}

bool BasicStatementTask::Execute()
{
    if (m_has_result)
    {
        ResultSet* result = m_conn->Query(m_sql);
        if (!result || !result->GetRowCount())
        {
            delete result;
           
            return false;
        }
        result->NextRow();
		m_callback(boost::shared_ptr<ResultSet>(result));
        return true;
    }

    return m_conn->Execute(m_sql);
}
