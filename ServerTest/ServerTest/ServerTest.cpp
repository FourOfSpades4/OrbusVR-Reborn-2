#define WIN32_LEAN_AND_MEAN

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

std::vector<SOCKET*> clients;


DWORD WINAPI HandleClientSend(LPVOID lpParameter) {
    char sendbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;

    while (true) {
        std::cout << clients.size() << std::endl;

        scanf_s("%s", sendbuf);

        for (SOCKET* client : clients) {
            iResult = send(*client, sendbuf, (int)strlen(sendbuf), 0);
        }
    }
}

void SendMessage(std::string message) {
    for (SOCKET* client : clients) {
        send(*client, message.c_str(), message.length(), 0);
    }
}

std::vector<std::string> split(const std::string& s, const char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

DWORD WINAPI HandleClientRecieve(LPVOID lpParameter) {
    SOCKET* ClientSocket = reinterpret_cast<SOCKET*>(lpParameter);
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iSendResult;
    int iResult;

    while (true) {
        std::fill_n(recvbuf, DEFAULT_BUFLEN, 0);

        iResult = recv(*ClientSocket, recvbuf, recvbuflen, 0);
        std::string str = std::string(recvbuf);

        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            printf("%s\n", recvbuf);
        }   
        if (iResult == SOCKET_ERROR) {
            printf("Client Disconnected.\n");
            clients.erase(std::remove(clients.begin(), clients.end(), ClientSocket), clients.end());
            return -1;
        }

        SendMessage(str);
    }

    return 0;
}

int main() {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;



    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    CreateThread(NULL, 0, HandleClientSend, NULL, 0, NULL);

    while (true) {
        SOCKET* ClientSocket = new SOCKET(accept(ListenSocket, NULL, NULL));

        if (*ClientSocket != INVALID_SOCKET) {
            printf("Connecting Client...\n");
            clients.push_back(ClientSocket);
            CreateThread(NULL, 0, HandleClientRecieve, reinterpret_cast<LPVOID>(ClientSocket), 0, NULL);
        }

    }

    return 0;
}