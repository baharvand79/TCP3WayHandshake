#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QCryptographicHash>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    void connectToServer();
    void sendData(const QString &message);

public slots:
    void connected();
    void disconnected();
    void readyRead();
    void sendAck();
    void sendSegment(const QByteArray &segment);

private:
    QTcpSocket *socket;
    quint32 clientSequenceNumber;
    quint32 serverSequenceNumber;
    void sendSyn();
};

#endif // TCPCLIENT_H
