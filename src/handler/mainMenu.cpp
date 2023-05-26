//------------------------------------------------------------------
// mainMenu.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "mainMenu.h"
#include "inkview.h"

#include <string>

namespace{
    constexpr auto CHOOSE_FOLDER_TITLE{"Create here"};
    constexpr auto EXCLUDE_FILES_TITLE{"Exclude and hide items"};
    constexpr auto EXIT_TITLE{"Close App"};
    constexpr auto INFO_TITLE{"Info"};
    constexpr auto LOGOUT_TITLE{"Logout"};
    constexpr auto MENU_TITLE{"Menu"};
    constexpr auto SORT_BY_TITLE{"Order items by"};
    constexpr auto SYNC_FOLDER_TITLE{"Actualize folder"};

    constexpr auto MENU_BUTTON_WIDTH_SCALE{2u};
    constexpr auto MENU_POSITION{0u};

    constexpr auto SCREEN_BEGIN{0u};
    constexpr auto MENU_PANEL_HEIGHT_SCALE{18u};
    constexpr auto MENU_PANEL_WIDTH_SCALE{3u};

    constexpr auto PANEL_OFF{0u};

    constexpr auto TEXT_MARGIN{10};
    constexpr auto TEXT_BEGIN{SCREEN_BEGIN+TEXT_MARGIN};

    constexpr auto MENU_FONT{"LiberationMono-Bold"};
    constexpr auto HEADER{0};
}

MainMenu::MainMenu(const std::string &p_name) {

    //menu panel size
    auto menuPanelHeight{ScreenHeight() / MENU_PANEL_HEIGHT_SCALE};
    auto menuPanelWidth{ScreenWidth() / MENU_PANEL_WIDTH_SCALE};
    m_menuPanelBeginX = ScreenWidth() - menuPanelWidth;
    m_menuPanelBeginY = SCREEN_BEGIN;

    //menu button
    auto menuButtonRectWidth{menuPanelWidth * MENU_BUTTON_WIDTH_SCALE};
    m_menuButtonRect = iRect(menuButtonRectWidth, m_menuPanelBeginY, menuPanelWidth, menuPanelHeight, ALIGN_RIGHT);

    auto menuPanelFontSize{menuPanelHeight/2};
    auto menuFont{OpenFont(MENU_FONT, menuPanelFontSize, FONT_STD)};
    SetFont(menuFont, BLACK);
    DrawTextRect(SCREEN_BEGIN,m_menuPanelBeginY, ScreenWidth(),menuPanelHeight,
            p_name.c_str(), ALIGN_CENTER);
    DrawTextRect2(&m_menuButtonRect, MENU_TITLE);
    CloseFont(menuFont);

    auto lineThickness{menuPanelHeight - 1};
    DrawLine(SCREEN_BEGIN,lineThickness, ScreenWidth(),lineThickness, BLACK);

    auto contentRectHeight{ScreenHeight() - menuPanelHeight};
    auto textEnd{ScreenWidth() - (TEXT_MARGIN*2)};
    m_contentRect = iRect(TEXT_BEGIN, menuPanelHeight, textEnd, contentRectHeight, ALIGN_FIT);

    SetPanelType(PANEL_OFF);
    PartialUpdate(SCREEN_BEGIN, m_menuPanelBeginY, ScreenWidth(), menuPanelHeight);
}

void MainMenu::panelHandlerStatic() {
  DrawPanel(NULL, "", NULL, -1);
  SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
}

void MainMenu::drawMenu(bool p_filePicker, bool p_loggedIn,
                         iv_menuhandler p_handler) const{
  imenu mainMenu[] = {
      {ITEM_HEADER, HEADER, const_cast<char*>(MENU_TITLE), NULL},
      // show logged in
      {p_loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, static_cast<short>(MainMenuOption::ActualizeCurrentFolder),const_cast<char*>(SYNC_FOLDER_TITLE),
          NULL},
      {p_loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, static_cast<short>(MainMenuOption::SortBy),const_cast<char*>(SORT_BY_TITLE), NULL},
      {p_loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, static_cast<short>(MainMenuOption::ExcludeFiles),const_cast<char*>(EXCLUDE_FILES_TITLE),
       NULL},
      // show if filePicker is shown
      {p_filePicker ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, static_cast<short>(MainMenuOption::ChooseFolder),const_cast<char*>(CHOOSE_FOLDER_TITLE),
       NULL},
      // show always
      {ITEM_ACTIVE, static_cast<short>(MainMenuOption::Info),const_cast<char*>(INFO_TITLE), NULL},
      // show logged in
      {p_loggedIn ? (short)ITEM_ACTIVE : (short)ITEM_HIDDEN, static_cast<short>(MainMenuOption::Logout),const_cast<char*>(LOGOUT_TITLE), NULL},
      // show always
      {ITEM_ACTIVE, static_cast<short>(MainMenuOption::Exit),const_cast<char*>(EXIT_TITLE), NULL},
      {0, 0, NULL, NULL}};

  OpenMenu(mainMenu, MENU_POSITION, m_menuPanelBeginX, m_menuPanelBeginY, p_handler);
}
