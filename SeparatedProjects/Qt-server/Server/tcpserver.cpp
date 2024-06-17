#include "tcpserver.h"
#include <QDebug>
#include <QDataStream>
#include <QRandomGenerator>

TcpServer::TcpServer(QObject *parent)
    : QTcpServer(parent)
{
    if (!listen(QHostAddress::Any, 1234)) {
        qCritical() << "Unable to start the server:" << errorString();
    } else {
        qDebug() << "Server started. Listening on port" << serverPort();
    }
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "Incoming connection";

    QTcpSocket *clientSocket = new QTcpSocket(this);
    if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
        qWarning() << "Error setting socket descriptor";
        return;
    }

    clientSequenceNumbers.insert(clientSocket, 0);
    serverSequenceNumbers.insert(clientSocket, 0);

    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::readyRead);
}

void TcpServer::readyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_5_15);

    while (!in.atEnd()) {
        quint8 flags;
        quint32 sequenceNumber;
        quint16 maxSegmentSize;
        quint16 windowSize;
        QByteArray data;

        in >> flags >> sequenceNumber >> maxSegmentSize >> windowSize;

        if (flags & 0b00000001) { // Check if SYN flag is set
            qDebug() << "Received SYN from client";

            clientSequenceNumbers[clientSocket] = sequenceNumber;
            qDebug() << "Client Sequence Number:" << sequenceNumber;

            // Send SYN-ACK in response to client's SYN
            qDebug() << "Sending SYN-ACK to client";
            QDataStream out(clientSocket);
            out.setVersion(QDataStream::Qt_5_15);

            flags = 0b00000011; // Set SYN and ACK flags
            quint32 serverSequenceNumber = QRandomGenerator::global()->generate();
            serverSequenceNumbers[clientSocket] = serverSequenceNumber;
            out << flags << serverSequenceNumber << maxSegmentSize << windowSize;

            // Now we wait for ACK from client to complete the handshake
        } else if (flags & 0b00000010) { // Check if ACK flag is set
            qDebug() << "Received ACK from client";
            // Connection is established, server can start processing data if needed
            qDebug() << "Connection established";
        } else {
            // Handle data segments
            qDebug() << "Received data from client";
            in >> data;
            qDebug() << "Data:" << QString::fromUtf8(data);
        }
    }
}
