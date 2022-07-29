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

    rs = sqlite3_exec(_db, "CREATE TABLE IF NOT EXISTS metadata (title VARCHAR, localPath VARCHAR, size VARCHAR, fileType VARCHAR, lasteditDate VARCHAR, type INT, state INT, etag VARCHAR, path VARCHAR PRIMARY KEY, parentPath VARCHAR)", NULL, 0, NULL);

    return true;
}

string SqliteConnector::getEtag(const string &path)
{
    open();

    int rs;
    sqlite3_stmt *stmt = 0;
    std::vector<WebDAVItem> items;
    string etag = "not found";


    rs = sqlite3_prepare_v2(_db, "SELECT etag FROM 'metadata' WHERE path = ? LIMIT 1;", -1, &stmt, 0);
    rs = sqlite3_bind_text(stmt, 1, path.c_str(), path.length(), NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        etag = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(_db);
    return etag;
}

FileState SqliteConnector::getState(const string &path)
{
    open();

    int rs;
    sqlite3_stmt *stmt = 0;
    std::vector<WebDAVItem> items;
    FileState state = FileState::ICLOUD;

    rs = sqlite3_prepare_v2(_db, "SELECT state FROM 'metadata' WHERE path = ? LIMIT 1;", -1, &stmt, 0);
    rs = sqlite3_bind_text(stmt, 1, path.c_str(), path.length(), NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        state =  static_cast<FileState>(sqlite3_column_int(stmt,0));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(_db);
    return state;
}

bool SqliteConnector::updateState(const string &path, FileState state)
{
    open();
    int rs;
    sqlite3_stmt *stmt = 0;

    rs = sqlite3_prepare_v2(_db, "UPDATE 'metadata' SET state=? WHERE path=?", -1, &stmt, 0);
    rs = sqlite3_bind_int(stmt, 1, state);
    rs = sqlite3_bind_text(stmt, 2, path.c_str(), path.length(), NULL);
    rs = sqlite3_step(stmt);

    if (rs != SQLITE_DONE)
    {
        Log::writeErrorLog(sqlite3_errmsg(_db) + std::string(" (Error Code: ") + std::to_string(rs) + ")");
    }
    rs = sqlite3_clear_bindings(stmt);
    rs = sqlite3_reset(stmt);

    sqlite3_finalize(stmt);
    sqlite3_close(_db);

    return true;
}

std::vector<WebDAVItem> SqliteConnector::getItemsChildren(const string &parentPath)
{
    open();

    int rs;
    sqlite3_stmt *stmt = 0;
    std::vector<WebDAVItem> items;

    rs = sqlite3_prepare_v2(_db, "SELECT title, localPath, path, size, etag, fileType, lastEditDate, type, state FROM 'metadata' WHERE path=? OR parentPath=? ORDER BY parentPath;", -1, &stmt, 0);
    rs = sqlite3_bind_text(stmt, 1, parentPath.c_str(), parentPath.length(), NULL);
    rs = sqlite3_bind_text(stmt, 2, parentPath.c_str(), parentPath.length(), NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        WebDAVItem temp;

        temp.title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        temp.localPath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        temp.path = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        temp.size = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        temp.etag = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        temp.fileType = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        temp.lastEditDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        temp.type =  static_cast<Itemtype>(sqlite3_column_int(stmt,7));
        temp.state =  static_cast<FileState>(sqlite3_column_int(stmt,8));

        if(iv_access(temp.localPath.c_str(), W_OK) != 0)
        {
            if(temp.type == Itemtype::IFILE)
                temp.state = FileState::ICLOUD;
            else
                iv_mkdir(temp.localPath.c_str(), 0777);
        }

        items.push_back(temp);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(_db);
    return items;
}

void SqliteConnector::deleteChildren(const string &parentPath)
{
    open();
    int rs;
    sqlite3_stmt *stmt = 0;
    rs = sqlite3_prepare_v2(_db, "DELETE FROM 'metadata' WHERE parentPath like ?", -1, &stmt, 0);
    rs = sqlite3_bind_text(stmt, 1, parentPath.c_str(), parentPath.length(), NULL);

    rs = sqlite3_step(stmt);
    if (rs != SQLITE_DONE)
    {
        Log::writeErrorLog(std::string("An error ocurred trying to delete items of the path ") + sqlite3_errmsg(_db) + std::string(" (Error Code: ") + std::to_string(rs) + ")");
    }
    rs = sqlite3_clear_bindings(stmt);
    rs = sqlite3_reset(stmt);

}

bool SqliteConnector::saveItemsChildren(const std::vector<WebDAVItem> &items)
{
    open();
    int rs;
    sqlite3_stmt *stmt = 0;
    string parent = items.at(0).path;

    //Sqlite version to old... is 3.18, require 3.24
    //Log::writeInfoLog(sqlite3_libversion());
    //rs = sqlite3_prepare_v2(_db, "INSERT INTO 'metadata' (title, localPath, path, size, parentPath, etag, fileType, lastEditDate, type, state, key) VALUES (?,?,?,?,?,?,?,?,?,?,?) ON CONFLICT(key) DO UPDATE SET etag=?, size=?, lastEditDate=? WHERE metadata.etag <> ?;", -1, &stmt, 0);
    deleteChildren(parent);

    for (auto item : items)
    {
        rs = sqlite3_prepare_v2(_db, "INSERT INTO 'metadata' (title, localPath, path, size, parentPath, etag, fileType, lastEditDate, type, state) VALUES (?,?,?,?,?,?,?,?,?,?);", -1, &stmt, 0);
        rs = sqlite3_exec(_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        rs = sqlite3_bind_text(stmt, 1, item.title.c_str(), item.title.length(), NULL);
        rs = sqlite3_bind_text(stmt, 2, item.localPath.c_str(), item.localPath.length(), NULL);
        rs = sqlite3_bind_text(stmt, 3, item.path.c_str(), item.path.length(), NULL);
        rs = sqlite3_bind_text(stmt, 4, item.size.c_str(), item.size.length(), NULL);
        rs = sqlite3_bind_text(stmt, 5, parent.c_str(), parent.length(), NULL);
        rs = sqlite3_bind_text(stmt, 6, item.etag.c_str(), item.etag.length(), NULL);
        rs = sqlite3_bind_text(stmt, 7, item.fileType.c_str(), item.fileType.length(), NULL);
        rs = sqlite3_bind_text(stmt, 8, item.lastEditDate.c_str(), item.lastEditDate.length(), NULL);
        rs = sqlite3_bind_int(stmt, 9, item.type);
        rs = sqlite3_bind_int(stmt, 10, item.state);

        rs = sqlite3_step(stmt);
        if (rs == SQLITE_CONSTRAINT)
        {
            rs = sqlite3_clear_bindings(stmt);
            rs = sqlite3_reset(stmt);

            rs = sqlite3_prepare_v2(_db, "UPDATE 'metadata' SET state=?, etag=?, lastEditDate=?, size=? WHERE path=?", -1, &stmt, 0);
            rs = sqlite3_bind_int(stmt, 1, item.state);
            rs = sqlite3_bind_text(stmt, 2, item.etag.c_str(), item.etag.length(), NULL);
            rs = sqlite3_bind_text(stmt, 3, item.lastEditDate.c_str(), item.lastEditDate.length(), NULL);
            rs = sqlite3_bind_text(stmt, 4, item.size.c_str(), item.size.length(), NULL);
            rs = sqlite3_bind_text(stmt, 5, item.path.c_str(), item.path.length(), NULL);
            rs = sqlite3_step(stmt);

            if (rs != SQLITE_DONE)
            {
                Log::writeErrorLog(sqlite3_errmsg(_db) + std::string(" (Error Code: ") + std::to_string(rs) + ")");
            }
            rs = sqlite3_clear_bindings(stmt);
            rs = sqlite3_reset(stmt);
        }
        else if (rs != SQLITE_DONE)
        {
            Log::writeErrorLog(std::string("error inserting into table ") + sqlite3_errmsg(_db) + std::string(" (Error Code: ") + std::to_string(rs) + ")");
        }
        rs = sqlite3_clear_bindings(stmt);
        rs = sqlite3_reset(stmt);
    }

    sqlite3_exec(_db, "END TRANSACTION;", NULL, NULL, NULL);

    sqlite3_finalize(stmt);
    sqlite3_close(_db);

    return true;
}
