#include "relay_server.h"

using namespace std;
using namespace cv;

int main()
{
	relay_server s_server;
    s_server.server_init("172.17.31.243", 8010);

    while(1)
    {
        s_server.relaydata();
    }

	return 0;
}
