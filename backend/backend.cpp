#include "backend.h"
#include <QDebug>

backend::backend(QObject *parent) : QObject(parent), m_isLoggedIn(false)
{
    // 初始化TCP socket
    m_socket = new QTcpSocket(this);

    // 连接信号槽
    connect(m_socket, &QTcpSocket::connected, this, &backend::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &backend::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &backend::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &backend::onSocketError);
}

void backend::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }

    // 连接到服务器
    m_socket->connectToHost(host, port);
}

void backend::disconnectFromServer()
{
    if (m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->disconnectFromHost();
        m_isLoggedIn = false;
        m_username.clear();
    }
}

void backend::login(const QString &username, const QString &password)
{
    if (m_socket->state() != QTcpSocket::ConnectedState) {
        emit errorOccurred("未连接到服务器，请先连接");
        return;
    }

    // 准备登录数据
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << username << password;

    // 发送登录请求
    sendMessage(LoginRequest, data);

    // 暂存用户名，登录成功后确认
    m_username = username;
}

void backend::sendTextMessage(const QString &recipient, const QString &message)
{

    // 构造纯文本格式的消息，与服务端兼容
    QString formattedMessage = recipient + ":" + message + "\0";

    // 直接发送文本数据，不使用QDataStream序列化
    QByteArray data = formattedMessage.toUtf8();
    m_socket->write(data);
    m_socket->flush();
}
bool backend::isConnected() const
{
    return m_socket->state() == QTcpSocket::ConnectedState;
}

bool backend::isLoggedIn() const
{
    return m_isLoggedIn;
}

void backend::onSocketConnected()
{
    emit connected();
    qDebug() << "已连接到服务器";
}

void backend::onSocketDisconnected()
{
    m_isLoggedIn = false;
    m_username.clear();
    emit disconnected();
    qDebug() << "已与服务器断开连接";
}

void backend::onSocketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    emit errorOccurred(m_socket->errorString());
    qDebug() << "Socket错误:" << m_socket->errorString();
}

void backend::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        // 读取消息头（消息类型 + 数据长度）
        if (m_socket->bytesAvailable() < (int)sizeof(quint8) + sizeof(quint32)) {
            return; // 数据不完整，等待更多数据
        }

        quint8 type;
        quint32 dataSize;

        QDataStream stream(m_socket);
        stream >> type >> dataSize;

        // 检查是否有足够的数据
        if (m_socket->bytesAvailable() < dataSize) {
            // 数据不足，将指针移回，等待更多数据
            m_socket->seek(m_socket->pos() - sizeof(quint8) - sizeof(quint32));
            return;
        }

        // 读取消息数据
        QByteArray data = m_socket->read(dataSize);

        // 解析消息
        parseMessage((MessageType)type, data);
    }
}

void backend::sendMessage(MessageType type, const QByteArray &data)
{
    if (m_socket->state() != QTcpSocket::ConnectedState) {
        emit errorOccurred("未连接到服务器，无法发送消息");
        return;
    }

    // 构建消息：类型(1字节) + 数据长度(4字节) + 数据
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);

    stream << (quint8)type;          // 消息类型
    stream << (quint32)data.size();  // 数据长度
    stream.writeRawData(data.data(), data.size());  // 数据内容

    // 发送消息
    m_socket->write(message);
}

void backend::parseMessage(MessageType type, const QByteArray &data)
{
    QDataStream stream(data);

    switch (type) {
    case LoginResponse: {
        quint8 result;
        QString message;
        stream >> result >> message;

        m_isLoggedIn = (result == LoginSuccess);
        emit loginResultReceived(m_isLoggedIn, message);
        break;
    }
    case TextMessage: {
        QString sender;
        QString message;
        stream >> sender >> message;

        emit textMessageReceived(sender, message);
        break;
    }
    case ErrorMessage: {
        QString error;
        stream >> error;
        emit errorOccurred(error);
        break;
    }
    default:
        emit errorOccurred(QString("收到未知类型的消息: %1").arg(type));
        break;
    }
}
