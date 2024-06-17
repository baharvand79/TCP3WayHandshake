#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

public slots:
    void connectToServer();
    void sendData(const QString &message); // New method to send data

private slots:
    void sendSyn();
    void connected();
    void disconnected();
    void readyRead();
    void sendAck();

private:
    QTcpSocket *socket;
    quint32 clientSequenceNumber;
    quint32 serverSequenceNumber;
};

#endif // TCPCLIENT_H
