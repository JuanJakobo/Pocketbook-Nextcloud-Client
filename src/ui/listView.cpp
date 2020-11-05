//------------------------------------------------------------------
// listView.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "item.h"
#include "util.h"
#include "listView.h"
#include "listViewEntry.h"

#include <string>
#include <vector>
#include <memory>

using std::string;
using std::vector;

ListView::ListView(irect *contentRect, const std::shared_ptr<vector<Item>> items) : _contentRect(contentRect), _items(items)
{
    FillAreaRect(_contentRect, WHITE);

    _titleFont = std::unique_ptr<ifont>(OpenFont("LiberationMono", 35, 1));
    _footerFont = std::unique_ptr<ifont>(OpenFont("LiberationMono", 30, 1));

    _entries.clear();

    int entrySize = (_contentRect->h - _footerHeight - _headerHeight) / _itemCount;

    _shownPage = 1;
    _page = 1;

    auto i = _items->size();
    auto z = 0;

    _entries.reserve(i);

    while (i > 0)
    {
        if (z >= _itemCount)
        {
            _page++;
            z = 0;
        }

        irect rect = iRect(_contentRect->x, z * entrySize + _headerHeight + _contentRect->y, _contentRect->w, entrySize, 0);
        this->_entries.emplace_back(_page, rect);
        i--;
        z++;
    }

    _pageButton = iRect(_contentRect->w - 100, _contentRect->h + _contentRect->y - _footerHeight, 100, _footerHeight, ALIGN_CENTER);

    drawEntries();
    drawFooter();
}

ListView::~ListView()
{
}

void ListView::drawHeader(string headerText)
{
    SetFont(_titleFont.get(), BLACK);
    headerText = Util::replaceString(headerText,"%20"," ");
    DrawTextRect(_contentRect->x, _contentRect->y, _contentRect->w, _headerHeight - 1, headerText.c_str(), ALIGN_LEFT);

    int line = (_contentRect->y + _headerHeight) - 2;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}

void ListView::drawFooter()
{
    SetFont(_footerFont.get(), WHITE);
    string footer = Util::valueToString<int>(_shownPage) + "/" + Util::valueToString<int>(_page);
    FillAreaRect(&_pageButton, BLACK);
    DrawTextRect2(&_pageButton, footer.c_str());
}

void ListView::drawEntry(int itemID)
{
    FillAreaRect(_entries[itemID].getPosition(),WHITE);
    _entries[itemID].draw(_items->at(itemID));
}

void ListView::drawEntries()
{
    for (auto i = 0; i < _entries.size(); i++)
    {
        if (_entries[i].getPage() == _shownPage)
            _entries[i].draw(_items->at(i));
    }
}

int ListView::listClicked(int x, int y)
{
    if (IsInRect(x, y, &_pageButton))
    {
        if (_page > 1)
        {
            FillArea(_contentRect->x,_contentRect->y+_headerHeight, _contentRect->w, _contentRect->h, WHITE);

            if (_shownPage >= _page)
            {
                _shownPage = 1;
            }
            else
            {
                _shownPage++;
            }

            drawEntries();
            drawFooter();
        }
    }
    else
    {
        for (unsigned int i = 0; i < _entries.size(); i++)
        {
            if (_entries[i].getPage() == _shownPage && IsInRect(x, y, _entries[i].getPosition()) == 1)
            {
                return i;
            }
        }
    }
    return -1;
}