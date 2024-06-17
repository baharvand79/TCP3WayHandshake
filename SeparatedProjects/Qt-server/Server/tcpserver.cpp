#include "tcpserver.h"
#include <QTcpSocket>
#include <QDebug>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
    // Start listening on all available network interfaces and port 1234
    if (!listen(QHostAddress::LocalHost, 1234)) {
        qDebug() << "Error: Unable to start server.";
    } else {
        qDebug() << "Server started. Listening on port 1234.";
    }
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "Incoming connection";

    // Create a new socket for the incoming connection
    QTcpSocket *clientSocket = new QTcpSocket(this);

    // Set up the socket with the incoming socket descriptor
    if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "Error setting socket descriptor";
        return;
    }

    // Connect signals/slots for communication
    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::readData);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);

    // Add the client socket to the list
    clients.append(clientSocket);
}

void TcpServer::readData()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    QByteArray data = socket->readAll();
    qDebug() << "Data from client:" << data;

    // Example of broadcasting data to all connected clients
    for (QTcpSocket *client : clients) {
        if (client != socket) {
            client->write(data);
        }
    }
}
