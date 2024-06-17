#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include "TCPHandshake.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Create server instance and move it to server thread
    QThread serverThread;
    TCPHandshake serverInstance;
    serverInstance.moveToThread(&serverThread);
    QObject::connect(&serverThread, &QThread::started, [&]() {
        serverInstance.startServer("12345");
    });
    serverThread.start();

    // Give the server time to start
    QThread::sleep(1);

    // Create client instances and move each to its own client thread
    std::vector<QThread*> clientThreads;
    std::vector<TCPHandshake*> clientInstances;
    for (int i = 0; i < 1; ++i) {
        QThread* thread = new QThread();
        clientThreads.push_back(thread);
        TCPHandshake* clientInstance = new TCPHandshake();
        clientInstances.push_back(clientInstance);
        clientInstance->moveToThread(thread);

        // Connect to thread started signal to start client
        QObject::connect(thread, &QThread::started, clientInstance, [&, i]() {
            clientInstances[i]->startClient("127.0.0.1", "12345", i);
        });

        // Start the thread
        thread->start();
    }

    // Wait for all clients to finish
    for (auto* thread : clientThreads) {
        thread->wait(); // Wait for thread to finish
        delete thread;  // Clean up thread object
    }

    // Quit and wait for the server thread to finish
    serverThread.quit();
    serverThread.wait();

    // Clean up client instances
    for (auto* clientInstance : clientInstances) {
        delete clientInstance;
    }

    return app.exec();
}
