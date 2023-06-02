//------------------------------------------------------------------
// log.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "log.h"

#include <fstream>
#include <string>

#include "eventHandler.h"

namespace
{
constexpr auto INFO_TEXT{"Info:"};
constexpr auto ERROR_TEXT{"Error:"};
constexpr auto LOGFILE_LOCATION{"/logfile.txt"};
} // namespace

// TODO use templates!

void Log::writeInfoLog(const std::string &text)
{
    writeLog(INFO_TEXT + text);
}

void Log::writeErrorLog(const std::string &text)
{
    writeLog(ERROR_TEXT + text);
}

void Log::writeLog(const std::string &text)
{
    std::stringstream ss;
    ss << CONFIG_FOLDER_LOCATION << LOGFILE_LOCATION;
    std::ofstream log(ss.str(), std::ios_base::app | std::ios_base::out);

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", timeinfo);

    log << buffer << ':' << text << "\n";

    log.close();
}
