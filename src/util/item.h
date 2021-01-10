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

enum FileState
{
    ICLOUD,
    ISYNCED,
    ILOCAL
};

class Item
{
public:

    /**
        * Creates an item by receiving the xml from nextcloud and parses it into an object 
        * 
        * @param xmlItem result of the nextcloud request
        */
    Item(const string &xmlItem);

    /**
     * Creates a new item by receiving localPath from the pocketbook
     * 
     * @param localPath path where the file is placed
     * @param FileState state of the file
     */
    Item(const string &localPath, FileState state);
    
    /**
     * Tries to open the item by checking the file format and then executes the fitting action
     */
    void open() const;


    bool removeFile();

    void setPath(const string &path) { _path = path; };
    string getPath() const { return _path; };

    string getLocalPath() const { return _localPath; };

    Itemtype getType() const { return _type; };

    void setTitle(const string &title) { _title = title; };
    string getTitle() const { return _title; };

    void setState(FileState state) { _state = state; };
    FileState getState() const { return _state; };

    string getLastEditDate() const { return _lastEditDate; };
    void setLastEditDate(const string &date) { _lastEditDate = date; };

    string getSize() const { return _size; };

    string getFiletype() const { return _fileType; };

private:
    string _path;
    Itemtype _type;
    string _title;
    FileState _state{FileState::ICLOUD};
    string _localPath;
    string _lastEditDate{"Error"};
    string _size{"Error"};
    string _fileType;

    /**
     * Converts the size to an easier readble format
     * 
     * @param tempSize Size of the item in bytes
     */
    void setSize(double tempSize);
};
#endif