//------------------------------------------------------------------
// mainMenu.cpp
//
// Author:           JuanJakobo
// Date:             14.06.2020
//
//-------------------------------------------------------------------

#include "mainMenu.h"

#include <string>

#include "inkview.h"

namespace
{
constexpr auto CHOOSE_FOLDER_TITLE{"Create here"};
constexpr auto EXCLUDE_FILES_TITLE{"Exclude and hide items"};
constexpr auto EXIT_TITLE{"Close App"};
constexpr auto INFO_TITLE{"Info"};
constexpr auto LOGOUT_TITLE{"Logout"};
constexpr auto MENU_TITLE{"Menu"};
constexpr auto SORT_BY_TITLE{"Order items by"};
constexpr auto SYNC_FOLDER_TITLE{"Actualize folder"};

constexpr auto MENU_BUTTON_WIDTH_SCALE{2};
constexpr auto MENU_POSITION{0u};

constexpr auto SCREEN_BEGIN{0u};
constexpr auto MENU_PANEL_HEIGHT_SCALE{18};
constexpr auto MENU_PANEL_WIDTH_SCALE{3};

constexpr auto PANEL_OFF{0u};

constexpr auto TEXT_MARGIN{10};
constexpr auto TEXT_BEGIN{SCREEN_BEGIN + TEXT_MARGIN};

constexpr auto MENU_FONT{"LiberationMono-Bold"};
constexpr auto HEADER{0};
} // namespace

MainMenu::MainMenu(const std::string &p_name) : m_name{p_name}
{
    auto menuPanelHeight{ScreenHeight() / MENU_PANEL_HEIGHT_SCALE};
    auto menuPanelWidth{ScreenWidth() / MENU_PANEL_WIDTH_SCALE};
    auto menuPanelBeginX{ScreenWidth() - menuPanelWidth};
    m_menuRect = iRect(menuPanelBeginX, SCREEN_BEGIN, menuPanelWidth, menuPanelHeight, ALIGN_CENTER);

    auto menuButtonRectWidth{menuPanelWidth * MENU_BUTTON_WIDTH_SCALE};
    m_menuButtonRect = iRect(menuButtonRectWidth, m_menuRect.y, m_menuRect.w, m_menuRect.h, ALIGN_RIGHT);

    auto contentRectHeight{ScreenHeight() - menuPanelHeight};
    auto textEnd{ScreenWidth() - (TEXT_MARGIN * 2)};
    m_contentRect = iRect(TEXT_BEGIN, menuPanelHeight, textEnd, contentRectHeight, ALIGN_FIT);

    SetPanelType(PANEL_OFF);
}

void MainMenu::draw() const
{
    auto menuPanelFontSize{m_menuRect.h / 2};
    auto menuFont{OpenFont(MENU_FONT, menuPanelFontSize, FONT_STD)};
    SetFont(menuFont, BLACK);
    DrawTextRect(SCREEN_BEGIN, m_menuRect.y, ScreenWidth(), m_menuRect.h, m_name.c_str(), ALIGN_CENTER);
    DrawTextRect2(&m_menuButtonRect, MENU_TITLE);
    CloseFont(menuFont);

    auto lineThickness{m_menuRect.h - 1};
    DrawLine(SCREEN_BEGIN, lineThickness, ScreenWidth(), lineThickness, BLACK);
    PartialUpdate(SCREEN_BEGIN, m_menuRect.y, ScreenWidth(), m_menuRect.h);
}

void MainMenu::panelHandlerStatic()
{
    DrawPanel(NULL, "", NULL, -1);
    SetHardTimer("PANELUPDATE", panelHandlerStatic, 110000);
}

void MainMenu::open(bool p_filePicker, bool p_loggedIn, iv_menuhandler p_handler) const
{
    imenu mainMenu[] = {
        {ITEM_HEADER, HEADER, const_cast<char *>(MENU_TITLE), NULL},
        // show logged in
        {p_loggedIn ? static_cast<short>(ITEM_ACTIVE) : static_cast<short>(ITEM_HIDDEN),
         static_cast<short>(MainMenuOption::ActualizeCurrentFolder), const_cast<char *>(SYNC_FOLDER_TITLE), NULL},
        {p_loggedIn ? static_cast<short>(ITEM_ACTIVE) : static_cast<short>(ITEM_HIDDEN),
         static_cast<short>(MainMenuOption::SortBy), const_cast<char *>(SORT_BY_TITLE), NULL},
        {p_loggedIn ? static_cast<short>(ITEM_ACTIVE) : static_cast<short>(ITEM_HIDDEN),
         static_cast<short>(MainMenuOption::ExcludeFiles), const_cast<char *>(EXCLUDE_FILES_TITLE), NULL},
        // show if filePicker is shown
        {p_filePicker ? static_cast<short>(ITEM_ACTIVE) : static_cast<short>(ITEM_HIDDEN),
         static_cast<short>(MainMenuOption::ChooseFolder), const_cast<char *>(CHOOSE_FOLDER_TITLE), NULL},
        // show always
        {ITEM_ACTIVE, static_cast<short>(MainMenuOption::Info), const_cast<char *>(INFO_TITLE), NULL},
        // show logged in
        {p_loggedIn ? static_cast<short>(ITEM_ACTIVE) : static_cast<short>(ITEM_HIDDEN),
         static_cast<short>(MainMenuOption::Logout), const_cast<char *>(LOGOUT_TITLE), NULL},
        // show always
        {ITEM_ACTIVE, static_cast<short>(MainMenuOption::Exit), const_cast<char *>(EXIT_TITLE), NULL},
        {0, 0, NULL, NULL}};

    OpenMenu(mainMenu, MENU_POSITION, m_menuRect.x, m_menuRect.y, p_handler);
}
