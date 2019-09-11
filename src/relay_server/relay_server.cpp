#include "relay_server.h"

void relay_server::relaydata()
{
    Mat src;

    io_mutex.lock();
    if (!recv_q_mat.empty())
    {
        send_q_mat.push(recv_q_mat.front());
        recv_q_mat.pop();
    }
    if (!recv_q_state_mes.empty())
    {
        send_q_state_mes.push(recv_q_state_mes.front());
        recv_q_state_mes.pop();
    }
    if (!recv_vedio_name.empty())
    {
        send_q_vedio_name
                .push(recv_vedio_name.front());
        recv_vedio_name.pop();
    }
    io_mutex.unlock();
}

int relay_server::login(std::string user, std::string pass)
{
    if (user == "server"&&pass == "123456")
        return 1;
    if (user == "admin"&&pass == "654321")
        return 2;
    if (user == "client"&&pass == "123")
        return 3;
    return 0;
}
