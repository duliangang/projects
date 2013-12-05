#include "curl/down.h"
#include "thread/apr/apr_base.h"
#pragma comment (lib,"httpdown.lib")
int complete_fun(void* pPar,curl_down_handle* complete_handle)
{
	char* filename=(char*)pPar;
	printf("%s�������\n",filename);
	return 0;
}
int progress_fun(void* pPar,unsigned int nowsize,unsigned int totalsize)
{
	char* filename=(char*)pPar;
	printf("%s�ļ��ܴ�С %d,������%d,���ذٷֱ�%f\n",filename,totalsize,nowsize,100*((double)nowsize/totalsize));
	return 0;
}
int error_fun(void* pPar,curl_down_handle* complete_handle)
{
	char* filename=(char*)pPar;
	printf("%s���ش���\n",filename);
	return 0;
}
//class testclass :public Task_Base
//{
//public:
//	curl_down_handle* handle;
//	virtual void svc()
//	{
//
//		char* str="c:\\testdown\\qq2013sp51.exe";
//		curl_download_set_complete_fun(handle,str,complete_fun);
//		curl_download_set_error_fun(handle,str,error_fun);
//		curl_download_set_progress_fun(handle,str,progress_fun);
//		if(curl_download_start(handle,10)!=0)
//		{
//			printf("%s����ʧ��\n",str);
//		}
//	}
//};
int main()
{
	curl_download_init();
	curl_down_handle * handle=curl_download_handle_create("http://10.3.253.82/rsync/HT-Security/HT_UPDATE/1.0.1/htup date.war","G:\\testdown\\htup date.war");

	char* str="G:\\testdown\\htup date.war";
	curl_download_set_complete_fun(handle,str,complete_fun);
	curl_download_set_error_fun(handle,str,error_fun);
	curl_download_set_progress_fun(handle,str,progress_fun);
	if(curl_download_start(handle,10)!=0)
	{
		printf("%s����ʧ��\n",str);
	}
	curl_download_handle_free(handle);
	return 0;
}
