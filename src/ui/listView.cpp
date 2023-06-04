//------------------------------------------------------------------
// listView.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "listView.h"

#include <string>
#include <vector>

#include "eventHandler.h"
#include "inkview.h"
#include "listViewEntry.h"

namespace
{
constexpr auto ENTRY_FONT{"LiberationMono"};
constexpr auto ENTRY_FONT_BOLD{"LiberationMono-Bold"};

constexpr auto FOOTER_HEIGHT_MANIPULATOR{15};
constexpr auto FOOTER_FONT_HEIGHT_MANIPULATOR{0.3};
constexpr auto ENTRY_FONT_HEIGHT_MANIPULATOR{45};
constexpr auto FOOTER_WIDTH_MANIPULATOR{20};
} // namespace

ListView::ListView(const irect &p_contentRect, int p_page) : m_contentRect(p_contentRect), m_shownPage(p_page)
{
    m_entries.clear();

    m_footerHeight = m_contentRect.h / FOOTER_HEIGHT_MANIPULATOR;
    m_footerFontHeight = FOOTER_FONT_HEIGHT_MANIPULATOR * m_footerHeight;
    m_entryFontHeight = m_contentRect.h / ENTRY_FONT_HEIGHT_MANIPULATOR;

    m_footerFont = OpenFont(ENTRY_FONT, m_footerFontHeight, FONT_BOLD);
    m_entryFont = OpenFont(ENTRY_FONT, m_entryFontHeight, FONT_BOLD);
    m_entryFontBold = OpenFont(ENTRY_FONT_BOLD, m_entryFontHeight, FONT_BOLD);

    SetFont(m_entryFont, BLACK);

    auto const footerWidth{m_contentRect.w / FOOTER_WIDTH_MANIPULATOR};

    m_pageIcon = iRect(m_contentRect.w - footerWidth * 2, m_contentRect.h + m_contentRect.y - m_footerHeight,
                       m_contentRect.w / 10, m_footerHeight, ALIGN_CENTER);

    m_firstPageButton = iRect(m_contentRect.x, m_contentRect.h + m_contentRect.y - m_footerHeight, m_contentRect.w / 8,
                              m_footerHeight, ALIGN_CENTER);

    m_prevPageButton = iRect(m_contentRect.x + footerWidth * 3, m_contentRect.h + m_contentRect.y - m_footerHeight,
                             m_contentRect.w / 8, m_footerHeight, ALIGN_CENTER);

    m_nextPageButton = iRect(m_contentRect.x + footerWidth * 6, m_contentRect.h + m_contentRect.y - m_footerHeight,
                             m_contentRect.w / 8, m_footerHeight, ALIGN_CENTER);

    m_lastPageButton = iRect(m_contentRect.x + footerWidth * 9, m_contentRect.h + m_contentRect.y - m_footerHeight,
                             m_contentRect.w / 8, m_footerHeight, ALIGN_CENTER);
}

ListView::~ListView()
{
    CloseFont(m_entryFont);
    CloseFont(m_entryFontBold);
    CloseFont(m_footerFont);
}

void ListView::draw()
{
    FillAreaRect(&m_contentRect, WHITE);
    drawEntries();
    drawFooter();
    PartialUpdate(m_contentRect.x, m_contentRect.y, m_contentRect.w, m_contentRect.h);
}

void ListView::reDrawCurrentEntry()
{
    FillAreaRect(&m_entries.at(m_selectedEntry)->getPosition(), WHITE);
    m_entries.at(m_selectedEntry)->draw(m_entryFont, m_entryFontBold, m_entryFontHeight);
    updateEntry(m_selectedEntry);
}

void ListView::invertCurrentEntryColor()
{
    InvertAreaBW(m_entries.at(m_selectedEntry)->getPosition().x, m_entries.at(m_selectedEntry)->getPosition().y,
                 m_entries.at(m_selectedEntry)->getPosition().w, m_entries.at(m_selectedEntry)->getPosition().h);
    updateEntry(m_selectedEntry);
}

void ListView::drawEntries()
{
    for (size_t i = 0; i < m_entries.size(); i++)
    {
        if (m_entries.at(i)->getPage() == m_shownPage)
            m_entries.at(i)->draw(m_entryFont, m_entryFontBold, m_entryFontHeight);
    }
}

bool ListView::checkIfEntryClicked(int p_x, int p_y)
{
    if (IsInRect(p_x, p_y, &m_firstPageButton))
    {
        firstPage();
    }
    else if (IsInRect(p_x, p_y, &m_nextPageButton))
    {
        nextPage();
    }
    else if (IsInRect(p_x, p_y, &m_prevPageButton))
    {
        prevPage();
    }
    else if (IsInRect(p_x, p_y, &m_lastPageButton))
    {
        actualizePage(m_page);
    }
    else
    {
        for (size_t i = 0; i < m_entries.size(); i++)
        {
            if (m_entries.at(i)->getPage() == m_shownPage && IsInRect(p_x, p_y, &m_entries.at(i)->getPosition()) == 1)
            {
                m_selectedEntry = i;
                return true;
            }
        }
    }
    return false;
}

void ListView::drawFooter()
{
    SetFont(m_footerFont, WHITE);

    std::stringstream ss;
    ss << m_shownPage << '/' << m_page;

    FillAreaRect(&m_pageIcon, BLACK);

    DrawTextRect2(&m_pageIcon, ss.str().c_str());
    FillAreaRect(&m_firstPageButton, BLACK);
    DrawTextRect2(&m_firstPageButton, "First");
    FillAreaRect(&m_prevPageButton, BLACK);
    DrawTextRect2(&m_prevPageButton, "Prev");
    FillAreaRect(&m_nextPageButton, BLACK);
    DrawTextRect2(&m_nextPageButton, "Next");
    FillAreaRect(&m_lastPageButton, BLACK);
    DrawTextRect2(&m_lastPageButton, "Last");
}

void ListView::updateEntry(size_t p_entryID)
{
    PartialUpdate(m_entries.at(p_entryID)->getPosition().x, m_entries.at(p_entryID)->getPosition().y,
                  m_entries.at(p_entryID)->getPosition().w, m_entries.at(p_entryID)->getPosition().h);
}

void ListView::actualizePage(int p_pageToShow)
{
    if (p_pageToShow > m_page)
    {
        Message(ICON_INFORMATION, "Info",
                "You have reached the last page, to return to the first, please "
                "click \"first.\"",
                TIMEOUT_MESSAGE);
    }
    else if (p_pageToShow < 1)
    {
        Message(ICON_INFORMATION, "Info", "You are already on the first page.", TIMEOUT_MESSAGE);
    }
    else
    {
        m_shownPage = p_pageToShow;
        FillArea(m_contentRect.x, m_contentRect.y, m_contentRect.w, m_contentRect.h, WHITE);
        drawEntries();
        drawFooter();
        PartialUpdate(m_contentRect.x, m_contentRect.y, m_contentRect.w, m_contentRect.h);
    }
}
