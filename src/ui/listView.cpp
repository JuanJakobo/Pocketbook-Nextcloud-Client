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

ListView::ListView(const irect *contentRect, const vector<Item> &items) : _contentRect(contentRect), _items(&items)
{
    FillAreaRect(_contentRect, WHITE);

    _entries.clear();

    int entrySize = _contentRect->h / (_itemCount + 1);

    _headerHeight = 0.25 * entrySize;
    _footerHeight = 0.75 * entrySize;

    _headerFontHeight = 0.8 * _headerHeight;
    _footerFontHeight = 0.3 * _footerHeight;
    _entryFontHeight = 0.2 * entrySize;

    _headerFont = OpenFont("LiberationMono", _headerFontHeight, FONT_STD);
    _footerFont = OpenFont("LiberationMono", _footerFontHeight, FONT_STD);
    _entryFont = OpenFont("LiberationMono", _entryFontHeight, FONT_STD);
    _entryFontBold = OpenFont("LiberationMono-Bold", _entryFontHeight, FONT_BOLD);

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

    _firstPageButton = iRect(_contentRect->x, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    _prevPageButton = iRect(_contentRect->x + 150, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    _nextPageButton = iRect(_contentRect->x + 300, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);
    _lastPageButton = iRect(_contentRect->x + 450, _contentRect->h + _contentRect->y - _footerHeight, 130, _footerHeight, ALIGN_CENTER);

    drawEntries();
    drawFooter();
}

ListView::~ListView()
{
    CloseFont(_entryFont);
    CloseFont(_entryFontBold);
    CloseFont(_headerFont);
    CloseFont(_footerFont);
}

void ListView::drawHeader(string headerText)
{
    SetFont(_headerFont, BLACK);
    Util::decodeUrl(headerText);
    DrawTextRect(_contentRect->x, _contentRect->y, _contentRect->w, _headerHeight - 1, headerText.c_str(), ALIGN_LEFT);

    int line = (_contentRect->y + _headerHeight) - 2;
    DrawLine(0, line, ScreenWidth(), line, BLACK);
}

void ListView::drawEntry(int itemID)
{
    FillAreaRect(_entries[itemID].getPosition(), WHITE);
    _entries[itemID].draw(_items->at(itemID), _entryFont, _entryFontBold, _entryFontHeight);
}

void ListView::drawEntries()
{
    for (unsigned int i = 0; i < _entries.size(); i++)
    {
        if (_entries[i].getPage() == _shownPage)
            _entries[i].draw(_items->at(i), _entryFont, _entryFontBold, _entryFontHeight);
    }
}

int ListView::listClicked(int x, int y)
{
    if (IsInRect(x, y, &_firstPageButton))
    {
        firstPage();
    }
    else if (IsInRect(x, y, &_nextPageButton))
    {
        nextPage();
    }
    else if (IsInRect(x, y, &_prevPageButton))
    {
        prevPage();
    }
    else if (IsInRect(x, y, &_lastPageButton))
    {
        actualizePage(_page);
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

void ListView::drawFooter()
{
    SetFont(_footerFont, WHITE);
    string footer = std::to_string(_shownPage) + "/" + std::to_string(_page);
    FillAreaRect(&_pageIcon, BLACK);

    DrawTextRect2(&_pageIcon, footer.c_str());
    FillAreaRect(&_firstPageButton, BLACK);
    DrawTextRect2(&_firstPageButton, "First");
    FillAreaRect(&_prevPageButton, BLACK);
    DrawTextRect2(&_prevPageButton, "Prev");
    FillAreaRect(&_nextPageButton, BLACK);
    DrawTextRect2(&_nextPageButton, "Next");
    FillAreaRect(&_lastPageButton, BLACK);
    DrawTextRect2(&_lastPageButton, "Last");
}

void ListView::updateEntry(int itemID)
{
    PartialUpdate(_entries[itemID].getPosition()->x, _entries[itemID].getPosition()->y, _entries[itemID].getPosition()->w, _entries[itemID].getPosition()->h);
}

void ListView::actualizePage(int pageToShown)
{
    if (pageToShown > _page)
    {
        Message(ICON_INFORMATION, "Info", "You have reached the last page, to return to the first, please click \"first.\"", 1200);
    }
    else if (pageToShown < 1)
    {
        Message(ICON_INFORMATION, "Info", "You are already on the first page.", 1200);
    }
    else
    {
        _shownPage = pageToShown;
        FillArea(_contentRect->x, _contentRect->y + _headerHeight, _contentRect->w, _contentRect->h, WHITE);
        drawEntries();
        drawFooter();
        PartialUpdate(_contentRect->x, _contentRect->y + _headerHeight, _contentRect->w, _contentRect->h);
    }
}