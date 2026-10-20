// UDP_Client.cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <string>

#define PORT 12000
#define SERVER_ADDR "127.0.0.1"
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
    struct sockaddr_in serverAddr;
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
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

    while (true) {
        char message[BUFFER_SIZE];

        std::cout << "Enter message: ";
        std::cin.getline(message, BUFFER_SIZE);

        // Игнорируем пустые сообщения (опционально, для стабильности)
        if (strlen(message) == 0) {
            continue;
        }

        // Send message to server (флаг MSG_CONFIRM заменен на 0)
        sendto(sockfd, message, (int)strlen(message), 0, (const struct sockaddr *) &serverAddr, sizeof(serverAddr));
        
        // Receive server response (флаг MSG_WAITALL заменен на 0)
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        
        if (n > 0) {
            buffer[n] = '\0';  // Null terminate the received message
            std::cout << "Server response: " << buffer << std::endl;
        } else {
            std::cerr << "Receive failed or connection closed." << std::endl;
            break;
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}