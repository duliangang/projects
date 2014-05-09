
#include "../shared/Define.h"
#define BOOST_TEST_MODULE mysqltest
#include <boost/test/unit_test.hpp> 
#include "TestDatabase.h"
#include "../QueryHolder.h"
BOOST_AUTO_TEST_SUITE (mysqltest) // name of the test suite is stringtest

	BOOST_AUTO_TEST_CASE (synch_query)
{
	TestDatabaseWorkerPool _test;
	_test.Open("127.0.0.1;3306;root;111111;test",2,2);

	QueryResult res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	for (int i=0;i!=100;i++)
	{	
		_test.PQuery(querytestsql[TESTSQL_INSERT],i,i,"teststring","now()");
	}
	res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==100);
	for (int i=0;i!=100;i++)
	{
		_test.PQuery(querytestsql[TESTSQL_UPDATECOLUMN1],i*10,i);
	}
	for (int i=0;i!=100;i++)
	{
		res=_test.PQuery(querytestsql[TESTSQL_SELECT],i);
		BOOST_REQUIRE(res);
		BOOST_REQUIRE(res->GetRowCount()==1);
		Field* field=res->Fetch();
		uint32 field0=field[0].GetUInt32();
		uint32 field1=field[1].GetUInt32();
		BOOST_REQUIRE(field1==field0*10);
	}

	for (int i=0;i!=100;i++)
	{
		_test.PQuery(querytestsql[TESTSQL_DELETE],i);
	}


	res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	_test.Close();
}
BOOST_AUTO_TEST_CASE (sqltransaction)
{
	TestDatabaseWorkerPool _test;
	_test.Open("127.0.0.1;3306;root;111111;test",2,2);

	QueryResult res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);

	SQLTransaction sqltransation=_test.BeginTransaction();
	for (int i=0;i!=100;i++)
	{
		sqltransation->PAppend(querytestsql[TESTSQL_INSERT],i,i,"teststring","now()");
	}
	_test.DirectCommitTransaction(sqltransation);
	res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==100);
	for (int i=0;i!=100;i++)
	{
		_test.PQuery(querytestsql[TESTSQL_UPDATECOLUMN1],i*10,i);
	}
	for (int i=0;i!=100;i++)
	{
		res=_test.PQuery(querytestsql[TESTSQL_SELECT],i);
		BOOST_REQUIRE(res);
		BOOST_REQUIRE(res->GetRowCount()==1);
		Field* field=res->Fetch();
		uint32 field0=field[0].GetUInt32();
		uint32 field1=field[1].GetUInt32();
		BOOST_REQUIRE(field1==field0*10);
	}
	SQLTransaction delteSqltransaction=_test.BeginTransaction();
	for (int i=0;i!=100;i++)
	{
		delteSqltransaction->PAppend(querytestsql[TESTSQL_DELETE],i);
	}
	_test.CommitTransaction(delteSqltransaction);

	_test.Close();
}

BOOST_AUTO_TEST_CASE (synch_stmt)
{
	TestDatabaseWorkerPool _test;
	_test.Open("127.0.0.1;3306;root;111111;test",2,2);
	PreparedStatement* _stmt=_test.GetPreparedStatement(TEST_COUNT_TESTDATA_STATEMENTS_SYNCH);
	PreparedQueryResult res=_test.Query(_stmt);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_INS_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,i);
		_stmt->setInt32(1,i);
		_stmt->setString(2,"teststring");
		_stmt->setString(3,"now()");
		res=_test.Query(_stmt);
	}
	_stmt=_test.GetPreparedStatement(TEST_COUNT_TESTDATA_STATEMENTS_SYNCH);
	res=_test.Query(_stmt);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==100);
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_UPD_TESTDATACOLUMN1_STATEMENTS_SYNCH);
		_stmt->setInt32(0,i*10);
		_stmt->setUInt32(1,i);
		_test.Query(_stmt);
	}
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_SEL_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,i);
		res=_test.Query(_stmt);
		BOOST_REQUIRE(res);
		BOOST_REQUIRE(res->GetRowCount()==1);
		Field* field=res->Fetch();
		BOOST_REQUIRE(field[1].GetUInt32()==field[0].GetUInt32()*10);
	}

	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_DEL_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,i);
		_test.Query(_stmt);
	}


	_stmt=_test.GetPreparedStatement(TEST_COUNT_TESTDATA_STATEMENTS_SYNCH);
	res=_test.Query(_stmt);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	_test.Close();
}
class TestCallback
{
	TestDatabaseWorkerPool& m_test;
public:
	
	TestCallback(TestDatabaseWorkerPool& _test):m_test(_test)
	{
		
	}
	void callback_select_Query(QueryResult res)
	{
		BOOST_CHECK(res);
		BOOST_CHECK(res->GetRowCount()==1);
		Field* field=res->Fetch();
		uint32 field0=field[0].GetUInt32();
		uint32 field1=field[1].GetUInt32();
		BOOST_REQUIRE(field1==field0*10);
		PreparedStatement*_stmt=m_test.GetPreparedStatement(TEST_DEL_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,field[0].GetUInt32());
		m_test.Query(_stmt);
	}
	void callback_select_PreparedQuery(PreparedQueryResult res)
	{
		BOOST_CHECK(res);
		BOOST_CHECK(res->GetRowCount()==1);
		Field* field=res->Fetch();
		BOOST_REQUIRE(field[1].GetUInt32()==field[0].GetUInt32()*10);
		PreparedStatement*_stmt=m_test.GetPreparedStatement(TEST_DEL_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,field[0].GetUInt32());
		m_test.Query(_stmt);
	}
	void callback_select_holder(SQLQueryHolder sqlquery)
	{
		for (int i=0;i!=sqlquery->GetSize();i++)
		{
			PreparedQueryResult  res=sqlquery->GetPreparedResult(i);
			BOOST_REQUIRE(res);
			BOOST_REQUIRE(res->GetRowCount()==1);
			Field* field=res->Fetch();
			uint32 field0=field[0].GetUInt32();
			uint32 field1=field[1].GetUInt32();
			BOOST_REQUIRE(field1==field0*10);
		}
	}
};

BOOST_AUTO_TEST_CASE (holder_test)
{
	TestDatabaseWorkerPool _test;
	boost::shared_ptr<TestCallback> testCallbck(new TestCallback(_test));
	_test.Open("127.0.0.1;3306;root;111111;test",2,2);

	PreparedStatement* _stmt=_test.GetPreparedStatement(TEST_COUNT_TESTDATA_STATEMENTS_SYNCH);
	PreparedQueryResult res=_test.Query(_stmt);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	SQLTransaction sqltransation=_test.BeginTransaction();
	
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_INS_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,i);
		_stmt->setInt32(1,i);
		_stmt->setString(2,"teststring");
		_stmt->setString(3,"now()");
		sqltransation->Append(_stmt);
	}
	_test.DirectCommitTransaction(sqltransation);
	sqltransation=_test.BeginTransaction();
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_UPD_TESTDATACOLUMN1_STATEMENTS_SYNCH);
		_stmt->setInt32(0,i*10);
		_stmt->setUInt32(1,i);
		sqltransation->Append(_stmt);
	}
	_test.DirectCommitTransaction(sqltransation);
	

	SQLQueryHolder holder(new QueryHolder());
	holder->SetSize(100);
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_SEL_TESTDATA_STATEMENTS_ASYNC);
		_stmt->setUInt32(0,i);
		
		holder->SetPreparedQuery(i,_stmt);
	}
	_test.DelayQueryHolder(holder,boost::bind(&TestCallback::callback_select_holder,testCallbck,_1));
	Sleep(5000);
	_test.Close();

}
BOOST_AUTO_TEST_CASE (async_query)
{
	TestDatabaseWorkerPool _test;
	boost::shared_ptr<TestCallback> testCallbck(new TestCallback(_test));
	_test.Open("127.0.0.1;3306;root;111111;test",2,2);

	QueryResult res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	for (int i=0;i!=100;i++)
	{
		_test.PQuery(querytestsql[TESTSQL_INSERT],i,i,"teststring","now()");
	}
	res=_test.Query(querytestsql[TESTSQL_COUNT]);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==100);
	for (int i=0;i!=100;i++)
	{
		_test.PQuery(querytestsql[TESTSQL_UPDATECOLUMN1],i*10,i);
	}
	for (int i=0;i!=100;i++)
	{
		_test.AsyncPQuery(boost::bind(&TestCallback::callback_select_Query,testCallbck,_1),querytestsql[TESTSQL_SELECT],i);
	}
	Sleep(5000);
	_test.Close();
}

BOOST_AUTO_TEST_CASE (async_stmt)
{
	TestDatabaseWorkerPool _test;
	boost::shared_ptr<TestCallback> testCallbck(new TestCallback(_test));
	_test.Open("127.0.0.1;3306;root;111111;test",2,2);
	PreparedStatement* _stmt=_test.GetPreparedStatement(TEST_COUNT_TESTDATA_STATEMENTS_SYNCH);
	PreparedQueryResult res=_test.Query(_stmt);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==0);
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_INS_TESTDATA_STATEMENTS_SYNCH);
		_stmt->setUInt32(0,i);
		_stmt->setInt32(1,i);
		_stmt->setString(2,"teststring");
		_stmt->setString(3,"now()");
		res=_test.Query(_stmt);
	}
	_stmt=_test.GetPreparedStatement(TEST_COUNT_TESTDATA_STATEMENTS_SYNCH);
	res=_test.Query(_stmt);
	BOOST_REQUIRE(res);
	BOOST_REQUIRE((*res)[0].GetUInt64()==100);
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_UPD_TESTDATACOLUMN1_STATEMENTS_SYNCH);
		_stmt->setInt32(0,i*10);
		_stmt->setUInt32(1,i);
		_test.Query(_stmt);
	}
	for (int i=0;i!=100;i++)
	{
		_stmt=_test.GetPreparedStatement(TEST_SEL_TESTDATA_STATEMENTS_ASYNC);
		_stmt->setUInt32(0,i);
		_test.AsyncQuery(_stmt,boost::bind(&TestCallback::callback_select_PreparedQuery,testCallbck,_1));
	}
	Sleep(5000);
	_test.Close();
}



BOOST_AUTO_TEST_SUITE_END( )