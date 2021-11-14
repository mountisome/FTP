#include "server.h"
#include "ui_server.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QTime>

Server::Server(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

    setWindowTitle("服务器");

    // 监听套接字
    tcpServer = new QTcpServer(this);
    tcpSocket = new QTcpSocket(this);
    tcpServer->listen(QHostAddress::Any, 8888);   
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

    // 文件传送套接字
    fileServer = new QTcpServer(this);
    fileSocket = new QTcpSocket(this);
    fileServer->listen(QHostAddress::Any, 6666);
    connect(fileServer, SIGNAL(newConnection()), this, SLOT(acceptFileConnection()));

}

Server::~Server()
{
    delete ui;
}

// 消息连接
void Server::acceptConnection()
{
    tcpSocket = tcpServer->nextPendingConnection();

    ui->editRead->append("连接成功");

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));
}

// 文件传输连接
void Server::acceptFileConnection()
{
    fileSocket = fileServer->nextPendingConnection();

    qDebug() << "文件传输连接成功";
}

// 接受消息
void Server::receiveData()
{
    QString str = tcpSocket->readAll();
    ui->editRead->append(str);

    QByteArray ba = str.toLatin1();

    char *str1 = ba.data();

    // 上传文件
    if (strncmp(str1, "put", 3) == 0) {
        connect(fileSocket, SIGNAL(readyRead()), this, SLOT(receiveFile()));

        strcpy(fileName, str1 + 4);

        QString str2(fileName);

        file.setFileName(str2);

        bool isOk = file.open(QIODevice::WriteOnly);

        if (!isOk) {
            qDebug() << "无法创建文件";
        }
    }

    // 下载文件
    else if (strncmp(str1, "get", 3) == 0) {
        strcpy(fileName, str1 + 4);

        QString str2(fileName);

        filePath = "D:/" + str2; // 下载文件路径可以选择，这里选择D盘下的文件

        QFileInfo info(filePath);

        fileSize = info.size();

        file.setFileName(filePath);

        qDebug() << "filePath:" << filePath;
        qDebug() << "fileSize:" << fileSize;

        bool isOk = file.open(QIODevice::ReadOnly);

        if (!isOk) {
            qDebug() << "文件打开失败";
        }
        else {
            sleep(20);

            sendFile();
        }
    }
}

// 发送消息
void Server::on_buttonSend_clicked()
{
    if (tcpSocket == NULL) {
        return;
    }
    // 获取编辑区内容
    QString str = ui->editWrite->toPlainText();
    // 给对方发送数据
    tcpSocket->write(str.toUtf8().data());
}

// 接受文件
void Server::receiveFile()
{
    // 取出接受的内容
    QByteArray buff = fileSocket->readAll();

    // 文件信息
    qint64 len = file.write(buff);

    qDebug() << "文件大小:" << len;

    // 关闭文件
    file.close();

    QMessageBox::information(this, "完成", "文件接受完成");

    fileSocket->disconnectFromHost();
    fileSocket->close();
    qDebug() << "文件传输连接已关闭";
}

// 发送文件
void Server::sendFile()
{
    sendSize = 0;
    qint64 len = 0;

    do {
        // 每次发送数据的大小
        char buff[1024] = {0};
        len = 0;

        // 往文件中读数据
        len = file.read(buff, sizeof(buff));

        // 发送数据
        len = fileSocket->write(buff, len);

        // 发送数据累加
        sendSize += len;

    } while(len > 0);

    if (sendSize == fileSize) {
        ui->editRead->append("文件发送完毕");
        file.close();

        // 把连接断开
        fileSocket->disconnectFromHost();
        fileSocket->close();
    }
    else {
        ui->editRead->append("文件发送失败");
    }
}

// 关闭连接
void Server::on_buttonClose_clicked()
{
    // 主动和客户端断开连接
    if (tcpSocket != NULL) {
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
        qDebug() << "消息连接已关闭";
    }

    if (fileSocket != NULL) {
        fileSocket->disconnectFromHost();
        fileSocket->close();
        qDebug() << "文件传输连接已关闭";
    }
}

void Server::sleep(unsigned int msec)
{
    QTime reachTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < reachTime);
}
