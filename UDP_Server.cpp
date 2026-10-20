
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

#include "Protocol.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 12000
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Привязываем сервер к порту
    if (bind(sockfd, (sockaddr*)&serverAddr,
             sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: "
                  << WSAGetLastError() << '\n';
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port "
              << PORT << '\n';

    float playerX = 0.0f;
    float playerY = 0.0f;
    float playerZ = 0.0f;

    while (true) {
        char buffer[BUFFER_SIZE];

        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);

        // Получаем пакет от клиента
        int n = recvfrom(
            sockfd, buffer, BUFFER_SIZE, 0,
            (sockaddr*)&clientAddr, &clientAddrLen
        );

        if (n == SOCKET_ERROR) {
            std::cerr << "Receive failed: "
                      << WSAGetLastError() << '\n';
            continue;
        }

        // Проверяем, что получен хотя бы заголовок
        if (n < sizeof(PacketHeader)) {
            std::cerr << "Packet too small\n";
            continue;
        }

        PacketHeader header{};
        memcpy(&header, buffer, sizeof(header));

        // Проверяем размер полезной нагрузки
        if (header.payloadSize !=
            n - static_cast<int>(sizeof(PacketHeader))) {
            std::cerr << "Invalid payload size\n";
            continue;
        }

        char* payload = buffer + sizeof(PacketHeader);
        ServerResponse response{};
        response.sequenceNumber = header.sequenceNumber;

        bool valid = true;

        // Обрабатываем команду
        switch (header.type) {
        case MOVEMENT: {
            if (header.payloadSize != sizeof(MovementData)) {
                valid = false;
                break;
            }

            MovementData data{};
            memcpy(&data, payload, sizeof(data));

            playerX = data.x;
            playerY = data.y;
            playerZ = data.z;

            std::cout << "[MOVEMENT] seq="
                      << header.sequenceNumber
                      << " position=(" << playerX << ", "
                      << playerY << ", " << playerZ << ")\n";

            strcpy_s(response.message, sizeof(response.message),
                     "Position updated");
            break;
        }

        case SHOOT: {
            if (header.payloadSize != sizeof(ShootData)) {
                valid = false;
                break;
            }

            ShootData data{};
            memcpy(&data, payload, sizeof(data));

            std::cout << "[SHOOT] seq="
                      << header.sequenceNumber
                      << " weaponId="
                      << static_cast<int>(data.weaponId)
                      << '\n';

            strcpy_s(response.message, sizeof(response.message),
                     "Shot registered");
            break;
        }

        default:
            valid = false;
            break;
        }

        if (!valid) {
            std::cerr << "Invalid command or payload\n";
            continue;
        }

        // Отправляем подтверждение клиенту
        PacketHeader replyHeader{
            ACK,
            header.sequenceNumber,
            sizeof(response)
        };

        char reply[sizeof(PacketHeader) + sizeof(ServerResponse)];

        memcpy(reply, &replyHeader, sizeof(replyHeader));
        memcpy(reply + sizeof(replyHeader),
               &response, sizeof(response));

        int replySize = sizeof(replyHeader) + sizeof(response);

        if (sendto(sockfd, reply, replySize, 0,
                   (sockaddr*)&clientAddr,
                   clientAddrLen) == SOCKET_ERROR) {
            std::cerr << "Send failed: "
                      << WSAGetLastError() << '\n';
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
