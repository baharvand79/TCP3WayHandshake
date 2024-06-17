#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void readData();

private:
    QList<QTcpSocket *> clients;
};

#endif // TCPSERVER_H
