#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <exception>

using namespace std;

unordered_map<string, string> load_config(const string& path) {
    unordered_map<string, string> config;
    ifstream file(path);

    if (!file.is_open()) {
        std::cout << "[!] Config not found, using defaults\n";
        return config;
    }

    string line;

    while (getline(file, line)) {
        size_t first = line.find_first_not_of(" \t\r\n");

        if (first == string::npos)
            continue;

        line = line.substr(first);

        if (line[0] == '#')
            continue;

        istringstream iss(line);
        string key, value;

        if (!(iss >> key))
            continue;

        getline(iss, value);

        size_t start = value.find_first_not_of(" \t");

        if (start == string::npos)
            continue;

        value = value.substr(start);

        if (value.size() >= 2 &&
            value.front() == '"' &&
            value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        config[key] = value;
    }

    return config;
}


string trim(const string& s) {
    const string whitespace = " \t\r\n";

    size_t first = s.find_first_not_of(whitespace);

    if (first == string::npos)
        return "";

    size_t last = s.find_last_not_of(whitespace);

    return s.substr(first, last - first + 1);
}

unordered_map<string, string> parse_message(
    const string& raw
) {
    unordered_map<string, string> data;
    istringstream stream(raw);
    string line;

    while (getline(stream, line)) {
        line = trim(line);

        size_t pos = line.find(':');

        if (pos != string::npos) {
            string key = trim(line.substr(0, pos));
            string value = trim(line.substr(pos + 1));

            data[key] = value;
        }
    }

    return data;
}

string answer(
    const string& ver,
    const string& name,
    const string& ip,
    const string& ip_type,
    const string& flag,
    const string& class_type,
    const string& ttl,
    const string& type
) {
    return
        "VERSION: " + ver + "\n" +
        "NAME: " + name + "\n" +
        "ADDRESS: " + ip + "\n" +
        "IP TYPE: " + ip_type + "\n" +
        "FLAG: " + flag + "\n" +
        "CLASS: " + class_type + "\n" +
        "TTL: " + ttl + "\n" +
        "TYPE: " + type + "\n";
}

unordered_map<string, string> CONFIG =
    load_config("isinned.conf");

string get_config(
    const string& key,
    const string& default_value
) {
    auto it = CONFIG.find(key);

    if (it != CONFIG.end())
        return it->second;

    return default_value;
}

string HOST = get_config("BindAddress", "0.0.0.0");
string VER = get_config("Version", "0.01");
string CHAIN_FILE = get_config("NameFile", "name.conf");

int PORT = 60;

bool send_response(
    SOCKET server_socket,
    const string& packet,
    const sockaddr_in& client_address
) {
    int sent = sendto(
        server_socket,
        packet.c_str(),
        static_cast<int>(packet.size()),
        0,
        reinterpret_cast<const sockaddr*>(&client_address),
        sizeof(client_address)
    );

    if (sent == SOCKET_ERROR) {
        cerr << "sendto failed: "
             << WSAGetLastError() << "\n";

        return false;
    }

    return true;
}

bool send_registrar_log(
    SOCKET server_socket,
    const string& requested_name,
    const sockaddr_in& client_address
) {
    const string registrar_file = "data/registrar.conf";

    ifstream file(registrar_file);

    if (!file.is_open()) {
        cerr << "Cannot open registrar.conf\n";
        return false;
    }

    string line;
    unordered_map<string, string> record;
    string log_path;
    bool found = false;

    auto process_record = [&]() -> bool {

        string registrar_name;

        if (record.count("Registrar Name"))
            registrar_name = record["Registrar Name"];
        else if (record.count("NAME"))
            registrar_name = record["NAME"];
        else if (record.count("Name"))
            registrar_name = record["Name"];

        if (registrar_name != requested_name)
            return false;

        auto log_it = record.find("log");

        if (log_it == record.end() ||
            log_it->second.empty()) {
            cerr << "Registrar log not specified\n";
            return false;
        }

        log_path = log_it->second;
        return true;
    };

    while (getline(file, line)) {

        line = trim(line);

        if (line.empty()) {

            if (process_record()) {
                found = true;
                break;
            }

            record.clear();
            continue;
        }

        size_t pos = line.find(':');

        if (pos != string::npos) {

            string key = trim(line.substr(0, pos));
            string value = trim(line.substr(pos + 1));

            record[key] = value;
        }
    }

    if (!found && process_record())
        found = true;

    if (!found) {
        cerr << "Registrar not found: "
             << requested_name << "\n";
        return false;
    }

    ifstream log_file(log_path, ios::binary);

    if (!log_file.is_open()) {
        cerr << "Cannot open registrar log: "
             << log_path << "\n";
        return false;
    }

    ostringstream buffer;
    buffer << log_file.rdbuf();

    if (log_file.bad()) {
        cerr << "Error reading registrar log\n";
        return false;
    }

    string log_content = buffer.str();

    if (log_content.empty()) {
        cerr << "Registrar log is empty\n";
        return false;
    }

    if (!send_response(
        server_socket,
        log_content,
        client_address
    )) {
        cerr << "Failed to send registrar log\n";
        return false;
    }

    cout << "Registrar log sent successfully: "
         << requested_name << "\n";

    return true;
}

void request_handler(
    SOCKET server_socket,
    string raw,
    sockaddr_in client_address
) {
    char client_ip[INET_ADDRSTRLEN] = {};

    strcpy(
        client_ip,
        inet_ntoa(client_address.sin_addr)
    );


    std::cout << "Received request from "
         << client_ip << ":"
         << ntohs(client_address.sin_port) << "\n";

    try {
        auto message = parse_message(raw);

        string name = message.count("NAME")
            ? message["NAME"] : "";

        string protocol_type = message.count("SYSTEM")
            ? message["SYSTEM"] : "";

        string flag = message.count("FLAG")
            ? message["FLAG"] : "";

        string ip_type = message.count("IP TYPE")
            ? message["IP TYPE"] : "";

        string class_type = message.count("CLASS")
            ? message["CLASS"] : "";

        if (name.empty() ||
            ip_type.empty() ||
            protocol_type.empty()) {
            std::cout << "Invalid request\n";
            return;
        }

        if (flag == "rgstr") {

                send_registrar_log(
                    server_socket,
                    name,
                    client_address
                );

                return;
        }

        // Construct the requested name.
        if (protocol_type == "wddd") {
            name = "idtp/::wddd:" + name;
        }
        else if (protocol_type == "www") {
            name = "www." + name;
        }
        else {
            std::cout << "Invalid system\n";
            return;
        }

        if (ip_type != "A" && ip_type != "AAAA") {
            std::cout << "Invalid IP type\n";
            return;
        }

        ifstream file(CHAIN_FILE);

        if (!file.is_open()) {
            cerr << "Cannot open record file: "
                 << CHAIN_FILE << "\n";
            return;
        }

        string line;
        unordered_map<string, string> record;
        bool found = false;

        // Process a single record block.
        auto process_record = [&]() -> bool {
            auto name_it = record.find("Imperian Name");

            if (name_it == record.end() ||
                name_it->second != name) {
                return false;
            }

            string ip_key =
                (ip_type == "A") ? "IP V4" : "IP V6";

            auto ip_it = record.find(ip_key);

            if (ip_it == record.end() ||
                ip_it->second.empty()) {
                return false;
            }

            if (flag == "wii") {

                auto log_it = record.find("log");

                if (log_it == record.end() || log_it->second.empty()) {
                    std::cout << "Log file not specified\n";
                    return false;
                }

                const string& log_path = log_it->second;

                ifstream log_file(log_path, ios::binary);

                if (!log_file.is_open()) {
                    std::cerr << "Cannot open log file: "
                        << log_path << "\n";
                    return false;
                }

                ostringstream buffer;
                buffer << log_file.rdbuf();

                if (log_file.bad()) {
                    cerr << "Error reading log file\n";
                    return false;
                }

                string log_content = buffer.str();

                if (log_content.empty()) {
                    std::cout << "Log file is empty\n";
                    return false;
                }

                if (!send_response(
                    server_socket,
                    log_content,
                    client_address
                )) {
                    cerr << "Failed to send log response\n";
                    return false;
                }

                return true;
            }
            else{

                string packet = answer(
                    VER,
                    name,
                    ip_it->second,
                    ip_type,
                    flag,
                    class_type,
                    "360",
                    protocol_type
                );

                send_response(
                    server_socket,
                    packet,
                    client_address
                );
            }

            return true;
        };

        while (getline(file, line)) {
            line = trim(line);

            if (line.empty()) {
                if (process_record()) {
                    found = true;
                    break;
                }

                record.clear();
                continue;
            }

            size_t pos = line.find(':');

            if (pos != string::npos) {
                string key = trim(line.substr(0, pos));
                string value = trim(line.substr(pos + 1));

                record[key] = value;
            }
        }

        if (!found && process_record())
            found = true;

        // Return a not-found response.
        if (!found) {
            string packet = answer(
                VER,
                name,
                "NO MATCHING RECORD FOUND",
                ip_type,
                "NFMPERIAN",
                class_type,
                "0",
                protocol_type
            );

            send_response(
                server_socket,
                packet,
                client_address
            );
        }
    }
    catch (const exception& e) {
        cerr << "Client error: " << e.what() << "\n";
    }
}

struct ThreadData {
    SOCKET server_socket;
    string data;
    sockaddr_in client_address;
};

DWORD WINAPI thread_wrapper(LPVOID parameter) {
    ThreadData* args = static_cast<ThreadData*>(parameter);

    SOCKET server_socket = args->server_socket;
    string data = args->data;
    sockaddr_in client_address = args->client_address;

    delete args;

    request_handler(
        server_socket,
        data,
        client_address
    );

    return 0;
}

int main() {
    WSADATA wsaData{};

    int startup_result = WSAStartup(
        MAKEWORD(2, 2),
        &wsaData
    );

    if (startup_result != 0) {
        cerr << "WSAStartup failed: "
             << startup_result << "\n";

        return 1;
    }

    try {
        PORT = stoi(get_config("Listen", "60"));

        if (PORT < 1 || PORT > 65535) {
            cerr << "Invalid port number\n";
            WSACleanup();
            return 1;
        }
    }
    catch (const exception&) {
        cerr << "Invalid Listen value in isinned.conf\n";
        WSACleanup();
        return 1;
    }

    SOCKET server_socket = socket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP
    );

    if (server_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed: "
             << WSAGetLastError() << "\n";

        WSACleanup();
        return 1;
    }

    BOOL opt = TRUE;

    if (setsockopt(
        server_socket,
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&opt),
        sizeof(opt)
    ) == SOCKET_ERROR) {
        cerr << "setsockopt failed: "
             << WSAGetLastError() << "\n";

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(
        static_cast<u_short>(PORT)
    );

    unsigned long ip_address = inet_addr(HOST.c_str());

    if (ip_address == INADDR_NONE &&
        HOST != "255.255.255.255") {
        cerr << "Invalid IPv4 bind address: "
            << HOST << "\n";

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    server_address.sin_addr.s_addr = ip_address;

    if (bind(
        server_socket,
        reinterpret_cast<sockaddr*>(&server_address),
        sizeof(server_address)
    ) == SOCKET_ERROR) {
        cerr << "Bind failed: "
             << WSAGetLastError() << "\n";

        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Internet System for Imperian Names and Numbers Exchange\n";
    std::cout << "UDP listening on " << HOST
         << ":" << PORT << "\n";

    while (true) {
        char buffer[4096];

        sockaddr_in client_address{};
        int client_length = sizeof(client_address);

        int bytes_received = recvfrom(
            server_socket,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr*>(&client_address),
            &client_length
        );

        if (bytes_received == SOCKET_ERROR) {
            int error = WSAGetLastError();

            cerr << "recvfrom failed: "
                 << error << "\n";

            continue;
        }

        string data(buffer, bytes_received);

        try {
            ThreadData* args = new ThreadData{
                server_socket,
                data,
                client_address
            };

            HANDLE thread_handle = CreateThread(
                NULL,
                0,
                thread_wrapper,
                args,
                0,
                NULL
            );

            if (thread_handle == NULL) {
                cerr << "CreateThread failed: "
                    << GetLastError() << "\n";

                delete args;
            } else {
                CloseHandle(thread_handle);
            }
        }
        catch (const exception& e) {
            cerr << "Could not create client thread: "
                 << e.what() << "\n";
        }
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
