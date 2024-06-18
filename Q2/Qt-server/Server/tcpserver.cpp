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
