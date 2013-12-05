#include "testlog.h"
enum LogFilterType
{
	LOG_FILTER_TEST=0,
	LOG_FILTER_TEST1,
	LOG_FILTER_TEST2,
	LOG_FILTER_TEST3,
};

int main()
{
	Config testConfig;
	if(!testConfig.ReadFile(_T("logtest.conf")))
	{
		printf("conf�ļ�����...����Ŀ¼��logtest.conf�ļ�");
		return 0;
	}
	sLog->LoadFromConfig(&testConfig);
	sLog->outDebug(LOG_FILTER_TEST,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
	sLog->outDebug(LOG_FILTER_TEST1,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
	sLog->outDebug(LOG_FILTER_TEST2,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
	sLog->outDebug(LOG_FILTER_TEST3,_T("this is a test string %s"),_T("����һ�� ��Ҫ����"));
	int i;
	std::cin>>i;
	return 1;
}