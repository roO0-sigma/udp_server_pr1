
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <thread>
#include <chrono>

#include "Protocol.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 12000
#define SERVER_ADDR "127.0.0.1"
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
    serverAddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_ADDR,
                  &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid server address\n";
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Тайм-аут ожидания ответа
    DWORD timeout = 2000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
               (const char*)&timeout, sizeof(timeout));

    uint16_t sequenceNumber = 0;
    int iteration = 0;

    std::cout << "Client started. Press Ctrl+C to stop.\n";

    while (true) {
        PacketHeader header{};
        char payload[sizeof(MovementData)]{};
        uint16_t payloadSize = 0;

        if (iteration > 0 && iteration % 5 == 0) {
            // Формируем команду выстрела
            header.type = SHOOT;

            ShootData data{1};
            memcpy(payload, &data, sizeof(data));
            payloadSize = sizeof(data);

            std::cout << "Sending SHOOT, seq="
                      << sequenceNumber << '\n';
        }
        else {
            // Формируем команду движения
            header.type = MOVEMENT;

            MovementData data{
                static_cast<float>(iteration),
                static_cast<float>(iteration / 2),
                0.0f
            };

            memcpy(payload, &data, sizeof(data));
            payloadSize = sizeof(data);

            std::cout << "Sending MOVEMENT, seq="
                      << sequenceNumber
                      << " position=(" << data.x << ", "
                      << data.y << ", " << data.z << ")\n";
        }

        header.sequenceNumber = sequenceNumber;
        header.payloadSize = payloadSize;

        // Объединяем заголовок и полезную нагрузку
        char packet[sizeof(PacketHeader) + sizeof(MovementData)];

        memcpy(packet, &header, sizeof(header));
        memcpy(packet + sizeof(header), payload, payloadSize);

        int packetSize = sizeof(header) + payloadSize;

        if (sendto(sockfd, packet, packetSize, 0,
                   (sockaddr*)&serverAddr,
                   sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Send failed: "
                      << WSAGetLastError() << '\n';
        }
        else {
            // Получаем ответ сервера
            char buffer[BUFFER_SIZE];

            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                             nullptr, nullptr);

            if (n == SOCKET_ERROR) {
                std::cerr << "Server response timeout or receive error: "
                          << WSAGetLastError() << '\n';
            }
            else if (n != sizeof(PacketHeader) +
                          sizeof(ServerResponse)) {
                std::cerr << "Invalid response size\n";
            }
            else {
                PacketHeader replyHeader{};
                ServerResponse response{};

                memcpy(&replyHeader, buffer, sizeof(replyHeader));
                memcpy(&response, buffer + sizeof(replyHeader),
                       sizeof(response));

                // Проверяем тип ответа и номер команды
                if (replyHeader.type == ACK &&
                    replyHeader.sequenceNumber == sequenceNumber &&
                    replyHeader.payloadSize == sizeof(ServerResponse)) {
                    std::cout << "Server response: "
                              << response.message << '\n';
                }
                else {
                    std::cerr << "Invalid server response\n";
                }
            }
        }

        ++sequenceNumber;
        ++iteration;

        std::this_thread::sleep_for(
            std::chrono::seconds(1));
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
