#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readyRead();
    void processMessage(QTcpSocket *clientSocket, const QByteArray &message);

private:
    void sendAck(QTcpSocket *clientSocket);

private:
    QMap<QTcpSocket*, qint32> expectedSequenceNumber;
};

#endif // TCPSERVER_H
