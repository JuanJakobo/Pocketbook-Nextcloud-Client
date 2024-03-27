#pragma once
//------------------------------------------------------------------
// fileModel.h
//
// Author:           JuanJakobo
// Date:             23.04.2021
// Description:
//-------------------------------------------------------------------
#include <string>
#include <ctime>

#include "model.h"

enum class Type
{
    FFOLDER,
    FFILE
};

struct FileItem : Entry
{
    std::string name;
    std::string path;
    Type type;
    // long long int size;
    std::tm lastEditDate;
};
