//------------------------------------------------------------------
// log.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "log.h"
#include "eventHandler.h"

#include <string>
#include <fstream>

void Log::writeInfoLog(const std::string &text)
{
    writeLog("Info:" + text);
}

void Log::writeErrorLog(const std::string &text)
{
    writeLog("Error:" + text);
}

void Log::writeLog(const std::string &text)
{
    std::ofstream log(CONFIG_FOLDER + std::string("/logfile.txt"), std::ios_base::app | std::ios_base::out);

    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", timeinfo);

    log << buffer << ':' << text << "\n";

    log.close();
}
