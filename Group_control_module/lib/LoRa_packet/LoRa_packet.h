#ifndef __LORA_PACKET_H__
#define __LORA_PACKET_H__

#include <Arduino.h>
#include <vector>

class LoRa_packet {
private:
    uint8_t* _data = nullptr; // Байты
    uint8_t _len  = 0;        // Количество байт
    uint8_t _rssi = 0;        // RSSI соединения
    // float _snr = 0;           // SNR соединения
    bool _crc_error = false;  // Ошибка контрольной суммы
public:
    LoRa_packet() = default;
    LoRa_packet(uint8_t* data, uint8_t len, bool crc_err=false, uint8_t rssi=0, float snr=0);
    ~LoRa_packet();
    // Функция заполенния объекта, в конструкторе с параметром происходит автоматически
    void set_packet(uint8_t* data, uint8_t len, bool crc_err=false, uint8_t rssi=0, float snr=0);

    // получение содержимого пакета
    std::vector<uint8_t> get_data();
    // получение одного байта
    uint8_t get_data(int num); 
    // получение длины пакета
    uint8_t get_len(); 
    // получение ошибки передачи пакета
    bool    get_crc_error(); 
    // получение RSSI пакета
    uint8_t get_rssi();
    // получение SNR пакета
    float   get_snr();

    // получение одного байта
    uint8_t& operator[](const int index);
    // перегрузка оператора копирования
    LoRa_packet& operator=(const LoRa_packet& right);
}typedef LoRa_packet;


#endif // __LORA_PACKET_H__