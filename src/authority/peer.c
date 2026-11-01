
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define MAX_INPUT_SIZE 1024
#define MAX_NAME_SIZE 256
#define MAX_IP_SIZE 64

#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int find_imperian(const char *name, char *output_ip, size_t ip_size) {
    FILE *conf_file = fopen("data/name.conf", "r");

    if (conf_file == NULL) {
        perror("Could not open data/name.conf");
        return 1;
    }

    char line[MAX_INPUT_SIZE];
    char file_name[MAX_NAME_SIZE];
    char file_ip[MAX_IP_SIZE];

    while (fgets(line, sizeof(line), conf_file) != NULL) {
        if (sscanf(line, "%255s %63s", file_name, file_ip) == 2) {
            if (strcmp(name, file_name) == 0) {
                snprintf(output_ip, ip_size, "%s", file_ip);
                fclose(conf_file);
                return 0; 
            }
        }
    }

    fclose(conf_file);
    return 2; 
}

int serverrun()

    {
    STARTUPINFOA si1 = {0}, si2 = {0};
    PROCESS_INFORMATION pi1 = {0}, pi2 = {0};

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
        printf("named.exe started.\n");
        CloseHandle(pi1.hThread);
        CloseHandle(pi1.hProcess);
    }
    else
    {
        printf("Failed to start named.exe: %lu\n", GetLastError());
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
        printf("request.exe started.\n");
        CloseHandle(pi2.hThread);
        CloseHandle(pi2.hProcess);
    }
    else
    {
        printf("Failed to start request.exe: %lu\n", GetLastError());
    }

    return 0;
}

int daemon(){
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_size = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initialization failed.\n");
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("GCAO TCP Server running on port %d\n", PORT);
    printf("Waiting for a client...\n");

    client_socket = accept(server_socket,
                           (struct sockaddr *)&client_addr,
                           &client_size);

    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Client connected!\n");

    int received = recv(client_socket, buffer,
                        BUFFER_SIZE - 1, 0);

    if (received > 0) {
        buffer[received] = '\0';
        printf("Client says: %s\n", buffer);

        const char *reply = "Hello from GCAO server!";
        send(client_socket, reply, (int)strlen(reply), 0);
    } else if (received == 0) {
        printf("Client disconnected.\n");
    } else {
        printf("Receive failed: %d\n", WSAGetLastError());
    }

    // Clean up
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    printf("Server stopped.\n");
    return 0;
}

int main(void) {
    char input[MAX_INPUT_SIZE];

    printf("GINNA Authority Operator CLI\n");

    while (true) {
        printf("GCAO@ISINNE~$ ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        char *command = strtok(input, " \t");

        if (strcmp(command, "exit") == 0) {
            printf("Exiting CLI. Goodbye!\n");
            break;
        } else if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  help          - Show this list\n");
            printf("  greet         - Display a greeting\n");
            printf("  search <name> - Search for a name\n");
            printf("  exit          - Close the CLI\n");
        } else if (strcmp(command, "greet") == 0) {
            printf("Hello! The C-based CLI is working perfectly.\n");
        } else if (strcmp(command, "search") == 0) {
            char *name = strtok(NULL, " \t");

            if (name == NULL) {
                printf("Usage: search <name>\n");
                continue;
            }

            char output_ip[MAX_IP_SIZE];
            int status = find_imperian(name, output_ip,
                                       sizeof(output_ip));

            if (status == 0) {
                printf("Found: %s -> %s\n", name, output_ip);
            } else if (status == 2) {
                printf("Name not found: %s\n", name);
            } else {
                printf("Search failed because the file could not be read.\n");
            }
        }else if (strcmp(command, "isin") == 0) {
            char *part2 = strtok(NULL, " \t");
            char *action = strtok(NULL, " \t");

            if (part2 != NULL && action != NULL &&
                strcmp(part2, "ne") == 0 &&
                strcmp(action, "run") == 0) {

                serverrun();

            } else {
                printf("Usage: isin ne run\n");
            }
        }else if (strcmp(command, "isinne-r") == 0){
            HANDLE server_thread = CreateThread(
                NULL,
                0,
                daemon,
                NULL,
                0,
                NULL
            );

            if (server_thread == NULL) {
                printf("Failed to start server thread: %lu\n",
                    GetLastError());
            } else {
                printf("TCP server thread started.\n");
                CloseHandle(server_thread);
            }
        }
        
        else {
            printf("INVALID COMMAND!\n");
        }
    }

    return 0;
}
