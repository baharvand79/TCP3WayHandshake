#include "TCPHandshake.h"

TCPHandshake::TCPHandshake() {
    std::srand(std::time(nullptr));
    generateInitialSequenceNumbers();
}

void TCPHandshake::generateInitialSequenceNumbers() {
    clientPacket.seqNumber = std::rand() % 10000;
    serverPacket.seqNumber = std::rand() % 10000;
    clientPacket.ackNumber = 0;
    serverPacket.ackNumber = 0;
    clientPacket.mss = 100;
    clientPacket.windowSize = 1000;
    serverPacket.mss = 100;
    serverPacket.windowSize = 1000;
}

void TCPHandshake::clientSendSYN(tcp::socket& socket) {
    clientPacket.syn = true;
    clientPacket.ack = false;
    clientPacket.ackNumber = 0;

    std::string syn_message = "SYN " + std::to_string(clientPacket.seqNumber);
    boost::asio::write(socket, boost::asio::buffer(syn_message));

    std::cout << "Client -> Server: " << syn_message << std::endl;
}

void TCPHandshake::serverReceiveSYN(tcp::socket& socket) {
    char request[1024];
    size_t request_length = boost::asio::read(socket, boost::asio::buffer(request, 3 + 5));
    std::cout << "Server received: " << std::string(request, request_length) << std::endl;

    std::string received(request, request_length);
    if (received.substr(0, 3) == "SYN") {
        clientPacket.seqNumber = std::stoi(received.substr(4));
    }
}

void TCPHandshake::serverSendSYNACK(tcp::socket& socket) {
    serverPacket.syn = true;
    serverPacket.ack = true;
    serverPacket.ackNumber = clientPacket.seqNumber + 1;

    std::string syn_ack_message = "SYN-ACK " + std::to_string(serverPacket.seqNumber) + " " + std::to_string(serverPacket.ackNumber);
    boost::asio::write(socket, boost::asio::buffer(syn_ack_message));

    std::cout << "Server -> Client: " << syn_ack_message << std::endl;
}

void TCPHandshake::clientReceiveSYNACK(tcp::socket& socket) {
    char reply[1024];
    size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, 7 + 5 + 5));
    std::cout << "Client received: " << std::string(reply, reply_length) << std::endl;

    std::string received(reply, reply_length);
    if (received.substr(0, 7) == "SYN-ACK") {
        size_t first_space = received.find(' ', 8);
        serverPacket.seqNumber = std::stoi(received.substr(8, first_space - 8));
        serverPacket.ackNumber = std::stoi(received.substr(first_space + 1));
        clientPacket.ackNumber = serverPacket.seqNumber + 1;
    }
}

void TCPHandshake::clientSendACK(tcp::socket& socket) {
    clientPacket.syn = false;
    clientPacket.ack = true;

    std::string ack_message = "ACK " + std::to_string(clientPacket.ackNumber);
    boost::asio::write(socket, boost::asio::buffer(ack_message));

    std::cout << "Client -> Server: " << ack_message << std::endl;
}

void TCPHandshake::serverReceiveACK(tcp::socket& socket) {
    char request[1024];
    size_t request_length = boost::asio::read(socket, boost::asio::buffer(request, 3 + 5));
    std::cout << "Server received: " << std::string(request, request_length) << std::endl;

    std::string received(request, request_length);
    if (received.substr(0, 3) == "ACK") {
        clientPacket.ackNumber = std::stoi(received.substr(4));
    }

    std::cout << "Connection Established" << std::endl;
}

void TCPHandshake::clientCloseConnection(tcp::socket& socket) {
    clientPacket.fin = true;
    std::string fin_message = "FIN " + std::to_string(clientPacket.seqNumber);
    boost::asio::write(socket, boost::asio::buffer(fin_message));
    std::cout << "Client -> Server: " << fin_message << std::endl;
}

void TCPHandshake::serverCloseConnection(tcp::socket& socket) {
    char request[1024];
    size_t request_length = boost::asio::read(socket, boost::asio::buffer(request, 3 + 5));
    std::cout << "Server received: " << std::string(request, request_length) << std::endl;

    std::string received(request, request_length);
    if (received.substr(0, 3) == "FIN") {
        std::string fin_ack_message = "FIN-ACK " + std::to_string(serverPacket.seqNumber);
        boost::asio::write(socket, boost::asio::buffer(fin_ack_message));
        std::cout << "Server -> Client: " << fin_ack_message << std::endl;

        request_length = boost::asio::read(socket, boost::asio::buffer(request, 3 + 5));
        std::cout << "Server received: " << std::string(request, request_length) << std::endl;

        if (std::string(request, request_length).substr(0, 3) == "ACK") {
            std::cout << "Connection Closed" << std::endl;
        }
    }
}

void TCPHandshake::startClient(const std::string& server_ip, const std::string& port) {
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(server_ip, port);
    tcp::socket socket(io_context);
    boost::asio::connect(socket, endpoints);

    clientSendSYN(socket);
    clientReceiveSYNACK(socket);
    clientSendACK(socket);
    clientCloseConnection(socket);
}

void TCPHandshake::startServer(const std::string& port) {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::stoi(port)));

    for (;;) {
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        serverReceiveSYN(socket);
        serverSendSYNACK(socket);
        serverReceiveACK(socket);
        serverCloseConnection(socket);
    }
}
