#include "client.h"
#include <algorithm>


vector<string> client::checkdata()
{
    vector<string> result;
    getFiles(basefile.c_str(), result);
    for_each(result.begin(), result.end(), [](const string &s){std::cout << s << std::endl; });
    return result;
}


void client::updata_alarm_data()
{
    const vector<string> vstr=checkdata();

    state_mes mes_begin;
    mes_begin.mode=11;
    send_buff_push(mes_begin,1);

    for(string i : vstr)
    {
        state_mes mes;
        mes.fromstring(i);
        mes.mode=12;
        send_buff_push(mes,1);
    }

    state_mes mes_end;
    mes_end.mode=13;
    send_buff_push(mes_end,1);
}
