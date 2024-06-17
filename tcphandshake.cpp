#include "TCPHandshake.h"
#include <QDebug>
#include <QThread>

TCPHandshake::TCPHandshake() {
    std::srand(std::time(nullptr));
}

TCPHandshake::Packet TCPHandshake::generateInitialPacket(int client_id) {
    Packet packet;
    packet.seqNumber = (std::rand() % 10000) + (client_id * 10000);
    packet.ackNumber = 0;
    packet.syn = false;
    packet.ack = false;
    packet.fin = false;
    packet.mss = 100;
    packet.windowSize = 1000;
    packet.senderId = client_id;
    return packet;
}

void TCPHandshake::clientSendSYN(tcp::socket& socket, Packet& clientPacket) {
    clientPacket.syn = true;
    clientPacket.ack = false;
    clientPacket.ackNumber = 0;

    std::string syn_message = "SYN " + std::to_string(clientPacket.seqNumber);
    boost::asio::write(socket, boost::asio::buffer(syn_message));

    emit clientLog(QString("Client(%1) is sending SYN.").arg(clientPacket.senderId));
    emit clientLog(QString("Client(%1) -> Server: %2").arg(clientPacket.senderId).arg(QString::fromStdString(syn_message)));
}

void TCPHandshake::serverReceiveSYN(tcp::socket& socket, Packet& serverPacket) {
    char request[1024];
    size_t request_length = socket.read_some(boost::asio::buffer(request, 1024));
    std::string received(request, request_length);
    qDebug() << "Server received: " << QString::fromStdString(received);

    if (received.substr(0, 3) == "SYN") {
        serverPacket.seqNumber = std::stoi(received.substr(4));
    }
}

void TCPHandshake::serverSendSYNACK(tcp::socket& socket, Packet& serverPacket, Packet& clientPacket) {
    serverPacket.syn = true;
    serverPacket.ack = true;
    serverPacket.ackNumber = clientPacket.seqNumber + 1;

    std::string syn_ack_message = "SYN-ACK " + std::to_string(serverPacket.seqNumber) + " " + std::to_string(serverPacket.ackNumber);
    boost::asio::write(socket, boost::asio::buffer(syn_ack_message));

    emit serverLog(QString("Server is sending SYN-ACK."));
    emit serverLog(QString("Server -> Client(%1): %2").arg(clientPacket.senderId).arg(QString::fromStdString(syn_ack_message)));
}

void TCPHandshake::clientReceiveSYNACK(tcp::socket& socket, Packet& clientPacket) {
    char reply[1024];
    size_t reply_length = socket.read_some(boost::asio::buffer(reply, 1024));
    std::string received(reply, reply_length);
    qDebug() << "Client(" << clientPacket.senderId << ") received: " << QString::fromStdString(received);

    if (received.substr(0, 7) == "SYN-ACK") {
        size_t first_space = received.find(' ', 8);
        clientPacket.ackNumber = std::stoi(received.substr(first_space + 1));
    }
}

void TCPHandshake::clientSendACK(tcp::socket& socket, Packet& clientPacket) {
    clientPacket.syn = false;
    clientPacket.ack = true;

    std::string ack_message = "ACK " + std::to_string(clientPacket.ackNumber);
    boost::asio::write(socket, boost::asio::buffer(ack_message));

    emit clientLog(QString("Client is sending ACK."));
    emit clientLog(QString("Client(%1) -> Server: %2").arg(clientPacket.senderId).arg(QString::fromStdString(ack_message)));
}

void TCPHandshake::serverReceiveACK(tcp::socket& socket, Packet& serverPacket, Packet& clientPacket) {
    char request[1024];
    size_t request_length = socket.read_some(boost::asio::buffer(request, 1024));
    std::string received(request, request_length);
    qDebug() << "Server received: " << QString::fromStdString(received);

    if (received.substr(0, 3) == "ACK") {
        serverPacket.ackNumber = std::stoi(received.substr(4));
    }

    qDebug() << "Server received ACK from Client(" << clientPacket.senderId << ").";
    qDebug() << "Connection Established for client(" << clientPacket.senderId << ").";
}

void TCPHandshake::clientCloseConnection(tcp::socket& socket, Packet& clientPacket) {
    clientPacket.fin = true;
    std::string fin_message = "FIN " + std::to_string(clientPacket.seqNumber);
    boost::asio::write(socket, boost::asio::buffer(fin_message));

    emit clientLog(QString("Client(%1) closes connection.").arg(clientPacket.senderId));
    emit clientLog(QString("Client(%1) -> Server: %2").arg(clientPacket.senderId).arg(QString::fromStdString(fin_message)));
}

void TCPHandshake::serverCloseConnection(tcp::socket& socket, Packet& serverPacket) {
    char request[1024];
    size_t request_length = socket.read_some(boost::asio::buffer(request, 1024));
    std::string received(request, request_length);
    qDebug() << "Server received: " << QString::fromStdString(received);

    if (received.substr(0, 3) == "FIN") {
        std::string fin_ack_message = "FIN-ACK " + std::to_string(serverPacket.seqNumber);
        boost::asio::write(socket, boost::asio::buffer(fin_ack_message));

        qDebug() << "Server -> Client(" << serverPacket.senderId << "): " << QString::fromStdString(fin_ack_message);

        // Wait for the final ACK from the client
        request_length = socket.read_some(boost::asio::buffer(request, 1024));
        qDebug() << "Server received: " << QString::fromStdString(std::string(request, request_length));

        if (std::string(request, request_length).substr(0, 3) == "ACK") {
            qDebug() << "Connection Closed";
        }
    }
}

void TCPHandshake::startClient(const std::string& server_ip, const std::string& port, int client_id) {
    try {
        qDebug() << "Client(" << client_id <<") is starting.";
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(server_ip, port);
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        Packet clientPacket = generateInitialPacket(client_id);

        clientSendSYN(socket, clientPacket);
        clientReceiveSYNACK(socket, clientPacket);
        clientSendACK(socket, clientPacket);
        clientCloseConnection(socket, clientPacket);
    } catch (const boost::system::system_error& e) {
        qWarning() << "Boost system error: " << e.what();
    } catch (const std::exception& e) {
        qWarning() << "Exception: " << e.what();
    }
}

void TCPHandshake::startServer(const std::string& port) {
    QThread* thread = new QThread();
    moveToThread(thread);

    connect(thread, &QThread::started, this, [=]() {
        try {
            qDebug() << "Server is starting.";
            qDebug() << "----------------------------------";
            boost::asio::io_context io_context;
            tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::stoi(port)));

            for (;;) {
                tcp::socket socket(io_context);
                acceptor.accept(socket);

                Packet serverPacket = generateInitialPacket(0);

                serverReceiveSYN(socket, serverPacket);
                Packet clientPacket; // Temporary client packet for server state
                serverSendSYNACK(socket, serverPacket, clientPacket);
                serverReceiveACK(socket, serverPacket, clientPacket);
                serverCloseConnection(socket, serverPacket);
            }
        } catch (const boost::system::system_error& e) {
            qWarning() << "Boost system error: " << e.what();
        } catch (const std::exception& e) {
            qWarning() << "Exception: " << e.what();
        }
    });

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}
