#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: "
                  << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(86);

    if (bind(listenSocket,
             reinterpret_cast<sockaddr*>(&address),
             sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: "
                  << WSAGetLastError() << '\n';
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: "
                  << WSAGetLastError() << '\n';
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Listening on port 86...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: "
                      << WSAGetLastError() << '\n';
            continue;
        }

        char buffer[1024];

        int bytesReceived = recv(
            clientSocket, buffer, sizeof(buffer) - 1, 0
        );

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';

            std::string searchName(buffer);

            while (!searchName.empty() &&
                   std::isspace(static_cast<unsigned char>(
                       searchName.back()))) {
                searchName.pop_back();
            }

            std::cout << "Search request: " << searchName << '\n';

            std::ifstream file("data/name.conf");
            bool found = false;
            std::string line;

            if (!file.is_open()) {
                std::cerr << "Error opening data/name.conf\n";

                const char* response = "ISINNE:ERR";
                send(clientSocket, response,
                     static_cast<int>(strlen(response)), 0);
            } else {
                const std::string prefix = "Imperian Name:";

                while (std::getline(file, line)) {
                    if (line.rfind(prefix, 0) == 0) {
                        std::string name = line.substr(prefix.size());

                        const auto start =
                            name.find_first_not_of(" \t");

                        if (start == std::string::npos) {
                            name.clear();
                        } else {
                            name = name.substr(start);
                        }

                        // Remove trailing spaces and CRLF.
                        while (!name.empty() &&
                               std::isspace(static_cast<unsigned char>(
                                   name.back()))) {
                            name.pop_back();
                        }

                        if (name == searchName) {
                            found = true;
                            break;
                        }
                    }
                }

                file.close();

                const char* response =
                    found ? "ISINNE:RNE" : "ISINNE:RNA";

                int result = send(
                    clientSocket,
                    response,
                    static_cast<int>(strlen(response)),
                    0
                );

                if (result == SOCKET_ERROR) {
                    std::cerr << "Send failed: "
                              << WSAGetLastError() << '\n';
                } else {
                    std::cout << "Response: " << response << '\n';
                }
            }

        } else if (bytesReceived == 0) {
            std::cout << "Client disconnected.\n";
        } else {
            std::cerr << "Receive failed: "
                      << WSAGetLastError() << '\n';
        }

        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}
