#include <iostream>
#include "TCPHandshake.h"
#include <thread>
#include <chrono>

int main() {
    TCPHandshake handshake;

    // Start server
    std::thread server_thread([&]() {
        handshake.startServer("12345");
    });

    // Give the server time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    try {
        std::vector<std::thread> client_threads;
        for (int i = 0; i < 3; ++i) {
            client_threads.emplace_back([&, i]() {
                handshake.startClient("127.0.0.1", "12345", i);
            });
        }

        // Wait for all clients to finish
        for (auto& thread : client_threads) {
            thread.join();
        }

        // Wait for the server to finish (in this example, it runs indefinitely)
        server_thread.join();
    } catch (const boost::system::system_error& e) {
        std::cerr << "Boost system error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
