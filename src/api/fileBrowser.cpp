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

using std::string;
using std::vector;


FileBrowser::FileBrowser(bool includeFiles) : _includeFiles(includeFiles)
{
}

std::vector<FileItem> FileBrowser::getFileStructure(const std::string &path)
{
    //get local files, https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
    DIR *dir;
    class dirent *ent;
    class stat st;
    string localPath = path;
    std::vector<FileItem> items;

    if (localPath.back() != '/')
        localPath = localPath + '/';

    FileItem temp;
    temp.path = localPath.substr(0,localPath.find_last_of('/'));
    temp.path = temp.path.substr(0,temp.path.find_last_of('/'));
    if (temp.path.empty())
        temp.path = "/";
    temp.name = "..";
    temp.type = Type::FFOLDER;
    items.push_back(temp);

    if (iv_access(localPath.c_str(), R_OK) == 0)
    {
        dir = opendir(localPath.c_str());
        while ((ent = readdir(dir)) != NULL)
        {
            const string fileName = ent->d_name;


            if (fileName[0] == '.')
                continue;

            const string fullFileName = localPath + fileName;

            if (stat(fullFileName.c_str(), &st) == -1)
                continue;

            if ((st.st_mode & S_IFDIR) != 0)
            {
                FileItem temp;
                temp.path = fullFileName + '/';
                temp.name = fullFileName.substr(fullFileName.find_last_of("/") + 1, fullFileName.length());
                temp.type = Type::FFOLDER;
                items.push_back(temp);
            }
            else if (_includeFiles)
            {
                FileItem temp;
                temp.path = fullFileName;
                temp.name = fullFileName.substr(fullFileName.find_last_of("/") + 1, fullFileName.length());
                temp.type = Type::FFILE;
                items.push_back(temp);
            }
        }
        closedir(dir);
    }
    return items;
}
