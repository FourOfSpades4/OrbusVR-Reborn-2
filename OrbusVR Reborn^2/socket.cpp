#define WIN32_LEAN_AND_MEAN

#include "client.h"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "socket.h"

#include "net_manager.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET* ConnectSocketPtr = NULL;

int Socket::SendData(std::string data) {
    data += "|";
    return send(*ConnectSocketPtr, data.c_str(), data.length(), 0);
}

DWORD WINAPI HandleSend(LPVOID lpParameter) {
    SOCKET ServerSocket = *reinterpret_cast<SOCKET*>(lpParameter);
    char sendbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;

    while (true) {
        scanf_s("%s", sendbuf);

        iResult = send(ServerSocket, sendbuf, (int)strlen(sendbuf), 0);
    }
}

DWORD WINAPI HandleRecieve(LPVOID lpParameter) {
    SOCKET ServerSocket = *reinterpret_cast<SOCKET*>(lpParameter);
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iSendResult;
    int iResult;

    printf("Sucessfully connected. Receiving.\n");

    while (true) {
        std::fill_n(recvbuf, DEFAULT_BUFLEN, 0);
        iResult = recv(ServerSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::string str = std::string(recvbuf);
            processData(str);
            // CreateThread(NULL, 0, HandleRecievedData, reinterpret_cast<LPVOID>(str), 0, NULL);
        }
    }
}

void Socket::InitializeSocket(void) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    int iResult;

    printf("Creating Socket...\n");

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return;
    }


    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        // Create a SOCKET for connecting to server

        ConnectSocketPtr = new SOCKET(socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol));

        ConnectSocket = *ConnectSocketPtr;
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return;
    }

    HANDLE recieveThread = CreateThread(NULL, 0, HandleRecieve, reinterpret_cast<LPVOID>(ConnectSocketPtr), 0, NULL);
#ifdef MASTER_CLIENT
    HANDLE sendThread = CreateThread(NULL, 0, HandleSend, reinterpret_cast<LPVOID>(ConnectSocketPtr), 0, NULL);
#endif

    WaitForSingleObject(recieveThread, INFINITE);

#ifdef MASTER_CLIENT
    WaitForSingleObject(sendThread, INFINITE);
#endif

    return;
}