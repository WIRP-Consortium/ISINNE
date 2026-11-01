
#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <fstream>
#include <vector>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

std::string cleanSpace(const std::string& input) {
    size_t start = input.find_first_not_of(" \t\r\n");

    if (start == std::string::npos)
        return "";

    size_t end = input.find_last_not_of(" \t\r\n");

    return input.substr(start, end - start + 1);
}

int main() {

    WSADATA wsaData{};

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << '\n';
        return 1;
    }

    while (true) {

        std::cout << "\nWhich Imperian Is Query? (type exit to quit)\n";
        std::cout << "WhichImperianIs (WII): ";

        std::string wii;

        if (!std::getline(std::cin, wii)) {
            break;
        }

        if (wii == "exit") {
            break;
        }

        std::ostringstream stream;

        stream << "VERSION: 0.1\n"
               << "TRANSACTION ID: hex\n"
               << "FLAG: wii\n"
               << "AUTHORITY: GINNA\n"
               << "QUESTION:\n"
               << "NAME: " << wii << "\n"
               << "SYSTEM: wddd\n"
               << "IP TYPE: A\n"
               << "CLASS: IN\n";

        std::string data = stream.str();

        SOCKET connectSocket = socket(
            AF_INET,
            SOCK_DGRAM,
            IPPROTO_UDP
        );

        if (connectSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed: "
                      << WSAGetLastError() << '\n';
            continue;
        }

        sockaddr_in clientAddr{};

        clientAddr.sin_family = AF_INET;
        clientAddr.sin_port = htons(60);

        clientAddr.sin_addr.s_addr = inet_addr("192.168.1.8");

        if (clientAddr.sin_addr.s_addr == INADDR_NONE) {
            std::cerr << "Invalid IP address.\n";
            closesocket(connectSocket);
            continue;
        }

        std::cout << "Connected successfully!\n";

        DWORD timeout = 10000;

        setsockopt(
            connectSocket,
            SOL_SOCKET,
            SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeout),
            sizeof(timeout)
        );

        int sent = sendto(
            connectSocket,
            data.c_str(),
            static_cast<int>(data.size()),
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),
            sizeof(clientAddr)
        );

        if (sent == SOCKET_ERROR) {
            std::cerr << "UDP send failed: "
                      << WSAGetLastError() << '\n';

            closesocket(connectSocket);
            continue;
        }

        std::cout << "UDP query sent successfully!\n";

        char response[8192];

        sockaddr_in responseAddress{};
        int responseLength = sizeof(responseAddress);

        std::cout << "Waiting for GINNA response...\n";

        int received = recvfrom(
            connectSocket,
            response,
            sizeof(response),
            0,
            reinterpret_cast<sockaddr*>(&responseAddress),
            &responseLength
        );

        if (received == SOCKET_ERROR) {
            std::cerr << "Receive failed: "
                      << WSAGetLastError() << '\n';

            closesocket(connectSocket);
            continue;
        }

        // Validate response source
        if (responseAddress.sin_addr.s_addr !=
                clientAddr.sin_addr.s_addr ||
            responseAddress.sin_port != clientAddr.sin_port) {

            std::cerr << "Response received from unexpected source.\n";

            closesocket(connectSocket);
            continue;
        }

        std::cout << "\n";

        std::string registrarLog;

        std::string response_data(response, received);

        std::map<std::string, std::string> recordmap;

        std::stringstream response_stream(response_data);
        std::string singleline;

        while (std::getline(response_stream, singleline)) {

            size_t primarycolon = singleline.find(':');

            if (primarycolon == std::string::npos)
                continue;

            std::string rawkey =
                singleline.substr(0, primarycolon);

            std::string rawvalue =
                singleline.substr(primarycolon + 1);

            recordmap[cleanSpace(rawkey)] =
                cleanSpace(rawvalue);
        }

        // Check for a valid record response.
        if (recordmap.find("REGISTRAR") == recordmap.end()) {
            std::cerr << "Invalid WII response: Registrar field missing.\n";

            std::cout << response_data << "\n";

            closesocket(connectSocket);
            continue;
        }

        std::string name = recordmap["NAME"];
        std::string id = recordmap["ID"];
        std::string imperian = recordmap["IMPERIAN"];
        std::string rgstr = recordmap["REGISTRAR"];
        std::string rgstrid = recordmap["REGISTRAR ID"];
        std::string com = recordmap["COMPANY"];
        std::string iso = recordmap["ISO CODE 3166-1 codes"];
        std::string num = recordmap["NUMBER"];
        std::string fax = recordmap["FAX"];

        std::string prov_state = recordmap["PROVINCE/STATE"];
        std::string street = recordmap["STREET"];
        std::string city = recordmap["CITY"];
        std::string postal = recordmap["POSTAL"];
        std::string addr = recordmap["ADDRESS"];
        std::string sl1 = recordmap["SL1"];
        std::string sl2 = recordmap["SL2"];
        std::string txt = recordmap["TXT"];
        std::string mail = recordmap["MAIL"];
        std::string admin = recordmap["ADMIN"];

        std::string created = recordmap["CREATED"];
        std::string expiry = recordmap["EXPIRY"];

        std::ostringstream registrarStream;

        registrarStream << "VERSION: 0.1\n"
                        << "TRANSACTION ID: hex\n"
                        << "FLAG: rgstr\n"
                        << "AUTHORITY: GINNA\n"
                        << "QUESTION:\n"
                        << "NAME: " << rgstr << "\n"
                        << "SYSTEM: wddd\n"
                        << "IP TYPE: A\n"
                        << "CLASS: IN\n";

        std::string registrarData = registrarStream.str();

        int registrarSent = sendto(
            connectSocket,
            registrarData.c_str(),
            static_cast<int>(registrarData.size()),
            0,
            reinterpret_cast<sockaddr*>(&clientAddr),
            sizeof(clientAddr)
        );

        if (registrarSent == SOCKET_ERROR) {
            std::cerr << "Registrar request failed: "
                      << WSAGetLastError() << '\n';
        }
        else {

            std::cout << "\nWaiting for registrar log...\n";

            char registrarResponse[8192];

            sockaddr_in registrarAddress{};
            int registrarAddressLength = sizeof(registrarAddress);

            int registrarReceived = recvfrom(
                connectSocket,
                registrarResponse,
                sizeof(registrarResponse),
                0,
                reinterpret_cast<sockaddr*>(&registrarAddress),
                &registrarAddressLength
            );

            if (registrarReceived == SOCKET_ERROR) {
                std::cerr << "Registrar response failed: "
                          << WSAGetLastError() << '\n';
            }
            else if (
                registrarAddress.sin_addr.s_addr !=
                    clientAddr.sin_addr.s_addr ||
                registrarAddress.sin_port != clientAddr.sin_port
            ) {
                std::cerr << "Registrar response from unexpected source.\n";
            }
            else {

                registrarLog.assign(
                    registrarResponse,
                    registrarReceived
                );
            }
        }

        std::cout << "\n IMPERIAN RECORD \n";

        std::cout << "Name: " << name << "\n";
        std::cout << "Imperian: " << imperian << "\n";
        std::cout << "GINNA Registered ID: " << id << "\n";
        std::cout << "Registrar: " << rgstr << "\n";
        std::cout << "Registrar ID: " << rgstrid << "\n";
        std::cout << "Company: " << com << "\n";
        std::cout << "ISO Code: " << iso << "\n";
        std::cout << "Number: " << num << "\n";
        std::cout << "Fax: " << fax << "\n";
        std::cout << "Created: " << created << "\n";
        std::cout << "Expiry: " << expiry << "\n";

        if (wii.size() >= 4 &&
            wii.compare(wii.size() - 4, 4, "-rev") == 0) {

            std::cout
                << "\nThis name " << name
                << " belongs to an authoritative query "
                << "('Thrichur Imperians').\n";

            std::cout
                << "The authoritative database contains "
                << "-REV, -RRN and -DAO Imperian extensions.\n";
        }

        std::cout
            << "\nFor more information visit "
            << "idtp/::wddd:ginna-rev\n";

        std::cout
            << "\nFor namespace owners, visit Contact or "
            << "search the registrar ID in ginna-rev.\n";

        std::string tou = R"TOU(
TERMS OF USE:

You are not authorized to access, query, or retrieve information from the Imperian Names and Numbers Exchange (GINNA) database through high-volume, automated, or abusive electronic processes, except as reasonably necessary for legitimate registration, verification, administration, or modification of existing Imperian name records.
The information contained in the GINNA database is provided for informational purposes only and to assist individuals and organizations in obtaining information about or related to Imperian name registrations. GINNA makes no guarantee regarding the accuracy, completeness, reliability, or continued availability of the information provided.
By submitting a WII (Which Imperian Is) query, you agree to use the retrieved information only for lawful purposes. Under no circumstances may you use the database or its contents to:
(1) Transmit, distribute, or facilitate unsolicited commercial advertisements, spam, or unauthorized solicitations through email, telephone, facsimile, or other communication channels;
(2) Conduct high-volume, automated, or excessive electronic queries that may interfere with the stability, performance, or availability of GINNA services;
(3) Collect, compile, reproduce, redistribute, or commercially exploit registration information without appropriate authorization;
(4) Misrepresent, manipulate, or misuse Imperian registration information for fraudulent, deceptive, or unlawful activities;
(5) Attempt to gain unauthorized access to GINNA systems, records, infrastructure, or associated services.
The compilation, reproduction, repackaging, redistribution, publication, or other commercial use of database information is prohibited without prior authorization from the appropriate rights holder.
GINNA reserves the right to monitor query activity, restrict access, suspend services, or terminate access privileges when necessary to protect database integrity, operational stability, and the legitimate interests of registered name holders.
GINNA further reserves the right to modify these Terms of Use at any time without prior notice. Continued use of the WII service following such modifications constitutes acceptance of the revised terms.
By continuing with this query, you acknowledge that you have read, understood, and agreed to abide by these Terms of Use.
            )TOU";

        std::cout << tou << "\n";

        std::cout << "\nNOTICE: The expiration date shown in this record is shown the now record \nIt can be change any time, The date itself does not guarentee the expiration\n";

        std::cout << "Imperian Name: " << imperian << "\n";
        std::cout << "GINNA Registered ID: " << id << "\n";
        std::cout << "Province/State: " << prov_state << "\n";
        std::cout << "Street: " << street << "\n";
        std::cout << "City: " << city << "\n";
        std::cout << "Postal: " << postal << "\n";
        std::cout << "Address: " << addr << "\n";
        std::cout << "Text: " << txt << "\n";
        std::cout << "Mail: " << mail << "\n";
        std::cout << "Admin: " << admin << "\n";
        std::cout << registrarLog;
        std::cout << "Server Location 1: " << sl1 << "\n";
        std::cout << "Server Location 2: " << sl2 << "\n";

        std::cout
            << "\n-------------------------------------------\n";

        closesocket(connectSocket);
    }

    WSACleanup();

    std::cout << "Client terminated.\n";

    return 0;
}
