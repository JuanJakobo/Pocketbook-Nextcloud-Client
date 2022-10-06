//------------------------------------------------------------------
// fileHandler.h
//
// Author:           RPJoshL
// Date:             03.10.2022
//
//-------------------------------------------------------------------

#ifndef FILEHANDLER
#define FILEHANDLER

#include "webDAVModel.h"

#include <regex>
#include <string>
#include <vector>

#include <memory>

class FileHandler
{
    public:
        FileHandler();
        ~FileHandler();
        bool excludeFile(std::string filename);
        bool excludeFolder(std::string foldername);
        HideState getHideState(Itemtype itemType, std::string prefixToStripe, std::string path, std::string title);

        std::string getStorageLocation();
        std::string getStorageUsername();
        static void update(std::string regex, std::string folderRegex, std::string extensions, int invertMatch);

    private:
        std::regex              _regex;
        std::regex              _folderRegex;
        // can't use pointers with regex... Why? -> unable to null check
        bool                    _useRegex = false;
        bool                    _useFolderRegex = false;
        std::vector<std::string> _extensions;
        bool                     _invertMatch;

        void parseConfig(std::string regex, std::string folderRegex, std::string extensions, int invertMatch);
        static std::vector<FileHandler *> _instances;

};
#endif