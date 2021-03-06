//------------------------------------------------------------------
// item.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "item.h"
#include "inkview.h"
#include "util.h"
#include "nextcloud.h"
#include "log.h"

#include <math.h>
#include <string>
#include <curl/curl.h>

using namespace std;

Item::Item(const string &xmlItem)
{
    _path = Util::getXMLAttribute(xmlItem, "d:href");
    
    //replaces everthing in front of /remote.php as this is already part of the url
    Log::writeLog("path before transformation " + _path);
    if(_path.find(NEXTCLOUD_START_PATH) != 0)
    {
        _path.erase(0,_path.find(NEXTCLOUD_START_PATH));
        Log::writeLog("path after transformation " + _path);
    }

    _lastEditDate = Util::getXMLAttribute(xmlItem, "d:getlastmodified");
    _title = _path;
    _localPath = Nextcloud::getLocalPath(_path);

    if (_path.back() == '/')
    {
        _localPath = _localPath.substr(0, _localPath.length() - 1);
        _type = Itemtype::IFOLDER;
        _title = _title.substr(0, _path.length() - 1);
        _size = atof(Util::getXMLAttribute(xmlItem, "d:quota-used-bytes").c_str());
    }
    else
    {
        _type = Itemtype::IFILE;
        _size = atof(Util::getXMLAttribute(xmlItem, "d:getcontentlength").c_str());
        _fileType = Util::getXMLAttribute(xmlItem, "d:getcontenttype");

        if (iv_access(_localPath.c_str(), W_OK) != 0)
        {
            _state = FileState::ICLOUD;
        }
        else
        {
            _state = FileState::ISYNCED;
        }
    }

    _title = _title.substr(_title.find_last_of("/") + 1, _title.length());
    Util::decodeUrl(_title);
}

Item::Item(const string &localPath, FileState state, Itemtype type) : _localPath(localPath), _state(state), _type(type)
{
    _title = _localPath;
    _title = _title.substr(_title.find_last_of("/") + 1, _title.length());
    Util::decodeUrl(_title);
}

bool Item::isBook() const
{
    if (_fileType.find("application/epub+zip") != string::npos ||
        _fileType.find("application/pdf") != string::npos ||
        _fileType.find("application/octet-stream") != string::npos ||
        _fileType.find("text/plain") != string::npos ||
        _fileType.find("text/html") != string::npos ||
        _fileType.find("text/rtf") != string::npos ||
        _fileType.find("application/msword") != string::npos ||
        _fileType.find("application/x-mobipocket-ebook") != string::npos ||
        _fileType.find("application/vnd.openxmlformats-officedocument.wordprocessingml.document") != string::npos ||
        _fileType.find("application/x-fictionbook+xml") != string::npos)
        return true;
    return false;
}

void Item::open() const
{
    if (_state == FileState::ICLOUD)
    {
        Message(ICON_ERROR, "File not found.", "Could not find file.", 1000);
    }
    else if(isBook())
    {

        OpenBook(_localPath.c_str(), "", 0);
    }
    /*
    else if (_fileType.find("audio/mpeg") != string::npos ||
             _fileType.find("audio/ogg") != string::npos ||
             _fileType.find("audio/mp4") != string::npos ||
             _fileType.find("audio/m4b") != string::npos)
    {
        PlayFile(_localPath.c_str());
        OpenPlayer();
    }
    else if (_fileType.find("image/jpeg") != string::npos)
    {
        Message(3, "Info", "Opening image", 600);
    }
    */
    else
    {
        Message(ICON_INFORMATION, "Warning", "The filetype is currently not supported.", 1200);
    }
}

bool Item::removeFile()
{
    if (_type == Itemtype::IFOLDER)
    {
        string cmd = "rm -rf " + _localPath + "/";
        system(cmd.c_str());
        return true;
    }

    if (remove(_localPath.c_str()) != 0)
        return false;
    if (_state == FileState::ISYNCED || _state == FileState::IOUTSYNCED)
    {
        _state = FileState::ICLOUD;
    }
    else
    {
        Message(ICON_INFORMATION, "Warning", "The file will be shown until next folder update.", 1200);
    }
    return true;
}

string Item::getSizeString() const
{

    if (_size < 1024)
        return "< 1 KB";

    double departBy;
    double tempSize;
    string unit;

    if (_size < 1048576)
    {
        departBy = 1024;
        unit = "KB";
    }
    else if (_size < 1073741824)
    {
        departBy = 1048576;
        unit = "MB";
    }
    else
    {
        departBy = 1073741824;
        unit = "GB";
    }

    tempSize = round((_size / departBy) * 10.0) / 10.0;
    return std::to_string(tempSize) + " " + unit;
}