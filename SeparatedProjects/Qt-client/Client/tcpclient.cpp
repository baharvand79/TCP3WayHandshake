#include "tcpclient.h"
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

    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);

    quint8 flags = 0b00000001; // Set SYN flag
    quint16 maxSegmentSize = 100; // Example: maximum segment size
    quint16 windowSize = 1000; // Example: window size

    // Generate a 32-bit random sequence number for client
    clientSequenceNumber = QRandomGenerator::global()->generate();

    out << flags << clientSequenceNumber << maxSegmentSize << windowSize;
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
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);

    while (!in.atEnd()) {
        quint8 flags;
        quint32 sequenceNumber;
        quint16 maxSegmentSize;
        quint16 windowSize;
        QByteArray data;

        in >> flags >> sequenceNumber >> maxSegmentSize >> windowSize;

        if (flags & 0b00000011) { // Check if SYN-ACK flag is set
            qDebug() << "Received SYN-ACK from server";
            serverSequenceNumber = sequenceNumber;

            // Send ACK to server to complete the handshake
            sendAck();
        } else if (flags & 0b00000010) { // Check if ACK flag is set
            qDebug() << "Received ACK from server";
            // Now connection is established and we can send data

        } else { // Handle data segments
            // qDebug() << "Received data from server";
            // in >> data;
            // qDebug() << "Data:" << QString::fromUtf8(data);

            qDebug() << "Sending Data\n";
            sendData("Hello from client!"); // Example message
            // If data is received, send an acknowledgment
            //sendAckForData(sequenceNumber);
        }
    }
}

void TcpClient::sendAck()
{
    qDebug() << "Sending ACK to server";
    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);

    quint8 flags = 0b00000010; // Set ACK flag

    // ACK acknowledges the SYN-ACK received from server
    out << flags << (serverSequenceNumber + 1) << quint16(0) << quint16(0); // Add dummy values for maxSegmentSize and windowSize
}

// void TcpClient::sendAckForData(quint32 sequenceNumber)
// {
//     qDebug() << "Sending ACK for data packet";
//     QDataStream out(socket);
//     out.setVersion(QDataStream::Qt_5_15);

//     quint8 flags = 0b00000010; // Set ACK flag

//     // ACK acknowledges the data packet received from server
//     out << flags << sequenceNumber << quint16(0) << quint16(0); // Add dummy values for maxSegmentSize and windowSize
// }

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

void TcpClient::sendSegment(const QByteArray &segment)
{
    QDataStream out(socket);
    out.setVersion(QDataStream::Qt_5_15);

    quint8 flags = 0b11111100; // Use a specific flag to denote data segments
    out << flags << clientSequenceNumber << quint16(0) << quint16(0) << segment;

    clientSequenceNumber++; // Increment client sequence number after sending each segment
}
