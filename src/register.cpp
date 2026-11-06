
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <thread>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <vector>
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <sstream>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")

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

bool name_exists(string name) {
    ifstream file("data/name.conf");
    string line;

    while (getline(file, line)) {
        auto pos = line.find(':');

        if (pos != string::npos) {
            string key = trim(line.substr(0, pos));
            string value = trim(line.substr(pos + 1));

            if (key == "Imperian Name" && value == name) {
                return true;
            }
        }
    }

    return false;
}

string registrar_exist(const string& name) {
    ifstream file("data/registrar.conf");
    string line;
    string currentName;

    while (getline(file, line)) {
        line = trim(line);

        if (line.rfind("Name:", 0) == 0) {
            currentName = trim(line.substr(5));
        }

        if (line.rfind("Public Key:", 0) == 0 &&
            currentName == name) {

            string value = trim(line.substr(11));

            if (value.size() >= 3 &&
                value.substr(0, 2) == "b'") {

                value = value.substr(2);

                if (!value.empty() && value.back() == '\'')
                    value.pop_back();
            }

            string decoded;

            for (size_t i = 0; i < value.size(); ++i) {
                if (value[i] == '\\' &&
                    i + 1 < value.size() &&
                    value[i + 1] == 'n') {

                    decoded += '\n';
                    ++i;
                }
                else {
                    decoded += value[i];
                }
            }

            return decoded;
        }
    }

    return "";
}

bool verify_signature(
    const string& publicKeyPem,
    const string& name,
    const string& signatureHex
) {
    ERR_clear_error();

    cout << "Name: [" << name << "]\n";
    cout << "Public key length: " << publicKeyPem.size() << "\n";
    cout << "Signature hex length: " << signatureHex.size() << "\n";

    if (publicKeyPem.empty()) {
        cerr << "ERROR: Public key is empty!\n";
        return false;
    }

    if (signatureHex.empty() || signatureHex.size() % 2 != 0) {
        cerr << "ERROR: Invalid signature hex length!\n";
        return false;
    }

    vector<unsigned char> signature;

    for (size_t i = 0; i < signatureHex.size(); i += 2) {
        try {
            size_t consumed = 0;

            unsigned long byte = stoul(
                signatureHex.substr(i, 2),
                &consumed,
                16
            );

            if (consumed != 2 || byte > 255) {
                cerr << "Invalid hexadecimal character!\n";
                return false;
            }

            signature.push_back(
                static_cast<unsigned char>(byte)
            );
        }
        catch (...) {
            cerr << "Signature decoding failed!\n";
            return false;
        }
    }

    BIO* bio = BIO_new_mem_buf(
        publicKeyPem.data(),
        static_cast<int>(publicKeyPem.size())
    );

    if (!bio) {
        cerr << "BIO allocation failed!\n";
        return false;
    }

    EVP_PKEY* publicKey =
        PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);

    BIO_free(bio);

    if (!publicKey) {
        cerr << "PUBLIC KEY PEM LOADING FAILED!\n";
        ERR_print_errors_fp(stderr);
        return false;
    }

    cout << "Public key loaded successfully!\n";

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx) {
        EVP_PKEY_free(publicKey);
        return false;
    }

    bool valid = false;

    if (EVP_DigestVerifyInit(
            ctx,
            nullptr,
            EVP_sha256(),
            nullptr,
            publicKey
        ) != 1) {

        cerr << "Verification initialization failed!\n";
        ERR_print_errors_fp(stderr);

    }
    else {
        int result = EVP_DigestVerify(
            ctx,
            signature.data(),
            signature.size(),
            reinterpret_cast<const unsigned char*>(name.data()),
            name.size()
        );

        if (result == 1) {
            cout << "ECDSA SIGNATURE VALID!\n";
            valid = true;
        }
        else if (result == 0) {
            cout << "SIGNATURE DOES NOT MATCH PUBLIC KEY OR MESSAGE!\n";
        }
        else {
            cerr << "OpenSSL verification error!\n";
            ERR_print_errors_fp(stderr);
        }
    }

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(publicKey);

    return valid;
}

string to_hex(const unsigned char* data, size_t len) {
    stringstream ss;

    for (size_t i = 0; i < len; i++) {
        ss << hex << setw(2) << setfill('0')
           << (unsigned int)data[i];
    }

    return ss.str();
}

string sha256_hex(const unsigned char* data, size_t len) {
    unsigned char hash[SHA256_DIGEST_LENGTH];

    SHA256(data, len, hash);

    return to_hex(hash, SHA256_DIGEST_LENGTH);
}

string key_to_pem(EVP_PKEY* key, bool privateKey) {
    BIO* bio = BIO_new(BIO_s_mem());

    if (!bio)
        throw runtime_error("BIO allocation failed");

    int result;

    if (privateKey) {
        result = PEM_write_bio_PrivateKey(
            bio, key, nullptr, nullptr, 0, nullptr, nullptr
        );
    }
    else {
        result = PEM_write_bio_PUBKEY(bio, key);
    }

    if (result != 1) {
        BIO_free(bio);
        throw runtime_error("PEM serialization failed");
    }

    char* data = nullptr;
    long len = BIO_get_mem_data(bio, &data);

    string output(data, len);

    BIO_free(bio);

    return output;
}

EVP_PKEY* generate_key() {

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(
        EVP_PKEY_EC, nullptr
    );

    if (!ctx)
        throw runtime_error("Key context allocation failed");

    EVP_PKEY* key = nullptr;

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(
            ctx, NID_secp256k1
        ) <= 0 ||
        EVP_PKEY_CTX_set_ec_param_enc(
            ctx, OPENSSL_EC_NAMED_CURVE
        ) <= 0 ||
        EVP_PKEY_keygen(ctx, &key) <= 0) {

        EVP_PKEY_CTX_free(ctx);
        throw runtime_error("Key generation failed");
    }

    EVP_PKEY_CTX_free(ctx);

    return key;
}

string sign_message(EVP_PKEY* privateKey, const string& message) {

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (!ctx)
        throw runtime_error("Digest context allocation failed");

    size_t sigLen = 0;

    if (EVP_DigestSignInit(
        ctx, nullptr, EVP_sha256(), nullptr, privateKey
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("Signature initialization failed");
    }

    if (EVP_DigestSign(
        ctx, nullptr, &sigLen,
        reinterpret_cast<const unsigned char*>(message.data()),
        message.size()
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("Signature size calculation failed");
    }

    vector<unsigned char> signature(sigLen);

    if (EVP_DigestSign(
        ctx, signature.data(), &sigLen,
        reinterpret_cast<const unsigned char*>(message.data()),
        message.size()
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        throw runtime_error("Signing failed");
    }

    EVP_MD_CTX_free(ctx);

    signature.resize(sigLen);

    return to_hex(signature.data(), signature.size());
}

int main() {
    WSADATA wsaData;
    SOCKET listensocket = INVALID_SOCKET;
    SOCKET clientsocket = INVALID_SOCKET;

    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << '\n';
        return 1;
    }

    listensocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listensocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: "
                  << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(23);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    result = bind(
        listensocket,
        reinterpret_cast<sockaddr*>(&serverAddress),
        sizeof(serverAddress)
    );

    if (result == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << '\n';
        closesocket(listensocket);
        WSACleanup();
        return 1;
    }

    result = listen(listensocket, SOMAXCONN);

    if (result == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << '\n';
        closesocket(listensocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port 23...\n";

    clientsocket = accept(listensocket, nullptr, nullptr);

    if (clientsocket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << '\n';
        closesocket(listensocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected!\n";

    string raw;
    char buffer[4096];
    bool receiveError = false;


    while (true) {
        int n = recv(
            clientsocket,
            buffer,
            sizeof(buffer),
            0
        );

        if (n > 0) {
            raw.append(buffer, n);
        }
        else if (n == 0) {
            break;
        }
        else {
            cerr << "Receive error: "
                << WSAGetLastError() << endl;
            break;
        }
    }

    if (!receiveError && !raw.empty()) {

        cout << "Request:\n" << raw << endl;

        try {
            auto message = parse_message(raw);

            string name = message["NAME"];
            string imperian = message["IMPERIAN"];
            string id = message["ID"];
            string ipv4 = message["IPV4"];
            string ipv6 = message["IPV6"];
            string email = message["EMAIL"];
            string txt = message["TXT"];
            string comp = message["COMPANY"];
            string phone = message["PHONE"];
            string iso = message["ISO"];
            string prov_state = message["PROVINCE/STATE"];
            string city = message["CITY"];
            string street = message["STREET"];
            string addr = message["ADDRESS"];
            string postal = message["POSTAL"];
            string fax = message["FAX"];
            string admin = message["ADMIN"];
            string rgstr = message["REGISTRAR"];
            string rgstrid = message["REGISTRAR ID"];
            string creation = message["CREATION"];
            string expiry = message["EXPIRY"];
            string sign = message["SIGNATURE"];

            cout << "Name: " << name << endl;
            cout << "Imperian: " << imperian << endl;

            EVP_PKEY* privateKey = generate_key();

            string publicPem = key_to_pem(privateKey, false);
            string privatePem = key_to_pem(privateKey, true);

            string publicHash = sha256_hex(
                reinterpret_cast<const unsigned char*>(publicPem.data()),
                publicPem.size()
            );

            string privateHash = sha256_hex(
                reinterpret_cast<const unsigned char*>(privatePem.data()),
                privatePem.size()
            );

            string message = name + "." + ip;

            string signature = sign_message(privateKey, message);


            if (name_exists(imperian)) {
                const char* response = "NAME REQUEST:EXISTS";
                send(clientsocket, response, static_cast<int>(strlen(response)), 0);
            }else{
                string key = registrar_exist(rgstr);
                if (verify_signature(key, imperian, sign)) {
                    cout << "Signature VALID!" << endl;
                    string sl1 = "thr-rsa-gns  A  192.168.1.8";
                    string sl2 = "gna-rsa-gns  A  192.168.1.8";
                    string sl3 = "wirp-rsa-gns  A  192.168.1.8";

                    string filename = "data/" + name + ".region";
                    ofstream file(filename);
                    if (!file.is_open()) {
                        cerr << "Unable to create region file!" << endl;
                    } 
                    else{
                        file << "$REGION " << name << "\n"
                            << "@       TIN        " << imperian << "\n"
                            << "@       ID         " << id << "\n"
                            << "\n"
                            << "@       AAOC       Global Internet Names and Numbers Authority\n"
                            << "$       IPT        V4 & V6\n"
                            << "$       SL         " << ipv4 << "\n"
                            << "$       SLV6       " << ipv6 << "\n"
                            << "wddd    LOC        " << ipv4 << " | " << ipv6 << "\n"
                            << "\n"
                            << "!       RGSTR      " << rgstr << "\n"
                            << "!       RGSTRID    " << rgstrid << "\n"
                            << "Server Location (\n"
                            << "    " << sl1 << "\n"
                            << "    " << sl2 << "\n"
                            << "    " << sl3 << "\n"
                            << ")\n"
                            << "\n"
                            << "Authoritative Server (\n"
                            << "    " << sl1 << "\n"
                            << "    " << sl2 << "\n"
                            << "    " << sl3 << "\n"
                            << ")\n"
                            << "\n"
                            << "@      MOINN       " << email << "\n"
                            << "\n"
                            << "?      COM         " << comp << "\n"
                            << "?      CC          " << iso << "\n"
                            << "?      NUM         " << phone << "\n"
                            << "?      FAX         " << fax << "\n"
                            << "\n"
                            << ">      ADDR        " << addr << "\n"
                            << ">      POSTAL      " << postal << "\n"
                            << ">      CITY        " << city << "\n"
                            << ">      STREET      " << street << "\n"
                            << ">      ADMIN       " << admin << "\n"
                            << ">      PROV/STE    " << prov_state << "\n"
                            << "\n"
                            << "INC     " << creation << "\n"
                            << "INE     " << expiry << "\n"
                            << "\n"
                            << "#       SECKEY     " << privatePem << "\n"
                            << "#       PUBKEY     " << publicPem << "\n"
                            << "\n"
                            << "#       SECKEYH    " << publicHash << "\n"
                            << "#       PUBKEYH    " << privateHash << "\n"
                            << "#       SIGN       " << signature << "\n"
                            << "\n"
                            << "@       TTL        3600\n";

                        file.flush();
                        if (!file) {
                            cerr << "Error writing region file!" << endl;
                        } else {
                            cout << "Region file created: " << filename << endl;
                        }
                        file.close();
                    }
                    string logfile = "data/" + name + "-name.log";
                        
                } else {
                    cout << "Invalid signature!" << endl;

                    const char* response = "NAME REQUEST:INVALID SIGNATURE";
                    send(clientsocket, response, strlen(response), 0);
                }
            }
            


        }
        catch (const exception& e) {
            cerr << e.what() << endl;
        }
        
    }
    else if (raw.empty() && !receiveError) {

        cout << "Client disconnected without sending data.\n";
    }
    else {
        std::cerr << "Receive failed: " << WSAGetLastError() << '\n';
    }

    closesocket(clientsocket);
    closesocket(listensocket);
    WSACleanup();

    std::cout << "Server shut down.\n";

    return 0;
}
