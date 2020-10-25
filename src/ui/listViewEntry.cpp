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

ListViewEntry::ListViewEntry(int page, irect rect) : _page(page), _position(rect)
{
}

void ListViewEntry::draw(const Item &item)
{
    int fontHeight = 30;
    _font = OpenFont("LiberationMono", fontHeight, 1);
    SetFont(_font, BLACK);

    if (item.getType() == IFILE)
    {
        DrawTextRect(_position.x, _position.y + fontHeight, _position.w, fontHeight, item.getFiletype().c_str(), ALIGN_LEFT);
        if (item.isDownloaded())
        {
            DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Synced", ALIGN_RIGHT);
        }
        else
        {
            DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Download", ALIGN_RIGHT);
        }
    }

    DrawTextRect(_position.x, _position.y, _position.w, fontHeight, item.getTitle().c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + 2 * fontHeight, _position.w, fontHeight, item.getLastEditDate().c_str(), ALIGN_LEFT);
    DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, item.getSize().c_str(), ALIGN_LEFT);

    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}