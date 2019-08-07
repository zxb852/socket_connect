#pragma once
#define Windows
//#define Linux

#ifdef Windows
#define _WINSOCK_DEPRECATED_NO_WARNINGS 0
#include <stdio.h>
#include <Winsock2.h>
#include <opencv2/opencv.hpp>
#include <vector> 
#include <fstream>
#include <thread>
#include <mutex>
#include <map>
#include <queue>
#include <utility>
#include<memory>
#pragma comment(lib,"ws2_32.lib")

#define _close(x) closesocket(x)
#define _SOCKET SOCKET
#define _SOCKADDR_IN SOCKADDR_IN
#define _socklen_t int
#define _SOCKADDR SOCKADDR
#define _s_addr S_un.S_addr
#define _CSLEEP(n) Sleep(1000*n)

#endif

#ifdef Linux
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <vector> 
#include <fstream>
#include <pthread.h>
#include <mutex>
#include <map>
#include <queue>
#include <utility>
#include <memory>

#define _close(x) close(x)
#define _SOCKET int
#define _SOCKADDR_IN sockaddr_in
#define _socklen_t socklen_t
#define _SOCKADDR sockaddr
#define _s_addr s_addr
#define _CSLEEP(n) sleep(n)

#endif



//using namespace std;
using namespace cv;


extern std::mutex io_mutex;
extern std::vector<uchar> data_encode;
const static int LISTEN_LENGTH = 20;

void* server_dataexchange(void *soc);
void* server_listen(void *soc);
void* server_heart(void * soc);
void* listencallback_send(void *soc);
void* listencallback_recv(void *soc);
void* client_heart(void *soc);

void socketinit();
void socketclose();

struct sample
{
	int a=0;
	char b[21] = {0};
	double c=0;
	
	sample() = default;

	sample(int ia, std::string ib, double ic) :a(ia), c(ic)
	{
        int strlen = ib.length();
		for (int i = 0;i<std::min(strlen,20);i++)
            b[i] = ib[i];
	}
};

struct login_mes
{
	char username[21] = { 0 };
	char password[21] = { 0 };
	login_mes() = default;
	login_mes(std::string iusername, std::string ipassword)
	{
		int strlen = iusername.length();
		for (int i = 0;i<std::min(strlen, 20);i++)
			username[i] = iusername[i];
		strlen = ipassword.length();
		for (int i = 0;i<std::min(strlen, 20);i++)
			password[i] = ipassword[i];

	}
};

struct device_change
{

};

struct state_mes
{

};

class socket_connect
{
public:
    typedef int socket_id;
    typedef std::pair<int,int> data_head;
    socket_connect()            //手动创建socket主对象  or 创建客户端（相当于子对象）
	{
		mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		ismain = true;
		heart_flag = true;
		d_flag = std::make_shared<bool>(false);
	}
    virtual ~socket_connect()	//析构函数，关闭本socket
	{
		*d_flag = true;
		shutdown(mysocket, 2);
		_close(mysocket);
		for (auto i : children)
			delete i.second;
	}
	void server_init(const char *ip, int port);
	bool s_connect(const char *ip, int port);
    //断开连接,当对象为子对象时，断开与服务器的连接.
    void disconnect()
    {
        s_send_base(-1, nullptr, 0,0);
    }
    //登录,设定用户类型
    virtual int login(std::string user, std::string pass);

	//######外部接口#########
	//发送数据，将数据压入发送队列
	//当tid==emptyid时，由客户端向服务器发送；当tid==threadid或者tid==具体id时，由服务器向客户端发送。
	void send_buff_push(Mat image, int tid);
	void send_buff_push(sample src, int tid);
    void send_buff_push(login_mes src, int tid = 1);
	//接收数据，将数据弹出接收队列
	bool recv_buff_pop(Mat &output, int &tid);
	bool recv_buff_pop(sample &output, int &tid);
	//########################


protected:
    //发送缓冲区(主对象 or 子对象),	socketid==threadid时 向所有客户端发送数据， 否则向socketid标识的客户端或主控服务器发送数据
    std::queue<std::pair<data_head, Mat>> send_q_mat;
    std::queue<std::pair<data_head, sample>> send_q_sample;
    std::queue<std::pair<data_head, login_mes>> send_q_login_mes;

    //接收缓冲区(主对象 or 子对象)，socketid标识着从哪个子线程获得的数据
    std::queue <std::pair<data_head, Mat>> recv_q_mat;
    std::queue<std::pair<data_head, sample>> recv_q_sample;
    std::queue<std::pair<data_head, login_mes>> recv_q_login_mes;
private:

    // 在中转服务器中两个int：from first to two。 1：服务器 2：所有客户端广播 >10某个具体的连接对象
    int mysocket;
    socket_id children_id = 10;
	bool ismain;									// false:子对象（or客户端）  true:主对象
	bool heart_flag;								//心跳标志位，当==false时，代表对象已断开。
	std::shared_ptr<bool> d_flag;
	std::map<socket_id, socket_connect*> children;	// 记录socketid与子对象的对应关系
    std::map<socket_id, int> childrenctrl;			// 记录socketid对应子对象的用户类型，0: 初始化  1: 服务器 2: 管理员  3: 普通用户

    socket_connect(int s)	//自动创建socket子对象
	{
		mysocket = s;
		ismain = false;
		heart_flag = true;
		d_flag = std::make_shared<bool>(false);
	}

	//主对象控制方法，实现客户端与服务端连接，以及创建子对象（主对象）
	bool s_bind(const char *ip, int port);
	//尝试获取连接（accept）	用于监听连接请求线程
	void s_listen();
	//关闭child的Socket连接
	void deletechild(socket_id child)
	{
		for (auto i = children.begin();i != children.end();i++)
		{
			if (i->first == child)
			{
				delete i->second;
				children.erase(i);
				break;
			}
		}
		for (auto i = childrenctrl.begin();i != childrenctrl.end();i++)
		{
			if (i->first == child)
			{
				childrenctrl.erase(i);
				break;
			}
		}
	}

	//缓冲区管理方法(主对象)	用于数据交换线程
	void updatesendbuff();
	void updaterecvbuff();

	//尝试发送和接收数据		用于子对象线程
	void s_send();
	void s_recv();

	//具体通信方法，实现子对象与客户端通信
    void s_send_base(int datatype, const char *data, int size,int send_tag);
    template<class T> void s_senddata(T src, socket_id tid);
    template<class T> T s_recvdata(char *data, int length);

    friend void* server_dataexchange(void *soc);
    friend void* server_listen(void *soc);
    friend void* server_heart(void * soc);
    friend void* listencallback_send(void *soc);
    friend void* listencallback_recv(void *soc);
    friend void* client_heart(void *soc);
};

template<>
void socket_connect::s_senddata<Mat>(Mat src, socket_id tid);

template<>
Mat socket_connect::s_recvdata<Mat>(char *data, int length);

/*
	各数据类型以及对应标识
		标识	数据类型
		1		Mat
		2		sample

*/

/*
	堵塞：
		recv
		accept
		lock
*/
