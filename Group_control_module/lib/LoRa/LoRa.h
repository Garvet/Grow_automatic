#ifndef __LORA_H__
#define __LORA_H__

#include <Arduino.h>
#include <iostream>
#include <SPI.h>
// #include <Address_field.h>
// #include <LoRa_register_data.h>
#include <LoRa_register.h>
#include <LoRa_packet.h>
#define HELTEC_LORA 4

class LoRa {
private:
    SPIClass* _spi; // Шина SPI
    SPISettings* _setting; // Настройки шины SPI
    uint8_t _nss;   // Сигнальный выход SPI
    uint8_t _reset; // Сброс LoRa-модуля
    uint8_t _dio0;  // Сигнальный выход DIO0
    uint8_t _dio1;  // Сигнальный выход DIO1
    uint8_t _dio3;  // Сигнальный выход DIO3

    ulong _frequency; // Частота радиоканала

    uint8_t _FifoTxBaseAddr; // Начальный адрес буфера отправки 
    uint8_t _packet_length;  // Текущая длина пакета (максимальная = 255)
    class LoRa_register _reg_field; // Регистры LoRa-модуля
    // class LoRa_packet packet; // Последний принятый пакет

    bool _init = false; // Была ли LoRa проинициализированна

    friend class LoRa_register;
public:
    LoRa();
    LoRa(uint8_t pin_reset, uint8_t spi_bus=HSPI, uint8_t spi_nss=0, uint8_t pin_dio0=0, 
         uint8_t pin_dio1=0, uint8_t pin_dio3=0);
    ~LoRa();

    bool init(uint8_t pin_reset, uint8_t spi_bus=HSPI, uint8_t spi_nss=0, uint8_t pin_dio0=0, 
              uint8_t pin_dio1=0, uint8_t pin_dio3=0);

    // Установить значение полю
    uint8_t field_set(Address_field field, uint32_t value, bool write=true);
    uint8_t field_set(Address_field* fields, uint32_t* values, uint8_t amt, bool write=true);
    // uint8_t field_set(std::vector<Address_field> fields, std::vector<uint32_t> values, bool write=true);

    // Получить значение поля
    uint8_t field_get(Address_field field, uint32_t* value, bool read=false);
    uint8_t field_get(Address_field* fields, uint32_t* values, uint8_t amt, bool read=false);
    // uint8_t field_get(std::vector<Address_field> fields, std::vector<uint32_t>* values, bool read=false);
    
    // Запуск LoRa-модуля
    uint8_t begin(ulong frequency, bool paboost=false, uint8_t signal_power=14, 
                  uint8_t SF=11, ulong SBW=125E3, uint8_t sync_word=0x4A);
    // Завершение работы LoRa-модуля с переходом в режим сна
    void end();

    // --- Настройка модуля --- 
    // Установка режимов
    uint8_t set_mode(uint8_t mode); // По значению
    uint8_t mode_sleep(); // Сон
    uint8_t mode_STDBY(); // Ожидание
    uint8_t mode_TX(bool set_dio=true); // Отправки (начинает передачу пакета, после - уходит в сон)
    uint8_t mode_FSTX(); // Подготовки к отправке
    uint8_t mode_RX_continuous(bool set_dio=true); // Безпрерывного приёма (постоянно в ожидании пакетов, в сон не уходит)
    uint8_t mode_RX_single(bool set_dio=true); // Приёма (начинает приём пакета, после приёма или через 2 секунды включает режим сна)
    uint8_t mode_FSRX(); // Режим подготовки к приёму
    uint8_t mode_CAD(bool set_dio=true); // Проверки активности сети
    // Установка силы сигнала
    uint8_t set_TX_power(uint8_t power, bool paboost, uint8_t max_power=0x07);
    // Установка частоты радиоканала
    uint8_t set_frequency(ulong frequency);
    //Установка коэффициента распространения
    uint8_t set_spreading_factor(uint8_t SF);
    // Установка пропускной способности
    uint8_t set_signal_bandwidth(ulong sbw);
    // Установка длины преамбулы сообщающей об отправке пакета (на неё реагирует приёмник)
    uint8_t set_preamble_length(uint length);
    // Установка кодового слова (LoRa-модуль будет отсеивать пакеты с несовпадающим SW, но не всегда, на шум не проверяется)
    uint8_t set_sync_word(uint8_t SW);
    // Включение проверки на ошибку (контрольная сумма в пакете)
    uint8_t crc_enable();
    // Выключение проверки на ошибку (контрольная сумма в пакете)
    uint8_t crc_disable();

    // --- Отправка данных --- 
    // Начать приём пакета (количество, время ожидания пакета, считывать ли rssi и snr)
    class LoRa_packet receiver_packet(uint8_t count=1, ulong wait=10000, bool rssi=false, bool snr=false);
    // Прочитать принятый пакет (установить значение ожибки, считывать ли rssi и snr)
    class LoRa_packet read_packet_data(bool crc_err, bool rssi=false, bool snr=false);
    // Считывание rssi
    uint8_t packet_rssi();
    // Считывание snr
    float packet_snr();

    // --- Приём данных --- 
    // Отправка пакета (wait - ожидание завершение отправки)
    bool sender_packet(uint8_t* packet, uint8_t len, bool wait=true);
    bool sender_packet(const std::vector<uint8_t>& packet, bool wait=true);
    // Подготовиться к отправке пакета
    bool packet_begin();
    // Добавить данные в пакет (суммарное ограничение 255 байт)
    bool packet_write(uint8_t* packet, uint8_t len);
    bool packet_write(const std::vector<uint8_t>& packet);
    // Завершить и отправить пакет
    bool packet_end(ulong wait=2000, bool sleep=false);
}typedef LoRa;

#endif // __LORA_H__