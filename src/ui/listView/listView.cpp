//------------------------------------------------------------------
// listView.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "listView.h"
#include "listViewEntry.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

ListView::ListView(const irect &contentRect, int page) : _contentRect(contentRect), _shownPage(page)
{
    _entries.clear();

    _footerHeight = _contentRect.h / 15;
    _footerFontHeight = 0.3 * _footerHeight;
    _entryFontHeight = contentRect.h/45;

    _footerFont = OpenFont("LiberationMono", _footerFontHeight, 1);
    _entryFont = OpenFont("LiberationMono", _entryFontHeight, 1);
    _entryFontBold = OpenFont("LiberationMono-Bold", _entryFontHeight, 1);

    SetFont(_entryFont, BLACK);

    int footerWidth = contentRect.w/20;

    _pageIcon = iRect(_contentRect.w - footerWidth*2, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/10, _footerHeight, ALIGN_CENTER);

    _firstPageButton = iRect(_contentRect.x, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);

    _prevPageButton = iRect(_contentRect.x + footerWidth*3, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);

    _nextPageButton = iRect(_contentRect.x + footerWidth*6, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);

    _lastPageButton = iRect(_contentRect.x + footerWidth*9, _contentRect.h + _contentRect.y - _footerHeight, contentRect.w/8, _footerHeight, ALIGN_CENTER);
}

ListView::~ListView()
{
    CloseFont(_entryFont);
    CloseFont(_entryFontBold);
    CloseFont(_footerFont);
}

void ListView::draw()
{
    FillAreaRect(&_contentRect, WHITE);
    drawEntries();
    drawFooter();
    PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
}

void ListView::reDrawCurrentEntry()
{
    FillAreaRect(&_entries.at(_selectedEntry)->getPosition(), WHITE);
    _entries.at(_selectedEntry)->draw(_entryFont, _entryFontBold, _entryFontHeight);
    updateEntry(_selectedEntry);
}

void ListView::invertCurrentEntryColor()
{
    InvertAreaBW(_entries.at(_selectedEntry)->getPosition().x, _entries.at(_selectedEntry)->getPosition().y, _entries.at(_selectedEntry)->getPosition().w, _entries.at(_selectedEntry)->getPosition().h);
    updateEntry(_selectedEntry);
}

void ListView::drawEntries()
{
    for (unsigned int i = 0; i < _entries.size(); i++)
    {
        if (_entries.at(i)->getPage() == _shownPage)
            _entries.at(i)->draw(_entryFont, _entryFontBold, _entryFontHeight);
    }
}

bool ListView::checkIfEntryClicked(int x, int y)
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
            if (_entries.at(i)->getPage() == _shownPage && IsInRect(x, y, &_entries.at(i)->getPosition()) == 1)
            {
                _selectedEntry = i;
                return true;
            }
        }
    }
    return false;
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

void ListView::updateEntry(int entryID)
{
    PartialUpdate(_entries.at(entryID)->getPosition().x, _entries.at(entryID)->getPosition().y, _entries.at(entryID)->getPosition().w, _entries.at(entryID)->getPosition().h);
}

void ListView::actualizePage(int pageToShow)
{
    if (pageToShow > _page)
    {
        Message(ICON_INFORMATION, "Info", "You have reached the last page, to return to the first, please click \"first.\"", 1200);
    }
    else if (pageToShow < 1)
    {
        Message(ICON_INFORMATION, "Info", "You are already on the first page.", 1200);
    }
    else
    {
        _shownPage = pageToShow;
        FillArea(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h, WHITE);
        drawEntries();
        drawFooter();
        PartialUpdate(_contentRect.x, _contentRect.y, _contentRect.w, _contentRect.h);
    }
}
