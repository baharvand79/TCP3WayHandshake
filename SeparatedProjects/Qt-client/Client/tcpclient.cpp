#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::connected, this, &TcpClient::connected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::readyRead);
}

void TcpClient::connectToServer()
{
    // Connect to the server on localhost and port 1234
    socket->connectToHost("localhost", 1234);
}

void TcpClient::connected()
{
    qDebug() << "Connected to server";

    // Example: send a message to the server
    sendMessage("Hello, server!");
}

void TcpClient::disconnected()
{
    qDebug() << "Disconnected from server";
}

void TcpClient::readyRead()
{
    QByteArray data = socket->readAll();
    qDebug() << "Data from server:" << data;
}

void TcpClient::sendMessage(const QString &message)
{
    // Example: send a message to the server
    socket->write(message.toUtf8());
}
