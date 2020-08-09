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
#include "item.h"
#include "listViewEntry.h"

#include <string>
#include <vector>

using namespace std;

class ListView
{
    public:

        /**
        * Displays a list view 
        * 
        * @param ContentRect area of the screen where the list view is placed
        * @param Items items that shall be shown in the listview
        */
        ListView(irect* ContentRect, const vector<Item> &Items);

        /**
        * Destructor 
        */
        ~ListView();

        /**
        * draws the header including an item to navigate a page up
        * 
        */
        void drawHeader(const string &headerText);

        /**
        * draws the footer including a page changer 
        * 
        */
        void drawFooter();

        /**
        * iterates through the items and sends them to the listViewEntry Class for drawing
        * 
        */
        void drawEntries();

        /**
        * Checkes if the listview has been clicked and either changes the page or returns item ID
        * 
        * @param x x-coordinate
        * @param y y-coordinate
        * @return int Item ID that has been clicked, -1 if no Item was clicked
        */
        int listClicked(int x, int y);

    private:
        irect* contentRect;
        const vector<Item> &items;
        
        irect  loadingScreenRect;
        ifont* font;

        vector<ListViewEntry>   entries;
        int page;
        int shownPage;
        irect pageButton;
        int footerHeight;
        int headerHeight;

};
#endif