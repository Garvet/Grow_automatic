#ifndef __GROW_SENSOR_H__
#define __GROW_SENSOR_H__

#include <Arduino.h>
#include <vector>
#include <Grow_sensor_component.h>

// (!) ----- (-) ----- перенести на статическую память

class Grow_sensor {
private:
    // (-) ----- ID добавь
    uint16_t address_; // адрес датчика
    std::vector<Grow_sensor_component> component_; // вектор компонентов датчиков (не как плата, а как механический модуль)

    unsigned long read_time_; // время последнего считывания
    unsigned long end_time_; // время последней проверки для считывания
    unsigned long period_; // период считывания в мс
    bool readout_signal_; // флаг готовности считывания

    uint8_t active_;  // состояние активации 0 - не активен, 1 - в процессе регистрации, 2 - зарегистрирован // (-) ----- обнулить значения при false запретить функции обработки
    bool change_value_; // флаг изменённого значения считанных показателей

    uint8_t setting_; // настройски (для LoRa-протокола)
public:
    Grow_sensor() = default;
    Grow_sensor(uint8_t amt_component, enum Type_sensor* type_sensor);
    Grow_sensor(uint8_t amt_component, uint8_t* type_sensor);
    Grow_sensor(std::vector<enum Type_sensor> type_sensor);
    ~Grow_sensor() = default;

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

    // получение типа компонента (передаётся в result), если ошибка возврат true
    bool get_type(uint8_t num, enum Type_sensor &result);
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
    bool set_value(std::vector<float> value);
    // Получить вектор значений считанных показателей
    std::vector<float> get_value();

    // Установить период опроса модуля
    void set_period(unsigned long period);
    // Получить период опроса модуля
    unsigned long get_period();  

    // Проверка необходимости считывания 
    bool check_time(unsigned long time);
    // Обновление времени считывания
    void update();
    // Получение сигнала готовности к считыванию
    bool signal(bool clear=false);

    // Получить количество компонентов
    uint8_t get_count_component();

    // Получить определённый компонент
    Grow_sensor_component get_component(uint8_t num);
    // Получить вектор компонентов
    std::vector<Grow_sensor_component> get_component();

    // Проверить совподает ли содержимое модулей, без учёта настроек (для отфильтровывания среди неподходящих)
    bool filter(Grow_sensor &sensor); // (-) ----- убрать привязку к порядку

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);

#if defined (SERIAL_LOG_OUTPUT)
    /// вывод класса в Serial
    void print();
#endif
};


#endif // __GROW_SENSOR_H__