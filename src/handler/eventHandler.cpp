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
        _menu.drawLoadingScreen();
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
        _listView.reset();
        _loginView = std::unique_ptr<LoginView>(new LoginView(_menu.getContentRect()));
        FullUpdate();
        break;
    }
    //Info
    case 104:
    {
        DialogSynchro(ICON_INFORMATION, "Information", "Version 0.5 \n For support please open a ticket at https://github.com/JuanJakobo/Pocketbook-Nextcloud-Client/issues", "", "", "Close");
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
                if (_nextcloud.getItems().at(itemID).getType() == Itemtype::IFOLDER)
                {
                    //TODO temp solution --> remove solution
                    //TODO if is the first option, go back and dont ask for sync
                    int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "What do you want to to do?", "Open folder", "Sync Folder", "Cancel");
                    switch (dialogResult)
                    {
                    case 1:
                        FillAreaRect(_menu.getContentRect(), WHITE);
                        _menu.drawLoadingScreen();

                        _tempPath = _nextcloud.getItems().at(itemID).getPath();
                        if (!_tempPath.empty())
                            _nextcloud.setItems(_nextcloud.getDataStructure(_tempPath));
                        _listView.reset(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
                        _listView->drawHeader(_tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));

                        break;
                    case 2:
                        //Sync folder
                        _nextcloud.downloadFolder(_nextcloud.getItems(), itemID);
                        //update the entry and say --> folder is synced 
                        //entries in visual and in nextlcoud are out of sync
                    default:
                        break;
                    }
                }
                else
                {

                    int dialogResult = 0;
                    if (_nextcloud.getItems().at(itemID).getState() != FileState::ICLOUD)
                    {
                        dialogResult = DialogSynchro(ICON_QUESTION, "Action", "What do you want to do?", "Open", "Remove", "Cancel");
                    }

                    switch (dialogResult)
                    {
                    case 1:
                        _nextcloud.getItems().at(itemID).open();
                        break;
                    case 2:
                        if (!_nextcloud.getItems().at(itemID).removeFile())
                            Message(ICON_WARNING, "Warning", "Could not delete the file, please try again.", 1200);
                        break;
                    case 3:
                        break;
                    default:
                        if (_nextcloud.isWorkOffline())
                        {
                            int dialogResult = DialogSynchro(ICON_QUESTION, "Action", "You are in offline modus. Go back online?", "Yes", "No", "Cancel");
                            if (dialogResult == 2 || dialogResult == 3)
                                return 0;
                            _nextcloud.switchWorkOffline();
                        }
                        OpenProgressbar(1, "Downloading...", "Check network connection", 0, EventHandler::DialogHandlerStatic);
                        _nextcloud.downloadItem(itemID);
                        CloseProgressbar();
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
                _menu.drawLoadingScreen();
                //TODO use progressbar and log (check what can go wrong?) catch? 
                if (_nextcloud.login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword()))
                {
                    _listView = std::unique_ptr<ListView>(new ListView(_menu.getContentRect(), _nextcloud.getItems()));
                    _loginView.reset();
                }
                else
                {
                    //redraw login screen so that loading disappears
                    Message(ICON_WARNING, "Warning", "Something went wrong...", 1200);
                }
                FullUpdate();
                return 1;
            }
        }
    }
    return 0;
}

void EventHandler::DialogHandlerStatic(const int clicked)
{
    //TODO cannot interact with it
    // make download in different thread https://github.com/pmartin/pocketbook-demo/blob/master/demo08-pthreads/demo08.cpp
    //CloseProgressbar();
}
