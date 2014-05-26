#include <WinSock2.h>
#include <Windows.h>
#include <boost/test/unit_test.hpp> 
#define BOOST_TEST_MODULE testlog
#include "log.h"
#include "../../config/config.h"
enum LogFilters
{
	TestConsole=0,
	TestFile,
	TestConsoleColorRed,
	TestFiletimestampMaxFileSize1M,
	MAXFILTER,
};
BOOST_AUTO_TEST_SUITE (mysqltest) // name of the test suite is stringtest

BOOST_AUTO_TEST_CASE (log_test)
{
	Config testConfig;
	if(!testConfig.ReadFile(_T("E:\\git-project\\tools\\bin\\libD\\log.conf")))
	{
		printf("conf�ļ�����...����Ŀ¼��logtest.conf�ļ�");
		BOOST_REQUIRE(false);
	}
	sLog->LoadFromConfig(&testConfig);
	for (int i=0;i!=1000;i++)
	{
		for (uint8_t filter=TestConsole;filter!=MAXFILTER;filter++)
		{
			sLog->outTrace(filter,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
		}
		for (uint8_t filter=TestConsole;filter!=MAXFILTER;filter++)
		{
			sLog->outDebug(filter,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
		}
		for (uint8_t filter=TestConsole;filter!=MAXFILTER;filter++)
		{
			sLog->outInfo(filter,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
		}
		for (uint8_t filter=TestConsole;filter!=MAXFILTER;filter++)
		{
			sLog->outWarn(filter,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
		}
		for (uint8_t filter=TestConsole;filter!=MAXFILTER;filter++)
		{
			sLog->outError(filter,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
		}
		for (uint8_t filter=TestConsole;filter!=MAXFILTER;filter++)
		{
			sLog->outFatal(filter,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
		}
	}
	Sleep(5000);
}
BOOST_AUTO_TEST_SUITE_END( )