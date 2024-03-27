#pragma once
//------------------------------------------------------------------
// util.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Various utility methods
//-------------------------------------------------------------------
#include "eventHandler.h"
#include "inkview.h"

#include <string>

using std::string;

struct ConfigDeleter
{
    void operator()(iconfig *config) const
    {
        CloseConfig(config);
    }
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
    static int writeData(void *ptr, int size, int nmemb, FILE *stream);

    /**
     * Checks if a network connection can be established
     *
     * @return true - network access succeeded, false - network access failed
     */
    static bool connectToNetwork();

    /**
     * Writes a value to the config
     * T defines the type of the item (e.g. int, string etc.)
     *
     * @param name of the requested item
     * @param value that shall be written
     * @param secret store the config securely
     */
    template <typename T> static void writeConfig(const std::string &name, T value, bool secret = false)
    {
        ConfigDeleter configDeleter;
        auto config =
            std::unique_ptr<iconfig, ConfigDeleter>(OpenConfig(CONFIG_FILE_LOCATION.c_str(), nullptr), configDeleter);

        if constexpr (std::is_same<T, std::string>::value)
        {
            if (secret)
            {
                WriteSecret(config.get(), name.c_str(), value.c_str());
            }
            else
            {
                WriteString(config.get(), name.c_str(), value.c_str());
            }
        }
        else if constexpr (std::is_same<T, int>::value)
        {
            WriteInt(config.get(), name.c_str(), value);
        }
    }

    /**
     * Reads the value from the config file
     * T defines the type of the item (e.g. int, string etc.)
     *
     * @param name of the requested item
     * @param defaultValue value to return when no was found
     * @param secret load the config from the secure storage
     *
     * @return value from config
     */
    template <typename T> static T getConfig(string name, T defaultValue = "error", bool secret = false)
    {
        ConfigDeleter configDeleter;
        auto config =
            std::unique_ptr<iconfig, ConfigDeleter>(OpenConfig(CONFIG_FILE_LOCATION.c_str(), nullptr), configDeleter);
        T returnValue;

        if constexpr (std::is_same<T, std::string>::value)
        {
            if (secret)
            {
                returnValue = ReadSecret(config.get(), name.c_str(), defaultValue.c_str());
            }
            else
            {
                returnValue = ReadString(config.get(), name.c_str(), defaultValue.c_str());
            }
        }
        else if constexpr (std::is_same<T, int>::value)
        {
            returnValue = ReadInt(config.get(), name.c_str(), defaultValue);
        }

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
     * Encodes an URL
     *
     * @param text text that shall be converted
     */
    static void encodeUrl(std::string &text);

    /**
     * Updates the library of the Pocketbook
     *
     */
    static void updatePBLibrary();

    /**
     * Convert string to tm
     *
     * @param timestring of the string
     *
     * @return time in tm format
     */
    static tm webDAVStringToTm(const std::string &timestring);

    /**
     * Convert tm to string
     *
     * @param time in tm format
     *
     * @return timestring of the string
     */
    static std::string webDAVTmToString(const tm &timestring);

  private:
    Util()
    {
    }
};
