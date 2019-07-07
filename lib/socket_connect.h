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

	socket_connect()			//�ֶ�����socket������  or �����ͻ��ˣ��൱���Ӷ���
	{
		threadid = std::this_thread::get_id();
		mysocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		ismain = true;
		heart_flag = true;
		d_flag = std::make_shared<bool>(false);
	}
	~socket_connect()			//�����������رձ�socket
	{
		*d_flag = true;
		shutdown(mysocket, 2);
		closesocket(mysocket);
		for (auto i : children)
			delete i.second;
	}
	void server_init(const char *ip, int port);
	bool s_connect(const char *ip, int port);

	//######�ⲿ�ӿ�#########
	//�������ݣ�������ѹ�뷢�Ͷ���
	void send_buff_push(socketid tid, Mat image);
	void send_buff_push(socketid tid, sample src);
	//�������ݣ������ݵ������ն���
	bool recv_buff_pop(socketid &tid, Mat &output);
	bool recv_buff_pop(socketid &tid, sample &output);
	//########################
	//�ر�child��Socket����
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
	//������Ϊ�Ӷ���ʱ���Ͽ��������������
	void disconnect()
	{
		s_send(-1, nullptr, 0);
	}


private:
	bool ismain;									// false:�Ӷ���or�ͻ��ˣ�  true:������
	bool heart_flag;								//������־λ����==falseʱ����������ѶϿ���
	std::shared_ptr<bool> d_flag;
	std::map<socketid, socket_connect*> children;	// ��¼socketid���Ӷ���Ķ�Ӧ��ϵ
	std::map<socketid, int> childrenctrl;			// ��¼socketid��Ӧ�Ӷ�����û����ͣ�0: ��ʼ��  1: ����Ա  2: ��ͨ�û�

	//���ͻ�����(������ or �Ӷ���),	socketid==threadidʱ �����пͻ��˷������ݣ� ������socketid��ʶ�Ŀͻ��˻����ط�������������
	std::queue<std::pair<socketid, Mat>> send_q_mat;
	std::queue<std::pair<socketid, sample>> send_q_sample;

	//���ջ�����(������ or �Ӷ���)��socketid��ʶ�Ŵ��ĸ����̻߳�õ�����
	std::queue <std::pair<socketid, Mat>> recv_q_mat;
	std::queue<std::pair<socketid, sample>> recv_q_sample;

	socket_connect(SOCKET s)	//�Զ�����socket�Ӷ���
	{
		mysocket = s;
		ismain = false;
		heart_flag = true;
		d_flag = std::make_shared<bool>(false);
	}

	//��������Ʒ�����ʵ�ֿͻ������������ӣ��Լ������Ӷ���������
	bool s_bind(const char *ip, int port);
	//���Ի�ȡ���ӣ�accept��	���ڼ������������߳�
	void s_listen();

	//������������(������)	�������ݽ����߳�
	void updatesendbuff();
	void updaterecvbuff();

	//���Է��ͺͽ�������		�����Ӷ����߳�
	void s_send();
	void s_recv();

	//����ͨ�ŷ�����ʵ���Ӷ�����ͻ���ͨ��
	void s_send(int datatype, const char *data, int size);
	void s_send(Mat image);
	template<class T> void s_send(T src)
	{
		//mode=-2������ mode=-1�Ͽ�����  mode=1Mat mode=2
		int mode = 0;
		char data[10000];

		memset(data, 0, sizeof(data));				// �Ը��ڴ�ν�����
		memcpy(data, &src, sizeof(src));			// ������ṹ���е���Ϣ���ڴ��ж��뵽�ַ���data��
													//����������temp����ַ����Ϳ�����
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
	�����������Լ���Ӧ��ʶ
		��ʶ	��������
		1		Mat
		2		sample

*/

/*
	������
		recv
		accept
		lock
*/