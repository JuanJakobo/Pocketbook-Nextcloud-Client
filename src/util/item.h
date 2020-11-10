//------------------------------------------------------------------
// item.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      Describes an WEBDAV item
//-------------------------------------------------------------------

#ifndef ITEM
#define ITEM

#include "inkview.h"

#include <string>

using std::string;

enum Itemtype
{
    IFILE,
    IFOLDER
};

class Item
{
public:
    Item(const string &xmlItem);

    Item(const string &localPath, bool downloaded);

    void setPath(const string &path) { _path = path; };
    string getPath() const { return _path; };

    string getLocalPath() const { return _localPath; };

    Itemtype getType() const { return _type; };

    void setTitle(const string &title) { _title = title; };
    string getTitle() const { return _title; };

    void setDownloaded(bool downloaded) { _downloaded = downloaded; };
    bool isDownloaded() const { return _downloaded; };

    string getLastEditDate() const { return _lastEditDate; };
    void setLastEditDate(const string &date){ _lastEditDate = date;};

    string getSize() const { return _size; };

    string getFiletype() const { return _fileType; };

    void open() const;

    bool removeFile();

private:
    string _path;
    Itemtype _type;
    string _title;
    bool _downloaded{false};
    string _localPath;
    string _lastEditDate{"Error"};
    string _size{"Error"};
    string _fileType;

    void setSize(double tempSize);
};
#endif