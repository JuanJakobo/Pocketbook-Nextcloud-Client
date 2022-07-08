//------------------------------------------------------------------
// main.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      sets the inkview main handler
//-------------------------------------------------------------------

#include "inkview.h"
#include "eventHandler.h"

EventHandler *events = nullptr;
/**
* Handles events and redirects them
*
* @param type event type
* @param par1 first argument of the event
* @param par2 second argument of the event
* @return int returns if the event was handled
*/
int Inkview_handler(int type, int par1, int par2)
{
    switch (type)
    {
        case EVT_INIT:
            {
                events = new EventHandler();
                return 1;
                break;
            }
        case EVT_EXIT:
        case EVT_HIDE:
            {
                delete events;
                return 1;
                break;
            }
        default:
            {
                return events->eventDistributor(type, par1, par2);
            }
    }
    return 0;
}

int main()
{
    OpenScreen();
    SetOrientation(0);

    //draw startscreen
    auto textHeight = ScreenHeight()/30;
    auto startscreenFont = OpenFont("LiberationMono", textHeight, FONT_BOLD);
    SetFont(startscreenFont, BLACK);
    DrawTextRect(0, (ScreenHeight() / 3) * 2, ScreenWidth(), textHeight, "Nextcloud Client", ALIGN_CENTER);
    CloseFont(startscreenFont);
    FullUpdate();

    InkViewMain(Inkview_handler);
    return 0;
}
