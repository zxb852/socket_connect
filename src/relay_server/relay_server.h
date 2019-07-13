#pragma once
#include "socket_connect.h"

class relay_server: public socket_connect
{
public:
	relay_server()
	{
		socketinit();
	}
	~relay_server()
	{
		socketclose();
	}

    void relaydata();
    int login(std::string user, std::string pass);
};
