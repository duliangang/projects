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
		printf("conf文件错误...请检查目录下logtest.conf文件");
		return 0;
	}
	sLog->LoadFromConfig(&testConfig);
	sLog->outDebug(LOG_FILTER_TEST,_T("this is a test string %s"),_T("测试一下 不要介意"));
	sLog->outDebug(LOG_FILTER_TEST1,_T("this is a test string %s"),_T("测试一下 不要介意"));
	sLog->outDebug(LOG_FILTER_TEST2,_T("this is a test string %s"),_T("测试一下 不要介意"));
	sLog->outDebug(LOG_FILTER_TEST3,_T("this is a test string %s"),_T("测试一下 不要介意"));
	int i;
	std::cin>>i;
	return 1;
}