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

private slots:
    void connected();
    void disconnected();
    void readyRead();
    void sendMessage(const QString &message);

private:
    QTcpSocket *socket;
};

#endif // TCPCLIENT_H
