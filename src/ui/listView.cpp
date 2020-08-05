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



ListView::ListView(irect* ContentRect, const vector<Item>& Items): contentRect(ContentRect), items(Items)
{
    loadingScreenRect = iRect(contentRect->w/2-100,contentRect->h/2-50,200,100,ALIGN_CENTER);

    font = OpenFont("LiberationMono",30,1);
    SetFont(font,BLACK);   
    FillAreaRect(contentRect,WHITE);

    entries.clear();

    int itemCount = 7;
    footerHeight =  100;
    int entrySize = (contentRect->h-footerHeight)/itemCount;

    shownPage = 1;
    page = 1;

    auto i = items.size();
    auto z = 0;

    while(i > 0)
    {   
        if(z>=itemCount)
        {
            page++;
            z=0;
        }

        irect rect = iRect(contentRect->x,z*entrySize+contentRect->y,contentRect->w,entrySize,0);
        this->entries.push_back(ListViewEntry(page, rect));
        i--;
        z++;
    }

    pageButton = iRect(contentRect->w-100,contentRect->h+contentRect->y-footerHeight,100,footerHeight,ALIGN_CENTER);

    drawEntries();
    drawFooter();

}

ListView::~ListView()
{
    delete font;
}

void ListView::drawFooter()
{
    string footer = Util::intToString(shownPage) + "/" + Util::intToString(page);
    SetFont(font, WHITE);
    FillAreaRect(&pageButton, BLACK);
    DrawTextRect2(&pageButton,footer.c_str());
}

void ListView::drawEntries()
{
    for(auto i = 0; i < entries.size(); i++)
    {
        if(entries[i].getPage()==shownPage)
            entries[i].draw(items[i]);
    }
}

int ListView::listClicked(int x, int y)
{
    SetFont(font,BLACK);   

    if(IsInRect(x,y,&pageButton))
    {
        if(page>1)
        {
            FillAreaRect(contentRect,WHITE);

            if(shownPage>=page)
            {
                shownPage = 1;
            }
            else
            {
                shownPage++;
            }

            drawEntries();
            drawFooter();
        }
    }
    else
    {
        FillAreaRect(contentRect,WHITE);

        for(unsigned int i = 0; i < entries.size(); i++)
        {
            if(entries[i].getPage()==shownPage && IsInRect(x,y,entries[i].getRect())==1)
            {
                DrawTextRect2(&loadingScreenRect,"Loading...");
                PartialUpdate(loadingScreenRect.x,loadingScreenRect.y,loadingScreenRect.w,loadingScreenRect.h);
                return i;
            }
        }
    }
    return -1;
}