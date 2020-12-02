#ifndef __GROW_SENSOR_COMPONENT_H__
#define __GROW_SENSOR_COMPONENT_H__

#include <Arduino.h>

// Количество всех типов датчиков
#define COUNT_TYPE_SENSOR 13
// Список всех типов датчиков
enum Type_sensor {
	Analog_signal = 0,
	Discrete_signal,
	Battery_charge,
	Air_humidity,
	Air_temperature,
	Water_temperature,
	Illumination_level,
	Lamp_power,
	Pump_power,
	Indicator_pH,
	Indicator_EC,
	Indicator_eCO2,
	Indicator_nYVOC
};

class Grow_sensor_component {
private:
    enum Type_sensor type_sensor_; // тип датчика
    uint8_t id_;  // номер датчика данного типа
    float value_; // показатель датчика
public:
    Grow_sensor_component(Type_sensor type_sensor=Analog_signal, uint8_t id=0);
    ~Grow_sensor_component() = default;

    // получение типа датчика
    enum Type_sensor get_type();
    
    // получение id
    uint8_t get_id();
    // установка id
    void set_id(uint8_t id);

    // Получить значение считанного показателя
    float get_value();
    // Установить значение считанного показателя
    bool set_value(float value);

    // Проверка совпадения неизменяемых программно частей модуля
    bool filter(Grow_sensor_component &component);

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);
};

#endif // __GROW_SENSOR_COMPONENT_H__
