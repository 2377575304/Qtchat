#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QDataStream>
#include <QString>

// 消息类型枚举
enum MessageType {
    LoginRequest = 0,    // 登录请求
    LoginResponse = 1,   // 登录响应
    TextMessage = 2,     // 文本消息
    ErrorMessage = 3     // 错误消息
};

// 登录响应结果
enum LoginResult {
    LoginSuccess = 0,    // 登录成功
    LoginFailed = 1,     // 登录失败
    ServerError = 2      // 服务器错误
};

class backend : public QObject
{
    Q_OBJECT
public:
    explicit backend(QObject *parent = nullptr);

    // 连接到服务器
    void connectToServer(const QString& host, quint16 port);

    // 断开连接
    void disconnectFromServer();

    // 登录到服务器
    void login(const QString& username, const QString& password);

    // 发送文本消息
    void sendTextMessage(const QString& recipient, const QString& message);

    // 检查连接状态
    bool isConnected() const;

    // 检查登录状态
    bool isLoggedIn() const;

signals:
    // 连接状态变化
    void connected();
    void disconnected();

    // 登录结果
    void loginResultReceived(bool success, const QString& message);

    // 收到消息
    void textMessageReceived(const QString& sender, const QString& message);

    // 错误信息
    void errorOccurred(const QString& errorString);

private slots:
    // 处理socket状态变化
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

    // 处理接收到的数据
    void onReadyRead();

private:
    // 发送消息的通用方法
    void sendMessage(MessageType type, const QByteArray& data);

    // 解析接收到的消息
    void parseMessage(MessageType type, const QByteArray& data);

    QTcpSocket* m_socket;      // TCP socket
    bool m_isLoggedIn;         // 登录状态
    QString m_username;        // 当前登录用户名
};

#endif // BACKEND_H
