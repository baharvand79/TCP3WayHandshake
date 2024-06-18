#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QCryptographicHash>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    void connectToServer();

public slots:
    void connected();
    void disconnected();
    void readyRead();
    void sendAck();
    void sendData(const QString &message);
    void sendSegment(const QByteArray &segment);
    void sendPeriodicMessages();

private:
    QTcpSocket *socket;
    quint32 clientSequenceNumber;
    quint32 serverSequenceNumber;
    QTimer timer;
    int messageCount;

    void sendSyn();
};

#endif // TCPCLIENT_H
