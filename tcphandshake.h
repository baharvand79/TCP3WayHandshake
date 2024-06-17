#ifndef TCP_HANDSHAKE_H
#define TCP_HANDSHAKE_H

#include <iostream>
#include <boost/asio.hpp>
#include <cstdlib>
#include <ctime>
#include <string>

using boost::asio::ip::tcp;

class TCPHandshake {
public:
    TCPHandshake();
    void startClient(const std::string& server_ip, const std::string& port, int client_id);
    void startServer(const std::string& port);

private:
    struct Packet {
        int seqNumber;
        int ackNumber;
        bool syn;
        bool ack;
        bool fin;
        int mss;
        int windowSize;
        int senderId;
    };

    Packet generateInitialPacket(int client_id);

    void clientSendSYN(tcp::socket& socket, Packet& clientPacket);
    void serverReceiveSYN(tcp::socket& socket, Packet& serverPacket);
    void serverSendSYNACK(tcp::socket& socket, Packet& serverPacket, Packet& clientPacket);
    void clientReceiveSYNACK(tcp::socket& socket, Packet& clientPacket);
    void clientSendACK(tcp::socket& socket, Packet& clientPacket);
    void serverReceiveACK(tcp::socket& socket, Packet& serverPacket, Packet& clientPacket);
    void clientCloseConnection(tcp::socket& socket, Packet& clientPacket);
    void serverCloseConnection(tcp::socket& socket, Packet& serverPacket);
};

#endif // TCP_HANDSHAKE_H
