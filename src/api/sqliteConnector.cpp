//------------------------------------------------------------------
// sqliteconnector.cpp
//
// Author:           JuanJakobo
// Date:             18.07.2021
// Description:
//
//-------------------------------------------------------------------

#include "sqliteConnector.h"
#include "sqlite3.h"
#include "log.h"

#include <string>
#include <vector>
using std::string;

SqliteConnector::SqliteConnector(const string &DBpath) : _dbpath(DBpath)
{
}

SqliteConnector::~SqliteConnector()
{
    sqlite3_close(_db);
    Log::writeInfoLog("closed DB");
}

bool SqliteConnector::open()
{
    int rs;

    rs = sqlite3_open(_dbpath.c_str(), &_db);

    if (rs)
    {
        Log::writeErrorLog("Could not open DB at " + _dbpath);
        return false;
    }
    rs = sqlite3_exec(_db, "CREATE TABLE IF NOT EXISTS metadata (etag VARCHAR, path VARCHAR, parentPath VARCHAR, PRIMARY KEY (path))", NULL, 0, NULL);

    return true;
}

std::vector<WebDAVItem> SqliteConnector::getItemsChildren(const string &parentPath)
{
    open();

    int rs;
    sqlite3_stmt *stmt = 0;
    std::vector<WebDAVItem> items;

    rs = sqlite3_prepare_v2(_db, "SELECT etag, path FROM 'metadata' WHERE parentPath = ?;", -1, &stmt, 0);
    rs = sqlite3_bind_text(stmt, 1, parentPath.c_str(), parentPath.length(), NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
						WebDAVItem temp;

						temp.etag = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
						temp.path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
						//temp.downloaded =  static_cast<IsDownloaded>(sqlite3_column_int(stmt,8));
                        //TODO check if is downloaded and set this status
                        //std::string title;
                        //std::string localPath;
                        //FileState state{FileState::ICLOUD};
                        //Filetype
                        //add type or do automatically? Itemtype type;
                        //TODO add to DB
                        //std::string lastEditDate{"Error"};
                        //double size;
						items.push_back(temp);

    }

    sqlite3_finalize(stmt);
    sqlite3_close(_db);
    return items;
}

//the first one is the parent and therefor should not be saved
bool SqliteConnector::saveItemsChildren(const std::vector<WebDAVItem> &children)
{
    open();
    int rs;

    //if (result)
        //could not open

        string parent = "test";
    string stmt;

    for (size_t i = 0; i < children.size(); i++)
    {
        //first item is parent
        //is cut off before, there change to somewhere else
        //not correct for main folder as there is no parent...
        //TODO is always the parent, i just cut it of for the first entry:tabf
        if(i == 0)
            parent = children.at(i).path;
        else
        {
            //TODO unquote etag
            Log::writeInfoLog(children.at(i).etag + "path " +  children.at(i).path);
            stmt = stmt + " INSERT INTO metadata (etag, path, parentPath) VALUES ('" +  children.at(i).etag + "','" + children.at(i).path +  "','" + parent + "');";
        }
    }

    rs = sqlite3_exec(_db, stmt.c_str(), NULL, 0, NULL);

    if (rs == SQLITE_CONSTRAINT)
    {
        //TODO what if item is already there? update? --> use replace?
    }
    else if (rs != SQLITE_DONE)
    {
        Log::writeErrorLog(sqlite3_errmsg(_db) + std::string(" (Error Code: ") + std::to_string(rs) + ")");
    }

    /*
          std::string _etag;
          std::string _path;
          std::string _title;
          std::string _localPath;
          FileState _state{FileState::ICLOUD};
          Itemtype _type;
          std::string _lastEditDate{"Error"};
          double _size;
          std::string _fileType;
          */

    sqlite3_close(_db);
    return true;
    //never need to get structure under the folder, as it is better to get it anyway?
}
