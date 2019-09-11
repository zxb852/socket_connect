#pragma once
#include "socket_connect.h"

class server : public socket_connect
{
public:
	server()
    {
	}
	~server()
	{
		disconnect();
	}

    void send_decinf(state_mes mes,Mat rgb,Mat ir, Mat uv, std::string vedio_name);

};
