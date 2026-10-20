
#pragma once

#include <cstdint>

// Типы сетевых команд
enum PacketType : uint8_t {
    MOVEMENT = 1,
    SHOOT = 2,
    ACK = 3
};

// Все структуры имеют одинаковую упаковку
#pragma pack(push, 1)

struct PacketHeader {
    uint8_t type;           // Тип команды
    uint16_t sequenceNumber; // Порядковый номер
    uint16_t payloadSize;    // Размер данных
};

struct MovementData {
    float x;
    float y;
    float z;
};

struct ShootData {
    uint8_t weaponId;
};

struct ServerResponse {
    uint16_t sequenceNumber;
    char message[128];
};

#pragma pack(pop)
