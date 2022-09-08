//------------------------------------------------------------------
// fileBrowser.h
//
// Author:           JuanJakobo
// Date:             29.07.2022
// Description: Interface to the filesystem fo the PB
//
//-------------------------------------------------------------------

#ifndef FILEBROWSER
#define FILEBROWSER

#include "fileModel.h"

#include <string>
#include <vector>

class FileBrowser
{
    public:
        static std::vector<FileItem> getFileStructure(const std::string &path, const bool includeFiles, const bool includeHeader);

    private:
        FileBrowser(){};

};
#endif
