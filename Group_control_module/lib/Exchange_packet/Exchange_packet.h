#ifndef __EXCHANGE_PACKET_H__
#define __EXCHANGE_PACKET_H__

#include <Arduino.h>
#include <vector>
#include <Packet_analyzer.h>
#include <LoRa_packet.h>

// (!) ----- (-) ----- перенести на статическую память

class Exchange_packet {
    uint8_t setting_ = 0; // Настройки пакета
public:
    class Packet_analyzer *packet = nullptr; // указатель на класс обработчик пакетов
    LoRa_packet packet_data;       // сам пакет
    // uint8_t *packet_data;       // указатель на сам пакет
    // uint8_t len = 0;            // длина пакета
    uint8_t type_packet = 0xFF; // тип пакета

    // Конструкторы
    Exchange_packet();
    Exchange_packet(const Exchange_packet &exchange_packet);
    Exchange_packet(Exchange_packet &&exchange_packet);
    ~Exchange_packet();
    // Получение пакета в виде вектора
    std::vector<uint8_t> get_packet() const;
    // Установка пакета из вектора
    void set_packet(const std::vector<uint8_t>& pack);

    // Получение настроек пакета
    uint8_t get_setting() const;
    // Установка настроек пакета
    void set_setting(uint8_t setting);

    // Создание пакета (-) ----- (убрать, использовалось из-за динамической памяти для работы с Packet_analyzer, т.к. небыло доступа при 0-й длине)
    void creat_packet(uint8_t len, uint8_t type_packet);
    // Создание пакета
    void clear_packet();
    // Оператор копирования
    class Exchange_packet& operator=(const class Exchange_packet& right);
    // Оператор перемещения
    class Exchange_packet& operator=(class Exchange_packet&& right);
};

#endif // __EXCHANGE_PACKET_H__