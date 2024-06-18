#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = nullptr);



protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readyRead();
    //void sendAck(QTcpSocket *clientSocket, quint32 sequenceNumber);
    //void sendAckForData(QTcpSocket *clientSocket, quint32 sequenceNumber);
    void sendSynAck(QTcpSocket *clientSocket);
    void sendAckForEstablishment(QTcpSocket *clientSocket, quint32 sequenceNumber);

private:
    QMap<QTcpSocket*, quint32> clientSequenceNumbers;
    QMap<QTcpSocket*, quint32> serverSequenceNumbers;
};

#endif // TCPSERVER_H
