#include "relay_server.h"
extern std::mutex io_mutex;
extern std::vector<uchar> data_encode;

int relay_server::relaydata()
{
    Mat src;

    io_mutex.lock();
    if (!recv_q_mat.empty())
    {
        send_q_mat.push(recv_q_mat.front());
        recv_q_mat.pop();
    }
    io_mutex.unlock();
}
