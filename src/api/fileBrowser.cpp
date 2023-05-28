//
//------------------------------------------------------------------
// fileBrowser.cpp
//
// Author:           JuanJakobo
// Date:             29.07.2022
//
//-------------------------------------------------------------------

#include "fileBrowser.h"

#include <experimental/filesystem>
#include <string>

#include "fileHandler.h"
#include "inkview.h"
#include "log.h"

using std::string;
using std::vector;

namespace fs = std::experimental::filesystem;

std::shared_ptr<FileHandler> FileBrowser::_fileHandler = std::make_shared<FileHandler>();
std::vector<FileItem> FileBrowser::getFileStructure(const std::string &path, const bool includeFiles,
                                                    const bool includeHeader)
{
    auto localPath{path};
    std::vector<FileItem> items;

    if (localPath.back() != '/')
        localPath = localPath + '/';

    FileItem temp;
    if (includeHeader)
    {
        temp.path = localPath.substr(0, localPath.find_last_of('/'));
        temp.path = temp.path.substr(0, temp.path.find_last_of('/'));
        if (temp.path.empty())
            temp.path = "/";
        temp.name = "..";
        temp.type = Type::FFOLDER;
        items.push_back(temp);
    }

    const int storageLocationLength = _fileHandler->getStorageLocation().length();
    if (iv_access(localPath.c_str(), R_OK) == 0)
    {
        for (const auto &entry : fs::directory_iterator(localPath))
        {
            // temp.size = fs::file_size(entry);
            auto time = std::chrono::system_clock::to_time_t(fs::last_write_time(entry));
            temp.lastEditDate = *gmtime(&time);

            string directoryPath = temp.path;
            if (directoryPath.length() > storageLocationLength + 1)
            {
                directoryPath = directoryPath.substr(storageLocationLength + 1);
            }
            if (is_directory(entry))
            {
                temp.path = entry.path();
                temp.name = temp.path.substr(temp.path.find_last_of('/') + 1, temp.path.length());
                temp.type = Type::FFOLDER;
                if (!_fileHandler->excludeFolder(directoryPath + "/"))
                {
                    items.push_back(temp);
                }
            }
            else if (includeFiles)
            {
                temp.path = entry.path();
                temp.name = temp.path.substr(temp.path.find_last_of('/') + 1, temp.path.length());
                temp.type = Type::FFILE;
                if (!_fileHandler->excludeFolder(directoryPath.substr(0, directoryPath.length())) ||
                    !_fileHandler->excludeFile(temp.name))
                {
                    items.push_back(temp);
                }
            }
        }
    }
    return items;
}
