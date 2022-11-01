//------------------------------------------------------------------
// hnCommentViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "listViewEntry.h"
#include "inkview.h"

ListViewEntry::ListViewEntry(int page, const irect &rect)
    : _page(page), _position(rect) {}