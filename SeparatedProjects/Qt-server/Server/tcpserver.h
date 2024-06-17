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

private:
    QMap<QTcpSocket*, quint32> clientSequenceNumbers;
    QMap<QTcpSocket*, quint32> serverSequenceNumbers;
};

#endif // TCPSERVER_H
