// #include "tcpserver.h"
// #include "packet.h" // Include the Packet structure

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

//     while (clientSocket->bytesAvailable() > 0) {
//         QByteArray receivedData = clientSocket->readAll();
//         Packet receivedPacket = Packet::deserialize(receivedData);

//         if (receivedPacket.flags & 0b00000001) { // SYN packet
//             qDebug() << "Received SYN from client";

//             // Process initial SYN (if necessary), not shown in the original code

//             // Respond with SYN-ACK
//             sendSynAck(clientSocket);
//         } else if (receivedPacket.flags & 0b00000010) { // ACK packet
//             qDebug() << "Received ACK from client";

//             // Process ACK for connection establishment
//             sendAckForEstablishment(clientSocket, receivedPacket.sequenceNumber);
//             // At this point, connection is established, you may start processing data
//             // or further communications depending on your application logic.
//             // Example: Start processing data if needed

//         } else if (receivedPacket.flags & 0b11111100) { // Data packet
//             qDebug() << "Received data from client with sequence number" << receivedPacket.sequenceNumber;

//             // Validate the packet (sequence number, checksum, etc.)
//             if (validatePacket(receivedPacket)) {
//                 // Process the data
//                 qDebug() << "Data:" << QString::fromUtf8(receivedPacket.data);

//                 // Example: Acknowledge the data packet
//                 sendAckForData(clientSocket, receivedPacket.sequenceNumber);
//             } else {
//                 qDebug() << "Invalid packet received, ignoring.";
//                 // Handle invalid packet scenario if needed
//             }
//         } else if (receivedPacket.flags & 0b11111110) { // Data ACK packet
//             qDebug() << "Received ACK for data segment with sequence number" << receivedPacket.sequenceNumber;

//             // Process acknowledgment for the data segment (if needed)
//         } else {
//             qDebug() << "Unknown packet received, ignoring."; // Handle unknown packets
//         }
//     }
// }

// void TcpServer::sendSynAck(QTcpSocket *clientSocket)
// {
//     qDebug() << "Sending SYN-ACK to client";
//     quint8 flags = 0b00000011; // Set SYN and ACK flags

//     quint32 serverSequenceNumber = QRandomGenerator::global()->generate();
//     serverSequenceNumbers[clientSocket] = serverSequenceNumber;

//     Packet synAckPacket(flags, serverSequenceNumber, 100, 1000); // Example maxSegmentSize and windowSize
//     clientSocket->write(synAckPacket.serialize());
// }

// void TcpServer::sendAckForData(QTcpSocket *clientSocket, quint32 sequenceNumber)
// {
//     qDebug() << "Sending ACK for data segment with sequence number" << sequenceNumber;
//     quint8 flags = 0b11111110; // Set ACK flag

//     Packet ackPacket(flags, sequenceNumber, 0, 0); // dummy maxSegmentSize and windowSize
//     clientSocket->write(ackPacket.serialize());
// }
// void TcpServer::sendAckForEstablishment(QTcpSocket *clientSocket, quint32 sequenceNumber)
// {
//     qDebug() << "Sending ACK for data packet";
//     quint8 flags = 0b11111100; // Set ACK flag

//     Packet ackPacket(flags, sequenceNumber, 0, 0); // dummy maxSegmentSize and windowSize
//     clientSocket->write(ackPacket.serialize());
// }
// QByteArray TcpServer::generateChecksum(const QByteArray &data)
// {
//     return QCryptographicHash::hash(data, QCryptographicHash::Md5);
// }
// bool TcpServer::validatePacket(const Packet &packet)
// {
//     QByteArray newChecksum = generateChecksum(packet.data);
//     if (newChecksum == packet.checksum){
//         return true;
//     }
//     return false;
// }
#include "tcpserver.h"
#include "packet.h"

#include <QDebug>
#include <QDataStream>
#include <QRandomGenerator>

ClientHandler::ClientHandler(qintptr socketDescriptor, QObject *parent)
    : QObject(parent), clientSequenceNumber(0), serverSequenceNumber(0)
{
    clientSocket = new QTcpSocket(this);
    if (!clientSocket->setSocketDescriptor(socketDescriptor)) {
        emit error(clientSocket->error());
        return;
    }

    connect(clientSocket, &QTcpSocket::readyRead, this, &ClientHandler::readyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &ClientHandler::disconnected);
}

ClientHandler::~ClientHandler()
{
    clientSocket->deleteLater();
}

void ClientHandler::readyRead()
{
    while (clientSocket->bytesAvailable() > 0) {
        QByteArray receivedData = clientSocket->readAll();
        Packet receivedPacket = Packet::deserialize(receivedData);
        processPacket(receivedPacket);
    }
}

void ClientHandler::disconnected()
{
    emit finished();
}

void ClientHandler::processPacket(const Packet &receivedPacket)
{
    if (receivedPacket.flags & 0b00000001) { // SYN packet
        qDebug() << "Received SYN from client";
        sendSynAck();
    } else if (receivedPacket.flags & 0b00000010) { // ACK packet
        qDebug() << "Received ACK from client";
        sendAckForEstablishment(receivedPacket.sequenceNumber);
    } else if (receivedPacket.flags & 0b11111100) { // Data packet
        qDebug() << "Received data from client with sequence number" << receivedPacket.sequenceNumber;
        if (validatePacket(receivedPacket)) {
            qDebug() << "Data:" << QString::fromUtf8(receivedPacket.data);
            sendAckForData(receivedPacket.sequenceNumber);
        } else {
            qDebug() << "Invalid packet received, ignoring.";
        }
    } else if (receivedPacket.flags & 0b11111110) { // Data ACK packet
        qDebug() << "Received ACK for data segment with sequence number" << receivedPacket.sequenceNumber;
    } else {
        qDebug() << "Unknown packet received, ignoring.";
    }
}

void ClientHandler::sendSynAck()
{
    qDebug() << "Sending SYN-ACK to client";
    quint8 flags = 0b00000011;

    quint32 serverSequenceNumber = QRandomGenerator::global()->generate();
    serverSequenceNumbers[clientSocket] = serverSequenceNumber;

    Packet synAckPacket(flags, serverSequenceNumber, 100, 1000);
    clientSocket->write(synAckPacket.serialize());
}

void ClientHandler::sendAckForData(quint32 sequenceNumber)
{
    qDebug() << "Sending ACK for data segment with sequence number" << sequenceNumber;
    quint8 flags = 0b11111110;

    Packet ackPacket(flags, sequenceNumber, 0, 0);
    clientSocket->write(ackPacket.serialize());
}

void ClientHandler::sendAckForEstablishment(quint32 sequenceNumber)
{
    qDebug() << "Sending ACK for data packet";
    quint8 flags = 0b11111100;

    Packet ackPacket(flags, sequenceNumber, 0, 0);
    clientSocket->write(ackPacket.serialize());
}

QByteArray ClientHandler::generateChecksum(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

bool ClientHandler::validatePacket(const Packet &packet)
{
    QByteArray newChecksum = generateChecksum(packet.data);
    return newChecksum == packet.checksum;
}

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
    QThread *thread = new QThread;
    ClientHandler *clientHandler = new ClientHandler(socketDescriptor);
    clientHandler->moveToThread(thread);

    connect(thread, &QThread::started, clientHandler, &ClientHandler::readyRead);
    connect(clientHandler, &ClientHandler::finished, thread, &QThread::quit);
    connect(clientHandler, &ClientHandler::finished, clientHandler, &ClientHandler::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(clientHandler, &ClientHandler::error, this, [=](QTcpSocket::SocketError socketError) {
        qDebug() << "ClientHandler error:" << socketError;
    });

    thread->start();
}
