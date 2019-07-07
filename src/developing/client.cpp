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
	while (loop++!=100)
	{
		cap >> frame;
		//imshow("frame", frame);
		resize(frame, frame, Size(160, 140));
		s_client.send_buff_push(s_client.threadid,frame.clone());
		//cvWaitKey(100);
	}
	s_client.disconnect();
	getchar();
	socketclose();
	return 0;
}