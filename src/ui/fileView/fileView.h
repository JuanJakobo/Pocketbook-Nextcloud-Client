#pragma once
//------------------------------------------------------------------
// fileView.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------
#include <memory>
#include <vector>

#include "fileModel.h"
#include "fileViewEntry.h"
#include "listView.h"

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
    FileView(const irect &p_contentRect, const std::vector<FileItem> &p_files, uint8_t page = 1);

    FileItem &getCurrentEntry()
    {
        return getEntry(m_selectedEntry);
    };

    FileItem &getEntry(size_t entryID)
    {
        return std::dynamic_pointer_cast<FileViewEntry>(m_entries.at(entryID))->get();
    };
};
