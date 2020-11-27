#ifndef __GROW_SENSOR_H__
#define __GROW_SENSOR_H__

#define SERIAL_LOG_OUTPUT

#include <Arduino.h>
#include <vector>
#include <Grow_sensor_component.h>

// (!) ----- (-) ----- перенести на статическую память

class Grow_sensor {
private:
    uint32_t system_id_;
    uint16_t address_; // адрес датчика
    std::vector<Grow_sensor_component> component_; // вектор компонентов датчиков (не как плата, а как механический модуль)

    unsigned long read_time_; // время последнего считывания
    unsigned long end_time_; // время последней проверки для считывания
    unsigned long period_; // период считывания в мс
    bool readout_signal_; // флаг готовности считывания

    uint8_t active_;  // состояние активации 0 - не активен, 1 - в процессе регистрации, 2 - зарегистрирован // (-) ----- обнулить значения при false запретить функции обработки
    bool change_value_; // флаг изменённого значения считанных показателей

    uint8_t setting_; // настройски (для LoRa-протокола)

    friend class Grow_sensor_interface;
public:
    Grow_sensor() = default;
    Grow_sensor(uint8_t amt_component, enum Type_sensor* type_sensor);
    Grow_sensor(uint8_t amt_component, uint8_t* type_sensor);
    Grow_sensor(const std::vector<enum Type_sensor>& type_sensor);
    ~Grow_sensor() = default;

    /// --- Поля класса-платы ---

    // установка индивидуального номера платы
    void set_system_id(uint32_t system_id_);
    // получение индивидуального номера платы
    uint32_t get_system_id() const;

    // установка состояния активности
    void set_active(uint8_t active);
    // получение состояния активности
    uint8_t get_active() const;

    // получение флага об изменённом состоянии)
    bool get_change_value() const;
    // сброс флага изменённого состояния
    void clear_change_value();

    // установка адреса модуля (ветви, без группы)
    bool set_address(uint16_t address);
    // получение адреса модуля (ветви, без группы)
    uint16_t get_address() const;

    // Установка настроек LoRa-передачи
    void set_setting(uint8_t setting);
    // Получение настроек LoRa-передачи
    uint8_t get_setting() const;

    /// --- Обработка времени ---

    // Установить период опроса модуля
    void set_period(unsigned long period);
    // Получить период опроса модуля
    unsigned long get_period() const;  

    // Проверка необходимости считывания 
    bool check_time(unsigned long time);
    // Обновление времени считывания
    void update();
    // Получение сигнала готовности к считыванию
    bool read_signal(bool clear=false);


    /// --- Поля компонентов ---

    // получение типа компонента (передаётся в result), если ошибка возврат true
    bool get_type(uint8_t num, enum Type_sensor &result);
    bool get_type(uint8_t num, uint8_t &result);
    // получение вектора типов компонентов
    std::vector<enum Type_sensor> get_type();
    // получение id компонента (передаётся в result | не путать с ID платы, этот номер для количества повторений), если ошибка возврат true
    bool get_id(uint8_t num, uint8_t &result);
    // получение вектора id компонентов
    std::vector<uint8_t> get_id();

    // Установить значение считанного показателя, если ошибка возврат true
    bool set_value(uint8_t num, float value);
    // Получить значение считанного показателя, если ошибка возврат true
    bool get_value(uint8_t num, float &result);
    // Установить значения считанных показателей, если ошибка возврат true
    bool set_value(const std::vector<float>& value);
    // Получить вектор значений считанных показателей
    std::vector<float> get_value();

    /// --- Информации о компонентах ---

    // Получить количество компонентов
    uint8_t get_count_component();

    // Получить определённый компонент
    Grow_sensor_component get_component(uint8_t num);
    // Получить вектор компонентов
    std::vector<Grow_sensor_component> get_component();

    /// --- Внешняя связь ---

    // Проверить совподает ли содержимое модулей, без учёта настроек (для отфильтровывания среди неподходящих)
    bool filter(Grow_sensor &sensor); // (-) ----- убрать привязку к порядку

#if defined (SERIAL_LOG_OUTPUT)
    /// вывод класса в Serial
    void print();
#endif
};


#endif // __GROW_SENSOR_H__