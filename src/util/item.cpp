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

#include <string>
#include <curl/curl.h>

using namespace std;

Item::Item(const string &xmlItem)
{
    _path = Util::getXMLAttribute(xmlItem, "d:href");
    _lastEditDate = Util::getXMLAttribute(xmlItem, "d:getlastmodified");
    _title = _path;

    if (_path.back() == '/')
    {
        _type = IFOLDER;
        _title = _title.substr(0, _path.length() - 1);
        _size = atoi(Util::getXMLAttribute(xmlItem, "d:quota-used-bytes").c_str());
    }
    else
    {
        _type = IFILE;
        _size = atoi(Util::getXMLAttribute(xmlItem, "d:getcontentlength").c_str());
        _fileType = Util::getXMLAttribute(xmlItem, "d:getcontenttype");

        //set local path and test if exists
        _localPath = Nextcloud::getLocalPath(_path);

        if (iv_access(_localPath.c_str(), W_OK) != 0)
        {
            _downloaded = false;
        }
        else
        {
            _downloaded = true;
        }
    }

    _title = _title.substr(_title.find_last_of("/") + 1, _title.length());
    _title = Util::replaceString(_title,"%20"," ");
}

void Item::open() const
{
    if (_fileType.find("application/epub+zip") != string::npos ||
        _fileType.find("application/pdf") != string::npos)
    {
        OpenBook(_localPath.c_str(), "", 0);
    }
    /*
    else if (_fileType.find("image/jpeg") != string::npos)
    {
        Message(3, "Info", "Opening image", 600);
    }
    else if (_fileType.find("text") != string::npos)
    {
        Message(3, "Info", "Opening image", 600);
    }
    */
    else
    {
        Message(3, "Warning", "The filetype is currently not supported. :/", 600);
    }
}

void Item::removeFile()
{
    remove(_localPath.c_str());
    _downloaded = false;
}