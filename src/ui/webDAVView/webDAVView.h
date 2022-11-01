//------------------------------------------------------------------
// webDAVView.h
//
// Author:           JuanJakobo
// Date:             08.09.2021
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef WEBDAVVIEW
#define WEBDAVVIEW

#include "listView.h"
#include "webDAVModel.h"
#include "webDAVViewEntry.h"

#include <memory>
#include <vector>

class WebDAVView final : public ListView {
public:
  /**
   * Displays a list view
   *
   * @param ContentRect area of the screen where the list view is placed
   * @param Items items that shall be shown in the listview
   * @param page page that is shown, default is 1
   */
  WebDAVView(const irect &contentRect, std::vector<WebDAVItem> &items,
             int page = 1);

  WebDAVItem &getCurrentEntry() { return getEntry(_selectedEntry); };

  WebDAVItem &getEntry(int entryID) {
    return std::static_pointer_cast<WebDAVViewEntry>(_entries.at(entryID))
        ->get();
  };
};
#endif
