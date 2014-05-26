#ifndef _COMMYSQL_H_
#define _COMMYSQL_H_
#include "log.h"
#include <boost/thread.hpp>
#define _LOG_DEBUG sLog->outDebug
#define _LOG_TRANCE sLog->outTrace
#define _LOG_INFO   sLog->outInfo
#define _LOG_WARN   sLog->outWarn
#define _LOG_ERROR  sLog->outError
#define _LOG_FATAL  sLog->outFatal
enum{
		LOG_FILTER_SQL_DRIVER=LOG_FILTER_SQL,
};
#define WPError(assertion, errmsg) { if (!(assertion)) { sLog->outError(LOG_FILTER_GENERAL, "%\n%s:%i in %s ERROR:\n  %s\n", __FILE__, __LINE__, __FUNCTION__, (char *)errmsg); *((volatile int*)NULL) = 0; } }
#define WPWarning(assertion, errmsg) { if (!(assertion)) { sLog->outError(LOG_FILTER_GENERAL, "\n%s:%i in %s WARNING:\n  %s\n", __FILE__, __LINE__, __FUNCTION__, (char *)errmsg); } }
#define WPFatal(assertion, errmsg) { if (!(assertion)) { sLog->outError(LOG_FILTER_GENERAL, "\n%s:%i in %s FATAL ERROR:\n  %s\n", __FILE__, __LINE__, __FUNCTION__, (char *)errmsg); boost::thread::sleep(boost::get_system_time()+boost::posix_time::seconds(10)); *((volatile int*)NULL) = 0; } }
#endif
