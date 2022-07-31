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
        /**
         * Creates a new FileBrowser object
         *
         */
        FileBrowser(bool includeFiles);

        std::vector<FileItem> getFileStructure(const std::string &path);

        void setIncludeFiles(bool includeFiles) { _includeFiles = includeFiles;};

    private:
        bool _includeFiles;


};
#endif
