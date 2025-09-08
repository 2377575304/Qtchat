#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "backend/backend.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_sent_clicked();
    


private:
    Ui::MainWindow *ui;
    backend *m_backend;
    QString m_currentUser;  // 当前登录用户
    QString m_currentRecipient;  // 当前聊天对象
};
#endif // MAINWINDOW_H
