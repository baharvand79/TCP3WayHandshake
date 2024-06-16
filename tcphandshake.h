#ifndef TCPHANDSHAKE_HPP
#define TCPHANDSHAKE_HPP

#include <iostream>
#include <boost/asio.hpp>
#include <ctime>
#include <cstdlib>

using boost::asio::ip::tcp;

class TCPHandshake {
public:
    TCPHandshake();

    void startClient(const std::string& server_ip, const std::string& port);
    void startServer(const std::string& port);

private:
    struct Packet {
        bool syn;
        bool ack;
        bool fin;
        int seqNumber;
        int ackNumber;
        int mss;
        int windowSize;
    };

    Packet clientPacket;
    Packet serverPacket;

    void generateInitialSequenceNumbers();
    void clientSendSYN(tcp::socket& socket);
    void serverReceiveSYN(tcp::socket& socket);
    void serverSendSYNACK(tcp::socket& socket);
    void clientReceiveSYNACK(tcp::socket& socket);
    void clientSendACK(tcp::socket& socket);
    void serverReceiveACK(tcp::socket& socket);
    void clientCloseConnection(tcp::socket& socket);
    void serverCloseConnection(tcp::socket& socket);
};

#endif // TCPHANDSHAKE_HPP
