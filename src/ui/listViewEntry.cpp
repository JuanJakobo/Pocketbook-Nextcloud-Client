//------------------------------------------------------------------
// listViewEntry.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "item.h"
#include "listViewEntry.h"
#include "util.h"

#include <memory>

ListViewEntry::ListViewEntry(int page, irect rect) : _page(page), _position(rect)
{
}

void ListViewEntry::draw(const Item &item)
{
    SetFont(_entryFontBold.get(), BLACK);
    DrawTextRect(_position.x, _position.y, _position.w, _fontHeight, item.getTitle().c_str(), ALIGN_LEFT);

    SetFont(_entryFont.get(), BLACK);
    if (item.getType() == IFILE)
    {
        DrawTextRect(_position.x, _position.y + _fontHeight, _position.w, _fontHeight, item.getFiletype().c_str(), ALIGN_LEFT);
        if (item.isDownloaded())
        {
            DrawTextRect(_position.x, _position.y + 3 * _fontHeight, _position.w, _fontHeight, "Synced", ALIGN_RIGHT);
        }
        else
        {
            DrawTextRect(_position.x, _position.y + 3 * _fontHeight, _position.w, _fontHeight, "Click to Download", ALIGN_RIGHT);
        }
    }
    DrawTextRect(_position.x, _position.y + 2 * _fontHeight, _position.w, _fontHeight, item.getLastEditDate().c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + 3 * _fontHeight, _position.w, _fontHeight, item.getSize().c_str(), ALIGN_LEFT);

    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}