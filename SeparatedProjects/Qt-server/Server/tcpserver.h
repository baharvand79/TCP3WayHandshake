// tcpserver.h
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "packet.h" // Include the Packet structure
#include <QCryptographicHash>
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readyRead();
    QByteArray generateChecksum(const QByteArray &data);

private:
    QMap<QTcpSocket*, quint32> clientSequenceNumbers;
    QMap<QTcpSocket*, quint32> serverSequenceNumbers;
    QMap<QTcpSocket*, Packet> lastAckedPacket;

    bool validatePacket(const Packet &packet);
    void sendSynAck(QTcpSocket *clientSocket);
    void sendAckForEstablishment(QTcpSocket *clientSocket, quint32 sequenceNumber);
    void sendAckForData(QTcpSocket *clientSocket, quint32 sequenceNumber);
    void sendDuplicateAck(QTcpSocket *clientSocket);
};

#endif // TCPSERVER_H
