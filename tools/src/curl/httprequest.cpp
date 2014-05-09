#include "down.h"
#include "curl/curl.h"
#include "shared/Convert.h"
#include "shared/Util.h"
#include <assert.h>

class httprequest
{
	struct post_list
	{
			std::string key;
			std::string val;
	};
	std::vector<post_list> m_sendlist;
	std::string  m_result;
	std::vector<char* > m_resouce;
public:
	~httprequest()
	{
		for (int i=0;i!=m_resouce.size();i++)
		{
			delete[] m_resouce[i];
		}
	}
	static size_t post_recv(void *ptr, size_t size, size_t nmemb, void *userp)
	{
		assert(userp!=NULL);
		httprequest* request=(httprequest*)userp;
		return request->_post_recv(ptr,size,nmemb);
	}
	size_t _post_recv(void *ptr, size_t size, size_t nmemb)
	{
		assert(size>=0&&nmemb>=0);
		sLog->outTrace(LOG_FILTER_CUR_RECV,_T("curl recv data"));
		sLog->outDebug(LOG_FILTER_CUR_RECV,_T("curl recv data size=%d,nmemb=%d"),size,nmemb);
		for(int m=0;m!=nmemb;m++)
		{
			for(int i=0;i!=size;i++)
			{
				m_result.push_back(*((char*)ptr+m*size+i));
			}
		}

		sLog->outTrace(LOG_FILTER_CUR_RECV,"当前buffe内容 :%s",m_result.c_str());
		return size*nmemb;
	}
	void curl_clear_send_data()
	{
		sLog->outTrace(LOG_FILTER_CURL_POST,_T("清空post数据"));
		m_sendlist.clear();
	}
	int curl_add_send_data(const std::string &key,const std::string& var)
	{
		post_list _senddata;
		_senddata.key=key;
		_senddata.val=var;
		m_sendlist.push_back(_senddata);
		return CURLE_OK;
	}

	int curl_get_data(std::string url, char** result,int* resultsize,int timeoutSec)
	{
		std::vector<char> _result;
		int ret=_curl_get_data(url,_result,timeoutSec);
		if(!_result.empty())
		{
		(*result)=new char[_result.size()];
		memcpy(*result,&(_result[0]),_result.size());
		m_resouce.push_back((*result));
		*resultsize=_result.size();
		}
		return ret;
	}
	int _curl_get_data(std::string url,std::vector<char>& result,int timeoutSec)
	{
		curl_download_init();
		CURLcode retcode;
		CURL* easy_headle=curl_easy_init();
		 curl_easy_setopt(easy_headle, CURLOPT_HEADER, 0);
		 char errorBuffer[CURL_ERROR_SIZE]={0};
		 curl_easy_setopt(easy_headle, CURLOPT_ERRORBUFFER, errorBuffer);
		 curl_easy_setopt(easy_headle,CURLOPT_URL,url.c_str());
		 curl_easy_setopt(easy_headle, CURLOPT_FOLLOWLOCATION, 1); 
		 curl_easy_setopt(easy_headle,CURLOPT_WRITEFUNCTION,post_recv);
		 curl_easy_setopt(easy_headle,CURLOPT_WRITEDATA,this);
		 retcode=curl_easy_perform(easy_headle);
			if(retcode!=CURLE_OK)
			{
				sLog->outError(LOG_FILTER_CURL_POST,_T("从 %s   GET数据失败,错误码%d"),url.c_str(),retcode);
				result.clear();
				m_result.clear();
				curl_easy_cleanup(easy_headle);
				return retcode;
			}
			int code=CURLE_OK;
			retcode=curl_easy_getinfo(easy_headle, CURLINFO_RESPONSE_CODE , &code);
			curl_easy_cleanup(easy_headle);
			if(retcode!=CURLE_OK)
			{
				sLog->outError(LOG_FILTER_CURL_POST,_T("从 %s   GET数据失败,错误码%d"),url.c_str(),retcode);
				result.clear();
				m_result.clear();
				return retcode;
			}
			if(code!=200)
			{
				sLog->outError(LOG_FILTER_CURL_POST,_T("从 %s   GET数据失败,返回值%d"),url.c_str(),code);
			}
			result.assign(m_result.begin(),m_result.end());
			m_result.clear();
			return code;
	}
	int  curl_send_data(std::string url,char** result,int* resultsize,int timeoutSec)
	{
		std::vector<char> _result;
		int ret=_curl_send_data(url,_result,timeoutSec);
		if(!_result.empty())
		{
			(*result)=new char[_result.size()];

			memcpy(*result,&(_result[0]),_result.size());
			m_resouce.push_back((*result));
			*resultsize=_result.size();
		}
		
		return ret;
	}
	int _curl_send_data(std::string url,std::vector<char>& result,int timeoutSec)
	{
		curl_download_init();
		CURLcode retcode;
		CURL* easy_headle=curl_easy_init();
		std::string poststr;
		for (int i=0;i!=m_sendlist.size();i++)
		{
			if(i!=0)poststr+="&";
			poststr+=m_sendlist[i].key;
			poststr+="=";
			poststr+=CConvert::enBase64(m_sendlist[i].val);
		}
		 curl_easy_setopt(easy_headle, CURLOPT_HEADER, 0);
		 char errorBuffer[CURL_ERROR_SIZE]={0};
		 curl_easy_setopt(easy_headle, CURLOPT_ERRORBUFFER, errorBuffer);
		curl_easy_setopt(easy_headle,CURLOPT_URL,url.c_str());
		curl_easy_setopt(easy_headle,CURLOPT_POSTFIELDS,poststr.c_str());
		curl_easy_setopt(easy_headle,CURLOPT_POST,1);
		curl_easy_setopt(easy_headle,CURLOPT_TIMEOUT,timeoutSec);
		curl_easy_setopt(easy_headle, CURLOPT_FOLLOWLOCATION, 1); 
		curl_easy_setopt(easy_headle,CURLOPT_WRITEFUNCTION,post_recv);
		curl_easy_setopt(easy_headle,CURLOPT_WRITEDATA,this);
	
		sLog->outDebug(LOG_FILTER_CURL_POST,"post data: %s",poststr.c_str());
		retcode=curl_easy_perform(easy_headle);

		if(retcode!=CURLE_OK)
		{
			sLog->outError(LOG_FILTER_CURL_POST,_T("POST 数据到 %s 失败,发送数据%s, 返回值 %d"),url.c_str(),poststr.c_str(),retcode);
			result.clear();
			m_result.clear();
			curl_easy_cleanup(easy_headle);
			return retcode;
		}
		int code=CURLE_OK;
	    retcode=curl_easy_getinfo(easy_headle, CURLINFO_RESPONSE_CODE , &code);
		curl_easy_cleanup(easy_headle);
		if(retcode!=CURLE_OK)
		{
			sLog->outError(LOG_FILTER_CURL_POST,_T("POST 数据到 %ls 失败,发送数据%ls, 返回值 %d"),url.c_str(),poststr.c_str(),retcode);
			result.clear();
			m_result.clear();
			return retcode;
		}
		if(code!=200)
		{
			sLog->outDebug(LOG_FILTER_CURL_POST,_T("POST 数据到 %ls 失败,发送数据%ls, 返回值 %d"),url.c_str(),poststr.c_str(),code);
		}
		result.assign(m_result.begin(),m_result.end());
		m_result.clear();
		return code;
	}
};
curl_http_handle* curl_http_create()
{	
	curl_download_init();
	return (curl_http_handle*)new httprequest;
}
void curl_http_free(curl_http_handle* handle)
{
	curl_download_init();
	delete (httprequest*)handle;
}
int curl_add_send_data(curl_http_handle* http_handle,const char *key,const char* var)
{
	curl_download_init();
	assert(http_handle!=NULL);
	httprequest* req=(httprequest*)http_handle;
	return req->curl_add_send_data(key,var);
}
void curl_clear_send_data(curl_http_handle* http_handle)
{
	curl_download_init();
	assert(http_handle!=NULL);
	httprequest* req=(httprequest*)http_handle;
	return req->curl_clear_send_data();
}

int  curl_send_data(curl_http_handle* http_handle,const char* url,char** result,int* resultsize,int timeoutSec)
{
	curl_download_init();
	assert(http_handle!=NULL);
	httprequest* req=(httprequest*)http_handle;
	return req->curl_send_data(url,result,resultsize,timeoutSec);
}
int curl_get_data_from_url(curl_http_handle* http_handle,const char* url,char** result,int* resultsize,int timeoutSec)
{
	curl_download_init();
	assert(http_handle!=NULL);
	httprequest* req=(httprequest*)http_handle;
	return req->curl_get_data(url,result,resultsize,timeoutSec);
}
