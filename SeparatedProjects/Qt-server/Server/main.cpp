#include <QCoreApplication>
#include "tcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TcpServer server;
    //qDebug() << "Server started on port" << server.serverPort();

    return a.exec();
}
