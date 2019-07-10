#include "relay_server.h"

using namespace std;
using namespace cv;

int main()
{
	socketinit();
	relay_server s_server;
	s_server.server_init("127.0.0.2", 8010);
	int tid;
	Mat src;
	sample test_sample;
	while (1)
	{
		if (s_server.recv_buff_pop(src, tid))
		{
			std::cout << "from " << tid << std::endl;
			imshow("src", src);
			cvWaitKey(1);
		}
		if (s_server.recv_buff_pop(test_sample, tid))
		{
			cout << test_sample.a << endl;
			cout << test_sample.c << endl;
			cout << test_sample.b << endl;
		}
	}
	getchar();
	socketclose();
	return 0;
}