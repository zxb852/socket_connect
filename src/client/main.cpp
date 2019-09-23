#include "client.h"

using namespace std;
using namespace cv;

int main()
{
	socketinit();
    client s_client;
    //s_client.s_connect("39.108.229.151", 8010);
    s_client.s_connect("127.0.0.1", 8010);
    s_client.send_buff_push(login_mes("client", "123"));
    s_client.setbasefile("/home/zxb/SRC_C/client_data/");

    s_client.updata_alarm_data();

//    while(true)
//    {
//        state_mes mes;
//        char tid;
//        if(s_client.recv_buff_pop(mes,tid))
//        {
//            cout<<"recv from "<<(int)tid<<endl;
//            cout<<mes.tostring()<<endl;
//            cout <<endl;
//        }
//    }

//    char tid;
//    Mat src;

//    while (1)
//    {
//        if (s_client.recv_buff_pop(src, tid))
//        {
//            std::cout << "mat mode: " << tid << std::endl;
//            //imshow("src", src);
//            cv::waitKey(1);
//        }
//    }
	socketclose();
	getchar();
	return 0;
}
