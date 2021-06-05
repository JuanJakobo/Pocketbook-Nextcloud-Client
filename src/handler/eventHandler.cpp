//------------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "eventHandler.h"
#include "mainMenu.h"
#include "contextMenu.h"
#include "listView.h"
#include "util.h"
#include "log.h"

#include <string>
#include <memory>

using std::string;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;

EventHandler::EventHandler()
{
    //create an copy of the eventhandler to handle methods that require static functions
    _eventHandlerStatic = std::unique_ptr<EventHandler>(this);

    _loginView = nullptr;
    _listView = nullptr;

    if (iv_access(NEXTCLOUD_CONFIG_PATH.c_str(), W_OK) == 0)
    {
        if (_nextcloud.login())
        {
            _listView = std::unique_ptr<ListView>(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
            FullUpdate();
            return;
        }
        else
        {
            Message(ICON_ERROR, "Error", "Could not login, please try again.", 1200);
            _nextcloud.logout();
        }
    }

    _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));

    FullUpdate();
}

int EventHandler::eventDistributor(const int type, const int par1, const int par2)
{
    if (ISPOINTEREVENT(type))
        return EventHandler::pointerHandler(type, par1, par2);
    else if (ISKEYEVENT(type))
        return EventHandler::keyHandler(type, par1, par2);

    return 0;
}

void EventHandler::mainMenuHandlerStatic(const int index)
{
    _eventHandlerStatic->mainMenuHandler(index);
}

void EventHandler::mainMenuHandler(const int index)
{
    switch (index)
    {
    //offlineModus
    case 101:
    {
        if (_nextcloud.isWorkOffline())
        {
            if (Util::connectToNetwork())
            {
                _nextcloud.switchWorkOffline();
            }
            else
            {
                Message(ICON_WARNING, "Warning", "Could not connect to the internet.", 1200);
            }
        }
        else
        {
            _nextcloud.switchWorkOffline();
        }

        break;
    }
    //Make startfolder
    case 102:
    {
        _nextcloud.setStartFolder(_tempPath);
        Message(ICON_INFORMATION, "Info", ("On the next startup the folder" + _tempPath + " will be shown.").c_str(), 1200);

        break;
    }
    //Logout
    case 103:
    {
        int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "Do you want to delete local files?", "Yes", "No", "Cancel");
        switch (dialogResult)
        {
        case 1:
            _nextcloud.logout(true);
            break;
        case 3:
            return;
        default:
            _nextcloud.logout();
            break;
        }
        _listView.release();
        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));
        FullUpdate();
        break;
    }
    //Info
    case 104:
    {
        Message(ICON_INFORMATION, "Warning", "Version 0.62 \n For support please open a ticket at https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client/issues", 1200);
        break;
    }
    //Exit
    case 105:
        CloseApp();
        break;
    default:
        break;
    }
}

void EventHandler::contextMenuHandlerStatic(const int index)
{
    _eventHandlerStatic->contextMenuHandler(index);
}
std::unique_ptr<ContextMenu> _contextMenu;

void EventHandler::contextMenuHandler(const int index)
{
    //invert color
    switch (index)
    {
    //Open
    case 101:
    {
        if (_nextcloud.getItems().at(_tempItemID).getType() == Itemtype::IFOLDER)
        {
            openFolder();
        }
        else
        {
            openItem();
        }

        break;
    }
    //Sync
    case 102:
    {
        startDownload();
        break;
    }
    //remove
    case 103:
    {
        OpenProgressbar(1, "Removing...", "Removing Files.", 0, EventHandler::DialogHandlerStatic);
        if (_nextcloud.removeItem(_tempItemID))
        {
            _listView->drawEntry(_tempItemID);
            Util::updatePBLibrary();
        }
        else
        {
            Message(ICON_WARNING, "Warning", "Could not delete the file, please try again.", 1200);
        }
        CloseProgressbar();
        break;
    }
    default:
    {
        _listView->invertEntryColor(_tempItemID);
        break;
    }

        _contextMenu.reset();
    }
}

int EventHandler::pointerHandler(const int type, const int par1, const int par2)
{
    //long press to open up context menu
    if (type == EVT_POINTERLONG)
    {
        if (_listView != nullptr)
        {
            _tempItemID = _listView->listClicked(par1, par2);
            _listView->invertEntryColor(_tempItemID);
            if (_tempItemID != -1)
            {
                if (_nextcloud.getItems().at(_tempItemID).getTitle().compare("...") != 0)
                {
                    _contextMenu = std::unique_ptr<ContextMenu>(new ContextMenu());
                    _contextMenu->createMenu(par2, _nextcloud.getItems().at(_tempItemID).getState(), EventHandler::contextMenuHandlerStatic);
                }
            }
        }
    }
    else if (type == EVT_POINTERUP)
    {
        //menu is clicked
        if (IsInRect(par1, par2, _menu.getMenuButtonRect()) == 1)
        {
            return _menu.createMenu(_nextcloud.isLoggedIn(), _nextcloud.isWorkOffline(), EventHandler::mainMenuHandlerStatic);
        }
        //if listView is shown
        else if (_listView != nullptr)
        {
            _tempItemID = _listView->listClicked(par1, par2);
            _listView->invertEntryColor(_tempItemID);
            if (_tempItemID != -1)
            {
                if (_nextcloud.getItems().at(_tempItemID).getType() == Itemtype::IFOLDER)
                {
                    openFolder();
                }
                else
                {
                    if (_nextcloud.getItems().at(_tempItemID).getState() == FileState::ISYNCED || (_nextcloud.isWorkOffline() && _nextcloud.getItems().at(_tempItemID).getState() == FileState::IOUTSYNCED))
                    {
                        openItem();
                    }
                    else
                    {
                        startDownload();
                    }
                }
            }

            return 1;
        }
        //if loginView is shown
        else if (_loginView != nullptr)
        {
            if (_loginView->logginClicked(par1, par2) == 2)
            {
                ShowHourglassForce();

                if (_nextcloud.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword()))
                {
                    _listView = std::unique_ptr<ListView>(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
                    _loginView.reset();

                    FullUpdate();
                }
                else
                {
                    HideHourglass();
                    Log::writeLog("login failed.");
                }
                return 1;
            }
        }
    }
    return 0;
}

void EventHandler::startDownload()
{
    if (_nextcloud.isWorkOffline())
    {
        int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "You are in offline modus. Go back online?", "Yes", "No", "Cancel");
        if (dialogResult == 2 || dialogResult == 3)
            return; // 1;
        _nextcloud.switchWorkOffline();
    }
    OpenProgressbar(1, "Downloading...", "Check network connection", 0, EventHandler::DialogHandlerStatic);
    try
    {
        _nextcloud.download(_tempItemID);
    }
    catch (const std::exception &e)
    {
        Log::writeLog(e.what());
        Message(ICON_ERROR, "Error", "Something has gone wrong. Please check the logs. (/system/config/nextcloud/)", 1200);
    }
    Util::updatePBLibrary();

    CloseProgressbar();
    _listView->drawEntry(_tempItemID);
}

void EventHandler::openItem()
{
    _listView->invertEntryColor(_tempItemID);
    _nextcloud.getItems().at(_tempItemID).open();
}

void EventHandler::openFolder()
{
    FillAreaRect(_menu.getContentRect(), WHITE);
    ShowHourglassForce();

    _tempPath = _nextcloud.getItems().at(_tempItemID).getPath();
    if (!_tempPath.empty())
        _nextcloud.setItems(_nextcloud.getDataStructure(_tempPath));
    _listView.release();
    _listView = std::unique_ptr<ListView>(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
    _listView->drawHeader(_tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));
    PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);
}

int EventHandler::keyHandler(const int type, const int par1, const int par2)
{
    if (type == EVT_KEYPRESS)
    {
        //menu button
        if (par1 == 23)
        {
            _listView->firstPage();
        }
        else if (_listView != nullptr)
        {
            //left button
            if (par1 == 24)
            {
                _listView->prevPage();
            }
            //right button
            else if (par1 == 25)
            {
                _listView->nextPage();
            }
        }
        else
        {
            return 0;
        }
        return 1;
    }

    return 0;
}

void EventHandler::DialogHandlerStatic(int clicked)
{
    //TODO cannot interact with it
    // make download in different thread https://github.com/pmartin/pocketbook-demo/blob/master/demo08-pthreads/demo08.cpp
    CloseProgressbar();
}
