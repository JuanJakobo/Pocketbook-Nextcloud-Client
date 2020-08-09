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
    int fontHeight = 30;

    irect pageButton = iRect(rect.x,rect.y,100,rect.h-30,ALIGN_LEFT);

    if(item.getType()==IFILE)
    {
        FillAreaRect(&pageButton, BLACK);
        DrawTextRect2(&pageButton,"bitmap");
        DrawTextRect(pageButton.w,rect.y+2*fontHeight,rect.w,fontHeight,item.getFiletype().c_str(),ALIGN_LEFT);
        if(item.isDownloaded())
        {
            DrawTextRect(rect.x,rect.y+3*fontHeight,rect.w,fontHeight,"Synced",ALIGN_RIGHT);
        }
        else
        {
            DrawTextRect(rect.x,rect.y+3*fontHeight,rect.w,fontHeight,"Download",ALIGN_RIGHT);
        }
        

    }
    else
    {
        FillAreaRect(&pageButton, BLACK);
        DrawTextRect2(&pageButton,"bitmap");
    }
    
    DrawTextRect(pageButton.w,rect.y,rect.w,fontHeight,item.getTitle().c_str(),ALIGN_LEFT);
    DrawTextRect(pageButton.w,rect.y+3*fontHeight,rect.w,fontHeight,item.getSize().c_str(),ALIGN_LEFT);

    DrawTextRect(rect.x,rect.y+2*fontHeight,rect.w,fontHeight,item.getLastEditDate().c_str(),ALIGN_RIGHT);


    int line = (rect.y+rect.h)-1;
    DrawLine(0,line,ScreenWidth(),line,BLACK);

}