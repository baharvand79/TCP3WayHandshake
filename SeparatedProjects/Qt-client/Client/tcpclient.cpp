#include "tcpclient.h"
#include "packet.h"

#include <QDebug>
#include <QDataStream>
#include <QRandomGenerator>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent), socket(new QTcpSocket(this)),
    clientSequenceNumber(0), serverSequenceNumber(0), messageCount(0)
{
    connect(socket, &QTcpSocket::connected, this, &TcpClient::connected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpClient::disconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpClient::readyRead);
    connect(&timer, &QTimer::timeout, this, &TcpClient::sendPeriodicMessages);
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

    clientSequenceNumber = QRandomGenerator::global()->generate();

    Packet synPacket(flags, clientSequenceNumber, maxSegmentSize, windowSize);
    socket->write(synPacket.serialize());
}

void TcpClient::connected()
{
    qDebug() << "Connected to server";
    sendSyn();
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

        if (receivedPacket.flags == 0b00000011) {
            qDebug() << "Received SYN-ACK from server";
            serverSequenceNumber = receivedPacket.sequenceNumber;

            sendAck();
        } else if (receivedPacket.flags == 0b11111100) {
            qDebug() << "Received ACK from server";
            qDebug() << "Connection is established.";
            timer.start(5000); // Start sending messages every 5 seconds
        } else if (receivedPacket.flags == 0b11111110) {
            qDebug() << "Received ACK from server for segment with sequence number" << receivedPacket.sequenceNumber;
        }
    }
}

void TcpClient::sendAck()
{
    qDebug() << "Sending ACK to server";
    quint8 flags = 0b00000010;

    Packet ackPacket(flags, serverSequenceNumber + 1, 0, 0);
    socket->write(ackPacket.serialize());
}

void TcpClient::sendData(const QString &message)
{
    int maxSegmentSize = 100;
    QByteArray data = message.toUtf8();
    int totalSegments = (data.size() + maxSegmentSize - 1) / maxSegmentSize;

    for (int i = 0; i < totalSegments; ++i) {
        int segmentSize = qMin(maxSegmentSize, data.size() - i * maxSegmentSize);
        QByteArray segment = data.mid(i * maxSegmentSize, segmentSize);

        qDebug() << "Sending data segment" << i + 1 << "/" << totalSegments << ":" << QString::fromUtf8(segment);
        sendSegment(segment);
    }
}

void TcpClient::sendSegment(const QByteArray &segment)
{
    quint8 flags = 0b11111100;

    clientSequenceNumber++;
    QByteArray checksum = QCryptographicHash::hash(segment, QCryptographicHash::Md5);
    Packet dataPacket(flags, clientSequenceNumber, 0, 0, segment, checksum);
    socket->write(dataPacket.serialize());
}

void TcpClient::sendPeriodicMessages()
{
    if (messageCount < 2) {
        sendData("Hello Server!");
        messageCount++;
    } else {
        sendData("This is a long message that should be split into segments. This is a long message that should be split into segments. This is a long message that should be split into segments. This is a long message that should be split into segments.");
        timer.stop();
        QTimer::singleShot(5000, socket, &QTcpSocket::disconnectFromHost);
    }

}
