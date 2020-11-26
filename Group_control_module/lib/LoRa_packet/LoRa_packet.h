#ifndef __LORA_PACKET_H__
#define __LORA_PACKET_H__

#include <Arduino.h>
#include <vector>
// #include <array>
// #include <algorithm>

#define SIZE_LORA_PACKET_MAX_LEN 250 // - 254 max
#define SIZE_LORA_PACKET_BUFFER 350

class LoRa_packet_data;
class LoRa_packet;

class LoRa_packet_data {
    bool free_object_ = true; // Свободный объект
public:
    uint8_t data[SIZE_LORA_PACKET_MAX_LEN]; // Байты
    uint8_t len = 0; // Количество байт

    bool set_data(uint8_t* data, uint8_t len);
    void set_data(const class LoRa_packet& lora_packet);
    void set_data(const class LoRa_packet_data& lora_packet);
    void set_data(class LoRa_packet_data&& lora_packet);

    class LoRa_packet_data& operator=(const class LoRa_packet& right);
    class LoRa_packet_data& operator=(const class LoRa_packet_data& right);

    friend class LoRa_packet;
};

class LoRa_packet {
private:
    LoRa_packet_data* packet_data;
    uint8_t rssi_ = 0;  // RSSI соединения
    bool crc_error_   = false; // Ошибка контрольной суммы

    friend class LoRa_packet_data;
    bool search_data();
public:
    LoRa_packet();
    LoRa_packet(uint8_t* data, uint8_t len, bool crc_err=false, uint8_t rssi=0, float snr=0);
    LoRa_packet(const LoRa_packet& right);
    LoRa_packet(LoRa_packet&& right);
    ~LoRa_packet();
    // Функция заполенния объекта, в конструкторе с параметром происходит автоматически
    bool set_packet(uint8_t* data, uint8_t len, bool crc_err=false, uint8_t rssi=0, float snr=0);

    // получение содержимого пакета
    LoRa_packet_data* get_packet();
    std::vector<uint8_t> get_data(); // (-) -----
    // получение одного байта
    uint8_t get_data(int num); 
    // получение длины пакета
    uint8_t get_len(); 
    // получение ошибки передачи пакета
    bool    get_crc_error(); 
    // получение RSSI пакета
    uint8_t get_rssi();

    // получение одного байта
    uint8_t& operator[](const int index);
    // перегрузка оператора копирования
    LoRa_packet& operator=(const LoRa_packet& right);
    // перегрузка оператора перемещения
    LoRa_packet& operator=(LoRa_packet&& right);
}typedef LoRa_packet;


#endif // __LORA_PACKET_H__