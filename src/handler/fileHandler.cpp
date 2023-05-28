//------------------------------------------------------------------
// fileHandler.cpp
//
// Author:           RPJoshL
// Date:             03.10.2022
//
//-------------------------------------------------------------------

#include "fileHandler.h"

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>

#include "log.h"
#include "util.h"
#include "webDAVModel.h"

using std::find;
using std::regex;
using std::string;

std::vector<FileHandler *> FileHandler::_instances;
FileHandler::FileHandler()
{
    _instances.push_back(this);

    parseConfig(Util::getConfig<string>("ex_pattern", ""), Util::getConfig<string>("ex_folderPattern", ""),
                Util::getConfig<string>("ex_extensionList", ""), Util::getConfig<int>("ex_invertMatch", 0));
}

void FileHandler::parseConfig(string regex, string folderRegex, string extensions, int invertMatch)
{
    _extensions.clear();
    // split the comma seperated string
    if (!extensions.empty())
    {
        string line;

        std::stringstream ss(extensions);
        while (getline(ss, line, ','))
        {
            _extensions.push_back(line);
        }
    }

    // parse the regex only onces
    if (!regex.empty())
    {
        try
        {
            _regex = std::regex(regex);
            _useRegex = true;
        }
        catch (std::regex_error err)
        {
            Log::writeErrorLog("Unable to parse regex '" + regex + "' for file: " + err.what());
        }
    }
    else
    {
        _useRegex = false;
    }

    if (!folderRegex.empty())
    {
        try
        {
            _folderRegex = std::regex(folderRegex);
            _useFolderRegex = true;
        }
        catch (std::regex_error err)
        {
            Log::writeErrorLog("Unable to parse regex '" + folderRegex + "' for folder: " + err.what());
        }
    }
    else
    {
        _useFolderRegex = false;
    }

    _invertMatch = invertMatch;
}

FileHandler::~FileHandler()
{
    _instances.erase(std::remove(_instances.begin(), _instances.end(), this), _instances.end());
}

bool FileHandler::excludeFile(std::string filename)
{
    // check for file extensions
    if (!_extensions.empty())
    {
        int indexOfDot = filename.find_last_of(".");
        if (indexOfDot != std::string::npos && filename.length() > indexOfDot + 1)
        {
            string extension = filename.substr(filename.find_last_of(".") + 1);
            if (std::find(_extensions.begin(), _extensions.end(), extension) != _extensions.end())
            {
                return !_invertMatch;
            }
        }
    }

    if (_useRegex)
    {
        try
        {
            bool t = std::regex_match(filename, _regex) != _invertMatch;
            return t;
        }
        catch (std::regex_error err)
        {
            string errM = err.what();
            Log::writeErrorLog("Unable to parse regex for file:'" + errM);
        }
    }

    return _invertMatch;
}

bool FileHandler::excludeFolder(std::string folderName)
{
    folderName = "/" + folderName;

    // always display root folder because that can't be matched
    if (folderName == "/" || folderName == "//")
    {
        return false;
    }

    if (_useFolderRegex)
    {
        try
        {
            bool t = std::regex_match(folderName, _folderRegex) != _invertMatch;
            return t;
        }
        catch (std::regex_error err)
        {
            string errM = err.what();
            Log::writeErrorLog("Unable to parse regex for folder:'" + errM);
        }
    }

    return _invertMatch;
}

HideState FileHandler::getHideState(Itemtype itemType, std::string prefix, std::string path, std::string title = "")
{
    string folderPath = path;
    if (path.find(prefix) != string::npos)
    {
        folderPath = path.substr(prefix.length());
        if (itemType == Itemtype::IFILE && folderPath.length() >= title.length())
        {
            folderPath = folderPath.substr(0, folderPath.length() - title.length());
        }
    }
    folderPath = "/" + folderPath;

    if (itemType == Itemtype::IFILE)
    {
        if (!excludeFolder(folderPath) && !excludeFile(title))
        {
            return HideState::ISHOW;
        }
        else
        {
            return HideState::IHIDE;
        }
    }
    else
    {
        if (!excludeFolder(folderPath))
        {
            return HideState::ISHOW;
        }
        else
        {
            return HideState::IHIDE;
        }
    }
}

void FileHandler::update(string regex, string folderRegex, string extensions, int invertMatch)
{
    for (FileHandler *handler : _instances)
    {
        handler->parseConfig(regex, folderRegex, extensions, invertMatch);
    }
}

string FileHandler::getStorageLocation()
{
    return Util::getConfig<string>(CONF_STORAGE_LOCATION) + getStorageUsername() + "/";
}
string FileHandler::getStorageUsername()
{
    return Util::getConfig<string>(CONF_USERNAME);
}
