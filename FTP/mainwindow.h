#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "server.h"
#include "client.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Server w1; // 服务器窗口
    Client w2; // 客户端窗口

    void userLogin(); // 登录
    int result;

signals:
    void sendLogin(int); // 传递身份信息

private slots:
    void on_loginButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
