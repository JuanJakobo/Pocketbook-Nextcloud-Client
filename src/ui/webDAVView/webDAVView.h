//------------------------------------------------------------------
// webDAVView.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef WEBDAVVIEW
#define WEBDAVVIEW

#include "webDAVModel.h"
#include "listView.h"
#include "webDAVViewEntry.h"

#include <vector>
#include <memory>

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
    WebDAVView(const irect *contentRect, const std::vector<WebDAVItem> &items, int page = 1);

//TODO make other pointer...
    WebDAVItem *getCurrentEntry() { return getEntry(_selectedEntry); };

    WebDAVItem *getEntry(int entryID) { return std::dynamic_pointer_cast<WebDAVViewEntry>(_entries.at(entryID))->get(); };
};
#endif
