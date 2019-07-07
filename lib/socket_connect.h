#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 0
#define LISTEN_LENGTH 20
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

//using namespace std;
using namespace cv;
//using cv::Mat;

void server_dataexchange(void *soc);
void server_listen(void *soc);
void server_heart(void * soc);
void listencallback_send(void *soc);
void listencallback_recv(void *soc);
void client_heart(void *soc);

void socketinit();
void socketclose();

struct sample
{
	int a;
	std::string b;
	double c;
};

struct login_mes
{

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
	typedef std::thread::id socketid;

	SOCKET mysocket;
	socketid threadid;

	socket_connect()			//手动创建socket主对象  or 创建客户端（相当于子对象）
	{
		threadid = std::this_thread::get_id();
		mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		ismain = true;
		heart_flag = true;
		d_flag = std::make_shared<bool>(false);
	}
	~socket_connect()			//析构函数，关闭本socket
	{
		*d_flag = true;
		shutdown(mysocket, 2);
		closesocket(mysocket);
		for (auto i : children)
			delete i.second;
	}
	void server_init(const char *ip, int port);
	bool s_connect(const char *ip, int port);

	//######外部接口#########
	//发送数据，将数据压入发送队列
	void send_buff_push(socketid tid, Mat image);
	void send_buff_push(socketid tid, sample src);
	//接收数据，将数据弹出接收队列
	bool recv_buff_pop(socketid &tid, Mat &output);
	bool recv_buff_pop(socketid &tid, sample &output);
	//########################
	//关闭child的Socket连接
	void deletechild(socketid child)
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
	//当对象为子对象时，断开与服务器的连接
	void disconnect()
	{
		s_send(-1, nullptr, 0);
	}


private:
	bool ismain;									// false:子对象（or客户端）  true:主对象
	bool heart_flag;								//心跳标志位，当==false时，代表对象已断开。
	std::shared_ptr<bool> d_flag;
	std::map<socketid, socket_connect*> children;	// 记录socketid与子对象的对应关系
	std::map<socketid, int> childrenctrl;			// 记录socketid对应子对象的用户类型，0: 初始化  1: 管理员  2: 普通用户

	//发送缓冲区(主对象 or 子对象),	socketid==threadid时 向所有客户端发送数据， 否则向socketid标识的客户端或主控服务器发送数据
	std::queue<std::pair<socketid, Mat>> send_q_mat;
	std::queue<std::pair<socketid, sample>> send_q_sample;

	//接收缓冲区(主对象 or 子对象)，socketid标识着从哪个子线程获得的数据
	std::queue <std::pair<socketid, Mat>> recv_q_mat;
	std::queue<std::pair<socketid, sample>> recv_q_sample;

	socket_connect(SOCKET s)	//自动创建socket子对象
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

	//缓冲区管理方法(主对象)	用于数据交换线程
	void updatesendbuff();
	void updaterecvbuff();

	//尝试发送和接收数据		用于子对象线程
	void s_send();
	void s_recv();

	//具体通信方法，实现子对象与客户端通信
	void s_send(int datatype, const char *data, int size);
	void s_send(Mat image);
	template<class T> void s_send(T src)
	{
		//mode=-2心跳包 mode=-1断开连接  mode=1Mat mode=2
		int mode = 0;
		char data[10000];

		memset(data, 0, sizeof(data));				// 对该内存段进行清
		memcpy(data, &src, sizeof(src));			// 把这个结构体中的信息从内存中读入到字符串data中
													//接下来传送temp这个字符串就可以了
		s_send(mode, data, sizeof(src));
	}

	Mat s_recvmat(char *data, int length);
	sample s_recvsample(char *data, int length);

	friend void server_dataexchange(void *soc);
	friend void server_listen(void *soc);
	friend void server_heart(void * soc);
	friend void listencallback_send(void *soc);
	friend void listencallback_recv(void *soc);
	friend void client_heart(void *soc);
};



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