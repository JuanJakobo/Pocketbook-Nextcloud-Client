//------------------------------------------------------------------
// webDAVItem.h
//
// Author:           JuanJakobo
// Date:             07.07.2022
// Description:
//-------------------------------------------------------------------

#ifndef WEBDAVITEM
#define WEBDAVITEM

#include "model.h"
//TODO use own
#include "item.h"

#include <string>

/*
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
*/

struct WebDAVItem : Entry{
    std::string etag;
    std::string path;
    std::string title;
    std::string localPath;
    FileState state{FileState::ICLOUD};
    Itemtype type;
    std::string lastEditDate{"Error"};
    double size;
    std::string fileType;
};

#endif
