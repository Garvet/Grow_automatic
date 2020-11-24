#ifndef __GROW_DEVICE_H__
#define __GROW_DEVICE_H__

#define SERIAL_LOG_OUTPUT

#include <Arduino.h>
#include <vector>
#include <Grow_device_component.h>

#include <Wire.h>
#include <RtcDateTime.h>

// (!) ----- (-) ----- перенести на статическую память
// (-) ----- перенести работу модуля на удалёнку и добавить период опроса

class Grow_device {
private:
    // (-) ----- период добавь
    uint32_t system_id_;
    uint16_t address_; // адрес устройства
    std::vector<Grow_device_component> component_; // вектор компонентов устройств (не как плата, а как механический модуль)

    unsigned long read_time_; // время последнего считывания
    unsigned long end_time_; // время последней проверки для считывания
    unsigned long period_; // период считывания в мс
    bool readout_signal_; // флаг готовности считывания

    uint8_t active_;  // состояние активации 0 - не активен, 1 - в процессе регистрации, 2 - зарегистрирован // (-) ----- обнулить значения при false запретить функции обработки
    bool change_value_; // флаг изменённого значения

    uint8_t setting_; // настройски (для LoRa-протокола)
public:
    Grow_device() = default;
    Grow_device(uint8_t amt_component, enum Type_device* type_sensor);
    Grow_device(uint8_t amt_component, uint8_t* type_sensor);
    Grow_device(std::vector<enum Type_device> type_device);
    ~Grow_device() = default;

    /// --- Поля класса-платы ---

    // установка индивидуального номера платы
    void set_system_id(uint32_t system_id_);
    // получение индивидуального номера платы
    uint32_t get_system_id();

    // установка состояния активности
    void set_active(uint8_t active);
    // получение состояния активности
    uint8_t get_active(); 

    // получение флага об изменённом состоянии)
    bool get_change_value();
    // сброс флага изменённого состояния
    void clear_change_value(); 

    // установка адреса модуля (ветви, без группы)
    bool set_address(uint16_t address); 
    // получение адреса модуля (ветви, без группы)
    uint16_t get_address(); 

    // Установка настроек LoRa-передачи
    void set_setting(uint8_t setting); 
    // Получение настроек LoRa-передачи
    uint8_t get_setting(); 

    /// --- Обработка времени ---

    // Установить период опроса модуля
    void set_period(unsigned long period);
    // Получить период опроса модуля
    unsigned long get_period();  

    // Проверка необходимости считывания 
    bool check_time(unsigned long time);
    // Обновление времени считывания
    void update();
    // Получение сигнала готовности к считыванию
    bool read_signal(bool clear=false);

    /// --- Поля компонентов ---

    // получение типа компонента (передаётся в result), если ошибка возврат true
    bool get_type(uint8_t num, enum Type_device &result); 
    // получение вектора типов компонентов
    std::vector<enum Type_device> get_type();
    // получение id компонента (передаётся в result | не путать с ID платы, этот номер для количества повторений), если ошибка возврат true
    bool get_id(uint8_t num, uint8_t &result); 
    // получение вектора id компонентов
    std::vector<uint8_t> get_id();

    // Установить значение ШИМ сигнала, если ошибка возврат true
    bool set_pwm_value(uint8_t num, uint16_t pwm_value);
    // Получить значение ШИМ сигнала, если ошибка возврат true
    bool get_pwm_value(uint8_t num, uint16_t &result);
    // Установить значения ШИМ сигналов, если ошибка возврат true
    bool set_pwm_value(std::vector<uint16_t> pwm_value);
    // Получить вектор значений ШИМ сигналов
    std::vector<uint16_t> get_pwm_value();

    // Установить значение считанного состояния, если ошибка возврат true
    bool set_state(uint8_t num, bool state);
    // Получить значение считанного состояния, если ошибка возврат true
    bool get_state(uint8_t num, bool &result);
    // Установить значения считанных состояний, если ошибка возврат true
    bool set_state(std::vector<bool> state);
    // Получить вектор значений считанных состояний
    std::vector<bool> get_state();

    // Установить настройки временного контроля, если ошибка возврат true
    bool set_time_control(uint8_t num, Time_control time_control);
    // Получить настройки временного контроля, если ошибка возврат true
    bool get_time_control(uint8_t num, Time_control &result);
    // Установить настройки временного контроля всех компонентов, если ошибка возврат true
    bool set_time_control(std::vector<Time_control> time_controls);
    // Получить настройки временного контроля всех компонентов
    std::vector<Time_control> get_time_control();

    /// --- Информации о компонентах ---

    // Получить количество компонентов
    uint8_t get_count_component();

    // Получить определённый компонент
    Grow_device_component get_component(uint8_t num);
    // Получить вектор компонентов
    std::vector<Grow_device_component> get_component();

    /// --- Внешняя связь ---

    // Проверить совподает ли содержимое модулей, без учёта настроек (для отфильтровывания среди неподходящих)
    bool filter(Grow_device &device); // (-) ----- убрать привязку к порядку

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса (-) ----- добавить генерацию пакетов (возможно другой класс)
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);

#if defined (SERIAL_LOG_OUTPUT)
    /// вывод класса в Serial
    void print(); // (-) ---- добавить в вывод обработку канала
#endif
};


#endif // __GROW_DEVICE_H__