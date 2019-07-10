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
	// ����ת������������int��from first to two�� 1�������� 2�����пͻ��˹㲥 >10ĳ����������Ӷ���
	typedef std::pair<int,int> data_head;

	SOCKET mysocket;
	socket_id children_id = 10;
	//socketid threadid;

	socket_connect()			//�ֶ�����socket������  or �����ͻ��ˣ��൱���Ӷ���
	{
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
	//��tid==emptyidʱ���ɿͻ�������������ͣ���tid==threadid����tid==����idʱ���ɷ�������ͻ��˷��͡�
	void send_buff_push(Mat image, int tid);
	void send_buff_push(sample src, int tid);
	void send_buff_push(login_mes src, int tid);
	//�������ݣ������ݵ������ն���
	bool recv_buff_pop(Mat &output, int &tid);
	bool recv_buff_pop(sample &output, int &tid);
	//########################
	//������Ϊ�Ӷ���ʱ���Ͽ��������������
	void disconnect()
	{
		s_send_base(-1, nullptr, 0,0);
	}
	int login(std::string user, std::string pass)
	{
		if (user == "server"&&pass == "123456")
			return 1;
		if (user == "admin"&&pass == "654321")
			return 2;
		if (user == "client"&&pass == "123")
			return 3;
	}


private:
	bool ismain;									// false:�Ӷ���or�ͻ��ˣ�  true:������
	bool heart_flag;								//������־λ����==falseʱ����������ѶϿ���
	std::shared_ptr<bool> d_flag;
	std::map<socket_id, socket_connect*> children;	// ��¼socketid���Ӷ���Ķ�Ӧ��ϵ
	std::map<socket_id, int> childrenctrl;			// ��¼socketid��Ӧ�Ӷ�����û����ͣ�0: ��ʼ��  1: ������ 2: ����Ա  3: ��ͨ�û�

	//���ͻ�����(������ or �Ӷ���),	socketid==threadidʱ �����пͻ��˷������ݣ� ������socketid��ʶ�Ŀͻ��˻����ط�������������
	std::queue<std::pair<data_head, Mat>> send_q_mat;
	std::queue<std::pair<data_head, sample>> send_q_sample;
	std::queue<std::pair<data_head, login_mes>> send_q_login_mes;

	//���ջ�����(������ or �Ӷ���)��socketid��ʶ�Ŵ��ĸ����̻߳�õ�����
	std::queue <std::pair<data_head, Mat>> recv_q_mat;
	std::queue<std::pair<data_head, sample>> recv_q_sample;
	std::queue<std::pair<data_head, login_mes>> recv_q_login_mes;

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
	//�ر�child��Socket����
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

	//������������(������)	�������ݽ����߳�
	void updatesendbuff();
	void updaterecvbuff();

	//���Է��ͺͽ�������		�����Ӷ����߳�
	void s_send();
	void s_recv();

	//����ͨ�ŷ�����ʵ���Ӷ�����ͻ���ͨ��
	void s_send_base(int datatype, const char *data, int size,int send_tag);
	void s_sendmat(Mat image, socket_id tid);
	template<class T> void s_senddata(T src, socket_id tid)
	{
		//mode=-2:������ mode=-1:�Ͽ�����  mode=1:Mat mode=2:sample mode=3:login_mes
		int mode = 0;
		//std::cout << typeid(T).name() << std::endl;
		//std::cout << typeid(sample).name() << std::endl;
		if (typeid(T) == typeid(sample))
			mode = 2;
		else if (typeid(T) == typeid(login_mes))
			mode = 3;
		
		char data[10000];

		memset(data, 0, sizeof(data));				// �Ը��ڴ�ν�����
		memcpy(data, &src, sizeof(src));			// ������ṹ���е���Ϣ���ڴ��ж��뵽�ַ���data��
													//����������temp����ַ����Ϳ�����
		s_send_base(mode, data, sizeof(src), tid);
	}

	Mat s_recvmat(char *data, int length);
	template<class T> T s_recvdata(char *data, int length)
	{
		T result;
		T *p = (T *)malloc(length);
		memcpy(p, data, length);
		result = *p;
		free(p);
		return result;
	}

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