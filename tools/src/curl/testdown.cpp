//#include "down.h"
//#include "../thread/apr/apr_base.h"
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <tchar.h>
#pragma comment(lib,"Ws2_32.lib")
//int complete_fun(void* pPar,curl_down_handle* complete_handle)
//{
//	char* filename=(char*)pPar;
//	printf("%s下载完成\n",filename);
//	return 0;
//}
//int progress_fun(void* pPar,unsigned int nowsize,unsigned int totalsize)
//{
//	char* filename=(char*)pPar;
//	printf("%s文件总大小 %d,已下载%d,下载百分比%f\n",filename,totalsize,nowsize,100*((double)nowsize/totalsize));
//	return 0;
//}
//int error_fun(void* pPar,curl_down_handle* complete_handle)
//{
//	char* filename=(char*)pPar;
//	printf("%s下载错误\n",filename);
//	return 0;
//}
#include <iostream>
int main()
{
//		curl_download_init();
//		curl_down_handle * handle=curl_download_handle_create("http://10.3.253.19/rsync/HT-Security/HT_UPDATE/1.1.0//data/%21vf1.dat","c:\\Program Files\\testdown\\qq2013sp51.exe");
//			char* str="c:\\testdown\\qq2013sp51.exe";
//			curl_download_set_complete_fun(handle,str,complete_fun);
//			curl_download_set_error_fun(handle,str,error_fun);
//			curl_download_set_progress_fun(handle,str,progress_fun);
//			if(curl_download_start(handle,10)!=0)
//			{
//				printf("%s下载失败\n",str);
//			}
//			curl_download_handle_free(handle);
//			return 0;
	in6_addr  buffer;
    InetPton(AF_INET6,_T("2001::da8:100"),&buffer);
	char bufferaddr[100];
	InetNtop(AF_INET6,&buffer,bufferaddr,sizeof(bufferaddr));
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
//			printf("%s下载失败\n",str);
//		}
//	}
//};
//int main()
//{
//	curl_download_init();
//	curl_down_handle * handle=curl_download_handle_create("//#include "down.h"
//#include "../thread/apr/apr_base.h"
//int complete_fun(void* pPar,curl_down_handle* complete_handle)
//{
//	char* filename=(char*)pPar;
//	printf("%s下载完成\n",filename);
//	return 0;
//}
//int progress_fun(void* pPar,unsigned int nowsize,unsigned int totalsize)
//{
//	char* filename=(char*)pPar;
//	printf("%s文件总大小 %d,已下载%d,下载百分比%f\n",filename,totalsize,nowsize,100*((double)nowsize/totalsize));
//	return 0;
//}
//int error_fun(void* pPar,curl_down_handle* complete_handle)
//{
//	char* filename=(char*)pPar;
//	printf("%s下载错误\n",filename);
//	return 0;
//}
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
//			printf("%s下载失败\n",str);
//		}
//	}
//};
//int main()
//{
//	curl_download_init();
//	curl_down_handle * handle=curl_download_handle_create("http://10.3.253.19/rsync/HT-Security/HT_UPDATE/1.1.0//data/%21vf1.dat","c:\\Program Files\\testdown\\qq2013sp51.exe");
//	testclass c;
//	c.handle=handle;
//	c.open();
//	Sleep(1000*20);
//	curl_download_stop(handle);
//	c.join();
//	curl_download_handle_free(handle);
//	return 0;
//}
//");
//	testclass c;
//	c.handle=handle;
//	c.open();
//	Sleep(1000*20);
//	curl_download_stop(handle);
//	c.join();
//	curl_download_handle_free(handle);
//	return 0;
//}
