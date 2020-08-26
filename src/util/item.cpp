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
        _downloaded = false;
    }

    _title = _title.substr(_title.find_last_of("/") + 1, _title.length());
}

string Item::isClicked()
{
    if (_type == IFILE)
    {
        //downloadFile();
    }
    else
    {
        return _path;
    }

    return "";
}