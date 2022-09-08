//
//------------------------------------------------------------------
// fileBrowser.cpp
//
// Author:           JuanJakobo
// Date:             29.07.2022
//
//-------------------------------------------------------------------

#include "fileBrowser.h"
#include "inkview.h"

#include <string>
#include <experimental/filesystem>

using std::string;
using std::vector;

namespace fs = std::experimental::filesystem;

std::vector<FileItem> FileBrowser::getFileStructure(const std::string &path, const bool includeFiles, const bool includeHeader)
{
    string localPath = path;
    std::vector<FileItem> items;

    if (localPath.back() != '/')
        localPath = localPath + '/';

    FileItem temp;
    if(includeHeader)
    {
        temp.path = localPath.substr(0,localPath.find_last_of('/'));
        temp.path = temp.path.substr(0,temp.path.find_last_of('/'));
        if (temp.path.empty())
            temp.path = "/";
        temp.name = "..";
        temp.type = Type::FFOLDER;
        items.push_back(temp);
    }

    if (iv_access(localPath.c_str(), R_OK) == 0)
    {
        for (const auto &entry : fs::directory_iterator(localPath))
        {
            //temp.size = fs::file_size(entry);
            auto time = std::chrono::system_clock::to_time_t(fs::last_write_time(entry));
            temp.lastEditDate = *gmtime(&time);


            if(is_directory(entry))
            {
                temp.path = entry.path();
                temp.name = temp.path.substr(temp.path.find_last_of('/') + 1, temp.path.length());
                temp.type = Type::FFOLDER;
                items.push_back(temp);
            }
            else if (includeFiles)
            {
                temp.path = entry.path();
                temp.name = temp.path.substr(temp.path.find_last_of('/') + 1, temp.path.length());
                temp.type = Type::FFILE;
                items.push_back(temp);
            }
        }
    }
    return items;
}
