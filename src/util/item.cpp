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
    if (_path.find(NEXTCLOUD_END_PATH) != std::string::npos)
        _path.erase(0, NEXTCLOUD_END_PATH.length());

    _lastEditDate = Util::getXMLAttribute(xmlItem, "d:getlastmodified");
    _title = _path;

    if (_path.back() == '/')
    {
        _type = IFOLDER;
        _title = _title.substr(0, _path.length() - 1);
        _size = atof(Util::getXMLAttribute(xmlItem, "d:quota-used-bytes").c_str());
    }
    else
    {
        _type = IFILE;
        _size = atof(Util::getXMLAttribute(xmlItem, "d:getcontentlength").c_str());
        _fileType = Util::getXMLAttribute(xmlItem, "d:getcontenttype");

        //set local path and test if exists
        _localPath = Nextcloud::getLocalPath(_path);

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

void Item::open() const
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
    return Util::valueToString(tempSize) + " " + unit;
}