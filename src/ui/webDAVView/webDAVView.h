#pragma once
//------------------------------------------------------------------
// webDAVView.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------
#include <memory>
#include <vector>

#include "listView.h"
#include "webDAVModel.h"
#include "webDAVViewEntry.h"

class WebDAVView final : public ListView
{
  public:
    /**
     * Displays a list view
     *
     * @param ContentRect area of the screen where the list view is placed
     * @param Items items that shall be shown in the listview
     * @param page page that is shown, default is 1
     */
    WebDAVView(const irect &p_contentRect, std::vector<WebDAVItem> &p_itemsUnfiltered, uint8_t p_page = 1);

    WebDAVItem &getCurrentEntry()
    {
        return getEntry(m_selectedEntry);
    };

    WebDAVItem &getEntry(size_t entryID)
    {
        return std::static_pointer_cast<WebDAVViewEntry>(m_entries.at(entryID))->get();
    };
};
