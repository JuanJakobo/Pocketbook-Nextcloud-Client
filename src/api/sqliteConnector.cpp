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

SqliteConnector::SqliteConnector(const string &DBpath) : _path(DBpath)
{
    //TODO create sql Database at root and this one is the check
    sqlite3 *db;

    int result;
    //change this

    //iv_unlink(DB_FILE);

    result = sqlite3_open(_path.c_str(), &db);


    if (result)
    {
        //could not open
    }
    //modtime INTEGER,
    result = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS metadata (etag VARCHAR, path VARCHAR, parentPath VARCHAR, PRIMARY KEY (path))", NULL, 0, NULL);

    //TODO move
    sqlite3_close(db);
}

SqliteConnector::~SqliteConnector()
{
    //TODO DB does 
    //sqlite3_close(_db);
    Log::writeInfoLog("closed DB");
}

std::vector<WebDAVItem> SqliteConnector::getItemsChildren(const string &parentPath)
{
    //TODO call open?
    //open();
    //
    sqlite3 *db;
    int result = sqlite3_open(_path.c_str(), &db);

    int rs;
    sqlite3_stmt *stmt = 0;
    std::vector<WebDAVItem> items;

    rs = sqlite3_prepare_v2(db, "SELECT etag, path FROM 'metadata' WHERE parentPath = ?;", -1, &stmt, 0);
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
    sqlite3_close(db);
    return items;
}

//the first one is the parent and therefor should not be saved
bool SqliteConnector::saveItemsChildren(const std::vector<WebDAVItem> &children)
{
    sqlite3 *db;

    int result;
    //TODO change this
    //iv_unlink(DB_FILE);

    result = sqlite3_open(_path.c_str(), &db);

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

    result = sqlite3_exec(db, stmt.c_str(), NULL, 0, NULL);

    if (result == SQLITE_CONSTRAINT)
    {
        //TODO what if item is already there? update? --> use replace?
    }
    else if (result != SQLITE_DONE)
    {
        Log::writeErrorLog(sqlite3_errmsg(db) + std::string(" (Error Code: ") + std::to_string(result) + ")");
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

    sqlite3_close(db);
    return true;
    //never need to get structure under the folder, as it is better to get it anyway?
}
