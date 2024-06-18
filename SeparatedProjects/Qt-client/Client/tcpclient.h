#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QCryptographicHash>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

public slots:
    void connectToServer();
    void sendData(const QString &message);
    void sendSegment(const QByteArray &segment);
    // void sendAckForData(quint32 sequenceNumber);

private slots:
    void sendSyn();
    void connected();
    void disconnected();
    void readyRead();
    void sendAck();
    //void readyReadData();

private:
    QTcpSocket *socket;
    quint32 clientSequenceNumber;
    quint32 serverSequenceNumber;
};

#endif // TCPCLIENT_H
