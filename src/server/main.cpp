#include "server.h"

using namespace std;
using namespace cv;

int main()
{
	socketinit();
    server s_server;
    //s_server.s_connect("39.108.229.151", 8010);
    s_server.s_connect("127.0.0.1", 8010);
    s_server.send_buff_push(login_mes("server", "123456"));

    VideoCapture cap;
    cap.open(0);
    Mat frame;

    int loop = 0;
    while (loop++ != 300)
    {
        cap >> frame;
        resize(frame, frame, Size(160, 140));
        s_server.send_buff_push(frame.clone(),2);
        cv::waitKey(100);
    }

	socketclose();
    getchar();
    
    return 0;
}
