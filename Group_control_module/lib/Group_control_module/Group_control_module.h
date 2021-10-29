#ifndef __GROUP_CONTROL_MODULE__H__
#define __GROUP_CONTROL_MODULE__H__

#include <Arduino.h>
#include <algorithm>
#include <vector>

#include <Wire.h>
#include <LoRa.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

#include <System_component.hpp>

#include <LoRa_contact_data.h>
#include <Grow_sensor.h>
#include <Grow_sensor_interface.h>
#include <Grow_device.h>
#include <Grow_device_interface.h>

// (!) ----- (-) ----- перенести на статическую память

#define SYNC_WORD  0x4A // Кодовое слово (должно совпадать у всех модулей сети)
#if defined(BUILD_BASEMENT)
#define REIST_BAND 43455E4
#elif defined( BUILD_OBJECT1 )
// #define REIST_BAND 43455E4
#define REIST_BAND 43455E4
#else
#define REIST_BAND 43325E4 // Частота работы при регистрации (-) ----- добавить остальные параметры канала
#endif

#define MAX_AMT_SENSOR 0x3FF5 // максимальное число датчиков (условно)
#define MAX_AMT_DEVICE 0x3FF5 // максимальное число устройств (условно)

#if defined( BUILD_BASEMENT )
#define SYSTEM_ID_GCM std::array<uint8_t, AMT_BYTES_SYSTEM_ID>({0x5f, 0xdc, 0xbd, 0xcb, 0x5f, 0x25, 0x97, 0x30, 0x86, 0x17, 0x08, 0x9b})
#else
#define SYSTEM_ID_GCM std::array<uint8_t, AMT_BYTES_SYSTEM_ID>({0x60, 0x86, 0xcc, 0xa2, 0x34, 0x2b, 0x73, 0x2b, 0x2d, 0x4a, 0x42, 0x74})
#endif

class Group_control_module : public scs::System_component {
// private: // (-) ----- верни приватность полей и некоторых функций
public:
    const std::array<uint8_t, AMT_BYTES_SYSTEM_ID> system_id_ = SYSTEM_ID_GCM; // id модуля
    std::vector<char> name_; // Имя модуля (в т.ч. Bluetooth)
    enum Mode {
        GT_SETTING = 0, // Режим настройки (установка конфигурации и регистрация моделей)
        GT_PROCESSING   // Режим работы системы
    } mode_; // Режим работы
    LoRa_contact_data contact_data_;   // Класс контроля протокола обмена
    // std::vector<> _calculated_data; // Класс математической интеграции (!) -----
    /// Есть ли необходимость общения с датчиками/устройствами
    bool check_sensor_ = false;        // Наличие датчиков для считывания
    bool check_device_ = false;        // Наличие устройств для изменения состояния
    /// Компоненты группы
    std::vector<Grow_device> devices_; // Список устройств конфигурации
    std::vector<Grow_sensor> sensors_; // Список датчиков конфигурации

    /// Устаревшая система адресации 
    std::vector<uint16_t> del_adr_; // Список удалённых адресов  // (--) ----- (исключается)
    uint16_t end_adr_;  // последний адрес                       // (--) ----- (исключается)

    /// Модули запросившие регистрацию (-) ---- интегрировать ID (-) ----- array
    std::vector<Grow_device> reg_devices_; // список устройств для регистрации
    std::vector<Grow_sensor> reg_sensors_; // список датчиков для регистрации

    /// RTC
    RtcDS1302<ThreeWire> rtc_; // Модуль реального времени 
    bool rtc_begin_ = false;  // RTC был запущен
    RtcDateTime date_time_;  // Последнее считанное время

    /// Регистрация модуля
    bool permission_regist_interrupt_ = true; // false когда идёт добавление/фильтрация модулей, чтобы список не мог измениться
    std::vector<std::array<uint8_t, AMT_BYTES_SYSTEM_ID>> filter_adr_; // вектор отфильтрованных адресов (-) ---- изменить на ID
    void enable_regist_interrupt();  // разрешается обработка LoRa
    void disable_regist_interrupt(); // запрещается обработка LoRa
    void interrupt_init(uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3); // функция прерывания для LoRa

    // uint16_t creat_address();                             // (--) ----- (исключается)
    // void delete_address(uint16_t adr);                    // (--) ----- (исключается)
    // bool add_sensors(uint16_t old_adr, uint16_t new_adr); // (--) ----- (исключается)
    // bool del_sensors(uint16_t adr);                       // (--) ----- (исключается)
    // bool add_device(uint16_t old_adr, uint16_t new_adr);  // (--) ----- (исключается)
    // bool del_device(uint16_t adr);                        // (--) ----- (исключается)

    // Добавить модуль в список запросивших регистрацию
    bool add_reg_module(const LoRa_packet &reg_packet);
    // Очистка переменных регистрации
    void clear_regist_data();

    /// Обработка модулей группы
    // Проверка необходимости считывания с устройств (-) ----- перенести на удалёнку
    bool check_device_period();
    bool check_device_read(); // (-) ----- костыль
    // Проверка необходимости считывания с датчиков
    bool check_sensor_read();

    // Произвести общение с устройством
    bool handler_devices();
    // Произвести общение с датчиком
    bool handler_sensors();

    // save в EEPROM // (-) ----- (2 шт. КФ и настроек, считываний)
    // load из EEPROM // (-) -----
public:
    Group_control_module();
    Group_control_module(uint8_t pin_reset, uint8_t spi_bus, uint8_t spi_nss, uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3);
    ~Group_control_module();
    // Инициализация параметров LoRa модуля (частота задаётся конфигурацией)
    bool LoRa_init(uint8_t pin_reset, uint8_t spi_bus=VSPI, uint8_t spi_nss=0, 
                   uint8_t pin_dio0=0, uint8_t pin_dio1=0, uint8_t pin_dio3=0);
    // Запуск 
    void rtc_begin();
    uint8_t LoRa_begin(); // 0 - correct, other - num LoRa_error
    uint8_t begin(); // 0 - correct, other - num LoRa_error
    /// Работа класса
    // работа модуля УУГ (проверяет систему, обрабатывает формулы (У|Д|Ф), гинерит пакеты, запускает передачу)
    uint8_t work_system(); 
    // функция прерывания с LoRa (контролирует каждый шаг передачи, по завершению запускает work_system, контролирует трансляцию, заносит запросы регистрации)
    void LoRa_interrupt(); 

    /// Обработка времени
    void set_date_time(RtcDateTime data_time); // установить значение RTC
    RtcDateTime get_date_time(); // считать значение RTC

    // Поиск номера устройства/датчика по параметру
    int search_device(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> search_id);
    int search_sensor(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> search_id);
    int search_device(uint16_t address);
    int search_sensor(uint16_t address);
    // на замену верхних 4-х функций
    int search_module(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> search_id);
    int search_module(uint16_t address);

    /// --- Настройка системы ---
    // все функции кроме set_mode работают только при mode_ = GT_SETTING

    /// Установка конфигурации
    // bool set_configuration(std::vector<uint8_t> data);
    size_t get_size();
    size_t get_data(uint8_t *data);
    size_t set_data(uint8_t *data, size_t available_size); // mode_ = GT_SETTING
    // Фильтрация регистрируемых модулей 
    bool filter_devices(Grow_device &devices);
    bool filter_sensors(Grow_sensor &sensor);
    // [T] Регистрация модуля (old_adr и new_adr задаются Bluetooth модулем)
    bool regist_device(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> device_id, uint16_t new_adr);
    bool regist_sensor(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> sensor_id, uint16_t new_adr);
    // [T] Сигнал модуля (?) ----- пока только в планах
    bool module_set_signal(uint16_t adr);
    // [C] Удаление модуля (?) ----- пока только в планах
    bool remove_module(uint16_t adr);
    // Установка режима работы системы
    bool set_mode(Group_control_module::Mode mode);
    // установка индивидуального номера платы
    std::array<uint8_t, AMT_BYTES_SYSTEM_ID> get_system_id() const;

    // --- Байтовая строка обмена ---
    // (-) ----- сделать функции обмена с Денисом, возможно ещё в set_configuration(std::vector<uint8_t> data);
};


#endif // __GROUP_CONTROL_MODULE__H__
