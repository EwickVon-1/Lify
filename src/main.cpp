#include <iostream>
#include <stdexcept>
#include <string>
#include <Spotify/crypto.hpp>

void testHarness();

int main() {
    testHarness();
    return 0;
}

std::string readMessage() {
    std::string message;

    if (!std::getline(std::cin, message)) 
        throw std::runtime_error("Failed to read input.");

    if (!message.empty() && message[0] == ' ')
        message.erase(0, 1);

    return message;
}

void testHarness() {
    std::string command;
    std::string message;

    while (std::cin >> command) {
        if (command == "sha") {
            std::cout << crypto::toHex(crypto::sha256(readMessage())) << '\n';

        } else if (command == "base64") {
            std::string message = readMessage();

            std::span<const unsigned char> bytes{
                reinterpret_cast<const unsigned char*>(message.data()),
                    message.size()
            };

            std::cout << crypto::base64_encode(bytes) << '\n';

        } else if (command == "exit") {
            break;
        } else {
            std::cout << "Invalid command\n";
        }
    }
}


