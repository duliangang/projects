//#include "down.h"
//#include "curl/curl.h"
//#include "shared/Convert.h"
//#include "shared/Util.h"
//#include <assert.h>
//#pragma comment(lib,"libcurl.lib")
//#pragma comment ( lib, "ws2_32.lib" )
//#pragma comment ( lib, "winmm.lib" )
//#pragma comment (lib, "wldap32.lib")
//struct keyval
//{
//	std::string key;
//	std::string val;
//};
//static  std::vector<keyval> sendlist;
//static bool init=false;
//std::wstring logstr;
//void curl_clear_send_data()
//{
//	sLog->outTrace(LOG_FILTER_CURL_POST,_T("清空post数据"));
//	sendlist.clear();
//}
//int curl_add_send_data(const std::string &key,const std::string& var)
//{
//  CURLcode retcode;
//  keyval _senddata;
//  _senddata.key=key;
//  _senddata.val=var;
//  sendlist.push_back(_senddata);
//  return CURLE_OK;
//}
//size_t post_recv(void *ptr, size_t size, size_t nmemb, void *userp)
//{
//	assert(size>=0&&nmemb>=0);
//	std::string* buffer=(std::string*)userp;
//	sLog->outTrace(LOG_FILTER_CUR_RECV,_T("curl recv data"));
//	sLog->outDebug(LOG_FILTER_CUR_RECV,_T("curl recv data size=%d,nmemb=%d"),size,nmemb);
//	for(int m=0;m!=nmemb;m++)
//	{
//		for(int i=0;i!=size;i++)
//		{
//			buffer->push_back(*((char*)ptr+m*size+i));
//		}
//	}
//	Utf8toWStr(buffer->c_str(),logstr);
//	sLog->outTrace(LOG_FILTER_CUR_RECV,_T("当前buffe内容 :%s"),logstr.c_str());
//	return size*nmemb;
//}
//int curl_send_data(std::string url,std::vector<char> result,int timeoutSec)
//{
//	if(!init)
//	{
//		sLog->outTrace(LOG_FILTER_CURL_SEND,_T("curl尚未初始化,就欲写入数据"));
//		return 0;
//	}
//	CURLcode retcode;
//	CURL* easy_headle=curl_easy_init();
//	std::string poststr;
//	for (int i=0;i!=sendlist.size();i++)
//	{
//		if(i!=0)poststr+="&";
//		poststr+=sendlist[i].key;
//
//
//		poststr+="=";
//
//		poststr+=CConvert::enBase64(sendlist[i].val);
//	}
//	std::string recvstr;
//	 curl_easy_setopt(easy_headle, CURLOPT_HEADER, 0);
//	 char errorBuffer[CURL_ERROR_SIZE*24]={0};
//	 curl_easy_setopt(easy_headle, CURLOPT_ERRORBUFFER, errorBuffer);
//	 curl_easy_setopt(easy_headle, CURLOPT_HEADER, 1);
//	curl_easy_setopt(easy_headle,CURLOPT_URL,url.c_str());
//	curl_easy_setopt(easy_headle,CURLOPT_POSTFIELDS,poststr.c_str());
//	curl_easy_setopt(easy_headle,CURLOPT_POST,1);
//	curl_easy_setopt(easy_headle,CURLOPT_TIMEOUT,timeoutSec);
//	curl_easy_setopt(easy_headle, CURLOPT_FOLLOWLOCATION, 1); 
//	curl_easy_setopt(easy_headle,CURLOPT_WRITEFUNCTION,post_recv);
//	curl_easy_setopt(easy_headle,CURLOPT_WRITEDATA,&recvstr);
//
//	//curl_easy_setopt(easy_headle,CURLOPT_READFUNCTION,post_recv);
//	//curl_easy_setopt(easy_headle,CURLOPT_READDATA,&recvstr);
//	sLog->outDebug(LOG_FILTER_CURL_POST,_T("post data: %s"),ConsoleToWStr(poststr,logstr).c_str());
//	retcode=curl_easy_perform(easy_headle);
//	if(retcode!=CURLE_OK)
//	{
//		
//		sLog->outError(LOG_FILTER_CURL_POST,_T("POST 数据到 %ls 失败,发送数据%ls, 返回值 %d"),ConsoleToWStr(url,logstr).c_str(),ConsoleToWStr(poststr,logstr).c_str(),retcode);
//		result.clear();
//		return retcode;
//	}
//	result.assign(recvstr.begin(),recvstr.end());
//	curl_easy_cleanup(easy_headle);
//	return 0;
//}
//
//int main()
//{
//	std::string version="1.0.0";
//	std::string option="0";
//	std::string cmac="00-00-00-00-00-00-00";
//	std::string straddr="128.0.0.2";
//	std::string os="windows";
//	std::string level="5";
//	std::string progid="1000";
//	std::string prouser="jingww";
//
//   curl_add_send_data("p1",version);
//   curl_add_send_data("p2",straddr);
//   curl_add_send_data("p3",cmac);
//   curl_add_send_data("p4",os);
//   curl_add_send_data("p5",level);
//   curl_add_send_data("p10",option);
//   curl_add_send_data("p100",progid);
//   curl_add_send_data("p101",prouser);
//   std::vector<char> result;
//   int retval=curl_send_data("http://10.3.253.81:8080/htupdate/htupdate",result,500);
//   if(retval!=0)
//   {
//	   printf("send error");
//   }
//   else
//   {
//	   printf("send succese");
//   }
//   curl_global_cleanup();
//   return 0;
//}