//------------------------------------------------------------------
// sqliteconnector.h
//
// Author:           JuanJakobo
// Date:             18.07.2021
// Description:
//
//-------------------------------------------------------------------

#ifndef SQLITECONNECTOR
#define SQLITECONNECTOR

#include "webDAVModel.h"

#include <string>
#include <vector>

class SqliteConnector
{
public:
    /**
    *
    */
    SqliteConnector(const std::string &DBpath);

    ~SqliteConnector();

    //TODO DOKU, open neede?
    bool open();

    std::vector<WebDAVItem> getItemsChildren(const std::string &parenthPath);
    bool saveItemsChildren(const std::vector<WebDAVItem> &children);

private:
    std::string _path;
};

#endif
