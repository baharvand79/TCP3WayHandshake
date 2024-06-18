// #include "tcpserver.h"
// #include <QDebug>
// #include <QDataStream>
// #include <QRandomGenerator>

// TcpServer::TcpServer(QObject *parent)
//     : QTcpServer(parent)
// {
//     if (!listen(QHostAddress::Any, 1234)) {
//         qCritical() << "Unable to start the server:" << errorString();
//     } else {
//         qDebug() << "Server started. Listening on port" << serverPort();
//     }
// }

// void TcpServer::incomingConnection(qintptr socketDescriptor)
// {
//     qDebug() << "Incoming connection";

//     QTcpSocket *clientSocket = new QTcpSocket(this);
//     if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
//         qWarning() << "Error setting socket descriptor";
//         return;
//     }

//     clientSequenceNumbers.insert(clientSocket, 0);
//     serverSequenceNumbers.insert(clientSocket, 0);

//     connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::readyRead);
// }

// void TcpServer::readyRead()
// {
//     QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
//     if (!clientSocket)
//         return;

//     QDataStream in(clientSocket);
//     in.setVersion(QDataStream::Qt_5_15);

//     while (!in.atEnd()) {
//         quint8 flags;
//         quint32 sequenceNumber;
//         quint16 maxSegmentSize;
//         quint16 windowSize;
//         QByteArray data;

//         in >> flags >> sequenceNumber >> maxSegmentSize >> windowSize;

//         if (flags & 0b00000001) { // Check if SYN flag is set
//             qDebug() << "Received SYN from client";

//             clientSequenceNumbers[clientSocket] = sequenceNumber;
//             qDebug() << "Client Sequence Number:" << sequenceNumber;

//             // Send SYN-ACK in response to client's SYN
//             qDebug() << "Sending SYN-ACK to client";
//             QDataStream out(clientSocket);
//             out.setVersion(QDataStream::Qt_5_15);

//             flags = 0b00000011; // Set SYN and ACK flags
//             quint32 serverSequenceNumber = QRandomGenerator::global()->generate();
//             serverSequenceNumbers[clientSocket] = serverSequenceNumber;
//             out << flags << serverSequenceNumber << maxSegmentSize << windowSize;

//             // Now we wait for ACK from client to complete the handshake
//         } else if (flags & 0b00000010) { // Check if ACK flag is set
//             qDebug() << "Received ACK from client";
//             // Connection is established, server can start processing data if needed
//             qDebug() << "Connection established";

//             QDataStream out(clientSocket);
//             out.setVersion(QDataStream::Qt_5_15);

//             flags = 0b11111100; // Set SYN and ACK flags

//             out << flags << sequenceNumber << maxSegmentSize << windowSize;

//         } else { // Handle data segments
//             qDebug() << "Received data from client";
//             in >> data;
//             qDebug() << "Data:" << QString::fromUtf8(data);

//             // Send acknowledgment for the data packet
//             //sendAckForData(clientSocket, sequenceNumber);
//         }
//     }
// }

// void TcpServer::sendAckForData(QTcpSocket *clientSocket, quint32 sequenceNumber)
// {
//     qDebug() << "Sending ACK for data packet";
//     QDataStream out(clientSocket);
//     out.setVersion(QDataStream::Qt_5_15);

//     quint8 flags = 0b00000010; // Set ACK flag

//     // ACK acknowledges the data packet received from client
//     out << flags << sequenceNumber << quint16(0) << quint16(0); // Add dummy values for maxSegmentSize and windowSize
// }
#include "tcpserver.h"
#include "packet.h" // Include the Packet structure

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

    while (clientSocket->bytesAvailable() > 0) {
        QByteArray receivedData = clientSocket->readAll();
        Packet receivedPacket = Packet::deserialize(receivedData);

        if (receivedPacket.flags & 0b00000001) { // Check if SYN flag is set
            qDebug() << "Received SYN from client";

            clientSequenceNumbers[clientSocket] = receivedPacket.sequenceNumber;
            qDebug() << "Client Sequence Number:" << receivedPacket.sequenceNumber;

            // Send SYN-ACK in response to client's SYN
            sendSynAck(clientSocket);
        } else if (receivedPacket.flags & 0b00000010) { // Check if ACK flag is set
            qDebug() << "Received ACK from client";
            // Connection is established, server can start processing data if needed
            qDebug() << "Connection established";

            sendAckForEstablishment(clientSocket, receivedPacket.sequenceNumber);

            // Handle ACK for SYN or data
        } else { // Handle data segments
            qDebug() << "Received data from client";
            qDebug() << "Data:" << QString::fromUtf8(receivedPacket.data);

            // Send acknowledgment for the data packet
            //sendAckForData(clientSocket, receivedPacket.sequenceNumber);
        }
    }
}

void TcpServer::sendSynAck(QTcpSocket *clientSocket)
{
    qDebug() << "Sending SYN-ACK to client";
    quint8 flags = 0b00000011; // Set SYN and ACK flags

    quint32 serverSequenceNumber = QRandomGenerator::global()->generate();
    serverSequenceNumbers[clientSocket] = serverSequenceNumber;

    Packet synAckPacket(flags, serverSequenceNumber, 100, 1000); // Example maxSegmentSize and windowSize
    clientSocket->write(synAckPacket.serialize());
}

void TcpServer::sendAckForEstablishment(QTcpSocket *clientSocket, quint32 sequenceNumber)
{
    qDebug() << "Sending ACK for data packet";
    quint8 flags = 0b11111100; // Set ACK flag

    Packet ackPacket(flags, sequenceNumber, 0, 0); // dummy maxSegmentSize and windowSize
    clientSocket->write(ackPacket.serialize());
}
