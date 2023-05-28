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

#include <memory>
#include <string>
#include <vector>

#include "fileHandler.h"
#include "sqlite3.h"
#include "webDAVModel.h"

class SqliteConnector
{
  public:
    /**
     *
     */
    SqliteConnector(const std::string &DBpath);

    ~SqliteConnector();
    /* ~SqliteConnector() = default; */

    bool open();

    int getDbVersion();

    void runMigration(int currentVersion);

    std::string getEtag(const std::string &path);

    FileState getState(const std::string &path);

    bool updateState(const std::string &path, FileState state);

    std::vector<WebDAVItem> getItemsChildren(const std::string &parenthPath);

    void deleteChildren(const std::string &parentPath);

    void deleteChild(const std::string &path, const std::string &title);

    void deleteItemsNotBeginsWith(std::string beginPath);

    bool resetHideState();

    bool saveItemsChildren(const std::vector<WebDAVItem> &children);

  private:
    std::string _dbpath;
    sqlite3 *_db;

    std::shared_ptr<FileHandler> _fileHandler;
};

#endif
