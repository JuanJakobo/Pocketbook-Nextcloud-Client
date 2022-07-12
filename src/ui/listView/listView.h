//------------------------------------------------------------------
// listView.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An UI class to display items in a listview
//-------------------------------------------------------------------

#ifndef LISTVIEW
#define LISTVIEW

#include "inkview.h"
#include "listViewEntry.h"
#include "model.h"

#include <vector>
#include <memory>

class ListView
{
public:
    /**
        * Displays a list view
        *
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
    ListView(const irect *contentRect, int page);

    virtual ~ListView();

    virtual Entry *getCurrentEntry() = 0;
    virtual Entry *getEntry(int entryID) = 0;

    int getShownPage(){return _shownPage;};

    /**
        * Navigates to the next page
        */
    void nextPage() { this->actualizePage(_shownPage + 1); };

    /**
        * Navigates to the prev page
        */
    void prevPage() { this->actualizePage(_shownPage - 1); };

    /**
        * Navigates to first page
        */
    void firstPage() { this->actualizePage(1); };

    /**
        * Draws an single entry to the screen
        */
    void reDrawCurrentEntry();

    /**
        * inverts the color of the currently selected entry 
        */
    void invertCurrentEntryColor();

    /**
        * Checkes if the listview has been clicked and either changes the page or returns item ID
        * 
        * @param x x-coordinate
        * @param y y-coordinate
        * @return true if was clicked
        */
    bool checkIfEntryClicked(int x, int y);

    int getCurrentEntryItertator() const {return _selectedEntry;};

    /**
        * Clears the screen and draws entries and footer 
        * 
        */
    void draw();
    
protected:
    int _footerHeight;
    int _footerFontHeight;
    int _entryFontHeight;
    const irect *_contentRect;
    std::vector<std::shared_ptr<ListViewEntry>> _entries;
    ifont *_footerFont;
    ifont *_entryFont;
    ifont *_entryFontBold;
    int _page = 1;
    int _shownPage;
    irect _pageIcon;
    irect _nextPageButton;
    irect _prevPageButton;
    irect _firstPageButton;
    irect _lastPageButton;
    int _selectedEntry;


    /**
        * Iterates through the items and sends them to the listViewEntry Class for drawing
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
    void updateEntry(int entryID);

    /**
        * Navigates to the selected page
        * 
        * @param pageToShow page that shall be shown
        */
    void actualizePage(int pageToShow);
};
#endif
