//------------------------------------------------------------------
// loginView.h
//
// Author:           JuanJakobo
// Date:             26.08.2020
// Description:
//-------------------------------------------------------------------

#ifndef LOGIN_SCREEN
#define LOGIN_SCREEN

#include "inkview.h"

#include <string>

using std::string;

const int MAX_CHAR_BUFF_LENGHT = 256;

class LoginView
{
public:
    LoginView(irect *contentRect);
    ~LoginView();

    void drawLoginView();
    int logginClicked(int x, int y);

    string getUsername() { return _username; };
    string getPassword() { return _password; };
    string getURL() { return _url; };

private:
    static LoginView *_loginViewStatic;
    ifont *_loginFont;
    irect *_contentRect;
    irect _urlButton;
    irect _loginButton;
    irect _usernameButton;
    irect _passwordButton;
    int _test;
    string _username;
    string _password;
    string _url;
    char *_charBuffer;

    /**
        * Functions needed to call C function, handles the panel
        * 
        * @return void
        */
    static void keyboardHandlerStatic(char *text);

    void keyboardHandler(char *text);
};

#endif