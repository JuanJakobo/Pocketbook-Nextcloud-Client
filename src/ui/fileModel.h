//------------------------------------------------------------------
// fileModel.h
//
// Author:           JuanJakobo
// Date:             23.04.2021
// Description:
//-------------------------------------------------------------------

#ifndef FILEMODEL
#define FILEMODEL

//#include "model.h"

#include <string>

enum Type
{
    FOLDER,
    FIL
};

struct File : Entry{
				std::string name;
				std::string path;
                Type type;
};

#endif
