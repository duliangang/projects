
#ifndef APPENDERFILE_H
#define APPENDERFILE_H

#include "Appender.h"

class AppenderFile: public Appender
{
    public:
        AppenderFile(uint8_t _id, std::_tstring const& _name, LogLevel level, const _tchar* filename, const _tchar* logDir, const _tchar* mode, AppenderFlags flags);
        ~AppenderFile();
        FILE* OpenFile(std::_tstring const& _name, std::_tstring const& _mode, bool _backup);

    private:
        void _write(LogMessage& message);
        FILE* logfile;
        std::_tstring filename;
        std::_tstring logDir;
        std::_tstring mode;
        bool dynamicName;
        bool backup;
};

#endif
