#if defined ( ERROR_CODE_NOT_USED )
#ifndef __GROW_DEVICE_COMPONENT_H__
#define __GROW_DEVICE_COMPONENT_H__

#if defined( ESP32 )
#include <Arduino.h>
#endif
#include <Time_control.h>

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

class Grow_device_component {
private:
    enum Type_device type_device_; // тип устройства
    uint8_t id_;                   // номер устройства данного типа
    uint16_t pwm_value_;           // значение ШИМ сигнала
    bool state_;                   // состояние устройства
    Time_control time_control_;    // настройки времени
public:
    Grow_device_component(enum Type_device type_device=Signal_PWM, uint8_t id=0, Time_control time_control=Time_control());
    ~Grow_device_component() = default;

    // получение типа устройства
    enum Type_device get_type() const;

    // получение id
    uint8_t get_id();
    // установка id
    void set_id(uint8_t id);

    // Получить значение ШИМ сигнала
    uint16_t get_pwm_value();
    // Установить значение ШИМ сигнала в рабочем состоянии
    bool set_pwm_value(uint16_t pwm_value);

    // Получить состояние устройства
    bool get_state();
    // Установить состояние устройства
    void set_state(bool state);

    // получение настройки временного контроля
    Time_control get_time_control();
    // установка настройки временного контроля
    void set_time_control(Time_control time_control);

    // Проверка совпадения неизменяемых программно частей модуля
    bool filter(Grow_device_component &component);

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса
    // Получение размера строки
    size_t get_size();                                     // (!) ----- (-) ----- переписать, изменилась структура
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);                        // (!) ----- (-) ----- переписать, изменилась структура
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size); // (!) ----- (-) ----- переписать, изменилась структура
};

#endif // __GROW_DEVICE_COMPONENT_H__

#endif // ERROR_CODE_NOT_USED