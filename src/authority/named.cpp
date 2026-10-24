#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map> 
#include <sstream> 

#pragma comment(lib, "Ws2_32.lib")

std::string cleanSpace(const std::string& input) {
    size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = input.find_last_not_of(" \t\r\n");
    return input.substr(start, (end - start + 1));
}

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
    address.sin_port = htons(80);

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

    std::cout << "Listening on port 80...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: "
                    << WSAGetLastError() << std::endl;
            continue;
        }    

        char buffer[1024];

        int bytesReceived = recv(
                clientSocket, buffer, sizeof(buffer), 0
            );

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';

            std::cout << "Message received: " << buffer << '\n';

            std::map<std::string, std::string> recordMap;
            std::stringstream stream(buffer);
            std::string singleLine;

            // Parse loop
            while (std::getline(stream, singleLine)) {
                size_t primaryColon = singleLine.find(':');
                if (primaryColon == std::string::npos) continue; 

                std::string rawKey = singleLine.substr(0, primaryColon);
                std::string rawValue = singleLine.substr(primaryColon + 1);

                recordMap[cleanSpace(rawKey)] = cleanSpace(rawValue);
            }

            std::string id = recordMap["Record ID"];
            std::string name = recordMap["Imperian Name"];
            std::string ipv6 = recordMap["IP V6"];
            std::string ipv4 = recordMap["IP V4"];
            std::string registrar = recordMap["Registrar"];
            std::string ginnaid = recordMap["Registrar ID"];
            std::string public_key = recordMap["Public Key"];
            std::string signature = recordMap["Signature"];
            std::string created = recordMap["Created"];
            std::string expiry = recordMap["Expires"];

            size_t lastColon = name.rfind(':');
    
            std::string imperian = "";
            if (lastColon != std::string::npos) {
                imperian = name.substr(lastColon + 1);
            }

            std::ostringstream logStream;

            logStream << "$REGISTRAR LOG " << name << "\n"
                      << "NAME: " << imperian << "\n"
                      << "IMPERIAN: " << name << "\n"
                      << "AAOC: Global Internet Names and Numbers Authority\n"
                      << "ID: " << ginnaid << "\n"
                      << "IP V4: " << ipv4 << "\n"
                      << "IP V6: " << ipv6 << "\n"
                      << "REGISTRAR: " << registrar << "\n"
                      << "REGISTRAR ID: " << ginnaid << "\n"
                      << "SIGNATURE: " << signature << "\n"
                      << "CREATED: " << created << "\n"
                      << "EXPIRY: " << expiry << "\n";

            std::string log = logStream.str();

            std::cout << log;

            std::string logname = name + "-name.log";

            std::ofstream logfile(logname, std::ios::app);
            std::ofstream file("data/name.conf", std::ios::app);
            
            logfile.write(log.data(), static_cast<std::streamsize>(log.size()));

            if (!file.is_open()) {
                std::cerr << "Error opening data/name.conf. "
                            "Check whether the data directory exists.\n";
            } else {
                file.write(buffer, bytesReceived);
                file.close();

                if (file) {
                    std::cout << "Data recorded successfully!\n";
                } else {
                    std::cerr << "Error writing to file.\n";
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
