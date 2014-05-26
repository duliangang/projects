
#ifndef _WORLDDATABASE_H
#define _WORLDDATABASE_H

#include "../DatabaseWorkerPool.h"
#include "../MySQLConnection.h"

class TestDatabaseConnection : public MySQLConnection
{
    public:
        //- Constructors for sync and async connections
        TestDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) {}
        TestDatabaseConnection(BlockQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) {}

        //- Loads database type specific prepared statements
        void DoPrepareStatements();
};

typedef DatabaseWorkerPool<TestDatabaseConnection> TestDatabaseWorkerPool;
enum 
{
	TESTSQL_INSERT,
	TESTSQL_UPDATE,
	TESTSQL_DELETE,
	TESTSQL_SELECT,
	TESTSQL_COUNT,
	TESTSQL_UPDATECOLUMN1,
	TESTSQL_UPDATECOLUMN2,
	TESTSQL_UPDATECOLUMN3,
};
static const char * querytestsql[]=
{
	"insert into mysql_test(id,test_column1,test_column2,test_column3) values (%d,%d,'%s',%s)",
	"update mysql_test set test_column1 = %d,test_column2 = %d, test_column3 = %s where id = %d",
	"delete  from mysql_test where id=%d",
	"select id,test_column1,test_column2,test_column3 from mysql_test where id =%d",
	"select count(*) from mysql_test",
	"update mysql_test set test_column1 = %d where id = %d",
	"update mysql_test set test_column2 = '%s' where id = %d",
	"update mysql_test set test_column3 = '%s' where id = %d",
};
static const char * stmttestsql[]=
{
	"insert into mysql_test(id,test_column1,test_column2,test_column3) values (?,?,?,?)",
	"update mysql_test set test_column1 = ?,test_column2 = ?, test_column3 = ? where id = ?",
	"delete from mysql_test where id=?",
	"select id,test_column1,test_column2,test_column3 from mysql_test where id =?",
	"select count(*) from mysql_test",
	"update mysql_test set test_column1 = ? where id = ?",
	"update mysql_test set test_column2 = ? where id = ?",
	"update mysql_test set test_column3 = ? where id = ?",
};
enum WorldDatabaseStatements
{
	TEST_INS_TESTDATA_STATEMENTS_ASYNC,
	TEST_UPD_TESTDATA_STATEMENTS_ASYNC,
	TEST_DEL_TESTDATA_STATEMENTS_ASYNC,
	TEST_SEL_TESTDATA_STATEMENTS_ASYNC,
	TEST_COUNT_TESTDATA_STATEMENTS_ASYNC,
	TEST_UPD_TESTDATACOLUMN1_STATEMENTS_ASYNC,
	TEST_UPD_TESTDATACOLUMN2_STATEMENTS_ASYNC,
	TEST_UPD_TESTDATACOLUMN3_STATEMENTS_ASYNC,

	TEST_INS_TESTDATA_STATEMENTS_SYNCH,
	TEST_UPD_TESTDATA_STATEMENTS_SYNCH,
	TEST_DEL_TESTDATA_STATEMENTS_SYNCH,
	TEST_SEL_TESTDATA_STATEMENTS_SYNCH,
	TEST_COUNT_TESTDATA_STATEMENTS_SYNCH,
	TEST_UPD_TESTDATACOLUMN1_STATEMENTS_SYNCH,
	TEST_UPD_TESTDATACOLUMN2_STATEMENTS_SYNCH,
	TEST_UPD_TESTDATACOLUMN3_STATEMENTS_SYNCH,

    MAX_TESTDATABASE_STATEMENTS
};

#endif
