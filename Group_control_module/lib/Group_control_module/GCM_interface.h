#ifndef GROUP_CONTROL_MODULE_H_
#define GROUP_CONTROL_MODULE_H_

#include <Arduino.h>
#include <Group_control_module.h>
// WiFi
#include <WiFi.h>
#include <WiFiMulti.h>
// #include <FS.h>
#include <WiFiClient.h>
// #include <WiFiUdp.h>
// Bluetooth
#include <BluetoothSerial.h>
//
#include <System_component.hpp>

// #include <Packet_analyzer.h>
// #include <LoRa_packet.h>
// #include <LoRa_contact_data.h>

#define AMT_BYTES_BUFFER 1000

#define AMT_BYTES_NETWORK_NAME     100
#define AMT_BYTES_NETWORK_PASSWORD 100
#define AMT_BYTES_NETWORK_ADDRESS  16

#define AMT_BYTES_BLUETOOTH_NAME   100

class GCM_interface {
private:
    Group_control_module* gcm_ = nullptr;
    // std::array<uint8_t, AMT_BYTES_BUFFER> buffer;
    // uint16_t len_buffer = 0;

    bool init_server = false;
    
    // Настройки подключения к Bluetooth
    std::array<char, AMT_BYTES_BLUETOOTH_NAME> bluetooth_name_; // "GreenHouseX"
    uint8_t  len_bluetooth_name_ = 0;

    BluetoothSerial SerialBT_;
    int command_code_ = 0;

    
    friend void WiFiEvent(WiFiEvent_t event);
    friend void connectToWiFi();
    friend bool WiFisend();
    
public:
    GCM_interface() = default;
    ~GCM_interface() = default;
    /// --- Инициализация --- 
    bool set_group_control_module(Group_control_module * gcm);
    bool init_server_connect(
        std::array<char, AMT_BYTES_NETWORK_NAME> network_name, uint8_t  len_network_name,
        std::array<char, AMT_BYTES_NETWORK_PASSWORD> network_pswd, uint8_t  len_network_pswd,
        std::array<char, AMT_BYTES_NETWORK_ADDRESS> network_adr, uint8_t  len_network_adr,
        uint32_t network_port);

    /// --- Сохранение в энергонезависимую память ---
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);

    /// --- Обмен с телефоном ---
    

    /// --- Отчётность ---
    // Регистрация в системе
    uint16_t report_to_server_regist_data(); // прописать пакет
    // Отправка данных датчиков и устройств
    uint16_t report_to_server_read_data(); // clear_change_value = false
    // Отправка данных об ошибках
    uint16_t report_to_server_error();

};

extern GCM_interface gcm_interface;

#endif // GROUP_CONTROL_MODULE_H_