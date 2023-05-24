#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define DEFAULT_ADDR "127.0.0.1"
#define DEFAULT_ADDR "100.35.75.61"

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

    while (true) {
        iResult = recv(ServerSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            printf("%s\n", recvbuf);
        }
    }
}

int main(void)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    SOCKET* ConnectSocketPtr = NULL;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
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
        return 1;
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
            return 1;
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
        return 1;
    }

    HANDLE recieveThread = CreateThread(NULL, 0, HandleRecieve, reinterpret_cast<LPVOID>(ConnectSocketPtr), 0, NULL);
    HANDLE sendThread = CreateThread(NULL, 0, HandleSend, reinterpret_cast<LPVOID>(ConnectSocketPtr), 0, NULL);

    WaitForSingleObject(recieveThread, INFINITE);
    WaitForSingleObject(sendThread, INFINITE);

    return 0;
}