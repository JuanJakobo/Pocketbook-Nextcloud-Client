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

using namespace std;

class Util
{
public:

    /**
    * Converts an int to an string, as C++11 command is not supported
    * 
    * @param value the int value that shall be converted
    * @return same value in string format
    */
    static string   intToString(int value);

    /**
    * Handles the return of curl command
    * 
    */
    static size_t  writeCallback(void *contents, size_t size, size_t nmemb, void *userp);

    /**
    * Checks if a network connection can be established
    * 
    * @return true - network access succeeded, false - network access failed
    */
    static bool     connectToNetwork();

private:
    Util() {}
};
#endif