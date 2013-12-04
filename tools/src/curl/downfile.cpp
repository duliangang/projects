#include "down.h"
#include <stdlib.h>
#include <io.h>
#include "curl/curl.h"
#include "shared/Convert.h"
#include "shared/Util.h"
#include <assert.h>
#define DOWN_FILE_APPEND_NAME ".td"
#define DOWN_CFG_FILE_APPEND_NAME ".td.cdf"
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
	int m_filesize;
	int m_notdeferror;
	typedef struct _local_file_info
	{
		int local_file_size;
		int local_total_size;
	}local_file_info;
	local_file_info m_localinfo;
	int getDownloadFileLenth()
	{
		long downloadFileLenth = 0;
		CURL *handle = curl_easy_init();
		curl_easy_setopt(handle, CURLOPT_URL, m_url.c_str());
		curl_easy_setopt(handle, CURLOPT_HEADER, 1);    //只需要header头
		curl_easy_setopt(handle, CURLOPT_NOBODY, 1);    //不需要body
		m_retCode =curl_easy_perform(handle);

		if (m_retCode == CURLE_OK) 
		{
			curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
		} 
		else 
		{
			sLog->outError(LOG_FILTER_CUR_DOWN,"获取文件信息失败,url= %s",m_url.c_str());
			downloadFileLenth = -1;
		}
		return downloadFileLenth;
	}
	int getLocalFileLength()
	{
		sLog->outTrace(LOG_FILTER_CUR_DOWN,"读取本地文件信息:文件:%s.%s",m_filename.c_str(),"td.cfd");

		
		if(access(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),0)!=0)
		{
			remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
			remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
			return 0;
		}

		FILE* fp=fopen(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str(),"r");
		if(fp==NULL)
		{
			return 0;
		}
		char filebuffer[1024*1024]={0};
		fread(filebuffer,1,sizeof(filebuffer),fp);
		fclose(fp);
		fp=NULL;
		int localsize=0,totalsize=0;
		int scanf_count=sscanf(filebuffer,"%d,%d",localsize,totalsize);
		if(scanf_count!=2)
		{
		  sLog->outDebug(LOG_FILTER_CUR_DOWN,"坏掉的本地文件:文件%s.%s,文件内容%s",m_filename.c_str(),"td.cdf",filebuffer);
		  remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
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
			m_localFile=fopen(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str(),"wb+");
			if(m_localFile==NULL)
			{
				sLog->outFatal(LOG_FILTER_CUR_DOWN,"从%s下载文件到%s错误,可能是由于权限不够或者该文件正在被使用导致",m_url.c_str(),m_filename.c_str());
				remove(std::string(m_filename+DOWN_CFG_FILE_APPEND_NAME).c_str());
				remove(std::string(m_filename+DOWN_FILE_APPEND_NAME).c_str());
				return CURL_WRITEFUNC_PAUSE;
			}
			fseek(m_localFile,m_localinfo.local_file_size,SEEK_SET);
		}
		size_t fsize=fwrite(buffer,size,nmemb,m_localFile);
		assert(fsize&&fsize+m_localinfo.local_file_size<=m_localinfo.local_total_size);
		fseek(m_localFile,fsize,SEEK_SET);
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
		fclose(m_localcfgFile);
		m_localcfgFile=NULL;
		if(m_progress_fun)
		m_progress_fun(m_progress_par,m_localinfo.local_file_size,m_localinfo.local_total_size);
		if(m_bstop)
		{
			m_notdeferror=0;
			sLog->outDebug(LOG_FILTER_CUR_DOWN,"停止从%s下载%s文件",m_url.c_str(),m_filename.c_str());
		}
		return m_bstop ? CURL_WRITEFUNC_PAUSE:fsize;
	}
	static size_t process_data(void * buffer, size_t size, size_t nmemb, void *downclass)
	{
		assert(downclass!=NULL&&buffer!=NULL);
		curl_download* down=(curl_download*)downclass;
		down->_process_data(buffer,size,nmemb);
	}
public:
	curl_download(const std::string& url,const std::string& filename):m_complete_par(NULL),m_error_par(NULL),m_resumeBrokenDown(true)\
		,m_complete_fun(NULL),m_error_fun(NULL),m_bstop(false),m_filesize(0),m_progress_fun(0),m_progress_par(NULL),m_localFile(NULL),m_localcfgFile(NULL)\
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
		sLog->outTrace(LOG_FILTER_CUR_DOWN,"请求停止下载%s文件",m_filename);
		m_bstop=true;
	}
#ifdef rename
#undef rename
#endif
	int curl_download_start(bool resumeBrokenDown)
	{
		sLog->outTrace(LOG_FILTER_CUR_DOWN,"文件%s开始下载,断点续传:%d",m_filename,resumeBrokenDown);
		m_resumeBrokenDown=resumeBrokenDown;
		m_filesize=getDownloadFileLenth();
		if(m_filesize<=0)
		{
			return m_retCode;
		}
		CURL *handle = curl_easy_init();
		assert(handle);
		int local_size=getLocalFileLength();
		if(local_size<0)
		{
			curl_easy_setopt(handle, CURLOPT_RESUME_FROM_LARGE, local_size); 
		}
		curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, process_data);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, this);
		m_retCode=curl_easy_perform(handle);
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



curl_down_handle * curl_download_handle_create(const char* url,const  char*  filename)
{
	curl_download* handle=new curl_download(url,filename);
	return (curl_down_handle*)handle;
}
int curl_download_stop(curl_down_handle* down_handle)
{
		assert(down_handle);
		curl_download* handle=(	curl_download* )down_handle;
		handle->curl_download_stop();
}
int curl_download_start(curl_down_handle* down_handle)
{
	assert(down_handle);
	curl_download* handle=(	curl_download* )down_handle;
	handle->curl_download_start(true);
}
int curl_download_set_complete_fun(curl_down_handle* down_handle,void* pPar,download_callback_fun complete_fun)
{
	assert(down_handle);
	curl_download* handle=(	curl_download* )down_handle;
	handle->curl_download_set_complete_fun(pPar,complete_fun);
}
int curl_download_set_error_fun(curl_down_handle* down_handle,void* pPar,download_callback_fun error_fun)
{
	assert(down_handle);
	curl_download* handle=(	curl_download* )down_handle;
	handle->curl_download_set_error_fun(pPar,error_fun);
}
int curl_download_set_progress_fun(curl_down_handle* down_handle,void* pPar,download_progress_fun progress_fun)
{
	assert(down_handle);
	curl_download* handle=(	curl_download* )down_handle;
	handle->curl_download_set_progress_fun(pPar,progress_fun);
}
int curl_download_handle_free(curl_down_handle* down_handle)
{
	assert(down_handle);
	curl_download* handle=(	curl_download* )down_handle;
	delete handle;
	return CURLE_OK;
}