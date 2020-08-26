//------------------------------------------------------------------
// listViewEntry.h
//
// Author:           JuanJakobo
// Date:             04.08.2020
// Description:      An listViewEntry that handles an item of a listview
//-------------------------------------------------------------------

#ifndef LISTVIEWENTRY
#define LISTVIEWENTRY

#include "inkview.h"
#include "item.h"

class ListViewEntry
{
public:
    /**
        * Creates an ListViewEntry 
        * 
        * @param Page site of the listView the Entry is shown
        * @param Rect area of the screen the item is positioned
        */
    ListViewEntry(int page, irect position);

    irect *getPosition() { return &_position; }
    int getPage() const { return _page; }

    /**
        * draws the listViewEntry to the screen
        * 
        * @param item item that shall be drawn
        */
    void draw(const Item &item) const;

private:
    int _page;
    irect _position;
};
#endif