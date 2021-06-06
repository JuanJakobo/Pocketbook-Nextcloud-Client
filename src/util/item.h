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

enum Itemtype
{
    IFILE,
    IFOLDER
};

enum FileState
{
    ICLOUD,
    ISYNCED,
    IOUTSYNCED,
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
    Item(const std::string &xmlItem);

    /**
     * Creates a new item by receiving localPath from the pocketbook
     * 
     * @param localPath path where the file is placed
     * @param state state of the file
     * @param type type of the item (folder/file)
     */
    Item(const std::string &localPath, FileState state, Itemtype type);


    /**
     * Checks if item is a book
     */
    bool isBook() const;

    /**
     * Tries to open the item by checking the file format and then executes the fitting action
     */
    void open() const;

    bool removeFile();

    void setPath(const std::string &path) { _path = path; };
    std::string getPath() const { return _path; };

    std::string getLocalPath() const { return _localPath; };

    Itemtype getType() const { return _type; };

    void setTitle(const std::string &title) { _title = title; };
    std::string getTitle() const { return _title; };

    void setState(FileState state) { _state = state; };
    FileState getState() const { return _state; };

    std::string getLastEditDate() const { return _lastEditDate; };
    void setLastEditDate(const std::string &date) { _lastEditDate = date; };

    double getSize() const { return _size; };
    std::string getSizeString() const;

    std::string getFiletype() const { return _fileType; };

private:
    std::string _path;
    std::string _title;
    std::string _localPath;
    FileState _state{FileState::ICLOUD};
    Itemtype _type;
    std::string _lastEditDate{"Error"};
    double _size;
    std::string _fileType;

    /**
     * Converts the size to an easier readble format
     * 
     * @param tempSize Size of the item in bytes
     */
    void setSize(double tempSize);
};
#endif