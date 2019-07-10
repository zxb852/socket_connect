#pragma once
#include "socket_connect.h"

class server : public socket_connect
{
public:
	server()
    {
		socketinit();
	}
	~server()
	{
		disconnect();
		socketclose();
	}

};
