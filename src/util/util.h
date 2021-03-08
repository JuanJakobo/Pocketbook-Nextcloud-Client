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
#include <fstream>
#include <sstream>

using std::ostringstream;

using std::string;

class Util
{
public:
    /**
    * Converts an value to an string, as C++11 command is not supported
    * 
    * @param value the int value that shall be converted
    * @return same value in string format
    */
    template <typename T>
    static string valueToString(const T value)
    {
        ostringstream stm;
        stm << value;
        return stm.str();
    };

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
    * get an XML Attribute from the buffer
    */
    static string getXMLAttribute(const string &buffer, const string &name);

    /**
     * Decodes an URL
     * 
     * @param text text that shall be converted
     */
    static void decodeUrl(string &text);

private:
    Util() {}
};
#endif