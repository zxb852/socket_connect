#pragma once
#include "socket_connect.h"

class client :public socket_connect
{
public:
	client()
	{
		//socketinit();
	}
	~client()
	{
		disconnect();
		//socketclose();
	}
    void updata_alarm_data();

private:
    vector<string> checkdata();

};
