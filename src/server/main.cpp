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

    //s_server.setbasefile("/home/zxb/SRC_C/");
    state_mes mes;
    //获取时间
//    time_t timep;
//    time(&timep);
//    struct tm *nowTime =localtime(&timep);
//    nowTime->tm_year += 1900;
//    nowTime->tm_mon += 1;
//    mes.year=nowTime->tm_year;
//    mes.mon =nowTime->tm_mon;
//    mes.day=nowTime->tm_mday;
//    mes.hour=nowTime->tm_hour;
//    mes.min=nowTime->tm_min;
//    mes.sec=nowTime->tm_sec;

        mes.year=2019;
        mes.mon =9;
        mes.day=7;
        mes.hour=11;
        mes.min=18;
        mes.sec=52;

    //s_server.send_buff_push(mes, 1);
    //usleep(5000);
    //s_server.send_buff_push("good.avi", 1);

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
