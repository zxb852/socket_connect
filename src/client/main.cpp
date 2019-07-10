#include "client.h"

int main()
{
	VideoCapture cap;
	cap.open(0); //´ò¿ªÉãÏñÍ·

	Mat frame;
	socketinit();
	client s_client;
	s_client.s_connect("127.0.0.2", 8010);

	int loop = 0;
	sample test(1, "asdf", 1.5);
	std::cout << "size" << sizeof(test) << std::endl;

	s_client.send_buff_push(test, 1);

	s_client.send_buff_push(login_mes("client", "123"), 1);
	while (loop++ != 15)
	{
		//cap >> frame;
		//imshow("frame", frame);
		//resize(frame, frame, Size(160, 140));
		//s_client.send_buff_push(frame.clone(),1);
		//cvWaitKey(100);
	}
	getchar();
	return 0;
}