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

enum Action
{
    IWriteSecret,
    IReadSecret,
    IWriteString,
    IReadString
};

class Util
{
public:
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

    static std::string accessConfig(const std::string &path, const Action &action, const std::string &name, const std::string &value = std::string());

    /**
    * Returns an integer representing the download progress
    *
    */
    static int progress_callback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

    /**
    * get an XML Attribute from the buffer
    */
    static std::string getXMLAttribute(const std::string &buffer, const std::string &name);

    /**
     * Decodes an URL
     *
     * @param text text that shall be converted
     */
    static void decodeUrl(std::string &text);

    /**
     * Updates the library of the Pocketbook
     *
     */
    static void updatePBLibrary(int seconds);

private:
    Util() {}
};
#endif
