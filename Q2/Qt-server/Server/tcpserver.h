#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QMap>
#include "packet.h"
#include <QCryptographicHash>
class ClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClientHandler(qintptr socketDescriptor, QObject *parent = nullptr);
    ~ClientHandler();

public slots:
    void readyRead();
    void disconnected();

signals:
    void error(QTcpSocket::SocketError socketError);
    void finished();

private:
    QTcpSocket *clientSocket;
    quint32 clientSequenceNumber;
    quint32 serverSequenceNumber;
    QMap<QTcpSocket*, quint32> clientSequenceNumbers;
    QMap<QTcpSocket*, quint32> serverSequenceNumbers;

    void processPacket(const Packet &receivedPacket);
    void sendSynAck();
    void sendAckForData(quint32 sequenceNumber);
    void sendAckForEstablishment(quint32 sequenceNumber);
    QByteArray generateChecksum(const QByteArray &data);
    bool validatePacket(const Packet &packet);
};

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

};

#endif // TCPSERVER_H
