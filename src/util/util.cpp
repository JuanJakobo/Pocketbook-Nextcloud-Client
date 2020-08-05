//------------------------------------------------------------------
// util.cpp
//
// Author:           JuanJakobo          
// Date:             04.08.2020
//   
//-------------------------------------------------------------------
#include "util.h"
#include "inkview.h"

#include <string>
#include <sstream>

string Util::intToString(int value)
{
    std::ostringstream stm;
    stm << value ;
    return stm.str();
}

size_t Util::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool Util::connectToNetwork()
{
	iv_netinfo *netinfo = NetInfo();
	if (netinfo->connected) 
		return true;

	const char *network_name = nullptr;
	int result = NetConnect2(network_name, 1);
	if (result != 0) {
        Message(3,"Warning","cannot connect to Internet.",200);
		return false;
	}

	netinfo = NetInfo();
	if (netinfo->connected) 
		return true;

    Message(3,"Warning","cannot connect to Internet.",200);
	return false;
}