#include "client.h"
#include "ui_client.h"
#include <QHostAddress>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QTime>

Client::Client(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);
    
    tcpSocket = NULL;
    fileSocket = NULL;
    setWindowTitle("客户端");
}

Client::~Client()
{
    delete ui;
}

void Client::getLogin(int info)
{
    result = info; 

    if (result == 0) {
        ui->buttonFile->setEnabled(false);
        ui->buttonFileSend->setEnabled(false);
    }
    else if (result == 1) {
        ui->buttonFile->setEnabled(false);
        ui->buttonFileSend->setEnabled(false);
    }
}

// 建立连接
void Client::on_buttonConnect_clicked()
{
    tcpSocket = new QTcpSocket(this);

    fileSocket = new QTcpSocket(this);

    // 获取服务器ip和端口
    QString ip = ui->editIP->text();
    qint16 port = ui->editPort->text().toInt();

    // 主动和服务器建立连接
    tcpSocket->connectToHost(QHostAddress(ip), port);

    ui->editRead->append("成功和服务器建立连接");

    fileSocket->connectToHost("127.0.0.1", 6666);

    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));

    connect(fileSocket, SIGNAL(readyRead()), this, SLOT(receiveFile()));
}

// 发送消息
void Client::on_buttonSend_clicked()
{
    // 获取编辑框内容
    QString str = ui->editWrite->toPlainText();

    if (tcpSocket == NULL) {
        qDebug() << "未连接";
        return;
    }

    if (result == 0 && strncmp(str.toUtf8().data(), "get", 3) == 0) {
        qDebug() << "游客无法下载文件";
        return;
    }
    if (result == 0 && strncmp(str.toUtf8().data(), "put", 3) == 0) {
        qDebug() << "游客无法上传文件";
        return;
    }

    if (result == 1 && strncmp(str.toUtf8().data(), "put", 3) == 0) {
        qDebug() << "普通用户无法上传文件";
        return;
    }

    // 发送数据
    tcpSocket->write(str.toUtf8().data());

    if (strncmp(str.toUtf8().data(), "get", 3) == 0 && result != 0) {
        strcpy(path, str.toUtf8().data() + 4);

        QString str1(path);

        file.setFileName(str1);

        bool isOk = file.open(QIODevice::WriteOnly);

        if (!isOk) {
            qDebug() << "无法打开文件";
        }
    }
}

// 接受消息
void Client::receiveData()
{
    QString str = tcpSocket->readAll();
    ui->editRead->append(str);
}

// 关闭连接
void Client::on_buttonClose_clicked()
{
    // 主动和对方断开连接
    if (tcpSocket != NULL) {
        tcpSocket->disconnectFromHost();
        tcpSocket->close();
        tcpSocket = NULL;
        qDebug() << "消息连接已关闭";
    }

    if (fileSocket != NULL) {
        fileSocket->disconnectFromHost();
        fileSocket->close();
        fileSocket = NULL;
        qDebug() << "文件传输连接已关闭";
    }
}

// 选择文件
void Client::on_buttonFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "open", "../");
    // 如果选择文件路径有效
    if (filePath.isEmpty() == false) {
        qDebug() << "filePath:" << filePath;

        fileName.clear();
        fileSize = 0;

        // 获取文件信息
        QFileInfo info(filePath);
        fileName = info.fileName(); // 获取文件名称
        fileSize = info.size(); // 获取文件大小

        sendSize = 0; // 发送文件的大小

        // 只读方式打开文件
        file.setFileName(filePath);

        // 打开文件
        bool isOk = file.open(QIODevice::ReadOnly);
        if (isOk == false) {
            qDebug() << "只读方式打开文件失败";
        }

        // 提示打开文件的路径
        ui->editRead->append(filePath);
    }
    else {
        qDebug() << "选择文件路径出错";
    }
}

// 发送文件
void Client::on_buttonFileSend_clicked()
{
    if (fileSocket == NULL) {
        qDebug() << "未连接";
        return;
    }

    sleep(20);

    // 发送文件信息
    sendFile();
}

// 发送文件数据
void Client::sendFile()
{
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
        fileSocket = NULL;
    }
    else {
        ui->editRead->append("文件发送失败");
    }
}

// 接受文件
void Client::receiveFile()
{
    // 取出接受的内容
    QByteArray buff = fileSocket->readAll();

    ui->editRead->append(QString(buff));

    qint64 len = file.write(buff);

    qDebug() << "文件大小:" << len;

    // 关闭文件
    file.close();

    QMessageBox::information(this, "下载", "文件下载完成");

    fileSocket->disconnectFromHost();
    fileSocket->close();
    fileSocket = NULL;
    qDebug() << "文件传输连接已关闭";
}

void Client::sleep(unsigned int msec)
{
    QTime reachTime = QTime::currentTime().addMSecs(msec);
    while(QTime::currentTime() < reachTime);
}
