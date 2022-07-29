//------------------------------------------------------------------
// fileView.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef FILEVIEW
#define FILEVIEW

#include "fileModel.h"
#include "listView.h"
#include "fileViewEntry.h"

#include <vector>
#include <memory>

class FileView final : public ListView
{
public:
    /**
        * Displays a list view
        *
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        * @param page page that is shown, default is 1
        */
    FileView(const irect &contentRect, const std::vector<FileItem> &files, int page = 1);

    FileItem &getCurrentEntry() { return getEntry(_selectedEntry); };

    FileItem &getEntry(int entryID) { return std::dynamic_pointer_cast<FileViewEntry>(_entries.at(entryID))->get(); };
};
#endif
