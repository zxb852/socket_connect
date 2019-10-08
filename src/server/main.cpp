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
    s_server.setbasefile("data/");

//    vector<string> vstr;
//    s_server.datalist(vstr);

//    state_mes mes;
//    mes.settime_now();
//    mes.mode=1;
//    Mat test=imread("test.jpg");
//    s_server.send_decinf(3,mes,test,test,test,"good2.mp4");


    sleep(100);
    s_server.disconnect();
    socketclose();
    getchar();


    return 0;
}
