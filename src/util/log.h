//------------------------------------------------------------------
// log.h
//
// Author:           JuanJakobo
// Date:             05.08.2020
// Description:      Deals with log entries
//-------------------------------------------------------------------

#ifndef LOG
#define LOG

#include "inkview.h"

#include <string>

class Log
{
public:
    /**
        * Writes a log entry to the log file
        * 
        * @param text that shall be written to the log
        */
    static void writeLog(const std::string &text);

private:
    Log() {}
};
#endif