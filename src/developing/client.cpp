#include "socket_connect.h"

int main()
{
	VideoCapture cap;
	cap.open(0); //´ò¿ªÉãÏñÍ·

	Mat frame;
	socketinit();
	socket_connect s_client;
	s_client.s_connect("127.0.0.2", 8010);

	int loop = 0;
	sample test ;
	test.a = 1;
	test.c = 1.5;
	test.b = "zxb send";
	s_client.send_buff_push(test, 1);
	s_client.send_buff_push(login_mes("client", "123"),1);
	while (loop++!=15)
	{
		//cap >> frame;
		//imshow("frame", frame);
		//resize(frame, frame, Size(160, 140));
		//s_client.send_buff_push(frame.clone(),1);
		//cvWaitKey(100);
	}
	getchar();
	s_client.disconnect();
	socketclose();
	return 0;
}