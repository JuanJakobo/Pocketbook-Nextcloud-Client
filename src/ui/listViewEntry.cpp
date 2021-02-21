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

void ListViewEntry::draw(const Item &item, ifont *entryFont, ifont *entryFontBold, int fontHeight)
{
    SetFont(entryFontBold, BLACK);
    DrawTextRect(_position.x, _position.y, _position.w, fontHeight, item.getTitle().c_str(), ALIGN_LEFT);

    SetFont(entryFont, BLACK);

    if (item.getState() == FileState::ILOCAL)
    {
        DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Local", ALIGN_RIGHT);
    }
    else
    {
        if (item.getType() == IFILE)
        {
            DrawTextRect(_position.x, _position.y + fontHeight, _position.w, fontHeight, item.getFiletype().c_str(), ALIGN_LEFT);

            if (item.getState() == FileState::ISYNCED)
            {
                DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Synced", ALIGN_RIGHT);
            }
            else if(item.getState() == FileState::IOUTSYNCED)
            {
                DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Out of sync", ALIGN_RIGHT);
            }
            else
            {
                DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Click to Download", ALIGN_RIGHT);
            }
        }
        else
        {
            if (item.getState() == FileState::ISYNCED)
            {
                DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, "Folder synced", ALIGN_RIGHT);
            }
        }

        DrawTextRect(_position.x, _position.y + 2 * fontHeight, _position.w, fontHeight, item.getLastEditDate().c_str(), ALIGN_LEFT);
        DrawTextRect(_position.x, _position.y + 3 * fontHeight, _position.w, fontHeight, item.getSizeString().c_str(), ALIGN_LEFT);
    }

    int line = (_position.y + _position.h) - 1;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}