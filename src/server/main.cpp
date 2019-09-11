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


    state_mes mes;
    mes.settime_now();
    Mat test=imread("test.jpg");
    s_server.send_decinf(mes,test,test,test,"good.avi");


	getchar();

    //VideoCapture cap;
    //cap.open(0);
    //Mat frame;
	
    //int loop = 0;
    //while (loop++ != 300)
    //{
    //    cap >> frame;
    //    resize(frame, frame, Size(160, 140));
    //    s_server.send_buff_push(frame.clone(),2);
    //    cv::waitKey(100);
    //}
    
	

	//ifstream infile;
	//ofstream outfile;

	//infile.open("good.avi", std::ios_base::binary);
	//outfile.open("test.avi", std::ios_base::binary);

	//char buf[1024];
	//

	//while (infile.read(buf, sizeof(buf)))
	//{
	//	cout << "a";
	//	outfile.write(buf, sizeof(buf));
	//}

	//infile.close();
	//outfile.close();

	//time_t timep;
	//time(&timep);
	//struct tm nowTime;
	//localtime_s(&nowTime, &timep);
	//cout << nowTime.tm_year+1900 << "  " << nowTime.tm_mon+1 << "  " << nowTime.tm_mday << "  " << nowTime.tm_hour << "  " << nowTime.tm_min << "  " << nowTime.tm_sec << endl;
   
	s_server.disconnect();
	socketclose();
	getchar();
    return 0;
}
