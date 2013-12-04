#include "../shared/Define.h"
#include "AppenderConsole.h"
#include "../config/config.h"
#include "../shared/Util.h"
#include <sstream>
#if PLATFORM == PLATFORM_WINDOWS
#include <Windows.h>
#endif

AppenderConsole::AppenderConsole(uint8_t id, std::_tstring const& name, LogLevel level, AppenderFlags flags):
Appender(id, name, APPENDER_CONSOLE, level, flags), _colored(false)
{
    for (uint8_t i = 0; i < MaxLogLevels; ++i)
        _colors[i] = ColorTypes(MaxColors);
}

void AppenderConsole::InitColors(std::_tstring const& str)
{
    if (str.empty())
    {
        _colored = false;
        return;
    }

    int color[MaxLogLevels];

    std::_tistringstream ss(str);

    for (uint8_t i = 0; i < MaxLogLevels; ++i)
    {
        ss >> color[i];

        if (!ss)
            return;

        if (color[i] < 0 || color[i] >= MaxColors)
            return;
    }

    for (uint8_t i = 0; i < MaxLogLevels; ++i)
        _colors[i] = ColorTypes(color[i]);

    _colored = true;
}

void AppenderConsole::SetColor(bool stdout_stream, ColorTypes color)
{
    #if PLATFORM == PLATFORM_WINDOWS
    static WORD WinColorFG[MaxColors] =
    {
        0,                                                  // BLACK
        FOREGROUND_RED,                                     // RED
        FOREGROUND_GREEN,                                   // GREEN
        FOREGROUND_RED | FOREGROUND_GREEN,                  // BROWN
        FOREGROUND_BLUE,                                    // BLUE
        FOREGROUND_RED |                    FOREGROUND_BLUE, // MAGENTA
        FOREGROUND_GREEN | FOREGROUND_BLUE,                 // CYAN
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, // WHITE
                                                            // YELLOW
        FOREGROUND_RED | FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
                                                            // RED_BOLD
        FOREGROUND_RED |                                      FOREGROUND_INTENSITY,
                                                            // GREEN_BOLD
        FOREGROUND_GREEN |                   FOREGROUND_INTENSITY,
        FOREGROUND_BLUE | FOREGROUND_INTENSITY,             // BLUE_BOLD
                                                            // MAGENTA_BOLD
        FOREGROUND_RED |                    FOREGROUND_BLUE | FOREGROUND_INTENSITY,
                                                            // CYAN_BOLD
        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
                                                            // WHITE_BOLD
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
    };

    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    SetConsoleTextAttribute(hConsole, WinColorFG[color]);
    #else
    enum ANSITextAttr
    {
        TA_NORMAL                                = 0,
        TA_BOLD                                  = 1,
        TA_BLINK                                 = 5,
        TA_REVERSE                               = 7
    };

    enum ANSIFgTextAttr
    {
        FG_BLACK                                 = 30,
        FG_RED,
        FG_GREEN,
        FG_BROWN,
        FG_BLUE,
        FG_MAGENTA,
        FG_CYAN,
        FG_WHITE,
        FG_YELLOW
    };

    enum ANSIBgTextAttr
    {
        BG_BLACK                                 = 40,
        BG_RED,
        BG_GREEN,
        BG_BROWN,
        BG_BLUE,
        BG_MAGENTA,
        BG_CYAN,
        BG_WHITE
    };

    static uint8_t UnixColorFG[MaxColors] =
    {
        FG_BLACK,                                          // BLACK
        FG_RED,                                            // RED
        FG_GREEN,                                          // GREEN
        FG_BROWN,                                          // BROWN
        FG_BLUE,                                           // BLUE
        FG_MAGENTA,                                        // MAGENTA
        FG_CYAN,                                           // CYAN
        FG_WHITE,                                          // WHITE
        FG_YELLOW,                                         // YELLOW
        FG_RED,                                            // LRED
        FG_GREEN,                                          // LGREEN
        FG_BLUE,                                           // LBLUE
        FG_MAGENTA,                                        // LMAGENTA
        FG_CYAN,                                           // LCYAN
        FG_WHITE                                           // LWHITE
    };

    _tfprintf((stdout_stream? stdout : stderr), _T("\x1b[%d%sm"), UnixColorFG[color], (color >= YELLOW && color < MaxColors ? ";1" : ""));
    #endif
}

void AppenderConsole::ResetColor(bool stdout_stream)
{
    #if PLATFORM == PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(stdout_stream ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
    #else
    _tfprintf((stdout_stream ? stdout : stderr), _T("\x1b[0m"));
    #endif
}

void AppenderConsole::_write(LogMessage& message)
{
    bool stdout_stream = message.level == LOG_LEVEL_ERROR || message.level == LOG_LEVEL_FATAL;
	std::string text;
	std::string prefix;
#ifdef _UNICODE 
	WStrToConsole(message.text,text);
	WStrToConsole(message.prefix,prefix);
#else
	text=message.text;
	prefix=message.prefix;
#endif 
    if (_colored)
    {
        uint8_t index;
        switch (message.level)
        {
            case LOG_LEVEL_TRACE:
               index = 5;
               break;
            case LOG_LEVEL_DEBUG:
               index = 4;
               break;
            case LOG_LEVEL_INFO:
               index = 3;
               break;
            case LOG_LEVEL_WARN:
               index = 2;
               break;
            case LOG_LEVEL_FATAL:
               index = 0;
               break;
            case LOG_LEVEL_ERROR: // No break on purpose
            default:
               index = 1;
               break;
        }

        SetColor(stdout_stream, _colors[index]);
        //utf8printf(stdout_stream ? stdout : stderr, _T("%s%s"), message.prefix.c_str(), message.text.c_str());
		fprintf(stdout_stream ? stdout : stderr, "%s%s", prefix.c_str(), text.c_str());
        ResetColor(stdout_stream);
    }
    else
     	fprintf(stdout_stream ? stdout : stderr, "%s%s", prefix.c_str(), text.c_str());
}
