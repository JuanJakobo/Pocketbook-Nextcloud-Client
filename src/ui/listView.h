#pragma once
//------------------------------------------------------------------
// listView.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------
#include <memory>
#include <vector>

#include "inkview.h"
#include "listViewEntry.h"
#include "model.h"

// TODO make template as in text view!
class ListView
{
  public:
    /**
     * Displays a list view
     *
     * @param ContentRect area of the screen where the list view is placed
     * @param Items items that shall be shown in the listview
     */
    ListView(const irect &p_contentRect, uint8_t p_page);

    virtual ~ListView();

    uint8_t getShownPage()
    {
        return m_shownPage;
    };

    /**
     * Navigates to the next page
     */
    void nextPage()
    {
        this->actualizePage(m_shownPage + 1);
    };

    /**
     * Navigates to the prev page
     */
    void prevPage()
    {
        this->actualizePage(m_shownPage - 1);
    };

    /**
     * Navigates to first page
     */
    void firstPage()
    {
        this->actualizePage(1);
    };

    /**
     * Draws an single entry to the screen
     */
    void reDrawCurrentEntry();

    /**
     * inverts the color of the currently selected entry
     */
    void invertCurrentEntryColor();

    /**
     * Checkes if the listview has been clicked and either changes the page or
     * returns item ID
     *
     * @param x x-coordinate
     * @param y y-coordinate
     * @return true if was clicked
     */
    bool checkIfEntryClicked(int p_x, int p_y);

    size_t getCurrentEntryItertator() const
    {
        return m_selectedEntry;
    };

    /**
     * Clears the screen and draws entries and footer
     *
     */
    void draw();

  protected:
    int m_footerHeight;
    int m_footerFontHeight;
    int m_entryFontHeight;
    const irect m_contentRect;
    std::vector<std::shared_ptr<ListViewEntry>> m_entries;
    ifont *m_footerFont;
    ifont *m_entryFont;
    ifont *m_entryFontBold;
    uint8_t m_page{1};
    uint8_t m_shownPage;
    irect m_pageIcon;
    irect m_nextPageButton;
    irect m_prevPageButton;
    irect m_firstPageButton;
    irect m_lastPageButton;
    size_t m_selectedEntry;

    /**
     * Iterates through the items and sends them to the listViewEntry Class for
     * drawing
     */
    void drawEntries();

    /**
     * Draws the footer including a page changer
     */
    void drawFooter();

    /**
     * updates an entry
     *
     * @param entryID the id of the item that shall be inverted
     */
    void updateEntry(size_t p_entryID);

    /**
     * Navigates to the selected page
     *
     * @param pageToShow page that shall be shown
     */
    void actualizePage(uint8_t p_pageToShow);
};
