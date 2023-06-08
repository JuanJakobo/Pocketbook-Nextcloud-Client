//------------------------------------------------------------------
// hnCommentViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "listViewEntry.h"

#include "inkview.h"

ListViewEntry::ListViewEntry(int p_page, const irect &p_position) : m_page(p_page), m_position(p_position)
{
}

void ListViewEntry::drawBottomLine() const
{
    auto const line{(m_position.y + m_position.h) - 1};
    DrawLine(m_position.x, line, m_position.w, line, BLACK);
}
