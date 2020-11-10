//------------------------------------------------------------------
// loginView.cpp
//
// Author:           JuanJakobo
// Date:             26.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "loginView.h"
#include "eventHandler.h"

#include <string>
#include <memory>

using std::string;

LoginView *LoginView::_loginViewStatic;

LoginView::LoginView(const irect *contentRect) : _contentRect(contentRect)
{
    _loginViewStatic = this;
    _loginFont = std::unique_ptr<ifont>(OpenFont("LiberationMono", 40, 1));

    SetFont(_loginFont.get(), BLACK);
    FillAreaRect(_contentRect, WHITE);

    _urlButton = iRect(50, 200, (ScreenWidth() - 100), 75, ALIGN_CENTER);

    DrawTextRect(_urlButton.x, _urlButton.y - 50, _urlButton.w, _urlButton.h, "Server address:", ALIGN_LEFT);
    DrawRect(_urlButton.x - 1, _urlButton.y - 1, _urlButton.w + 2, _urlButton.h + 2, BLACK);

    _usernameButton = iRect(50, 400, ScreenWidth() - 100, 75, ALIGN_CENTER);
    DrawTextRect(_usernameButton.x, _usernameButton.y - 50, _usernameButton.w, _usernameButton.h, "Username:", ALIGN_LEFT);
    DrawRect(_usernameButton.x - 1, _usernameButton.y - 1, _usernameButton.w + 2, _usernameButton.h + 2, BLACK);

    _passwordButton = iRect(50, 600, (ScreenWidth() - 100), 75, ALIGN_CENTER);
    DrawTextRect(_passwordButton.x, _passwordButton.y - 50, _passwordButton.w, _passwordButton.h, "Password:", ALIGN_LEFT);
    DrawRect(_passwordButton.x - 1, _passwordButton.y - 1, _passwordButton.w + 2, _passwordButton.h + 2, BLACK);

    _loginButton = iRect(50, 750, (ScreenWidth() - 100), 75, ALIGN_CENTER);

    FillAreaRect(&_loginButton, BLACK);
    SetFont(_loginFont.get(), WHITE);
    DrawTextRect2(&_loginButton, "Login");
}

void LoginView::keyboardHandlerStatic(char *text)
{
    _loginViewStatic->keyboardHandler(text);
}

void LoginView::keyboardHandler(char *text)
{
    if (!text)
        return;

    string s(text);
    if (s.empty())
        return;

    if (_keyboardValue == 1)
    {
        _url = s.c_str();
        FillAreaRect(&_urlButton, WHITE);
        DrawTextRect2(&_urlButton, s.c_str());
    }
    else if (_keyboardValue == 2)
    {
        _username = s.c_str();
        FillAreaRect(&_usernameButton, WHITE);
        DrawTextRect2(&_usernameButton, s.c_str());
    }
    else
    {
        _password = s.c_str();
        FillAreaRect(&_passwordButton, WHITE);

        string pass;
        for (auto i = 0; i < s.length(); i++)
        {
            pass += "*";
        }

        DrawTextRect2(&_passwordButton, pass.c_str());
    }

    free(_charBuffer);
}

int LoginView::logginClicked(int x, int y)
{
    _charBuffer = (char *)malloc(_bufferSize);

    if (IsInRect(x, y, &_urlButton))
    {
        _keyboardValue = 1;
        OpenKeyboard("Server address", _charBuffer, MAX_CHAR_BUFF_LENGHT - 1, KBD_NORMAL, &keyboardHandlerStatic);
        return 1;
    }

    else if (IsInRect(x, y, &_usernameButton))
    {
        _keyboardValue = 2;
        OpenKeyboard("Username", _charBuffer, MAX_CHAR_BUFF_LENGHT - 1, KBD_NORMAL, &keyboardHandlerStatic);
        return 1;
    }
    else if (IsInRect(x, y, &_passwordButton))
    {
        _keyboardValue = 3;
        OpenKeyboard("Password", _charBuffer, MAX_CHAR_BUFF_LENGHT - 1, KBD_PASSWORD, &keyboardHandlerStatic);

        return 1;
    }
    else if (IsInRect(x, y, &_loginButton))
    {
        if (_username.empty() || _password.empty() || _url.empty())
        {
            Message(ICON_ERROR, "Error", "Please set url, username and password.", 1200);
            return 1;
        }

        return 2;
    }

    return 0;
}