//------------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "eventHandler.h"
#include "menuHandler.h"
#include "listView.h"
#include "util.h"

#include <string>
#include <memory>

using std::string;

std::unique_ptr<EventHandler> EventHandler::_eventHandlerStatic;

EventHandler::EventHandler()
{
    //create a event to create handlers
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
    }

    _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));

    FullUpdate();
}

int EventHandler::eventDistributor(const int type, const int par1, const int par2)
{
    if (ISPOINTEREVENT(type))
        return EventHandler::pointerHandler(type, par1, par2);

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
                Message(ICON_WARNING, "Warning", "Could not connect to the internet.", 600);
            }
        }
        else
        {
            _nextcloud.switchWorkOffline();
        }

        break;
    }
    //Logout
    case 102:
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
        _listView.reset();
        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));
        FullUpdate();
        break;
    }
    //Exit
    case 103:
        CloseApp();
        break;
    default:
        break;
    }
}

int EventHandler::pointerHandler(const int type, const int par1, const int par2)
{
    if (type == EVT_POINTERDOWN)
    {
        if (IsInRect(par1, par2, _menu.getMenuButtonRect()) == 1)
        {
            return _menu.createMenu(_nextcloud.isLoggedIn(), _nextcloud.isWorkOffline(), EventHandler::mainMenuHandlerStatic);
        }
        else if (_listView != nullptr)
        {
            int itemID = _listView->listClicked(par1, par2);
            if (itemID != -1)
            {
                if (_nextcloud.getItems()->at(itemID).getType() == Itemtype::IFOLDER)
                {
                    FillAreaRect(_menu.getContentRect(), WHITE);
                    _menu.drawLoadingScreen();

                    string tempPath = _nextcloud.getItems()->at(itemID).getPath();

                    if (!tempPath.empty())
                        _nextcloud.getDataStructure(tempPath);

                    _listView.reset(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
                    _listView->drawHeader(tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));
                }
                else
                {
                    int dialogResult = 2;
                    if (_nextcloud.getItems()->at(itemID).isDownloaded())
                    {
                        dialogResult = DialogSynchro(ICON_QUESTION, "Action", "What do you want to do?", "Open", "Sync", "Remove");
                    }

                    switch (dialogResult)
                    {
                    case 1:
                        _nextcloud.getItems()->at(itemID).open();
                        break;
                    case 2:
                        OpenProgressbar(1, "Downloading...", "Check network connection", 0, EventHandler::DialogHandlerStatic);
                        _nextcloud.downloadItem(itemID);
                        CloseProgressbar();
                        break;
                    case 3:
                        if (!_nextcloud.removeFile(itemID))
                            Message(ICON_WARNING, "Warning", "Could not delete the file, please try again.", 600);
                        break;

                    default:
                        break;
                    }
                    _listView->drawEntry(itemID);
                }
            }

            PartialUpdate(_menu.getContentRect()->x, _menu.getContentRect()->y, _menu.getContentRect()->w, _menu.getContentRect()->h);

            return 1;
        }
        else if (_loginView != nullptr)
        {
            if (_loginView->logginClicked(par1, par2) == 2)
            {
                if (_nextcloud.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword()))
                {
                    _listView = std::unique_ptr<ListView>(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
                    _loginView.reset();
                    FullUpdate();
                }
                return 1;
            }
        }
    }
    return 0;
}

void EventHandler::DialogHandlerStatic(const int clicked)
{
    //TODO cannot interact with it
    //CloseProgressbar();
}
