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

using std::string;

LoginView *LoginView::_loginViewStatic;

LoginView::LoginView(irect *contentRect) : _contentRect(contentRect)
{
    _loginViewStatic = this;
    _loginFont = OpenFont("LiberationMono", 40, 1);
}

LoginView::~LoginView()
{
    CloseFont(_loginFont);
}

void LoginView::drawLoginView()
{
    FillAreaRect(_contentRect, WHITE);

    _urlButton = iRect(50, 200, (ScreenWidth() - 50), 75, ALIGN_CENTER);
    DrawLine(20, 275, (ScreenWidth() - 20), 275, BLACK);
    SetFont(_loginFont, BLACK);
    DrawTextRect2(&_urlButton, "Url");

    _usernameButton = iRect(50, 400, ScreenWidth() - 50, 75, ALIGN_CENTER);
    DrawLine(20, 475, (ScreenWidth() - 20), 475, BLACK);
    SetFont(_loginFont, BLACK);
    DrawTextRect2(&_usernameButton, "Username");

    _passwordButton = iRect(50, 600, (ScreenWidth() - 50), 75, ALIGN_CENTER);
    DrawLine(20, 675, (ScreenWidth() - 20), 675, BLACK);
    SetFont(_loginFont, BLACK);
    DrawTextRect2(&_passwordButton, "Password");

    _loginButton = iRect(ScreenWidth() / 2 - (200 / 2), 700, 200, 50, ALIGN_CENTER);

    FillAreaRect(&_loginButton, BLACK);
    SetFont(_loginFont, WHITE);
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

    if (_test == 1)
    {
        _url = s.c_str();
        FillAreaRect(&_urlButton, WHITE);
        DrawTextRect2(&_urlButton, s.c_str());
    }
    else if (_test == 2)
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

    _charBuffer = NULL;
}

int LoginView::logginClicked(int x, int y)
{
    _charBuffer = new char[4 * MAX_CHAR_BUFF_LENGHT + 1];

    if (IsInRect(x, y, &_urlButton))
    {
        _test = 1;
        OpenKeyboard("Url", _charBuffer, MAX_CHAR_BUFF_LENGHT - 1, KBD_NORMAL, &keyboardHandlerStatic);
        return 1;
    }

    else if (IsInRect(x, y, &_usernameButton))
    {
        _test = 2;
        OpenKeyboard("Username", _charBuffer, MAX_CHAR_BUFF_LENGHT - 1, KBD_NORMAL, &keyboardHandlerStatic);
        return 1;
    }
    else if (IsInRect(x, y, &_passwordButton))
    {
        _test = 3;
        OpenKeyboard("Password", _charBuffer, MAX_CHAR_BUFF_LENGHT - 1, KBD_PASSWORD, &keyboardHandlerStatic);

        return 1;
    }
    else if (IsInRect(x, y, &_loginButton))
    {
        if (_username.empty() || _password.empty() || _url.empty())
        {
            Message(ICON_ERROR, "Error", "Please set url, username and password.", 600);
            return 1;
        }

        return 2;
    }

    return 0;
}