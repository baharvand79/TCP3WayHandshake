#include <QCoreApplication>
#include "tcpclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TcpClient client;
    client.connectToServer();

    return a.exec();
}
