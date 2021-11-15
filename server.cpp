#include "winsock.h"
#include "windows.h"
#include <iostream>
#include <string>
using namespace std;

#define RECV_PORT 6666 // ���ն˿�  
#define SEND_PORT 8888 // ���Ͷ˿�   
#pragma comment(lib, "wsock32.lib")

SOCKET sockClient, sockServer;
sockaddr_in severAddr; // ��������ַ   
sockaddr_in clientAddr; // �ͻ��˵�ַ   

int addrLen; // ��ַ����  
char fileName[20]; // �ļ���  
char order[20]; // ����  
char rbuff[1024]; // ���ջ�����   
char sbuff[1024]; // ���ͻ�����  

char loginUser[100] = "user1 123456"; // ��ͨ�û��û���������   
char loginAdmin[100] = "admin1 123456789"; // ϵͳ����Ա�û���������    

// ��������  
DWORD startSocket(); // ����winsock����ʼ�� 
DWORD createSocket(); // ����socket 
int sendFileRecord(SOCKET datatcps, WIN32_FIND_DATA *pfd); // ���͵�ǰ���ļ���¼ 
int sendFileList(SOCKET datatcps); // �����ļ��б�  
int sendFile(SOCKET datatcps, FILE* file); // �����ļ�  
DWORD connectProcess(); // �Ϳͻ��˽�������   

// ����winsock����ʼ��   
DWORD startSocket() {
	WSADATA WSAData;
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout<<"��ʼ��ʧ��"<<endl;
		return -1;
	}
	return 1;
}

// ����socket 
DWORD createSocket() {
	sockClient = socket(AF_INET, SOCK_STREAM, 0);
	if(sockClient == SOCKET_ERROR) {
		cout<<"����ʧ��"<<endl;
		WSACleanup();
		return -1;
	}
	severAddr.sin_family = AF_INET;
	severAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	severAddr.sin_port = htons(RECV_PORT);
	if(bind(sockClient, (struct sockaddr FAR*)&severAddr, sizeof(severAddr)) == SOCKET_ERROR) {
		//bind�������ڽ�socket�͵�ַ�ṹ��   
		cout<<"��ʧ��"<<endl;
		return -1;
	}
	return 1;
}

// �Ϳͻ��˽�������   
DWORD connectProcess() {
	addrLen = sizeof(clientAddr); // addrLen�Ƕ����ַ�ĳ���   
	if(listen(sockClient, 10) < 0) { // ���׽��ֽ��뱻������״̬������2Ϊ������е���󳤶�  
		cout<<"����ʧ��"<<endl;
		return -1;
	}
	cout<<"���������ڼ�����"<<endl;
	while(1) {
		// acceptȡ������ͷ������������  
		// sockclient�Ǵ��ڼ������׽���  
		// clientAddr �Ǽ����Ķ����ַ  
		sockServer = accept(sockClient, (struct sockaddr FAR*)&clientAddr, &addrLen);
		while(1) {
			memset(rbuff, 0, sizeof(rbuff));
			memset(sbuff, 0, sizeof(sbuff));
			if(recv(sockServer, rbuff, sizeof(rbuff), 0) <= 0) {
				break;
			}
			cout<<endl<<"��ȡ��ִ�е����"<<rbuff<<endl;
			// get
			if(strncmp(rbuff, "get", 3) == 0) {
				strcpy(fileName, rbuff + 4);
				FILE* file; // ����һ���ļ�����ָ��  
				// ���������ļ�����  
				file = fopen(fileName, "rb"); // ���ļ���ֻ�����  
				if(file) {
					sprintf(sbuff, "get %s", fileName);
					if(!send(sockServer, sbuff, sizeof(sbuff), 0)) {
						fclose(file);
						return 0;
					}
					// ���������������Ӵ�������   
					else {
						if(!sendFile(sockServer, file)) {
							return 0;
						}
						fclose(file);
					}
				}
				else {
					strcpy(sbuff, "�޷����ļ�\n");
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
					cout<<"�޷����ļ�"<<fileName<<endl;
					return 0;
				}
				sprintf(sbuff, "put %s", fileName);
				if(!send(sockServer, sbuff, sizeof(sbuff), 0)) {
					fclose(fd);
					return 0;
				}
				memset(sbuff, '\0', sizeof(rbuff));
				while((cnt = recv(sockServer, rbuff, sizeof(rbuff), 0)) > 0) {
					// ��cnt�����ݳ���Ϊchar�����ݴ�rbuff���뵽fdָ����ļ�  
					fwrite(rbuff, sizeof(char), cnt, fd);
				}
				cout<<"�ɹ�����ļ�"<<fileName<<endl;
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
				// ���õ�ǰĿ¼ 
				SetCurrentDirectory(fileName);
			}
			// ��ͨ�û�    
			else if(strncmp(rbuff, "user", 4) == 0) {
				char tbuff[1024];
				strcpy(tbuff, rbuff + 5);
				strcat(tbuff, " ");
				memset(rbuff, '\0', sizeof(rbuff));
				strcpy(sbuff, "�ɹ���ȡ��ͨ�û��û���\0");
				send(sockServer, sbuff, sizeof(sbuff), 0);
				recv(sockServer, rbuff, sizeof(rbuff), 0);
				cout<<endl<<"��ȡ��ִ�е����"<<rbuff<<endl;
				strcat(tbuff, rbuff + 4);
				//  ��֤�Ƿ���ȷ���������ݸ��ͻ���   
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
				strcpy(sbuff, "�ɹ���ȡ����Ա�û���\0");
				send(sockServer, sbuff, sizeof(sbuff), 0);
				recv(sockServer, rbuff, sizeof(rbuff), 0);
				cout<<endl<<"��ȡ��ִ�е����"<<rbuff<<endl;
				strcat(tbuff, rbuff + 4);
				//  ��֤�Ƿ���ȷ���������ݸ��ͻ���   
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

// �����ļ�   
int sendFile(SOCKET datatcps, FILE* file) {
	cout<<"���ڷ����ļ���"<<endl;
	memset(sbuff, '\0', sizeof(sbuff)); 
	while(1) {
		int len = fread(sbuff, 1, sizeof(sbuff), file);
		
		if(send(datatcps, sbuff, sizeof(sbuff), 0) == SOCKET_ERROR) {
		cout<<"����ʧ��"<<endl;
		closesocket(datatcps);
		return 0;
		}
		
		// �ļ����ͽ���   
		if (len < sizeof(sbuff)) {
			break;
		}
	}
	// cout<<sum<<endl;
	closesocket(datatcps);
	cout<<"���ͳɹ�"<<endl;
	return 1;
}

// �����ļ��б�   
int sendFileList(SOCKET datatcps) {
	HANDLE hff; // ����һ���߳�   
	WIN32_FIND_DATA fd; // �����ļ� 
	hff = FindFirstFile("*", &fd); // �����ļ����Ѵ������ļ���������Զ�ȡ��WIN32_FIND_DATA�ṹ��ȥ 
	// ��������   
	if(hff == INVALID_HANDLE_VALUE) {
		const char *errStr = "�г��ļ��б�ʱ��������\n";
		cout<<*errStr<<endl;
		if(send(datatcps, errStr, strlen(errStr), 0) == SOCKET_ERROR) {
			cout<<"����ʧ��"<<endl;
		}
		closesocket(datatcps);
		return 0;
	}
	BOOL flag = TRUE;
	// �����ļ���Ϣ  
	while(flag) {
		if(!sendFileRecord(datatcps, &fd)) {
			closesocket(datatcps);
			return 0;
		}
		// ������һ���ļ� 
		flag = FindNextFile(hff, &fd);
	}
	closesocket(datatcps);
	return 1;
}

// ���͵�ǰ���ļ���¼ 
int sendFileRecord(SOCKET datatcps, WIN32_FIND_DATA *pfd) {
	char fileRecord[MAX_PATH + 32];
	
	// �ļ��Ľ���ʱ��  
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
	// ͨ��datatcps�ӿڷ���fileRecord���ݣ��ɹ����ط��͵��ֽ��� 
	if(send(datatcps, fileRecord, strlen(fileRecord), 0) == SOCKET_ERROR) {   
		cout << "����ʧ��" << endl;
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

