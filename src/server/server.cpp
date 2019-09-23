#include "server.h"
#include <algorithm>

void server::savefault(const state_mes &mes,Mat &rgb,Mat &ir, Mat &uv)
{
    string base=mes.tostring();

    string statename=basefile+base+"/"+base+"_mes.txt";
    string rgbname=basefile+base+"/"+base+"_rgb.jpg";
    string irname=basefile+base+"/"+base+"_ir.jpg";
    string uvname=basefile+base+"/"+base+"_uv.jpg";

    mes.save(statename);
    savemat(rgb,rgbname);
    savemat(ir,irname);
    savemat(uv,uvname);

}

void server::saveaf(Mat &rgb,Mat &ir, Mat &uv, const counter &crgb, const counter &cir, const counter &cuv)
{
    static int index = 0;

    ++index;
    string base = "labdata/";

    string statename=base+to_string(index)+"_mes.txt";
    string rgbname=base+to_string(index)+"_rgb.jpg";
    string irname=base+to_string(index)+"_ir.jpg";
    string uvname=base+to_string(index)+"_uv.jpg";
    string crgbname=base+to_string(index)+"_rgb.txt";
    string cirname=base+to_string(index)+"_ir.txt";
    string cuvname=base+to_string(index)+"_uv.txt";

    state_mes mes;
    mes.settime_now();
    mes.save(statename);
    savemat(rgb,rgbname);
    savemat(ir,irname);
    savemat(uv,uvname);
    savecontour(crgb,crgbname);
    savecontour(cir,cirname);
    savecontour(cuv,cuvname);


}

//tag==3 广播 tag>10 具体某个目标
void server::send_decinf(char tag, state_mes mes,Mat rgb,Mat ir, Mat uv, std::string vedio_name)
{
    if(tag==3 || tag >10)
    {
        send_buff_push(mes,tag);
        usleep(200000);
        send_buff_push(rgb,51);
        send_buff_push(ir,52);
        send_buff_push(uv,53);
        usleep(300000);
        send_buff_push(vedio_name,tag);
    }
    usleep(500000);
}

void server::update_alarm_data(std::pair<socket_id,vector<string>> alarm_data)
{
    socket_id tag =alarm_data.first;
    vector<string> &updata=alarm_data.second;
    vector<string> updata_list=datalist(updata);
    //for_each(updata_list.begin(), updata_list.end(), [](const string &s){std::cout << s << std::endl; });
    for(string name: updata_list)
    {
        string statename=basefile+name+"/"+name+"_mes.txt";
        string rgbname=basefile+name+"/"+name+"_rgb.jpg";
        string irname=basefile+name+"/"+name+"_ir.jpg";
        string uvname=basefile+name+"/"+name+"_uv.jpg";
        string Vedioname=basefile+name+"/"+name+"_vedio.mp4";

        state_mes mes;
        mes.getfromfile(statename);
        Mat rgb=imread(rgbname);
        Mat ir=imread(irname);
        Mat uv=imread(uvname);
        send_decinf(tag,mes,rgb,ir,uv,Vedioname);
    }
}

vector<string> server::datalist(vector<string> &str)
{
    vector<string> result;
    vector<string> files;
    string flod=string("./")+basefile;
    getFiles(flod.c_str(), files);
    std::sort(str.begin(), str.end());
    std::sort(files.begin(), files.end());
    std::set_difference(files.begin(), files.end(),str.begin(), str.end(),std::back_inserter(result));

    //for_each(result.begin(), result.end(), [](const string &s){std::cout << s << std::endl; });
    return result;
}


void savemat(const Mat& input,string filename)
{
    imwrite(filename,input);
}

void savecontour(const vector<vector<Point>>& input,string filename)
{
    std::ofstream outFile;
    outFile.open(filename);
    for(const vector<Point> &i : input)
    {
        for(const Point &ii : i)
        {
            outFile<<ii;
        }
        outFile<<'<';
        outFile<<"\n";
    }
}



void getmat(Mat& input,string filename)
{
    input=imread(filename);
}

void getcontour(vector<vector<Point>>& input,string filename)
{
    std::ifstream inFile;
    inFile.open(filename);
    vector<Point> step;
    while(!inFile.eof())
    {
        char c;
        int x,y;
        inFile>>c;
        if(c=='[')
        {
            inFile>>x>>c>>y>>c;
            step.push_back(Point(x,y));
        }
        else if(c=='<')
        {
            input.push_back(step);
            step.clear();
        }

    }

}
