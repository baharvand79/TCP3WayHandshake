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

        if (receivedPacket.flags & 0b00000001) { // SYN packet
            qDebug() << "Received SYN from client";

            // Process initial SYN (if necessary), not shown in the original code

            // Respond with SYN-ACK
            sendSynAck(clientSocket);
        } else if (receivedPacket.flags & 0b00000010) { // ACK packet
            qDebug() << "Received ACK from client";

            // Process ACK for connection establishment
            sendAckForEstablishment(clientSocket, receivedPacket.sequenceNumber);
            // At this point, connection is established, you may start processing data
            // or further communications depending on your application logic.
            // Example: Start processing data if needed

        } else if (receivedPacket.flags & 0b11111100) { // Data packet
            qDebug() << "Received data from client with sequence number" << receivedPacket.sequenceNumber;

            // Validate the packet (sequence number, checksum, etc.)
            if (validatePacket(receivedPacket)) {
                // Process the data
                qDebug() << "Data:" << QString::fromUtf8(receivedPacket.data);

                // Example: Acknowledge the data packet
                sendAckForData(clientSocket, receivedPacket.sequenceNumber);
            } else {
                qDebug() << "Invalid packet received, ignoring.";
                // Handle invalid packet scenario if needed
            }
        } else if (receivedPacket.flags & 0b11111110) { // Data ACK packet
            qDebug() << "Received ACK for data segment with sequence number" << receivedPacket.sequenceNumber;

            // Process acknowledgment for the data segment (if needed)
        } else {
            qDebug() << "Unknown packet received, ignoring."; // Handle unknown packets
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

void TcpServer::sendAckForData(QTcpSocket *clientSocket, quint32 sequenceNumber)
{
    qDebug() << "Sending ACK for data segment with sequence number" << sequenceNumber;
    quint8 flags = 0b11111110; // Set ACK flag

    Packet ackPacket(flags, sequenceNumber, 0, 0); // dummy maxSegmentSize and windowSize
    clientSocket->write(ackPacket.serialize());
}
void TcpServer::sendAckForEstablishment(QTcpSocket *clientSocket, quint32 sequenceNumber)
{
    qDebug() << "Sending ACK for data packet";
    quint8 flags = 0b11111100; // Set ACK flag

    Packet ackPacket(flags, sequenceNumber, 0, 0); // dummy maxSegmentSize and windowSize
    clientSocket->write(ackPacket.serialize());
}
QByteArray TcpServer::generateChecksum(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}
bool TcpServer::validatePacket(const Packet &packet)
{
    QByteArray newChecksum = generateChecksum(packet.data);
    if (newChecksum == packet.checksum){
        return true;
    }
    return false;
}
