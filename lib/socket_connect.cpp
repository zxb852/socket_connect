#include "socket_connect.h"
std::mutex io_mutex;
std::vector<uchar> data_encode;

void socketinit()
{
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 0), &wsd);
}

void socketclose()
{
	WSACleanup();
}

void socket_connect::updatesendbuff()
{
	io_mutex.lock();
	for (auto i : children)
	{
		if (childrenctrl[i.first] != 0)
		{
			socket_connect* childsocket = i.second;
			if (!send_q_mat.empty())
				if (send_q_mat.front().first == threadid || send_q_mat.front().first == i.first)
					childsocket->send_q_mat.push(send_q_mat.front());
			if (!send_q_sample.empty())
				if (send_q_sample.front().first == threadid || send_q_sample.front().first == i.first)
					childsocket->send_q_sample.push(send_q_sample.front());
		}
	}
	if(!send_q_mat.empty())
		send_q_mat.pop();
	if (!send_q_sample.empty())
		send_q_sample.pop();
	io_mutex.unlock();
}

void socket_connect::updaterecvbuff()
{
	io_mutex.lock();
	for (auto i : children)
	{
		socket_connect* childsocket = i.second;
		if (!childsocket->recv_q_mat.empty())
		{
			recv_q_mat.push(childsocket->recv_q_mat.front());
			childsocket->recv_q_mat.pop();
		}

		if (!childsocket->recv_q_sample.empty())
		{
			recv_q_sample.push(childsocket->recv_q_sample.front());
			childsocket->recv_q_sample.pop();
		}
	}
	io_mutex.unlock();
}

bool socket_connect::s_bind(const char *ip, int port)
{
	SOCKADDR_IN addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(ip);
	addrServer.sin_port = htons(port);
	bool result = bind(mysocket, (SOCKADDR*)&addrServer, sizeof(SOCKADDR)) == 0;
	if (listen(mysocket, LISTEN_LENGTH) == SOCKET_ERROR)
		std::cout << "listen error!" << std::endl;
	return result;
}

void socket_connect::s_listen()
{
	//循环接收客户端连接请求并创建服务线程
	SOCKET conn;
	SOCKADDR_IN addr;
	int len = sizeof(SOCKADDR);
	conn = accept(mysocket, (SOCKADDR*)&addr, &len);
	char recvBuf[10000];
	
	std::cout << "一个客户端已连接到服务器，socket是：" << conn << std::endl;
	socket_connect *socketctl=new socket_connect(conn);	

	std::thread t1(listencallback_send, socketctl);
	std::cout << "send thread id :" << t1.get_id() << std::endl;
	t1.detach();

	std::thread t2(listencallback_recv, socketctl);
	std::cout << "recv thread id :" << t2.get_id() << std::endl;
	t2.detach();

	socketctl->threadid = t1.get_id();
	children[t1.get_id()] = socketctl;
	childrenctrl[t1.get_id()] = 0;
}

void socket_connect::server_init(const char * ip, int port)
{
	if (s_bind(ip, port))
	{
		std::cout << "bind" << ip << ":" << port << "    successfully" << std::endl;

		std::thread tlisten(server_listen, this);
		std::cout << "creat listen thread " << tlisten.get_id()<<"    successfully" << std::endl;
		tlisten.detach();

		std::thread tdata(server_dataexchange, this);
		std::cout << "creat data thread "<< tdata.get_id()<<"         successfully" << std::endl;
		tdata.detach();

		std::thread theart(server_heart, this);
		std::cout << "creat heart thread " << theart.get_id() << "    successfully" << std::endl;
		theart.detach();
	}
}

bool socket_connect::s_connect(const char *ip, int port)
{
	SOCKADDR_IN addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = inet_addr(ip);
	addrServer.sin_port = htons(port);
	ismain = false;
	if (connect(mysocket, (SOCKADDR*)&addrServer, sizeof(SOCKADDR)) == -1)
		return false;

	std::thread t(listencallback_send, this);
	t.detach();
	std::thread t2(listencallback_recv, this);
	t2.detach();
	std::thread t3(client_heart, this);
	t3.detach();
	std::cout << "client data contrl thread id set up : " << t.get_id() << std::endl;
	
	return true;
}

void socket_connect::send_buff_push(socketid tid, Mat image)
{
	io_mutex.lock();
	send_q_mat.push(std::pair<socketid, Mat>(tid, image));
	io_mutex.unlock();
}

void socket_connect::send_buff_push(socketid tid, sample src)
{
	io_mutex.lock();
	send_q_sample.push(std::pair<socketid, sample>(tid, src));
	io_mutex.unlock();
}

bool socket_connect::recv_buff_pop(socketid &tid, Mat & output)
{
	io_mutex.lock();
	if (!recv_q_mat.empty())
	{
		auto &step = recv_q_mat.front();
		tid = step.first;
		output = step.second.clone();
		recv_q_mat.pop();
		io_mutex.unlock();
		return true;
	}
	io_mutex.unlock();
	return false;
}

bool socket_connect::recv_buff_pop(socketid &tid,  sample & output)
{
	io_mutex.lock();
	if (!recv_q_sample.empty())
	{
		auto &step = recv_q_sample.front();
		tid = step.first;
		output = step.second;
		recv_q_sample.pop();
		io_mutex.unlock();
		return true;
	}
	io_mutex.unlock();
	return false;
}

void socket_connect::s_send()
{
	io_mutex.lock();
	if(ismain)
		updatesendbuff();
	else
	{
		//s_send(0, nullptr, 0);				//发送心跳‘

		if (!send_q_mat.empty())
		{
			s_send(send_q_mat.front().second);
			send_q_mat.pop();
		}
		if (!send_q_sample.empty())
		{
			s_send(send_q_sample.front().second);
			send_q_sample.pop();
		}
	}
	io_mutex.unlock();
}

void socket_connect::s_send(int datatype, const char *data, int size)
{
	char send_char[100000] = { 0 };
	std::string type = std::to_string(datatype);
	std::string len = std::to_string(size);
	
	std::cout << data_encode.size() << std::endl;
	std::cout << len << std::endl;
	std::cout << len.length() << std::endl;

	for (int i = 0;i < type.length();++i)
		send_char[i] = type[i];
	for (int i = 16; i < 16 + len.length(); ++i)
		send_char[i] = len[i-16];
	for (int i = 32;i < size + 32;++i)
		send_char[i] = data[i - 32];

	send(mysocket, send_char, 32 + size, 0);
}

void socket_connect::s_send(Mat image)
{
	data_encode.clear();
	imencode(".jpg", image, data_encode);
	char data[100000]={0};
	for (int i = 0; i < data_encode.size(); i++)
		data[i] = data_encode[i];
	s_send(1, data, data_encode.size());
}

void socket_connect::s_recv()
{
	if (ismain)
	{
		io_mutex.lock();
		updaterecvbuff();
		io_mutex.unlock();
	}
	else
	{
		char mode[16];
		memset(mode, 0, sizeof(mode));
		if (recv(mysocket, mode, 16, 0))	//Linux下flag设置为MSG_DONTWAIT，将recv工作在非阻塞模式下
		{
			char length[16];
			char data[100000];
			memset(length, 0, sizeof(length));
			memset(data, 0, sizeof(data));

			recv(mysocket, length, 16, 0);
			int intmode = atoi(mode);
			int intlength = atoi(length);

			for (int i = 0;i < intlength;++i)
			{
				char recvBuf_1[1];
				recv(mysocket, recvBuf_1, 1, 0);
				data[i] = recvBuf_1[0];
			}
			io_mutex.lock();
			if (intmode != 0)
			{
				heart_flag = true;
				if (intmode == -1)
					*d_flag = true;
				else if (intmode == 1)
					recv_q_mat.push(std::pair<socketid, Mat>(threadid, s_recvmat(data, intlength)));
				else if (intmode == 2)
					recv_q_sample.push(std::pair<socketid, sample>(threadid, s_recvsample(data, intlength)));
			}
			io_mutex.unlock();
		}
	}
}

Mat socket_connect::s_recvmat(char * data, int length)
{
	//std::cout << "recv mat" << std::endl;
	Mat result;
	std::vector<uchar> vdata;
	for (int i = 0;i < length;++i)
		vdata.push_back(data[i]);
	result = cv::imdecode(vdata, CV_LOAD_IMAGE_COLOR);
	resize(result, result, cv::Size(640, 480));
	return result;
}

sample socket_connect::s_recvsample(char * data, int length)
{
	sample result;
	memcpy(&result, data, length);
	return result;
}

void server_dataexchange(void * soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		if (*d_flag == true)
			break;
		socket_ptr->updatesendbuff();
		socket_ptr->updaterecvbuff();
	}
}

void server_listen(void * soc)
{
	//开始线程，获得本线程socket指针。
	//std::cout << std::this_thread::get_id();
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		if (*d_flag == true)
			break;
		socket_ptr->s_listen();
	}
}

void server_heart(void * soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		Sleep(10000);
		if (*d_flag == true)
			break;
		io_mutex.lock();
		//std::cout << "only for test" << std::endl;
		for (auto i = socket_ptr->children.begin();i != socket_ptr->children.end();)
		{
			if (socket_ptr->children.empty())
				break;
			if (*(i->second->d_flag) == true || i->second->heart_flag == false)
			{
				std::cout << "thread bad" << std::endl;
				socket_ptr->deletechild(i->first);
			}
			else
			{
				std::cout << "thread ok" << std::endl;
				i->second->heart_flag = false;
				i++;
			}
			
		}
		io_mutex.unlock();
	}
}

void listencallback_send(void *soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;

	while (1)
	{
		//std::cout << "asaaaaaa" << std::endl;
		if (*d_flag == true)
			break;
		socket_ptr->s_send();
		

	}
}

void listencallback_recv(void *soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		//std::cout << "only for test" << std::endl;
		if (*d_flag == true)
			break;
		socket_ptr->s_recv();
		
	}
}

void client_heart(void *soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		//std::cout << "only for test" << std::endl;
		if (*d_flag == true)
			break;
		Sleep(5000);
		io_mutex.lock();
		//std::cout << "发了心跳" << std::endl;
		socket_ptr->s_send(-2, nullptr, 0);
		io_mutex.unlock();
	}
}
