//------------------------------------------------------------------
// listViewEntry.cpp
//
// Author:           JuanJakobo          
// Date:             04.08.2020
//  
//-------------------------------------------------------------------

#include "inkview.h"
#include "item.h"
#include "listViewEntry.h"

using namespace std;

ListViewEntry::ListViewEntry(int Page, irect Rect): rect(Rect), page(Page)
{

}

void ListViewEntry::draw(const Item &item) const
{  

    DrawTextRect(rect.x,rect.y,rect.w,rect.h,item.getTitle().c_str(),ALIGN_LEFT);
    DrawTextRect(rect.x,rect.y+30,rect.w,rect.h,item.getPath().c_str(),ALIGN_LEFT);

    int line = (rect.y+rect.h)-1;
    DrawLine(0,line,ScreenWidth(),line,BLACK);

}