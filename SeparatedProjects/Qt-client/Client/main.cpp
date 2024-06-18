// #include <QCoreApplication>
// #include <QTimer>
// #include "tcpclient.h"

// int main(int argc, char *argv[])
// {
//     QCoreApplication a(argc, argv);

//     TcpClient client;
//     client.connectToServer();

//     // Send messages at intervals
//     QTimer timer;
//     QObject::connect(&timer, &QTimer::timeout, [&client]() {
//         client.sendData("Hello Server!");
//     });
//     timer.start(5000); // Send a message every 5 seconds

//     return a.exec();
// }

#include <QCoreApplication>
#include "tcpclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TcpClient client;
    client.connectToServer();

    return a.exec();
}
