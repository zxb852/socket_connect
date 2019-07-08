#include "socket_connect.h"

using namespace std;
using namespace cv;

int main()
{
	socketinit();
	socket_connect s_server;
	s_server.server_init("127.0.0.2", 8010);
	int tid;
	Mat src;
	while (1)
	{
		if (s_server.recv_buff_pop(src, tid))
		{
			std::cout << "from " << tid << std::endl;
			imshow("src", src);
			cvWaitKey(1);
		}
	}
	getchar();
	socketclose();
	return 0;
}


//#include <stdio.h>
//#include <Winsock2.h>
//#include <opencv2/opencv.hpp>
//#include <vector> 
//#pragma comment(lib,"ws2_32.lib")
//
//using namespace cv;
//using namespace std;
//
//void main()
//{
//	WSADATA wsaData;
//	SOCKET sockServer;
//	SOCKADDR_IN addrServer;
//	SOCKET conn;
//	SOCKADDR_IN addr;
//
//	WSAStartup(MAKEWORD(2, 2), &wsaData);
//	//����Socket  
//	sockServer = socket(AF_INET, SOCK_STREAM, 0);
//	//׼��ͨ�ŵ�ַ  
//	addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//	addrServer.sin_family = AF_INET;
//	addrServer.sin_port = htons(8010);
//	//��  
//	bind(sockServer, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
//	//����  
//	listen(sockServer, 5);
//	printf("Waiting for images...\n");
//
//	int len = sizeof(SOCKADDR);
//	//��������  
//	conn = accept(sockServer, (SOCKADDR*)&addr, &len);
//	cout << "ok!" << endl;
//	char recvBuf[16];
//	char recvBuf_1[1];
//	Mat img_decode;
//
//	while (1)
//	{
//		cout << "pool" << endl;
//		vector<uchar> data;
//		if (recv(conn, recvBuf, 16, 0))
//		{
//			for (int i = 0; i < 16; i++)
//			{
//				if (recvBuf[i]<'0' || recvBuf[i]>'9') recvBuf[i] = ' ';
//			}
//			cout << "mode :" << atoi(recvBuf) << endl;
//			recv(conn, recvBuf, 16, 0);
//			for (int i = 0; i < 16; i++)
//			{
//				if (recvBuf[i]<'0' || recvBuf[i]>'9') recvBuf[i] = ' ';
//			}
//			cout << "length :" << atoi(recvBuf) << endl;
//
//			for (int i = 0; i < atoi(recvBuf); i++)
//			{
//				recv(conn, recvBuf_1, 1, 0);
//				data.push_back(recvBuf_1[0]);
//			}
//			
//
//			data.resize(atoi(recvBuf));
//			printf("Image recieved successfully!\n");
//			send(conn, "Server has recieved messages!", 29, 0);
//			img_decode = imdecode(data, CV_LOAD_IMAGE_COLOR);
//			imshow("server", img_decode);
//			if (waitKey(30) == 27) break;
//		}
//	}
//	closesocket(conn);
//	WSACleanup();
//}
