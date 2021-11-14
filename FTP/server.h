#ifndef SERVER_H
#define SERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>

namespace Ui {
class Server;
}

class Server : public QWidget
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = 0);
    ~Server();

    void sleep(unsigned int msec); // 延时

private slots:
    void on_buttonSend_clicked(); // 发送信息

    void on_buttonClose_clicked(); // 关闭连接

    void acceptConnection(); // 消息连接

    void acceptFileConnection(); // 文件传输连接

    void receiveData(); // 接受数据

    void receiveFile(); // 接受文件

    void sendFile(); // 发送文件

private:
    Ui::Server *ui;

    QTcpServer *tcpServer; // 监听套接字
    QTcpSocket *tcpSocket; // 通信套接字

    QTcpServer *fileServer; // 文件监听套接字
    QTcpSocket *fileSocket; // 文件通信套接字

    QFile file; // 文件对象
    char fileName[20]; // 文件名称
    qint64 fileSize; // 文件大小
    qint64 sendSize; // 发送文件大小
    qint64 recvSize; // 已经接受的文件大小
    QString filePath; // 文件路径
};

#endif // SERVER_H
