#pragma once
//------------------------------------------------------------------
// fileHandler.h
//
// Author:           RPJoshL
// Date:             03.10.2022
//
//-------------------------------------------------------------------
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "webDAVModel.h"

class FileHandler
{
  public:
    FileHandler();
    ~FileHandler();

    /**
     * Determines if a file should be excluded
     *
     * @param filename name of the file to check
     * @return bool returns true if file should be excluded, else false
     */
    bool excludeFile(std::string filename);

    /**
     * Determines if a folder should be excluded
     *
     * @param foldername name of the folder to check
     * @return bool returns true if folder should be excluded, else false
     */
    bool excludeFolder(std::string foldername);

    /**
     * Returns the hide state for a given item
     */
    HideState getHideState(Itemtype itemType, std::string prefixToStripe, std::string path, std::string title);

    std::string getStorageLocation();
    std::string getStorageUsername();
    static void update(std::string regex, std::string folderRegex, std::string extensions, int invertMatch);

  private:
    std::regex _regex;
    std::regex _folderRegex;
    // TODO can't use pointers with regex... Why? -> unable to null check
    bool _useRegex = false;
    bool _useFolderRegex = false;
    std::vector<std::string> _extensions;
    bool _invertMatch;

    void parseConfig(std::string regex, std::string folderRegex, std::string extensions, int invertMatch);
    static std::vector<FileHandler *> _instances;
};
