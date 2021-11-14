#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>

namespace Ui {
class Client;
}

class Client : public QWidget
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();

    void sendFile(); // 发送文件数据

    void sleep(unsigned int msec); // 延时


private slots:
    void on_buttonConnect_clicked(); // 建立连接

    void on_buttonSend_clicked(); // 发送信息

    void on_buttonClose_clicked(); // 关闭连接

    void on_buttonFile_clicked(); // 选择文件

    void on_buttonFileSend_clicked(); // 发送文件

    void receiveData(); // 接受消息

    void receiveFile(); // 接受文件

    void getLogin(int info); // 接受身份信息
            
private:
    Ui::Client *ui;

    QTcpSocket *tcpSocket;
    QTcpSocket *fileSocket;

    QFile file; // 文件对象
    QString fileName; // 文件名称
    qint64 fileSize; // 文件大小
    qint64 sendSize; // 已经发送文件的大小

    char path[20]; // 文件路径
    int result; // 身份信息

};

#endif // CLIENT_H
