#include "log/log.h"
#include "config/config.h"
#include <string>
#include <vector>
enum LogFilterType
{
	LOG_FILTER_CUR_RET,
	LOG_FILTER_CUR_URL,
	LOG_FILTER_CURL_SEND,
	LOG_FILTER_CURL_POST,
	LOG_FILTER_CURL_SET,
	LOG_FILTER_CUR_RECV,
	LOG_FILTER_CUR_HEAD,
	LOG_FILTER_CUR_DOWN,
};
struct curl_http_handle;
struct curl_down_handle;
typedef int (*download_callback_fun)(void* pPar,curl_down_handle* complete_handle);
typedef int (*download_progress_fun)(void* pPar,unsigned int totalsize,unsigned int nowsize);

curl_http_handle* curl_http_create();
int curl_add_send_data(curl_http_handle* http_handle,const char *key,const char* var);
void curl_clear_send_data(curl_http_handle* http_handle);
int  curl_send_data(curl_http_handle* http_handle,std::string url,std::vector<char> result,int timeoutSec);


curl_down_handle * curl_download_handle_create(const char* url,const  char*  filename);
int curl_download_stop(curl_down_handle* down_handle);
int curl_download_start(curl_down_handle* down_handle);
int curl_download_set_complete_fun(curl_down_handle* down_handle,void* pPar,download_callback_fun complete_fun);
int curl_download_set_error_fun(curl_down_handle* down_handle,void* pPar,download_callback_fun error_fun);
int curl_download_set_progress_fun(curl_down_handle* down_handle,void* pPar,download_progress_fun progress_fun);
int curl_download_handle_free(curl_down_handle* down_handle);