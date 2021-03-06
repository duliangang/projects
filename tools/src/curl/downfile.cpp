#include "down.h"
#include <direct.h>
#include <map>
#include <stdint.h>
#include <stdlib.h>
#include <io.h>
#include "curl/curl.h"
#include "shared/Convert.h"
#include "shared/Util.h"
#include <assert.h>
#define DOWN_FILE_APPEND_NAME ".td"
#define DOWN_CFG_FILE_APPEND_NAME ".td.cdf"
#pragma comment(lib,"libcurl.lib")
#pragma comment ( lib, "ws2_32.lib" )
#pragma comment ( lib, "winmm.lib" )
#pragma comment (lib, "wldap32.lib")
class curl_download
{
private:
	std::string m_url;
	std::string m_filename;
	std::wstring m_tempstr;
	download_callback_fun m_complete_fun;
	download_callback_fun m_error_fun;
	download_progress_fun m_progress_fun;
	void* m_complete_par;
	void* m_error_par;
	void* m_progress_par;
	bool m_resumeBrokenDown;
	bool m_bstop;
	CURLcode m_retCode;

private:
	int m_notdeferror;
	typedef struct _local_file_info
	{
		int local_file_size;
		int local_total_size;
	}local_file_info;
	local_file_info m_localinfo;
	int getDownloadFileLenth()
	{
		double downloadFileLenth = -1.0f;
		int count=0;
		while(count!=5)
		{
			CURL *handle = curl_easy_init();

			curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
			curl_easy_setopt(handle,CURLOPT_URL,m_url.c_str());
			curl_easy_setopt(handle, CURLOPT_HEADER, 1);    //只需要header头
			curl_easy_setopt(handle, CURLOPT_NOBODY, 1);    //不需要body
			m_retCode =curl_easy_perform(handle);

			if (m_retCode == CURLE_OK) 
			{
				curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
			} 
			else 
			{
				sLog->outError(LOG_FILTER_CUR_DOWN,"获取文件信息失败,url= %s,错误码%d",m_url.c_str(),m_retCode);
				downloadFileLenth = -1;
				count++;
				continue;
			}
			curl_easy_cleanup(handle);
			return downloadFileLenth;
		}
		
	}
	//return cache file size
	int getlocalCacheFileInfo()
	{
		sLog->outTrace(LOG_FILTER_CUR_DOWN,"读取本地文件信息:文件:%s.%s",m_filename.c_str(),"td.cfd");

		
		if(access(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),0)!=0)
		{
			remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
			remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
			m_localinfo.local_file_size=0;
			return 0;
		}

		FILE* fp=fopen(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str(),"r");
		if(fp==NULL)
		{
			return 0;
		}
		char filebuffer[1024*2]={0};
		fread(filebuffer,1,sizeof(filebuffer),fp);
		fclose(fp);
		fp=NULL;
		int localsize=0,totalsize=0;
		int scanf_count=sscanf(filebuffer,"%d,%d",&localsize,&totalsize);
		if(scanf_count!=2)
		{
		  sLog->outDebug(LOG_FILTER_CUR_DOWN,"坏掉的本地文件:文件%s.%s,文件内容%s",m_filename.c_str(),"td.cdf",filebuffer);
		  remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
		  remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
		  m_localinfo.local_file_size=0;
		  return 0;
		}
		m_localinfo.local_file_size=localsize;
		m_localinfo.local_total_size=totalsize;

		return localsize;
	}
	FILE* m_localFile;
	FILE* m_localcfgFile;
	size_t _process_data(void * buffer, size_t size, size_t nmemb)
	{
		assert(buffer!=NULL);
		
		if(m_localFile==NULL)
		{
			FILE* _fp=fopen(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),"rb");
			long filesize=0;
			if(_fp!=NULL)
			{
				fseek(_fp,0,SEEK_END);
				filesize=ftell(_fp);
				fclose(_fp);
			}
			if(filesize!=0)
			{
				//已存在 只需要改动
				m_localFile=fopen(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),"rb+");
			}
			else
			{
				//不存在 需要创建
				m_localFile=fopen(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),"wb+");
			}
			if(m_localFile==NULL)
			{
				sLog->outFatal(LOG_FILTER_CUR_DOWN,"从%s下载文件到%s错误,可能是由于权限不够或者该文件正在被使用导致",m_url.c_str(),m_filename.c_str());
				remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
				remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
				return CURL_WRITEFUNC_PAUSE;
			}
			//判断此缓存文件是否是需要下载的文件
			sLog->outTrace(LOG_FILTER_CUR_DOWN,"缓存文件大小为%d",filesize);
			if(filesize!=0&&filesize!=m_localinfo.local_total_size)
			{
				sLog->outDebug(LOG_FILTER_CUR_DOWN,"删除错误的缓存文件%s,重新下载",std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
				fclose(m_localFile);
				remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
				remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
				
				m_localFile=fopen(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),"wb+");
				filesize=0;
			}

			if(filesize==0)
			{
				fseek(m_localFile,m_localinfo.local_total_size-1,SEEK_SET);
				fputc('\0',m_localFile);
			}
			fseek(m_localFile,m_localinfo.local_file_size,SEEK_SET);
		}
		size_t fsize=fwrite(buffer,size,nmemb,m_localFile);
		assert(fsize&&fsize+m_localinfo.local_file_size<=m_localinfo.local_total_size);
		m_localinfo.local_file_size+=fsize;

		//写入 cfg
		if(m_localcfgFile!=NULL)
		{
			fclose(m_localcfgFile);
			m_localcfgFile=NULL;
		}
		m_localcfgFile=fopen(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str(),"w+");
		if(m_localcfgFile==NULL)
		{
			sLog->outFatal(LOG_FILTER_CUR_DOWN,"从%s下载文件到%s 配置文件错误,可能是由于权限不够或者该文件正在被使用导致",m_url.c_str(),m_filename.c_str());
			return fsize;
		}
		fprintf(m_localcfgFile,"%d,%d",m_localinfo.local_file_size,m_localinfo.local_total_size);
		fflush(m_localcfgFile);
		fclose(m_localcfgFile);
		m_localcfgFile=NULL;
		if(m_progress_fun)
		m_progress_fun(m_progress_par,m_localinfo.local_file_size,m_localinfo.local_total_size);
		if(m_bstop)
		{
			m_notdeferror=0;
			sLog->outDebug(LOG_FILTER_CUR_DOWN,"停止从%s下载%s文件",m_url.c_str(),m_filename.c_str());
		}
		fflush(m_localFile);
		return m_bstop ? -1:fsize;
	}
	static size_t process_data(void * buffer, size_t size, size_t nmemb, void *downclass)
	{
		assert(downclass!=NULL&&buffer!=NULL);
		curl_download* down=(curl_download*)downclass;
		return down->_process_data(buffer,size,nmemb);
	}
	

	
	 long  GetLocalFileLength()
	{
		FILE* fp=fopen(m_filename.c_str(),"rb");
		if(fp==NULL)return -1;
		fseek(fp,0,SEEK_END);
		long length=ftell(fp);
		fclose(fp);
		return length;
	}
public:
	
	curl_download(const std::string& url,const std::string& filename):m_complete_par(NULL),m_error_par(NULL),m_resumeBrokenDown(true)\
		,m_complete_fun(NULL),m_error_fun(NULL),m_bstop(false),m_progress_fun(0),m_progress_par(NULL),m_localFile(NULL),m_localcfgFile(NULL)\
		,m_notdeferror(1)
	{
		assert(!url.empty()&&!url.empty());
		m_url=url;
		m_localinfo.local_file_size=0;
		m_localinfo.local_total_size=0;
		m_filename=filename;
	}
	void curl_download_stop()
	{
		sLog->outTrace(LOG_FILTER_CUR_DOWN,"请求停止下载%s文件",m_filename.c_str());
		m_bstop=true;
	}
#ifdef rename
#undef rename
#endif
	int curl_download_start(bool resumeBrokenDown,int connect_timeout)
	{
		sLog->outTrace(LOG_FILTER_CUR_DOWN,"文件%s开始下载",m_filename.c_str());
		m_resumeBrokenDown=resumeBrokenDown;
		m_localinfo.local_total_size=getDownloadFileLenth();
		long realyfilesize=GetLocalFileLength();
		if(realyfilesize>0&&realyfilesize==m_localinfo.local_total_size)
		{
			sLog->outDebug(LOG_FILTER_CUR_DOWN,"从%s下载的文件%s已存在",m_url.c_str(),m_filename.c_str());
			int tmp=0;
			std::string tmpstr;
			while(true)
			{
				char tmpc[60]={0};
				sprintf(tmpc,"._save%d",tmp++);
				if(access(std::string(m_filename+tmpc).c_str(),0)!=0)
				{
					tmpstr=m_filename+tmpc;
					break;
				}
			}
			sLog->outDebug(LOG_FILTER_CUR_DOWN,"原文件%s重命名为%s",m_filename.c_str(),tmpstr.c_str());
			rename(m_filename.c_str(),tmpstr.c_str());
		}
		if(m_localinfo.local_total_size<=0)
		{
			sLog->outError(LOG_FILTER_CUR_DOWN,"从%s下载%s 服务器返回一个错误的文件长度%d",m_url.c_str(),m_filename.c_str(),m_localinfo.local_total_size);
			m_localinfo.local_total_size=0;
			return m_retCode;
		}
		CURL *handle = curl_easy_init();
		assert(handle);
		curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
		long local_size=getlocalCacheFileInfo();

		sLog->outTrace(LOG_FILTER_CUR_DOWN,"文件%s本地缓存信息:大小%d,总大小%d",m_filename.c_str(),local_size,m_localinfo.local_total_size);
		if(local_size>0)
		{
			char tmpc[1024]={0};
			sprintf(tmpc,"%d-%d",m_localinfo.local_file_size,m_localinfo.local_total_size);
			curl_easy_setopt(handle,CURLOPT_RANGE,tmpc);
			//curl_easy_setopt(handle, CURLOPT_RESUME_FROM_LARGE, local_size); 
		}
		curl_easy_setopt(handle,CURLOPT_URL,m_url.c_str());
		curl_easy_setopt(handle,CURLOPT_CONNECTTIMEOUT,connect_timeout);
		curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, process_data);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
	
	
		std::string exePath=m_filename;
		int pos=exePath.find_last_of('\\');
		if (pos!=std::string::npos)
		{
			exePath.assign(exePath.begin(),exePath.begin()+pos);
			_mkdir(exePath.c_str());
		}
		m_retCode=curl_easy_perform(handle);
		if(m_localFile)
		{
			sLog->outDebug(LOG_FILTER_CUR_DOWN,"下载文件%s结束，关闭文件",m_filename.c_str());
			fclose(m_localFile);
		}
		if(m_notdeferror!=0) //not stop download
		{
			if(m_retCode!=CURLE_OK||m_localinfo.local_total_size!=m_localinfo.local_file_size)
			{
				sLog->outError(LOG_FILTER_CUR_DOWN,"从%s下载文件%s失败,错误码%d",m_url.c_str(),m_filename.c_str(),m_retCode);
				if(m_error_fun)
					m_error_fun(m_error_par,(curl_down_handle*)this);
				remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
				remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
				return m_retCode;
			}
			remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());

			rename(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),m_filename.c_str());
			sLog->outDebug(LOG_FILTER_CUR_DOWN,"从%s下载文件%s成功",m_url.c_str(),m_filename.c_str());
			if(m_complete_fun)
				m_complete_fun(m_complete_par,(curl_down_handle*)this);
		}
		curl_easy_cleanup(handle);
		return CURLE_OK;
	}
	int curl_download_set_complete_fun(void* pPar,download_callback_fun complete_fun)
	{
		m_complete_fun=complete_fun;
		m_complete_par=pPar;
		return CURLE_OK;
	}
	int curl_download_set_error_fun(void* pPar,download_callback_fun error_fun)
	{
		m_error_fun=error_fun;
		m_error_par=pPar;
		return CURLE_OK;
	}
	int curl_download_set_progress_fun(void* pPar,download_progress_fun progress_fun)
	{
		m_progress_fun=progress_fun;
		m_progress_par=pPar;
		return CURLE_OK;
	}
};
std::string stringReplace(const std::string& input,
	const std::string& find,
	const std::string& replaceWith)
{
	std::string strOut(input);
	int curPos = 0;

	int pos;
	while((pos = strOut.find(find, curPos)) != std::string::npos)
	{
		// 一次替换
		strOut.replace(pos, find.size(), replaceWith);
		// 防止循环替换!!
		curPos = pos + replaceWith.size();
	}
	return strOut;
}
void curl_easy_escape_skip_chars(std::string& escape_str,const char* _skip_chars)
{
	CURL *handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
	std::string skip_chars=_skip_chars;
	std::map<std::string,std::string> skip_list;
	for (int i=0;i!=skip_chars.size();i++)
	{
		char tmp[2];
		tmp[0]=skip_chars[i];
		tmp[1]=0;
		skip_list.insert(make_pair(curl_easy_escape(handle,tmp,1),tmp));
	}
	std::string escape=curl_easy_escape(handle,escape_str.c_str(),escape_str.length());
	for (std::map<std::string,std::string>::iterator itr=skip_list.begin();itr!=skip_list.end();++itr)
	{
		escape=stringReplace(escape,itr->first,itr->second);
	}
	escape_str=escape;
	curl_easy_cleanup(handle);
	return ;
}


curl_down_handle * curl_download_handle_create(const char* url,const  char*  filename)
{
	curl_download_init();
	std::string m_url=url;
	curl_easy_escape_skip_chars(m_url,"/:");
	curl_download* handle=new curl_download(m_url,filename);
	return (curl_down_handle*)handle;
}
int curl_download_stop(curl_down_handle* down_handle)
{
		assert(down_handle);
		curl_download_init();
		curl_download* handle=(	curl_download* )down_handle;
		handle->curl_download_stop();
		return 0;
}
int curl_download_start(curl_down_handle* down_handle,int connect_timeout_sec)
{
	assert(down_handle);
	curl_download_init();
	curl_download* handle=(	curl_download* )down_handle;
	return handle->curl_download_start(true,connect_timeout_sec);
}
int curl_download_set_complete_fun(curl_down_handle* down_handle,void* pPar,download_callback_fun complete_fun)
{
	assert(down_handle);
	curl_download_init();
	curl_download* handle=(	curl_download* )down_handle;
	return handle->curl_download_set_complete_fun(pPar,complete_fun);
}
int curl_download_set_error_fun(curl_down_handle* down_handle,void* pPar,download_callback_fun error_fun)
{
	assert(down_handle);
	curl_download_init();
	curl_download* handle=(	curl_download* )down_handle;
	return handle->curl_download_set_error_fun(pPar,error_fun);
}
int curl_download_set_progress_fun(curl_down_handle* down_handle,void* pPar,download_progress_fun progress_fun)
{

	assert(down_handle);
	curl_download_init();
	curl_download* handle=(	curl_download* )down_handle;
	return handle->curl_download_set_progress_fun(pPar,progress_fun);
}
int curl_download_handle_free(curl_down_handle* down_handle)
{
	
	assert(down_handle);
	curl_download_init();
	curl_download* handle=(	curl_download* )down_handle;
	delete handle;
	return CURLE_OK;
}
static bool init=false;
Thread_Mutex mutex;
int curl_download_init()
{
	if(init)
	{
		return 0;
	}
	mutex .acquire();
	if(init)
	{
		mutex.release();
		return 0;
	}
	init =true;
	mutex.release();
	Config config;
	std::_tstring Patch=_T("curl.conf");//0808:配置文件位置
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
	std::_tstring exePath ;
	int length=GetModuleFileName(NULL,exeFullPath,MAX_PATH);
	exePath=exeFullPath;
	int pos=exePath.find_last_of('\\');
	if (pos!=std::wstring::npos)
	{
		exePath.assign(exePath.begin(),exePath.begin()+pos);
		Patch=exePath+_T("\\")+Patch;

		if(config.ReadFile(Patch))
			sLog->LoadFromConfig(&config);
	}

	CURLcode ret=curl_global_init(CURL_GLOBAL_WIN32);
	if(ret!=CURLE_OK)
	{
		sLog->outFatal(LOG_FILTER_CUR_DOWN,"curl库初始化失败");
		
	}
	return 0;
}
