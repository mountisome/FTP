#include "winsock.h"
#include "windows.h"
#include <iostream>
#include <string>
using namespace std;

#define RECV_PORT 6666 // 接收端口  
#define SEND_PORT 8888 // 发送端口   
#pragma comment(lib, "wsock32.lib")

SOCKET sockClient, sockServer;
sockaddr_in severAddr; // 服务器地址   
sockaddr_in clientAddr; // 客户端地址   

int addrLen; // 地址长度  
char fileName[20]; // 文件名  
char order[20]; // 命令  
char rbuff[1024]; // 接收缓冲区   
char sbuff[1024]; // 发送缓冲区  

char loginUser[100] = "user1 123456"; // 普通用户用户名和密码   
char loginAdmin[100] = "admin1 123456789"; // 系统管理员用户名和密码    

// 函数声明  
DWORD startSocket(); // 启动winsock并初始化 
DWORD createSocket(); // 创建socket 
int sendFileRecord(SOCKET datatcps, WIN32_FIND_DATA *pfd); // 发送当前的文件记录 
int sendFileList(SOCKET datatcps); // 发送文件列表  
int sendFile(SOCKET datatcps, FILE* file); // 发送文件  
DWORD connectProcess(); // 和客户端进行连接   

// 启动winsock并初始化   
DWORD startSocket() {
	WSADATA WSAData;
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout<<"初始化失败"<<endl;
		return -1;
	}
	return 1;
}

// 创建socket 
DWORD createSocket() {
	sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if(sockClient == SOCKET_ERROR) {
		cout<<"创建失败"<<endl;
		WSACleanup();
		return -1;
	}
	severAddr.sin_family = AF_INET;
	severAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	severAddr.sin_port = htons(RECV_PORT);
	if(bind(sockClient, (struct sockaddr FAR*)&severAddr, sizeof(severAddr)) == SOCKET_ERROR) {
		//bind函数用于将socket和地址结构绑定   
		cout<<"绑定失败"<<endl;
		return -1;
	}
	return 1;
}

// 和客户端进行连接   
DWORD connectProcess() {
	addrLen = sizeof(clientAddr); // addrLen是对象地址的长度   
	if(listen(sockClient, 10) < 0) { // 让套接字进入被动监听状态，参数2为请求队列的最大长度  
		cout<<"监听失败"<<endl;
		return -1;
	}
	cout<<"服务器正在监听中"<<endl;
	while(1) {
		// accept取出队列头部的连接请求  
		// sockclient是处于监听的套接字  
		// clientAddr 是监听的对象地址  
		sockServer = accept(sockClient, (struct sockaddr FAR*)&clientAddr, &addrLen);
		while(1) {
			memset(rbuff, 0, sizeof(rbuff));
			memset(sbuff, 0, sizeof(sbuff));
			if(recv(sockServer, rbuff, sizeof(rbuff), 0) <= 0) {
				break;
			}
			cout<<endl<<"获取并执行的命令："<<rbuff<<endl;
			// get
			if(strncmp(rbuff, "get", 3) == 0) {
				strcpy(fileName, rbuff + 4);
				FILE* file; // 定义一个文件访问指针  
				// 处理下载文件请求  
				file = fopen(fileName, "rb"); // 打开文件，只允许读  
				if(file) {
					sprintf(sbuff, "get %s", fileName);
					if(!send(sockServer, sbuff, sizeof(sbuff), 0)) {
						fclose(file);
						return 0;
					}
					// 创建额外数据连接传送数据   
					else {
						if(!sendFile(sockServer, file)) {
							return 0;
						}
						fclose(file);
					}
				}
				else {
					strcpy(sbuff, "无法打开文件\n");
					if(send(sockServer, sbuff, sizeof(sbuff), 0)) {
						return 0;
					}
				}
			}
			// put 
			else if(strncmp(rbuff, "put", 3) == 0) {
				FILE* fd;
				int cnt;
				strcpy(fileName, rbuff + 4);
				fd = fopen(fileName, "wb");
				if(fd == NULL) {
					cout<<"无法打开文件"<<fileName<<endl;
					return 0;
				}
				sprintf(sbuff, "put %s", fileName);
				if(!send(sockServer, sbuff, sizeof(sbuff), 0)) {
					fclose(fd);
					return 0;
				}
				memset(sbuff, '\0', sizeof(rbuff));
				while((cnt = recv(sockServer, rbuff, sizeof(rbuff), 0)) > 0) {
					// 把cnt个数据长度为char的数据从rbuff输入到fd指向的文件  
					fwrite(rbuff, sizeof(char), cnt, fd);
				}
				cout<<"成功获得文件"<<fileName<<endl;
				fclose(fd);
			}
			// dir 
			else if(strncmp(rbuff, "dir", 3) == 0) {
				strcpy(sbuff, rbuff);
				send(sockServer, sbuff, sizeof(sbuff), 0);
				sendFileList(sockServer);
			}
			// cd 
			else if(strncmp(rbuff, "cd", 2) == 0) {
				strcpy(fileName, rbuff + 3);
				strcpy(sbuff, rbuff);
				send(sockServer, sbuff, sizeof(sbuff), 0);
				// 设置当前目录 
				SetCurrentDirectory(fileName);
			}
			// 普通用户    
			else if(strncmp(rbuff, "user", 4) == 0) {
				char tbuff[1024];
				strcpy(tbuff, rbuff + 5);
				strcat(tbuff, " ");
				memset(rbuff, '\0', sizeof(rbuff));
				strcpy(sbuff, "成功获取普通用户用户名\0");
				send(sockServer, sbuff, sizeof(sbuff), 0);
				recv(sockServer, rbuff, sizeof(rbuff), 0);
				cout<<endl<<"获取并执行的命令："<<rbuff<<endl;
				strcat(tbuff, rbuff + 4);
				//  验证是否正确并返回数据给客户端   
				if(strcmp(tbuff, loginUser) == 0) {
					send(sockServer, "right\0", sizeof(sbuff), 0);
				}
				else{
					send(sockServer, "wrong\0", sizeof(sbuff), 0);
				}
			}
			else if(strncmp(rbuff, "admin", 5) == 0) {
				char tbuff[1024];
				strcpy(tbuff, rbuff + 6);
				strcat(tbuff, " ");
				memset(rbuff, '\0', sizeof(rbuff));
				strcpy(sbuff, "成功获取管理员用户名\0");
				send(sockServer, sbuff, sizeof(sbuff), 0);
				recv(sockServer, rbuff, sizeof(rbuff), 0);
				cout<<endl<<"获取并执行的命令："<<rbuff<<endl;
				strcat(tbuff, rbuff + 4);
				//  验证是否正确并返回数据给客户端   
				if(strcmp(tbuff, loginAdmin) == 0) {
					send(sockServer, "right\0", sizeof(sbuff), 0);
				}
				else{
					send(sockServer, "wrong\0", sizeof(sbuff), 0);
				}
			}
			closesocket(sockServer);
		}
	}
}

// 发送文件   
int sendFile(SOCKET datatcps, FILE* file) {
	cout<<"正在发送文件…"<<endl;
	memset(sbuff, '\0', sizeof(sbuff)); 
	while(1) {
		int len = fread(sbuff, 1, sizeof(sbuff), file);
		
		if(send(datatcps, sbuff, sizeof(sbuff), 0) == SOCKET_ERROR) {
		cout<<"连接失败"<<endl;
		closesocket(datatcps);
		return 0;
		}
		
		// 文件传送结束   
		if (len < sizeof(sbuff)) {
			break;
		}
	}
	// cout<<sum<<endl;
	closesocket(datatcps);
	cout<<"发送成功"<<endl;
	return 1;
}

// 发送文件列表   
int sendFileList(SOCKET datatcps) {
	HANDLE hff; // 建立一个线程   
	WIN32_FIND_DATA fd; // 搜索文件 
	hff = FindFirstFile("*", &fd); // 查找文件来把待操作文件的相关属性读取到WIN32_FIND_DATA结构中去 
	// 发生错误   
	if(hff == INVALID_HANDLE_VALUE) {
		const char *errStr = "列出文件列表时发生错误\n";
		cout<<*errStr<<endl;
		if(send(datatcps, errStr, strlen(errStr), 0) == SOCKET_ERROR) {
			cout<<"发送失败"<<endl;
		}
		closesocket(datatcps);
		return 0;
	}
	BOOL flag = TRUE;
	// 发送文件信息  
	while(flag) {
		if(!sendFileRecord(datatcps, &fd)) {
			closesocket(datatcps);
			return 0;
		}
		// 查找下一个文件 
		flag = FindNextFile(hff, &fd);
	}
	closesocket(datatcps);
	return 1;
}

// 发送当前的文件记录 
int sendFileRecord(SOCKET datatcps, WIN32_FIND_DATA *pfd) {
	char fileRecord[MAX_PATH + 32];
	
	// 文件的建立时间  
	FILETIME ft;
	FileTimeToLocalFileTime(&pfd -> ftLastWriteTime, &ft);
	
	SYSTEMTIME lastWriteTime;
	FileTimeToSystemTime(&ft, &lastWriteTime);
	
	const char *dir = pfd -> dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? "<DIR>" : " ";
	sprintf(fileRecord, "%04d-%02d-%02d %02d:%02d %5s %10d   %-20s\n",
		lastWriteTime.wYear,
		lastWriteTime.wMonth,
		lastWriteTime.wDay,
		lastWriteTime.wHour,
		lastWriteTime.wMinute,
		dir,
		pfd -> nFileSizeLow,
		pfd -> cFileName
	);
	// 通过datatcps接口发送fileRecord数据，成功返回发送的字节数 
	if(send(datatcps, fileRecord, strlen(fileRecord), 0) == SOCKET_ERROR) {   
		cout << "发送失败" << endl;
		return 0;
	}
	return 1;
}

int main() {
	if(startSocket() == -1 || createSocket() == -1 || connectProcess() == -1) {
		return -1;
	}
	system("pause");
	return 0;
}

