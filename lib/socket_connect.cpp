#include "socket_connect.h"
std::mutex io_mutex;
std::vector<uchar> data_encode;

void socketinit()
{
}

void socketclose()
{
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
			{
				if (childrenctrl[i.first] == 1 && send_q_mat.front().first.second== 1)
					childsocket->send_q_mat.push(send_q_mat.front());
				else if (send_q_mat.front().first.second == 2 || send_q_mat.front().first.second == i.first)
					childsocket->send_q_mat.push(send_q_mat.front());
			}
			if (!send_q_sample.empty())
			{
				if (childrenctrl[i.first] == 1 && send_q_sample.front().first.second == 1)
					childsocket->send_q_sample.push(send_q_sample.front());
				if (send_q_sample.front().first.second == 2 || send_q_sample.front().first.second == i.first)
					childsocket->send_q_sample.push(send_q_sample.front());
			}
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
			auto &step = childsocket->recv_q_mat.front();
			//step.first.first = i.first;
			//发送的标志位：1：RGB 2：红丸 3：紫外 4:融合 11：录像 12：截图
			step.first.first = step.first.second;
			recv_q_mat.push(step);
			childsocket->recv_q_mat.pop();
		}

		if (!childsocket->recv_q_sample.empty())
		{
			auto &step = childsocket->recv_q_sample.front();
			step.first.first = i.first;
			recv_q_sample.push(step);
			childsocket->recv_q_sample.pop();
		}
		if (!childsocket->recv_q_login_mes.empty())
		{
			auto &step = childsocket->recv_q_login_mes.front();
			std::cout << "child identity changed from:" << childrenctrl[i.first];
			childrenctrl[i.first] = login(step.second.username, step.second.password);
			step.second.username[0] = 'a';
			std::cout << "	to:" << childrenctrl[i.first]<<std::endl;
			childsocket->recv_q_login_mes.pop();
		}
	}
	io_mutex.unlock();
}

bool socket_connect::s_bind(const char *ip, int port)
{
    sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = inet_addr(ip);
	addrServer.sin_port = htons(port);
    bool result = bind(mysocket, (sockaddr*)&addrServer, sizeof(addrServer)) == 0;
    listen(mysocket, LISTEN_LENGTH);
	return result;
}

void socket_connect::s_listen()
{
	//循环接收客户端连接请求并创建服务线程
    int conn;
    int res;
    sockaddr_in addr;
    socklen_t len = sizeof(sockaddr);
    conn = accept(mysocket, (sockaddr*)&addr, &len);
	char recvBuf[10000];
	
	std::cout << "一个客户端已连接到服务器，socket是：" << conn << std::endl;
	socket_connect *socketctl=new socket_connect(conn);	

    pthread_t t1;
    res = pthread_create(&t1, NULL, listencallback_send, (void*)socketctl);
    std::cout << "send thread id :" << t1 << std::endl;

    pthread_t t2;
    res = pthread_create(&t2, NULL, listencallback_recv, (void*)socketctl);
    std::cout << "recv thread id :" << t2 << std::endl;

	children[children_id] = socketctl;
	childrenctrl[children_id] = 0;
	children_id++;
}

void socket_connect::server_init(const char * ip, int port)
{
    int res;
	if (s_bind(ip, port))
	{
		std::cout << "bind" << ip << ":" << port << "    successfully" << std::endl;

        pthread_t tlisten;
        res = pthread_create(&tlisten, NULL, server_listen, (void*)this);
        std::cout << "creat listen thread " << tlisten<<"    successfully" << std::endl;

        pthread_t tdata;
        res = pthread_create(&tdata, NULL, server_dataexchange, (void*)this);
        std::cout << "creat data thread "<< tdata<<"         successfully" << std::endl;

        pthread_t theart;
        res = pthread_create(&theart, NULL, server_heart, (void*)this);
        std::cout << "creat heart thread " << theart << "    successfully" << std::endl;
	}
}

bool socket_connect::s_connect(const char *ip, int port)
{
    int res;
    sockaddr_in addrServer;
	addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = inet_addr(ip);
	addrServer.sin_port = htons(port);
	ismain = false;
    if (connect(mysocket, (sockaddr*)&addrServer, sizeof(sockaddr)) == -1)
		return false;

    pthread_t t;
    res = pthread_create(&t, NULL, listencallback_send, (void*)this);
    pthread_t t2;
    res = pthread_create(&t2, NULL, listencallback_recv, (void*)this);
    pthread_t t3;
    res = pthread_create(&t3, NULL, client_heart, (void*)this);
    std::cout << "client data contrl thread id set up : " << t << std::endl;
	
	return true;
}

void socket_connect::send_buff_push(Mat image, int tid)
{
	io_mutex.lock();
	send_q_mat.push(std::pair<data_head, Mat>(data_head(tid, 0), image));
	io_mutex.unlock();
}

void socket_connect::send_buff_push(sample src, int tid)
{
	io_mutex.lock();
	send_q_sample.push(std::pair<data_head, sample>(data_head(tid, 0), src));
	io_mutex.unlock();
}

void socket_connect::send_buff_push(login_mes src, int tid)
{
	io_mutex.lock();
	send_q_login_mes.push(std::pair<data_head, login_mes>(data_head(tid, 0), src));
	io_mutex.unlock();
}

bool socket_connect::recv_buff_pop(Mat & output, int &tid)
{
	io_mutex.lock();
	if (!recv_q_mat.empty())
	{
		auto &step = recv_q_mat.front();
		tid = step.first.second;
		output = step.second.clone();
		recv_q_mat.pop();
		io_mutex.unlock();
		return true;
	}
	io_mutex.unlock();
	return false;
}

bool socket_connect::recv_buff_pop(sample & output, int &tid)
{
	io_mutex.lock();
	if (!recv_q_sample.empty())
	{
		auto &step = recv_q_sample.front();
		tid = step.first.second;
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
			s_sendmat(send_q_mat.front().second,send_q_mat.front().first.first);//发送数据部分以及head的第一位。
			send_q_mat.pop();
		}
		if (!send_q_sample.empty())
		{
			s_senddata(send_q_sample.front().second, send_q_sample.front().first.first);
			send_q_sample.pop();
		}
		if (!send_q_login_mes.empty())
		{
			s_senddata(send_q_login_mes.front().second, send_q_login_mes.front().first.first);
			send_q_login_mes.pop();
		}
	}
	io_mutex.unlock();
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
			char tag[16];
			char data[100000];
			memset(length, 0, sizeof(length));
			memset(tag, 0, sizeof(tag));
			memset(data, 0, sizeof(data));

			recv(mysocket, length, 16, 0);
			recv(mysocket, tag, 16, 0);
			int intmode = atoi(mode);
			int intlength = atoi(length);
			int inttag = atoi(tag);

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
					recv_q_mat.push(std::pair<data_head, Mat>(data_head(0, inttag), s_recvmat(data, intlength)));
				else if (intmode == 2)
					recv_q_sample.push(std::pair<data_head, sample>(data_head(0, inttag), s_recvdata<sample>(data, intlength)));
				else if (intmode == 3)
					recv_q_login_mes.push(std::pair<data_head, login_mes>(data_head(0, inttag), s_recvdata<login_mes>(data, intlength)));

			}
			io_mutex.unlock();
		}
	}
}

void socket_connect::s_send_base(int datatype, const char *data, int size,int send_tag)
{
	char send_char[100000] = { 0 };
	std::string type = std::to_string(datatype);
	std::string len = std::to_string(size);
	std::string tag = std::to_string(send_tag);
	
	std::cout << data_encode.size() << std::endl;
	std::cout << len << std::endl;
	std::cout << "tag:"<<len.length() << std::endl;

	for (int i = 0;i < type.length();++i)
		send_char[i] = type[i];
	for (int i = 16; i < 16 + len.length(); ++i)
		send_char[i] = len[i - 16];
	for (int i = 32; i < 32 + tag.length(); ++i)
		send_char[i] = tag[i - 32];
	for (int i = 48;i < size + 48;++i)
		send_char[i] = data[i - 48];

	send(mysocket, send_char, 48 + size, 0);
}

void socket_connect::s_sendmat(Mat image, socket_id tid)
{
	data_encode.clear();
	imencode(".jpg", image, data_encode);
	char data[100000]={0};
	for (int i = 0; i < data_encode.size(); i++)
		data[i] = data_encode[i];
	s_send_base(1, data, data_encode.size(), tid);
}

Mat socket_connect::s_recvmat(char * data, int length)
{
	//std::cout << "recv mat" << std::endl;
	Mat result;
	std::vector<uchar> vdata;
	for (int i = 0;i < length;++i)
		vdata.push_back(data[i]);
    result = cv::imdecode(vdata, IMREAD_COLOR);
	resize(result, result, cv::Size(640, 480));
	return result;
}

void* server_dataexchange(void * soc)
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

void* server_listen(void * soc)
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

void* server_heart(void * soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		if (*d_flag == true)
			break;

        for (int i = 0;i < 10;i++)
		{
            sleep(1);
			io_mutex.lock();
			for (auto i = socket_ptr->children.begin();i != socket_ptr->children.end();)
			{
				if (socket_ptr->children.empty())
					break;
				if (*(i->second->d_flag) == true)
				{
					std::cout << "disconnect require" << std::endl;
					socket_ptr->deletechild(i->first);
				}
				else
					i++;

			}
			io_mutex.unlock();
		}

		io_mutex.lock();
		//std::cout << "only for test" << std::endl;
		for (auto i = socket_ptr->children.begin();i != socket_ptr->children.end();)
		{
			if (socket_ptr->children.empty())
				break;
			if (*(i->second->d_flag) == true || i->second->heart_flag == false)
			{
				if (*(i->second->d_flag) == true)
					std::cout << "disconnect require" << std::endl;
				else if (i->second->heart_flag == false)
					std::cout << "no heart" << std::endl;
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

void* listencallback_send(void *soc)
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

void* listencallback_recv(void *soc)
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

void* client_heart(void *soc)
{
	//开始线程，获得本线程socket指针。
	socket_connect *socket_ptr = (socket_connect*)soc;
	std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
	while (1)
	{
		//std::cout << "only for test" << std::endl;
		if (*d_flag == true)
			break;
        sleep(5);
		io_mutex.lock();
		//std::cout << "发了心跳" << std::endl;
		socket_ptr->s_send_base(-2, nullptr, 0,0);
		io_mutex.unlock();
	}
}
