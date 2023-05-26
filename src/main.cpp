//------------------------------------------------------------------
// main.h
//
// Author:           JuanJakobo
// Date:             14.06.2020
// Description:      sets the inkview main handler
//-------------------------------------------------------------------

#include "eventHandler.h"
#include "inkview.h"

namespace {
    constexpr auto MENU_FONT{"LiberationMono"};
    constexpr auto SCREEN_BEGIN{0};
}

std::unique_ptr<EventHandler> events;
/**
 * Handles events and redirects them
 *
 * @param type event type
 * @param par1 first argument of the event
 * @param par2 second argument of the event
 * @return int returns if the event was handled
 */
int Inkview_handler(int type, int par1, int par2) {
  switch (type) {
  case EVT_INIT: {
    events = std::make_unique<EventHandler>();
    return 1;
    break;
  }
  case EVT_EXIT:
  case EVT_HIDE: {
    return 1;
    break;
  }
  default: {
    return events->eventDistributor(type, par1, par2);
  }
  }
  return 0;
}

int main() {
  OpenScreen();
  SetOrientation(0);

  // draw startscreen
  const auto fontSize{ScreenHeight() / 30};
  const auto startscreenFont{OpenFont(MENU_FONT, fontSize, FONT_BOLD)};
  SetFont(startscreenFont, BLACK);
  const auto logoHeight{(ScreenHeight()/3)*2};
  DrawTextRect(SCREEN_BEGIN, logoHeight, ScreenWidth(), fontSize,
               APPLICATION_NAME, ALIGN_CENTER);
  CloseFont(startscreenFont);
  FullUpdate();

  InkViewMain(Inkview_handler);
  return 0;
}
