#include "server.h"

void server::send_decinf(state_mes mes,Mat rgb,Mat ir, Mat uv, std::string vedio_name)
{
    send_buff_push(mes,1);
    usleep(5000);
    send_buff_push(rgb,51);
    send_buff_push(ir,52);
    send_buff_push(uv,53);
    send_buff_push(vedio_name,1);
}
