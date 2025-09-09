#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "backend/backend.h"
#include <QMessageBox>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_backend = new backend(this);
    
    // 不再连接backend的信号

    QString hot ="192.168.196.129";
    QString& host =hot;

    m_backend->connectToServer(host, 8080);
    // 在Qt客户端中添加连接状态检查

    // 不再连接到服务器
    
    // 初始化联系人列表
    ui->listWidget->addItem("用户1");
    ui->listWidget->addItem("用户2");
    ui->listWidget->addItem("用户3");
    
    // 连接联系人列表的点击信号
    connect(ui->listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        m_currentRecipient = item->text();
        ui->listWidget_2->clear();
        ui->listWidget_2->addItem(QString("与 %1 的聊天记录").arg(m_currentRecipient));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_login_clicked()
{
    QString username = ui->lineEdit_account->text();
    QString password = ui->lineEdit_password->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "登录失败", "账号和密码不能为空");
        return;
    }
    


    QMessageBox::information(this, "登录成功", "登录成功！");
    
    // 禁用登录相关控件
    ui->lineEdit_account->setEnabled(false);
    ui->lineEdit_password->setEnabled(false);
    ui->pushButton_login->setEnabled(false);
}

void MainWindow::on_pushButton_sent_clicked()
{
    QString message = ui->lineEdit_3->text();
    
    if (message.isEmpty()) {
        return;
    }
    
    if (m_currentRecipient.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "请先选择一个联系人");
        return;
    }
    

    m_backend->sendTextMessage(m_currentRecipient, message);
    
    // 清空输入框
    ui->lineEdit_3->clear();
    
    // 在聊天记录中显示发送的消息
    ui->listWidget_2->addItem(QString("我: %1").arg(message));
    
    // 模拟收到回复
   // ui->listWidget_2->addItem(QString("%1: 收到您的消息：'%2'").arg(m_currentRecipient, message));
}

