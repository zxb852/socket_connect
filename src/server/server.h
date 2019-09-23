#pragma once
#include "socket_connect.h"


class server : public socket_connect
{
public:
    typedef vector<vector<Point>> counter;
	server()
    {
	}
	~server()
	{
		disconnect();
	}

    void savefault(const state_mes &mes,Mat &rgb,Mat &ir, Mat &uv);
    void saveaf(Mat &rgb,Mat &ir, Mat &uv, const counter &crgb, const counter &cir, const counter &cuv);
    vector<string> datalist(vector<string> &str);
    void send_decinf(char tag, state_mes mes,Mat rgb,Mat ir, Mat uv, std::string vedio_name);
private:
    virtual void update_alarm_data(std::pair<socket_id,vector<string>>);

};

void savemat(const Mat& input,string filename);
void savecontour(const vector<vector<Point>>& input,string filename);

void getmat(Mat& input,string filename);
void getcontour(vector<vector<Point>>& input,string filename);
