#include "socket_connect.h"
std::mutex io_mutex;
std::vector<uchar> data_encode;
std::ofstream outfile;

void socketinit()
{
#ifdef Windows
    WSADATA wsd;
    WSAStartup(MAKEWORD(2, 0), &wsd);
#endif // Windows

}

void socketclose()
{
#ifdef Windows
    WSACleanup();
#endif // Windows
}

void socket_connect::updatesendbuff()
{
    mymutex m;
    for (auto i : children)
    {
        if (childrenctrl[i.first] != 0)
        {
            socket_connect* childsocket = i.second;

            if (!send_q_mat.empty())
            {
                //0: 初始化  1: 服务器 2: 管理员  3: 普通用户
                if (childrenctrl[i.first] == 1)
                    std::cout << "admin" << std::endl;
                //childsocket->send_q_mat.push(send_q_mat.front());
                else
                {
                    std::cout << "client" << std::endl;
                    childsocket->send_q_mat.push(send_q_mat.front());
                }
            }
            if (!send_q_state_mes.empty())
            {
                //0: 初始化  1: 服务器 2: 管理员  3: 普通用户
                if (childrenctrl[i.first] == 1)
                    std::cout << "admin" << std::endl;
                //childsocket->send_q_mat.push(send_q_mat.front());
                else
                {
                    std::cout << "client" << std::endl;
                    childsocket->send_q_state_mes.push(send_q_state_mes.front());
                }
            }
            if (!send_q_vedio_name.empty())
            {
                //0: 初始化  1: 服务器 2: 管理员  3: 普通用户
                if (childrenctrl[i.first] == 1)
                    std::cout << "admin" << std::endl;
                //childsocket->send_q_mat.push(send_q_mat.front());
                else
                {
                    std::cout << "client" << std::endl;
                    childsocket->send_q_vedio_name.push(send_q_vedio_name.front());
                }
            }
            if (!send_q_sample.empty())
            {
                if (childrenctrl[i.first] == 1 && send_q_sample.front().first.second == 1)
                    childsocket->send_q_sample.push(send_q_sample.front());
                else if (send_q_sample.front().first.second == 2 || send_q_sample.front().first.second == i.first)
                    childsocket->send_q_sample.push(send_q_sample.front());
            }
        }
    }

    if(!send_q_mat.empty())
        send_q_mat.pop();
    if (!send_q_state_mes.empty())
        send_q_state_mes.pop();
    if (!send_q_vedio_name.empty())
        send_q_vedio_name.pop();
    if (!send_q_sample.empty())
        send_q_sample.pop();
}

void socket_connect::updaterecvbuff()
{
    mymutex m;
    for (auto i : children)
    {
        socket_connect* childsocket = i.second;
        if (!childsocket->recv_q_mat.empty())
        {
            auto &step = childsocket->recv_q_mat.front();
            //step.first.first = i.first;
            //发送的标志位：1：RGB 2：红丸 3：紫外 4:融合 51:故障rgb 52：故障红外 53：故障紫外
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
            std::cout << "	to:" << childrenctrl[i.first]<<std::endl;
            childsocket->recv_q_login_mes.pop();
        }
        if (!childsocket->recv_q_state_mes.empty())
        {
            auto &step = childsocket->recv_q_state_mes.front();
            step.first.first = i.first;
            recv_q_state_mes.push(step);
            childsocket->recv_q_state_mes.pop();
        }
        if (!childsocket->recv_vedio_name.empty())
        {
            auto &step = childsocket->recv_vedio_name.front();
            step.first.first = i.first;
            recv_vedio_name.push(step);
            childsocket->recv_vedio_name.pop();
        }
    }
}

bool socket_connect::s_bind(const char *ip, int port)
{
    _SOCKADDR_IN addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = inet_addr(ip);
    addrServer.sin_port = htons(port);
    bool result = bind(mysocket, reinterpret_cast<_SOCKADDR*>(&addrServer), sizeof(addrServer)) == 0;
    listen(mysocket, LISTEN_LENGTH);
    return result;
}

void socket_connect::s_listen()
{
    //循环接收客户端连接请求并创建服务线程
    _SOCKET conn;
    _SOCKADDR_IN addr;
    _socklen_t len = sizeof(_SOCKADDR);
    conn = accept(mysocket, reinterpret_cast<_SOCKADDR*>(&addr), &len);

    std::cout << "一个客户端已连接到服务器，socket是：" << conn << std::endl;
    socket_connect *socketctl=new socket_connect(conn);

#ifdef Windows
    std::thread t1(listencallback_send, socketctl);
    std::cout << "send thread id :" << t1.get_id() << std::endl;
    t1.detach();

    std::thread t2(listencallback_recv, socketctl);
    std::cout << "recv thread id :" << t2.get_id() << std::endl;
    t2.detach();
#endif // Windows


#ifdef Linux
    pthread_t t1;
    pthread_create(&t1, nullptr, listencallback_send, reinterpret_cast<void*>(socketctl));
    std::cout << "send thread id :" << t1 << std::endl;

    pthread_t t2;
    pthread_create(&t2, nullptr, listencallback_recv, reinterpret_cast<void*>(socketctl));
    std::cout << "recv thread id :" << t2 << std::endl;

#endif // Linux

    children[children_id] = socketctl;
    childrenctrl[children_id] = 0;
    children_id++;
}

void socket_connect::server_init(const char * ip, int port)
{
    if (s_bind(ip, port))
    {
        std::cout << "bind" << ip << ":" << port << "    successfully" << std::endl;
#ifdef Windows
        std::thread tlisten(server_listen, this);
        std::cout << "creat listen thread " << tlisten.get_id() << "    successfully" << std::endl;
        tlisten.detach();

        std::thread tdata(server_dataexchange, this);
        std::cout << "creat data thread " << tdata.get_id() << "         successfully" << std::endl;
        tdata.detach();

        std::thread theart(server_heart, this);
        std::cout << "creat heart thread " << theart.get_id() << "    successfully" << std::endl;
        theart.detach();
#endif

#ifdef Linux
        pthread_t tlisten;
        pthread_create(&tlisten, nullptr, server_listen, reinterpret_cast<void*>(this));
        std::cout << "creat listen thread " << tlisten << "    successfully" << std::endl;

        pthread_t tdata;
        pthread_create(&tdata, nullptr, server_dataexchange, reinterpret_cast<void*>(this));
        std::cout << "creat data thread " << tdata << "         successfully" << std::endl;

        pthread_t theart;
        pthread_create(&theart, nullptr, server_heart, reinterpret_cast<void*>(this));
        std::cout << "creat heart thread " << theart << "    successfully" << std::endl;
#endif
        
    }
    else
    {
        std::cout << "bind failed" << std::endl;
    }
}

bool socket_connect::s_connect(const char *ip, int port)
{
    _SOCKADDR_IN addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr._s_addr = inet_addr(ip);
    addrServer.sin_port = htons(port);
    ismain = false;
    if (connect(mysocket, reinterpret_cast<_SOCKADDR*>(&addrServer), sizeof(_SOCKADDR)) == -1)
        return false;

#ifdef Windows
    std::thread t(listencallback_send, this);
    t.detach();
    std::thread t2(listencallback_recv, this);
    t2.detach();
    std::thread t3(client_heart, this);
    t3.detach();
    std::cout << "client data contrl thread id set up : " << t.get_id() << std::endl;
#endif

#ifdef Linux
    pthread_t t;
    pthread_create(&t, nullptr, listencallback_send, reinterpret_cast<void*>(this));
    pthread_t t2;
    pthread_create(&t2, nullptr, listencallback_recv, reinterpret_cast<void*>(this));
    pthread_t t3;
    pthread_create(&t3, nullptr, client_heart, reinterpret_cast<void*>(this));
    std::cout << "client data contrl thread id set up : " << t << std::endl;
#endif
    

    return true;
}

void socket_connect::send_buff_push(Mat image, int tid)
{
    mymutex m;
    send_q_mat.push(std::pair<data_head, Mat>(data_head(tid, 0), image));
}

void socket_connect::send_buff_push(sample src, int tid)
{
    mymutex m;
    send_q_sample.push(std::pair<data_head, sample>(data_head(tid, 0), src));
}

void socket_connect::send_buff_push(login_mes src, int tid)
{
    mymutex m;
    send_q_login_mes.push(std::pair<data_head, login_mes>(data_head(tid, 0), src));
}

void socket_connect::send_buff_push(state_mes src, int tid)
{
    mymutex m;
    send_q_state_mes.push(std::pair<data_head, state_mes>(data_head(tid, 0), src));
}

void socket_connect::send_buff_push(std::string src, int tid)
{
    mymutex m;
    send_q_vedio_name.push(std::pair<data_head, std::string>(data_head(tid, 0), src));
}

bool socket_connect::recv_buff_pop(Mat & output, int &tid)
{
    mymutex m;
    if (!recv_q_mat.empty())
    {
        auto &step = recv_q_mat.front();
        tid = step.first.second;
        output = step.second.clone();
        recv_q_mat.pop();
        return true;
    }
    return false;
}

bool socket_connect::recv_buff_pop(sample & output, int &tid)
{
    mymutex m;
    if (!recv_q_sample.empty())
    {
        auto &step = recv_q_sample.front();
        tid = step.first.second;
        output = step.second;
        recv_q_sample.pop();
        return true;
    }
    return false;
}

int socket_connect::login(std::string user, std::string pass)
{
    std::cout << user << "----" << pass << std::endl;
    return 0;
}

void socket_connect::s_send()
{
    mymutex m;
    if(ismain)
        updatesendbuff();
    else
    {
        //s_send(0, nullptr, 0);				//发送心跳‘

        if (!send_q_mat.empty())
        {
            s_senddata(send_q_mat.front().second,send_q_mat.front().first.first);//发送数据部分以及head的第一位。
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
        if (!send_q_vedio_name.empty())
        {
            s_senddata(send_q_vedio_name.front().second, send_q_vedio_name.front().first.first);
            send_q_vedio_name.pop();
        }
    }
}

std::string filename;
void socket_connect::s_recv()
{
    if (ismain)
    {
        mymutex m;
        updaterecvbuff();
    }
    else
    {
        char mode[1];
        memset(mode, 0, sizeof(mode));
        if (recv(mysocket, mode, 1, 0))	//Linux下flag设置为MSG_DONTWAIT，将recv工作在非阻塞模式下
        {
            char recvBuf_1[1];

            char length[4];
            char tag[1];
            char pnum[1];
            char save[2];
            char data[100000];

            memset(length, 0, sizeof(length));
            memset(tag, 0, sizeof(tag));
            memset(pnum, 0, sizeof(pnum));
            memset(save, 0, sizeof(save));
            memset(data, 0, sizeof(data));

            recv(mysocket, length, 4, 0);
            recv(mysocket, tag, 1, 0);
            recv(mysocket, pnum, 1, 0);
            recv(mysocket, save, 2, 0);

            int intmode = mode[0];

            DWORD2 la = 0X00FFFFFF | (length[3] << 24);
            DWORD2 lb = 0XFF00FFFF | (length[2] << 16);
            DWORD2 lc = 0XFFFF00FF | (length[1] << 8);
            DWORD2 ld = 0XFFFFFF00 | length[0];
            DWORD2 intlength= la&lb&lc&ld;

            int inttag = tag[0];

            int intpnum = pnum[0];

            for (int i = 0;i < intlength;++i)
            {
                recv(mysocket, recvBuf_1, 1, 0);
                data[i] = recvBuf_1[0];
            }

            if (intmode == 5)		//接收到的是视频
            {
                //视频结束标志 -1
                if(intpnum == -1)
                {
                    outfile.close();
                    recv_vedio_name.push(std::pair<data_head, std::string>(data_head(0, 2), basefile+filename+"/"+filename+"_vedio"+".avi"));
                }
                //故障信息
                else if(intpnum==-2)
                {
                    state_mes mes=s_recvdata<state_mes>(data, intlength);
                    recv_q_state_mes.push(std::pair<data_head, state_mes>(data_head(0, inttag), mes));

                    std::string folderPath = basefile+filename;
                    std::string command;
                    command = "mkdir -p " + folderPath;
                    system(command.c_str());
                }
                //故障rgb图像
                if(intpnum==-3)
                {
                    Mat mat=s_recvdata<Mat>(data, intlength);
                    recv_q_mat.push(std::pair<data_head, Mat>(data_head(0, inttag), mat));
                    imwrite(basefile+filename+"/"+filename+"_rgb"+".jpg",mat);

                }
                //故障红外图像
                if(intpnum==-4)
                {
                    Mat mat=s_recvdata<Mat>(data, intlength);
                    recv_q_mat.push(std::pair<data_head, Mat>(data_head(0, inttag), mat));
                    imwrite(basefile+filename+"/"+filename+"_ir"+".jpg",mat);
                }
                //故障紫外图像
                if(intpnum==-5)
                {
                    Mat mat=s_recvdata<Mat>(data, intlength);
                    recv_q_mat.push(std::pair<data_head, Mat>(data_head(0, inttag), mat));
                    imwrite(basefile+filename+"/"+filename+"_uv"+".jpg",mat);
                }
                else if (intpnum == 1)
                {

                    //打开文件
                    outfile.open(basefile+filename+"/"+filename+"_vedio"+".avi", std::ios_base::binary);
                    //recv(mysocket, data, intlength, 0);
                    outfile.write(data, intlength);
                }
                else
                {
                    //recv(mysocket, data, intlength, 0);
                    outfile.write(data, intlength);
                }
            }
            else
            {
                mymutex m;
                if (intmode != 0)
                {
                    heart_flag = true;
                    if (intmode == -1)
                        *d_flag = true;
                    else if (intmode == 1)
                        recv_q_mat.push(std::pair<data_head, Mat>(data_head(0, inttag), s_recvdata<Mat>(data, intlength)));
                    else if (intmode == 2)
                        recv_q_sample.push(std::pair<data_head, sample>(data_head(0, inttag), s_recvdata<sample>(data, intlength)));
                    else if (intmode == 3)
                        recv_q_login_mes.push(std::pair<data_head, login_mes>(data_head(0, inttag), s_recvdata<login_mes>(data, intlength)));

                }
            }
        }
    }
}

void socket_connect::s_send_base(char datatype, const char *data, DWORD2 size,char send_tag,char pnum)
{
    char send_char[100000] = { 0 };

    //std::cout << data_encode.size() << std::endl;
    //std::cout << "sending message......" << std::endl;
    //std::cout << "len:" << size << std::endl;
    //std::cout << "tag:" << send_tag << std::endl;

    send_char[0] = datatype;

    char csize[4];
    memset(csize, 0, sizeof(csize));
    memcpy(csize, &size, sizeof(DWORD2));
    std::string cmd = std::string(csize);
    reverse(cmd.begin(), cmd.end());

    for (int i = 1; i < 5 ; ++i)
        send_char[i] = csize[i - 1];

    send_char[5] = send_tag;
    send_char[6] = pnum;

    for (int i = 9;i < size + 9;++i)
        send_char[i] = data[i - 9];

    send(mysocket, send_char, 9 + size, 0);
}

template<class T>
void socket_connect::s_senddata(T src, socket_id tid)
{
    //mode=-2:心跳包 mode=-1:断开连接  mode=1:Mat mode=2:sample mode=3:login_mes
    char mode = 0;
    char pnum = 0;
    //std::cout << typeid(T).name() << std::endl;
    //std::cout << typeid(sample).name() << std::endl;
    if (typeid(T) == typeid(sample))
        mode = 2;
    else if (typeid(T) == typeid(login_mes))
        mode = 3;
    else if (typeid(T) == typeid(state_mes))
    {
        pnum=-2;
        mode = 5;
    }

    char data[10000];

    memset(data, 0, sizeof(data));				// 对该内存段进行清
    memcpy(data, &src, sizeof(src));			// 把这个结构体中的信息从内存中读入到字符串data中
    //接下来传送temp这个字符串就可以了
    s_send_base(mode, data, sizeof(src), tid, pnum);
}

template<>
void socket_connect::s_senddata<Mat>(Mat src, socket_id tid)
{
    int mode=1;
    int pum=0;
    if(tid==51)
    {
        mode=5;
        pum=-3;
    }
    else if(tid==52)
    {
        mode=5;
        pum=-4;
    }
    else if(tid==53)
    {
        mode=5;
        pum=-5;
    }
    data_encode.clear();
    imencode(".jpg", src, data_encode);
    char data[100000]={0};
    for (int i = 0; i < static_cast<int>(data_encode.size()); i++)
        data[i] = data_encode[i];
    s_send_base(mode, data, data_encode.size(), tid,pum);
}

template<>
void socket_connect::s_senddata(std::string src, socket_id tid)
{

    char mode = 5;
    char buf[10000];
    int bei = 0, yue = 0;
    std::ifstream infile;
    std::cout<< src << std::endl;
    infile.open(src, std::ios_base::binary);
    if (!infile.is_open()) return;

    infile.seekg(0, std::ios::end);
    //std::cout << infile.tellg() << std::endl;
    DWORD2 length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    bei = length / 10000;
    yue = length % 10000;

    int pnum = 0;
    for(int i=0;i<bei;i++)
    {
        infile.read(buf, sizeof(buf));
        s_send_base(mode, buf, sizeof(buf), tid, ++pnum);
    }
    infile.read(buf, yue);
    s_send_base(mode, buf, yue, tid, ++pnum);
    s_send_base(mode, nullptr, 0, 0, -1);
    infile.close();
}

template<class T> T socket_connect::s_recvdata(char *data, DWORD2 length)
{
    T result;
    T *p = reinterpret_cast<T *>(malloc(length));
    memcpy(p, data, length);
    result = *p;
    free(p);
    return result;
}

template<>
Mat socket_connect::s_recvdata<Mat>(char *data, DWORD2 length)
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
    socket_connect *socket_ptr = reinterpret_cast<socket_connect*>(soc);
    std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
    while (1)
    {
        if (*d_flag == true)
            break;
        socket_ptr->updatesendbuff();
        socket_ptr->updaterecvbuff();
    }
    return nullptr;
}

void* server_listen(void * soc)
{
    //开始线程，获得本线程socket指针。
    //std::cout << std::this_thread::get_id();
    socket_connect *socket_ptr = reinterpret_cast<socket_connect*>(soc);
    std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
    while (1)
    {
        if (*d_flag == true)
            break;
        socket_ptr->s_listen();
    }
    return nullptr;
}

void* server_heart(void * soc)
{
    //开始线程，获得本线程socket指针。
    socket_connect *socket_ptr = reinterpret_cast<socket_connect*>(soc);
    std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
    while (1)
    {
        if (*d_flag == true)
            break;

        for (int i = 0;i < 10;i++)
        {
            _CSLEEP(1);
            mymutex m;
            for (auto i = socket_ptr->children.begin();i != socket_ptr->children.end();i++)
            {
                if (*(i->second->d_flag) == true)
                {
                    std::cout << "disconnect require" << std::endl;
                    socket_ptr->deletechild(i->first);
                    i=socket_ptr->children.begin();
                }
            }
        }

        mymutex m;
        //std::cout << "only for test" << std::endl;
        for (auto i = socket_ptr->children.begin();i != socket_ptr->children.end();i++)
        {
            if (*(i->second->d_flag) == true || i->second->heart_flag == false)
            {
                if (*(i->second->d_flag) == true)
                    std::cout << "disconnect require" << std::endl;
                else if (i->second->heart_flag == false)
                    std::cout << "no heart" << std::endl;
                std::cout << "thread bad" << std::endl;
                socket_ptr->deletechild(i->first);
                i=socket_ptr->children.begin();
            }
            else
            {
                std::cout << "thread ok" << std::endl;
                i->second->heart_flag = false;
            }

        }
    }
    return nullptr;
}

void* listencallback_send(void *soc)
{
    //开始线程，获得本线程socket指针。
    socket_connect *socket_ptr = reinterpret_cast<socket_connect*>(soc);
    std::shared_ptr<bool> d_flag = socket_ptr->d_flag;

    while (1)
    {
        //std::cout << "asaaaaaa" << std::endl;
        if (*d_flag == true)
            break;
        socket_ptr->s_send();
    }
    return nullptr;

}

void* listencallback_recv(void *soc)
{
    //开始线程，获得本线程socket指针。
    socket_connect *socket_ptr = reinterpret_cast<socket_connect*>(soc);
    std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
    while (1)
    {
        //std::cout << "only for test" << std::endl;
        if (*d_flag == true)
            break;
        socket_ptr->s_recv();

    }
    return nullptr;
}

void* client_heart(void *soc)
{
    //开始线程，获得本线程socket指针。
    socket_connect *socket_ptr = reinterpret_cast<socket_connect*>(soc);
    std::shared_ptr<bool> d_flag = socket_ptr->d_flag;
    while (1)
    {
        //std::cout << "only for test" << std::endl;
        if (*d_flag == true)
            break;
        _CSLEEP(5);
        mymutex m;
        //std::cout << "发了心跳" << std::endl;
        socket_ptr->s_send_base(-2, nullptr, 0,0,0);
    }
    return nullptr;
}
