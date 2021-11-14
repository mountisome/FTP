#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QString>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("FTP");
    // 设置固定窗口大小
    setFixedSize(800, 600);
    result = 0;

    connect(this, SIGNAL(sendLogin(int)), &w2, SLOT(getLogin(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 登录
void MainWindow::userLogin() {
    char loginUser[30] = "user1 123456"; // 普通用户
    char loginAdmin[30] = "admin1 123456789"; // 系统管理员
    QString str1 = ui->username->text(); // 用户名
    QString str2 = ui->password->text(); // 密码
    QString loginName = ui->loginer->currentText();
    QString str3 = "普通用户";
    QString str4 = "系统管理员";
    QString str5 = "游客";
    QByteArray ba1 = str1.toLatin1();
    char *username = ba1.data();
    QByteArray ba2 = str2.toLatin1();
    char *password = ba2.data();
    char buff[30];
    strcat(buff, username);
    strcat(buff, " ");
    strcat(buff, password);
    if (str3.compare(loginName) == 0 && strcmp(loginUser, buff) == 0) {
        result = 1; // 普通用户
        w1.show();
        w2.show();
        this->close();
    }
    else if (str4.compare(loginName) == 0 && strcmp(loginAdmin, buff) == 0) {
        result = 2; // 系统管理员
        w1.show();
        w2.show();
        this->close();
    }
    else if (str5.compare(loginName) == 0 && strcmp(buff, " ") == 0) {
        w1.show();
        w2.show();
        this->close();
    }
    else {
        QMessageBox::information(this, "登录", "登录失败");
        return;
    }
}

void MainWindow::on_loginButton_clicked()
{
    userLogin();
    emit sendLogin(result);
}
