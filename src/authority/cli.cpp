#include <iostream>
#include <string.h>

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <cctype>
#include <cstring>
#include <windows.h>
#include <thread>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#pragma comment(lib, "Ws2_32.lib")

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

//GCAO - Ginna Certified Authoritative Operator
void enableColors() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD consoleMode = 0;

    if (GetConsoleMode(hConsole, &consoleMode)) {
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hConsole, consoleMode);
    }
}

int daemon() {
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
    address.sin_port = htons(57);

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

    std::cout << GREEN << "[NETWORK] Listening on port 57...\n" << RESET;

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


            const char* response = "ACK";

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

int serverrun()
{
    STARTUPINFOA si1{};
    STARTUPINFOA si2{};

    PROCESS_INFORMATION pi1{};
    PROCESS_INFORMATION pi2{};

    si1.cb = sizeof(si1);
    si2.cb = sizeof(si2);

    if (CreateProcessA(
        "named.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si1,
        &pi1))
    {
        std::cout << "[SUCCESS] named.exe started.\n";

        CloseHandle(pi1.hThread);
        CloseHandle(pi1.hProcess);
    }
    else
    {
        std::cout << "[ERROR] named.exe failed: "
                  << GetLastError() << '\n';
    }

    if (CreateProcessA(
        "request.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si2,
        &pi2))
    {
        std::cout << "[SUCCESS] request.exe started.\n";

        CloseHandle(pi2.hThread);
        CloseHandle(pi2.hProcess);
    }
    else
    {
        std::cout << "[ERROR] request.exe failed: "
                  << GetLastError() << '\n';
    }

    return 0;
}

int main(){
    bool serverRunning = false;
    enableColors();
    std::cout << MAGENTA << "       GINNA AUTHORITIVE OPERATOR DAEMON!" << RESET << std::endl;
    std::cout << YELLOW << "[SYSTEM] ISINNE initialized" << RESET << std::endl;
    while (true)
    {
        std::string command;
        std::cout << "\033[1;32mGCAO\033[0m@\033[1;36mISINNE\033[0m~$ ";
        std::getline(std::cin, command);

        if (command == "help"){
            std::cout << "search <name> - Search for a name" << std::endl;
        }else if (command == "exit"){
            std::cout << YELLOW << "EXITING...." << RESET << std::endl;
            return 1;
        }else if (command == ""){
            
        }else if (command == "start isinne"){
            std::cout << GREEN << "[INFO] Authority Server!" << RESET << std::endl;
            daemon();
        }else if (command == "start daemon"){
            std::cout << CYAN << BOLD << "[NETWORK] DAEMON RUNNING" << RESET << std::endl;
            serverrun();
        }
        else{
            std::cout << RED << "[ERROR] INVALID OPTION!!!" << RESET << std::endl;
        }
    }
    return 0;
}
