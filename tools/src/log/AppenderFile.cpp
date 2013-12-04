
#include "AppenderFile.h"
#include "../shared/Util.h"
AppenderFile::AppenderFile(uint8_t id, std::_tstring const& name, LogLevel level, const _tchar* _filename, const _tchar* _logDir, const _tchar* _mode, AppenderFlags _flags)
    : Appender(id, name, APPENDER_FILE, level, _flags)
    , filename(_filename)
    , logDir(_logDir)
    , mode(_mode)
{
    dynamicName = std::string::npos != filename.find(_T("%s"));
    backup = _flags & APPENDER_FLAGS_MAKE_FILE_BACKUP;

    logfile = !dynamicName ? OpenFile(_filename, _mode, backup) : NULL;
}

AppenderFile::~AppenderFile()
{
    if (logfile)
    {
        fclose(logfile);
        logfile = NULL;
    }
}

void AppenderFile::_write(LogMessage& message)
{
    if (dynamicName)
    {
        _tchar namebuf[DEF_PATH_MAX];
        _tsnprintf(namebuf, DEF_PATH_MAX, filename.c_str(), message.param1.c_str());
        logfile = OpenFile(namebuf, mode, backup);
    }

    if (logfile)
    {
		std::string text;
		std::string prefix;
#ifdef _UNICODE 
	   WStrToConsole(message.text,text);
	   WStrToConsole(message.prefix,prefix);
#else
		text=message.text;
		prefix=message.prefix;
#endif 
		fprintf(logfile,"%s%s",prefix.c_str(),text.c_str());
        fflush(logfile);

        if (dynamicName)
        {
            fclose(logfile);
            logfile = NULL;
        }
    }
}

FILE* AppenderFile::OpenFile(std::_tstring const &filename, std::_tstring const &mode, bool backup)
{
    if (mode == _T("w") && backup)
    {
        std::_tstring newName(filename);
        newName.push_back('.');
        newName.append(LogMessage::getTimeStr(time(NULL)));
        _trename(filename.c_str(), newName.c_str()); // no error handling... if we couldn't make a backup, just ignore
    }
    return _tfopen((logDir + filename).c_str(), mode.c_str());
}
