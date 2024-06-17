#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QRandomGenerator>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);

public slots:
    void connectToServer();

private slots:
    void connected();
    void disconnected();
    void readyRead();
    void sendAck();
    void sendSyn();

private:
    QTcpSocket *socket;
    quint32 clientSequenceNumber;
    quint32 serverSequenceNumber;
};

#endif // TCPCLIENT_H
