#include "tcpclient.h"
#include "packet.h" // Include the Packet structure

#include <QDebug>
#include <QDataStream>
#include <QRandomGenerator>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this)),
    clientSequenceNumber(0), serverSequenceNumber(0)
{
    connect(socket, &QTcpSocket::connected, this, &TcpClient::connected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::readyRead);
}

void TcpClient::connectToServer()
{
    qDebug() << "Connecting to server...";
    socket->connectToHost("localhost", 1234);
}

void TcpClient::sendSyn()
{
    qDebug() << "Sending SYN message to server";

    quint8 flags = 0b00000001; // Set SYN flag
    quint16 maxSegmentSize = 100; // Example: maximum segment size
    quint16 windowSize = 1000; // Example: window size

    // Generate a 32-bit random sequence number for client
    clientSequenceNumber = QRandomGenerator::global()->generate();

    Packet synPacket(flags, clientSequenceNumber, maxSegmentSize, windowSize);
    socket->write(synPacket.serialize());
}

void TcpClient::connected()
{
    qDebug() << "Connected to server";
    sendSyn(); // Send SYN message upon connection
}

void TcpClient::disconnected()
{
    qDebug() << "Disconnected from server";
}

void TcpClient::readyRead()
{
    while (socket->bytesAvailable() > 0) {
        QByteArray receivedData = socket->readAll();
        Packet receivedPacket = Packet::deserialize(receivedData);

        if (receivedPacket.flags == 0b00000011) { // Check if SYN-ACK flag is set
            qDebug() << "Received SYN-ACK from server";
            serverSequenceNumber = receivedPacket.sequenceNumber;

            // Send ACK to server to complete the handshake
            sendAck();
        } else if (receivedPacket.flags == 0b11111100) { // Check if ACK flag is set
            qDebug() << "Received ACK from server";
            // Connection is established, send data
            qDebug() << "Connection is established.";
            QString message = "Hello Server!";
            qDebug() << "Data is: " << message;
            sendData(message);
        } else if (receivedPacket.flags == 0b11111110) { // Data ACK
            qDebug() << "Received ACK from server for segment with sequence number" << receivedPacket.sequenceNumber;
        }
    }
}

void TcpClient::sendAck()
{
    qDebug() << "Sending ACK to server";
    quint8 flags = 0b00000010; // Set ACK flag

    Packet ackPacket(flags, serverSequenceNumber + 1, 0, 0); // dummy maxSegmentSize and windowSize
    socket->write(ackPacket.serialize());
}

void TcpClient::sendData(const QString &message)
{
    int maxSegmentSize = 100; // Example segment size
    QByteArray data = message.toUtf8();
    int totalSegments = (data.size() + maxSegmentSize - 1) / maxSegmentSize;

    for (int i = 0; i < totalSegments; ++i) {
        int segmentSize = qMin(maxSegmentSize, data.size() - i * maxSegmentSize);
        QByteArray segment = data.mid(i * maxSegmentSize, segmentSize);

        qDebug() << "Sending data segment" << i + 1 << "/" << totalSegments << ":" << QString::fromUtf8(segment);
        sendSegment(segment);
    }
}

QByteArray generateChecksum(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

void TcpClient::sendSegment(const QByteArray &segment)
{
    quint8 flags = 0b11111100; // Use a specific flag to denote data segments

    clientSequenceNumber++; // Increment client sequence number for sending each segment
    QByteArray checksum = generateChecksum(segment);
    Packet dataPacket(flags, clientSequenceNumber, 0, 0, segment, checksum); // dummy maxSegmentSize and windowSize
    socket->write(dataPacket.serialize());
}
