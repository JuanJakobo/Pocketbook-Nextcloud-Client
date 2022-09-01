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
#include "eventHandler.h"

#include <string>

enum class Action
{
    IWriteSecret,
    IReadSecret,
    IWriteString,
    IReadString,
    IWriteInt,
    IReadInt
};

const std::string CONFIG_PATH = CONFIG_FOLDER + "/nextcloud.cfg";

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

    /**
     * Read and write access to config file
     * T defines the type of the item (e.g. int, string etc.)
     *
     * @param action option that shall be done
     * @param name of the requested item
     * @param value that shall be written in case
     *
     * @return value that is saved in case
     */
    template <typename T>
    static T accessConfig(const Action &action, const std::string &name, T value)
    {
        iconfigedit *temp = nullptr;
        iconfig *config = OpenConfig(CONFIG_PATH.c_str(), temp);
        T returnValue;

        if constexpr(std::is_same<T, Entry>::value)
        {
        }
        if constexpr(std::is_same<T, std::string>::value)
        {
            switch (action)
            {
                case Action::IWriteSecret:
                    WriteSecret(config, name.c_str(), value.c_str());
                    returnValue = {};
                    break;
                case Action::IReadSecret:
                    returnValue = ReadSecret(config, name.c_str(), "");
                    break;
                case Action::IWriteString:
                    WriteString(config, name.c_str(), value.c_str());
                    returnValue = {};
                    break;
                case Action::IReadString:
                    returnValue = ReadString(config, name.c_str(), "");
                    break;
                default:
                    break;
            }
        }
        else if constexpr(std::is_same<T, int>::value)
        {
            switch(action)
            {
                case Action::IWriteInt:
                    WriteInt(config, name.c_str(), value);
                    returnValue = 0;
                    break;
                case Action::IReadInt:
                    returnValue = ReadInt(config, name.c_str(), 0);
                    break;
                default:
                    break;
            }
        }
        CloseConfig(config);

        return returnValue;
    }

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
