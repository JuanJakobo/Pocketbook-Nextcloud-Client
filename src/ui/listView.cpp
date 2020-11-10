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

ListView::ListView(const irect *contentRect, const std::shared_ptr<vector<Item>> items) : _contentRect(contentRect), _items(items)
{
    FillAreaRect(_contentRect, WHITE);

    _titleFont = std::unique_ptr<ifont>(OpenFont("LiberationMono", 35, 1));
    _footerFont = std::unique_ptr<ifont>(OpenFont("LiberationMono", 30, 1));

    _entries.clear();

    int entrySize = (_contentRect->h - _footerHeight - _headerHeight) / _itemCount;

    _page = 1;
    _shownPage = _page;

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

    _pageIcon = iRect(_contentRect->w - 100, _contentRect->h + _contentRect->y - _footerHeight, 100, _footerHeight, ALIGN_CENTER);

    //TODO draw botton back and next, draw just once?
    _firstPageButton = iRect(_contentRect->x, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    _lastPageButton = iRect(_contentRect->x + 150, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    _nextPageButton = iRect(_contentRect->x + 300, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);

    drawEntries();
    drawFooter();
}

ListView::~ListView()
{
}

void ListView::drawHeader(string headerText)
{
    SetFont(_titleFont.get(), BLACK);
    Util::decodeUrl(headerText);
    DrawTextRect(_contentRect->x, _contentRect->y, _contentRect->w, _headerHeight - 1, headerText.c_str(), ALIGN_LEFT);

    int line = (_contentRect->y + _headerHeight) - 2;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}

void ListView::drawFooter()
{
    SetFont(_footerFont.get(), WHITE);
    string footer = Util::valueToString<int>(_shownPage) + "/" + Util::valueToString<int>(_page);
    FillAreaRect(&_pageIcon, BLACK);

    DrawTextRect2(&_pageIcon, footer.c_str());
    FillAreaRect(&_firstPageButton, BLACK);
    DrawTextRect2(&_firstPageButton, "First");
    FillAreaRect(&_lastPageButton, BLACK);
    DrawTextRect2(&_lastPageButton, "Last");
    FillAreaRect(&_nextPageButton, BLACK);
    DrawTextRect2(&_nextPageButton, "Next");
}

void ListView::drawEntry(int itemID)
{
    FillAreaRect(_entries[itemID].getPosition(), WHITE);
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

void ListView::actualizePage(int _pageToShown)
{
    if (_pageToShown > _page)
    {
        Message(ICON_INFORMATION, "Info", "You have reached the last page, to return to the first, please click home.", 1200);
    }
    else if (_pageToShown < 1)
    {
        Message(ICON_INFORMATION, "Info", "You are already on the first page.", 1200);
    }
    else
    {
        _shownPage = _pageToShown;
        FillArea(_contentRect->x, _contentRect->y + _headerHeight, _contentRect->w, _contentRect->h, WHITE);
        drawEntries();
        drawFooter();
    }
}

int ListView::listClicked(int x, int y)
{
    if (IsInRect(x, y, &_firstPageButton))
    {
        actualizePage(1);
    }
    else if (IsInRect(x, y, &_nextPageButton))
    {
        actualizePage(_shownPage+1);
    }
    else if (IsInRect(x, y, &_lastPageButton))
    {
        actualizePage(_shownPage-1);
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