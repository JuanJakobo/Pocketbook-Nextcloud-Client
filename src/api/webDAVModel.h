#pragma once
//------------------------------------------------------------------
// webDAVItem.h
//
// Author:           JuanJakobo
// Date:             07.07.2022
// Description:
//-------------------------------------------------------------------
#include <string>

#include "model.h"

enum class Itemtype
{
    IFILE = 0,
    IFOLDER
};

enum class FileState
{
    ICLOUD = 0,
    ISYNCED,
    IOUTSYNCED,
    ILOCAL,
    IDOWNLOADED
};

enum class HideState
{
    INOTDEFINED = 0,
    ISHOW,
    IHIDE
};

struct WebDAVItem : Entry
{
    std::string etag;
    std::string path;
    std::string title;
    std::string localPath;
    FileState state{FileState::ICLOUD};
    Itemtype type;
    tm lastEditDate{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::string size;
    std::string fileType;
    HideState hide;
};
