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
        FileBrowser();

        void test();

        std::vector<FileItem> getFileStructure(const std::string &path);


    private:
        std::string _currentLocation;

};
#endif
