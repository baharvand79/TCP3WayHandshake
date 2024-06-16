#include <QCoreApplication>
// #include <iostream>
// int main(int argc, char *argv[])
// {
//     QCoreApplication a(argc, argv);

//     std::cout << "Hello, World!\n";
//     return a.exec();
// }

#include "TCPHandshake.h"
#include <thread>

int main() {
    TCPHandshake handshake;

    std::thread server_thread([&]() {
        handshake.startServer("12345");
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    handshake.startClient("127.0.0.1", "12345");

    server_thread.join();

    return 0;
}
