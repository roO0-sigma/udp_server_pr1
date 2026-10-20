// UDP_Server.cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <cctype>

#define PORT 12000
#define BUFFER_SIZE 1024

// Winsock в Visual Studio
#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsaData;
    // Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return -1;
    }

    SOCKET sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen; // В Windows используется int, а не socklen_t
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        WSACleanup();
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    
    // Configure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket 
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Wait for client messages
    while (true) {
        clientAddrLen = sizeof(clientAddr);
        
        // Receive message (флаг MSG_WAITALL заменен на 0)
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &clientAddr, &clientAddrLen);
        
        if (n > 0) {
            buffer[n] = '\0';  // Null terminate the received message

            std::cout << "Received message: " << buffer << std::endl;
            
            // UpperCase buffer
            for (int i = 0; i < n; i++) {
                // Приведение к unsigned char предотвращает неопределенное поведение для отрицательных значений char
                buffer[i] = toupper((unsigned char)buffer[i]);
            }
            
            // Send response (флаг MSG_CONFIRM заменен на 0)
            sendto(sockfd, buffer, n, 0, (const struct sockaddr *) &clientAddr, clientAddrLen);
        } else {
            std::cerr << "Receive failed." << std::endl;
            break;
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}