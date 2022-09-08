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
    ILOCAL,
    IDOWNLOADED
};

struct WebDAVItem : Entry{
    std::string etag;
    std::string path;
    std::string title;
    std::string localPath;
    FileState state{FileState::ICLOUD};
    Itemtype type;
    tm lastEditDate = { 0 };
    std::string size;
    std::string fileType;
};

#endif
