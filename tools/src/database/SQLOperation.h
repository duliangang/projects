#ifndef _SQLOPERATION_H
#define _SQLOPERATION_H
//- Forward declare (don't include header to prevent circular includes)
class PreparedStatement;
class PreparedResultSet;
class ResultSet;
//- Union that holds element data
union SQLElementUnion
{
    PreparedStatement* stmt;
    const char* query;
};

//- Type specifier of our element data
enum SQLElementDataType
{
    SQL_ELEMENT_RAW,
    SQL_ELEMENT_PREPARED
};

//- The element
struct SQLElementData
{
    SQLElementUnion element;
    SQLElementDataType type;
};

//- For ambigious resultsets
union SQLResultSetUnion
{
    PreparedResultSet* presult;
    ResultSet* qresult;
};

class MySQLConnection;

class SQLOperation
{
    public:
        SQLOperation(): m_conn(0) {}
        virtual int call()
        {
            Execute();
            return 0;
        }
        virtual bool Execute() = 0;
        virtual void SetConnection(MySQLConnection* con) { m_conn = con; }

        MySQLConnection* m_conn;
};

#endif
