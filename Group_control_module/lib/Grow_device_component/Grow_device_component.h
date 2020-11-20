#ifndef __GROW_DEVICE_COMPONENT_H__
#define __GROW_DEVICE_COMPONENT_H__

#include <Arduino.h>
#include <Period_adjuster.h>

// Количество всех типов устройств
#define COUNT_TYPE_DEVICE 6
// Список всех типов устройств
enum Type_device {
	Signal_PWM = 0,
	Signal_digital,
	Fan_PWM,
	Pumping_system,
	Phytolamp_digital,
	Phytolamp_PWM
};

// Список всех типов периуда устройств
enum Period_type {
    SEC = 0,
    MIN,
    HOUR
};

class Grow_device_component {
private:
    enum Type_device type_device_; // тип устройства
    Period_adjuster period_;       // рабочий период
    enum Period_type period_type_; // уровень периода (секунды, минуты, часы)
    int8_t state_change_;          // сигнал-периода
    uint16_t value_;               // значение сигнала
    uint8_t id_;                   // номер устройства данного типа

    bool signal_;                  // сигнал смены состояния
public:
    Grow_device_component(enum Type_device type_device=Signal_PWM, uint8_t id=0);
    Grow_device_component(enum Type_device type_device, uint8_t id, Period_adjuster period, enum Period_type period_type);
    ~Grow_device_component() = default;

    // получение типа устройства
    enum Type_device get_type();
    // получение id
    uint8_t get_id();
    // установка id
    void set_id(uint8_t id);

    // установка настройки
    void set_period(Period_adjuster period, enum Period_type period_type);
    // получение настройки работы
    Period_adjuster get_period();
    // получение настройки типа времени
    enum Period_type get_period_type();

    // Проверка времени без обновления, на случай изменения состояния
    int8_t check_time(bool clear = false);
    // Проверка времени с обновлением, на случай изменения состояния
    int8_t check_time(uint8_t time, bool clear = false);

    // Установить значение сигнала в рабочем состоянии
    bool set_value(uint16_t value); // влияет на _signal

    // Получить наличие сигнала
    bool get_signal();
    // Получить изменение сигнала (-1 выключается, 0 не изменен, +1 включается)
    int8_t get_state_change();
    // Получить значение сигнала
    uint16_t get_value();

    // Проверка совпадения неизменяемых программно частей модуля
    bool filter(Grow_device_component &component);

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);
};

#endif // __GROW_DEVICE_COMPONENT_H__
