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

using std::string;
using std::vector;

ListView::ListView(irect *contentRect, const vector<Item> &items) : _contentRect(contentRect), _items(items)
{
    _font = OpenFont("LiberationMono", 30, 1);
    SetFont(_font, BLACK);
    FillAreaRect(_contentRect, WHITE);

    _entries.clear();

    int itemCount = 7;
    _footerHeight = 100;
    _headerHeight = 40;
    int entrySize = (_contentRect->h - _footerHeight - _headerHeight) / itemCount;

    _shownPage = 1;
    _page = 1;

    auto i = _items.size();
    auto z = 0;

    _entries.reserve(i);

    while (i > 0)
    {
        if (z >= itemCount)
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
    delete _font;
}

void ListView::drawHeader(string headerText)
{
    headerText = Util::replaceString(headerText,"%20"," ");

    _font = OpenFont("LiberationMono", 35, 1);
    SetFont(_font, BLACK);

    DrawTextRect(_contentRect->x, _contentRect->y, _contentRect->w, _headerHeight - 1, headerText.c_str(), ALIGN_LEFT);

    int line = (_contentRect->y + _headerHeight) - 2;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}

void ListView::drawFooter()
{
    string footer = Util::intToString(_shownPage) + "/" + Util::intToString(_page);
    SetFont(_font, WHITE);
    FillAreaRect(&_pageButton, BLACK);
    DrawTextRect2(&_pageButton, footer.c_str());
}

void ListView::drawEntries()
{
    for (auto i = 0; i < _entries.size(); i++)
    {
        if (_entries[i].getPage() == _shownPage)
            _entries[i].draw(_items[i]);
    }
}

int ListView::listClicked(int x, int y)
{
    SetFont(_font, BLACK);

    if (IsInRect(x, y, &_pageButton))
    {
        if (_page > 1)
        {
            FillAreaRect(_contentRect, WHITE);

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