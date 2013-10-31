
#ifndef APPENDERCONSOLE_H
#define APPENDERCONSOLE_H

#include "Appender.h"
#include <string>

enum ColorTypes
{
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    GREY,
    YELLOW,
    LRED,
    LGREEN,
    LBLUE,
    LMAGENTA,
    LCYAN,
    WHITE
};

const uint8_t MaxColors = uint8_t(WHITE) + 1;

class AppenderConsole: public Appender
{
    public:
        AppenderConsole(uint8_t _id, std::_tstring const& name, LogLevel level, AppenderFlags flags);
        void InitColors(const std::_tstring& init_str);

    private:
        void SetColor(bool stdout_stream, ColorTypes color);
        void ResetColor(bool stdout_stream);
        void _write(LogMessage& message);
        bool _colored;
        ColorTypes _colors[MaxLogLevels];
};

#endif
