//------------------------------------------------------------------
// util.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Various utility methods
//-------------------------------------------------------------------

#ifndef UTIL
#define UTIL

#include "inkview.h"

#include <string>

using std::string;

class Util
{
public:
    /**
    * Converts an int to an string, as C++11 command is not supported
    * 
    * @param value the int value that shall be converted
    * @return same value in string format
    */
    static string intToString(const int value);

    /**
    * Handles the return of curl command
    * 
    */
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);

    /**
    * Saves the return of curl command
    * 
    */
    static size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream);

    /**
    * Checks if a network connection can be established
    * 
    * @return true - network access succeeded, false - network access failed
    */
    static bool connectToNetwork();

    /**
    * Returns an integer representing the download progress
    * 
    */
    static int progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    /**
    * Returns an integer representing the download progress
    * 
    */
    static string getXMLAttribute(const string &buffer, const string &name);

    static string replaceString(string item,const string& find,const string& to);

private:
    Util() {}
};
#endif