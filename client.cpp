#include "winsock.h"
#include "windows.h"
#include "time.h"
#include "stdio.h"
#include <iostream>
using namespace std;

#define RECV_PORT 6666	//接收端口  
#define SEND_PORT 8888	//发送端口 
#pragma comment(lib, "wsock32.lib")	//加载ws2_32.dll，它是Windows Sockets应用程序接口， 用于支持Internet和网络应用程序。

SOCKET sockClient; // 客户端对象 
sockaddr_in serverAddr; // 服务器地址 
char inputIP[20]; // 存储输入的服务器IP 
char fileName[20]; // 文件名 
char rbuff[1024]; // 接收缓冲区  
char sbuff[1024]; // 发送缓冲区 
bool flag = false; // 标志是否登录  
char loginer[100]; // 用户身份     

DWORD startSocket(); // 启动winsock并初始化 
DWORD createSocket(); // 创建socket 
DWORD callServer(); // 发送连接请求 
void help(); // 帮助菜单 
void list(SOCKET sockfd); // 列出远方当前目录 
DWORD sendToServer(char data[]); // 发送要执行的命令至服务端 
int username(); // 用户名 
int password(); // 密码 
int sendFile(SOCKET datatcps, FILE* file);	//上传一个文件 

int main() {
	while(1) {
		char operation[10], name[20]; // 操作与文件名 
		char order[30] = "\0"; // 输入的命令 
		char buff[80]; // 用来存储经过字符串格式化的order 
		FILE *fd1, *fd2; // File协议主要用于访问本地计算机中的文件，fd指针指向要访问的目标文件  
		int cnt;

		startSocket(); // 启动winsock并初始化 
		
		if(callServer() == -1) { // 发送连接请求失败 
			continue;
		}

		// 发送连接请求成功，初始化数据 
		memset(operation, 0, sizeof(operation));
		memset(name, 0, sizeof(name));
		memset(order, 0, sizeof(order));
		memset(buff, 0, sizeof(buff));
		memset(rbuff, 0, sizeof(rbuff));
		memset(sbuff, 0, sizeof(sbuff));

		// 判断是否登录 
		if(flag == false) {
			if(username() && password()) {
				flag = true;
				continue;
			}
			else {
				continue;
			}
		}

		cout<<endl<<"请输入要执行的指令(输入 help 可以打开帮助菜单) : ";
		cin>>operation;
		// 需要输入功能 
		if(strncmp(operation, "get", 3) == 0 || strncmp(operation, "put", 3) == 0 || strncmp(operation, "cd", 2) == 0) { 
			cin>>name;
		}
		// 退出功能  
		else if(strncmp(operation, "quit", 4) == 0) { 
			cout << "感谢您的使用" << endl;
			closesocket(sockClient);
			WSACleanup();
			break;
		}
		// 帮助菜单功能  
		else if(strncmp(operation, "help", 4) == 0) { 
			help();
		}

		if (strncmp(operation, "put", 3) == 0 && strncmp(loginer, "user", 4) == 0) {
			cout << "普通用户无法上传文件" << endl;
			closesocket(sockClient); // 关闭连接 
			WSACleanup();
			continue;
		}
		
		// 将指令整合进order，并存放进buff 
		strcat(order, operation);
		strcat(order, " ");
		strcat(order, name);
		sprintf(buff, order);
		sendToServer(buff); // 发送指令 
		recv(sockClient, rbuff, sizeof(rbuff), 0); // 接收信息  
		cout<<rbuff<<endl;
		// get,下载功能 
		if(strncmp(rbuff, "get", 3) == 0) {
			fd1 = fopen(name, "wb"); // 用二进制文件方式打开文件 
			if(fd1 == NULL) {
				cout<<"打开或者新建"<<name<<"文件失败"<< endl;
				continue;
			}
			
			memset(rbuff, '\0', sizeof(rbuff));
			
			while((cnt = recv(sockClient, rbuff, sizeof(rbuff), 0)) > 0) {
				fwrite(rbuff, cnt, 1, fd1);
			}
			fclose(fd1); // 关闭文件 
		}
		// put,上传功能 
		else if(strncmp(rbuff, "put", 3) == 0) {
			strcpy(fileName, rbuff + 4);
			fd2 = fopen(fileName, "rb"); // 打开一个二进制文件，文件必须存在 
			// 成功打开 
			if(fd2) {
				if(!sendFile(sockClient, fd2)) {
					cout<<"发送失败"<<endl;
					return 0;
				}
				fclose(fd2);
			}
			else{
				strcpy(sbuff, "无法打开文件\n");
				if(send(sockClient, sbuff, sizeof(sbuff), 0)) {
					return 0;
				}
			}
		}
		// dir 
		else if(strncmp(rbuff, "dir", 3) == 0) {
			list(sockClient);
		}
		closesocket(sockClient); // 关闭连接 
		WSACleanup();
	}
	system("pause");
	return 0;
}
 
// 启动winsock并初始化  
DWORD startSocket() {
	WSADATA WSAData;
	char a[20];
	memset(a, 0, sizeof(a));
	// 加载winsock版本 
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout<<"socket初始化失败"<<endl;
		return -1;
	}
	if(strncmp(inputIP, a, sizeof(a)) == 0) {
		cout<<"请输入要连接的服务器IP：";
		cin>>inputIP;
	}
	//设置地址结构 
	serverAddr.sin_family = AF_INET; // 表明底层是使用的哪种通信协议来递交数据的，AF_INET表示使用 TCP/IPv4进行通信   
	serverAddr.sin_addr.s_addr = inet_addr(inputIP); // 指定服务器IP，十进制转化成二进制IPV4地址  
	serverAddr.sin_port = htons(RECV_PORT); // 设置端口号，htons用于将主机字节序改为网络字节序  
	return 1;
}

// 创建socket
DWORD createSocket() {
	// 要使用套接字，首先必须调用socket()函数创建一个套接字描述符，就如同操作文件时，首先得调用fopen()函数打开一个文件。
	sockClient = socket(AF_INET, SOCK_STREAM, 0); // 当scoket函数成功调用时返回一个新的SOCKET(Socket Descriptor) //SOCK_STREAM（流式套接字）:Tcp连接，提供序列化的、可靠的、双向连接的字节流。支持带外数据传输   
	if(sockClient == SOCKET_ERROR) {
		cout<<"创建socket失败"<<endl;
		WSACleanup(); // 终止Ws2_32.dll的使用  
		return -1;
	}
	return 1;
}

// 发送连接请求 
DWORD callServer() {
	createSocket();
	// connect创建与指定外部端口的连接 
	if(connect(sockClient, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout<<"连接失败"<<endl;
		memset(inputIP, 0, sizeof(inputIP));
		return -1;
	}
	return 1;
}

// 帮助菜单  
void help() {
	cout << "        ___________________________________________  " << endl
		 << "       |                FTP帮助菜单                |   " << endl
		 << "       | 1、get 下载文件 [输入格式: get 文件名 ]   |   " << endl
		 << "       | 2、put 上传文件 [输入格式：put 文件名]    |   " << endl
		 << "       | 4、dir 显示远方当前目录的文件             |   " << endl
		 << "       | 5、cd  改变远方当前目录和路径             |   " << endl
		 << "       |         进入下级目录: cd 路径名           |   " << endl
		 << "       | 6、help 进入帮助菜单                      |   " << endl
		 << "       | 7、quit 退出FTP                           |   " << endl
		 << "       |___________________________________________|    " << endl;
}

// 发送要执行的命令至服务端 
DWORD sendToServer(char data[]) {
	int length = send(sockClient, data, strlen(data), 0);
	if(length <= 0) {
		cout<<"发送命令至服务端失败"<<endl;
		closesocket(sockClient); // 当不使用socket()创建的套接字时，应该调用closesocket()函数将它关闭，就如同调用fclose()函数关闭一个文件，用来进行套接字资源的释放。  
		WSACleanup();
		return -1;
	}
	return 1;
}

// 传送给远方一个文件  
int sendFile(SOCKET datatcps, FILE* file) {
	cout<<"正在传输文件…"<<endl;
	memset(sbuff, '\0', sizeof(sbuff));
	// 从文件中循环读取数据并发送    
	while(1) {
		int len = fread(sbuff, 1, sizeof(sbuff), file);
		if(send(datatcps, sbuff, len, 0) == SOCKET_ERROR) {
			cout<<"与客户端的连接中断"<<endl;
			closesocket(datatcps);
			return 0;
		}
		if(len < sizeof(sbuff)) {
			break;
		}
	}
	closesocket(datatcps);
	cout<<"传输完成"<<endl;
	return 1;
}

// 列出远方当前目录  
void list(SOCKET sockfd) {
	int nRead;
	memset(sbuff, '\0', sizeof(sbuff));
	while(1) {
		nRead = recv(sockClient, rbuff, sizeof(rbuff), 0);
		// recv通过sockClient套接口接受数据存入rbuff缓冲区，返回接收到的字节数  
		if(nRead == SOCKET_ERROR) {
			cout<<"读取时发生错误"<<endl;
			exit(1);
		}
		// 数据读取结束  
		if(nRead == 0) {
			break;
		}
		// 显示数据  
		rbuff[nRead] = '\0';
		cout<<rbuff<<endl;
	}
}

int username() {
	char operation[10], name[20]; // 操作与文件名  
	char order[30] = "\0"; // 输入的命令  
	char buff[80]; // 用来存储经过字符串格式化的order  
	cout<<"请输入用户名指令（user/admin 用户名）：";
	cin>>operation;
	cin>>name;
	strcpy(loginer, operation);
	strcat(order, operation);
	strcat(order, " ");
	strcat(order, name);
	sprintf(buff, order);
	sendToServer(buff); // 发送指令 
	recv(sockClient, rbuff, sizeof(rbuff), 0); // 接收信息   
	cout<<rbuff<<endl;
	return 1;
}

int password() {
	char operation[10], name[20]; // 操作与文件名  
	char order[30] = "\0"; // 输入的命令  
	char buff[80]; // 用来存储经过字符串格式化的order  
	cout<<"请输入密码指令（pwd 密码）：";
	cin>>operation;
	cin>>name;
	strcat(order, operation);
	strcat(order, " ");
	strcat(order, name);
	sprintf(buff, order);
	sendToServer(buff); // 发送指令  
	recv(sockClient, rbuff, sizeof(rbuff), 0); // 接收信息   
	cout<<rbuff<<endl;
	if(strcmp(rbuff, "wrong") == 0) {
		return 0;
	}
	return 1;
}

