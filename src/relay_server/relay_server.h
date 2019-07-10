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
	int login(std::string user, std::string pass)
	{
		if (user == "server"&&pass == "123456")
			return 1;
		if (user == "admin"&&pass == "654321")
			return 2;
		if (user == "client"&&pass == "123")
			return 3;
	}
};