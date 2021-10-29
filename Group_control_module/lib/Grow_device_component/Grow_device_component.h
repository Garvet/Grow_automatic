#ifndef __GROW_DEVICE_COMPONENT_H__
#define __GROW_DEVICE_COMPONENT_H__

#if defined( ESP32 )
#include <Arduino.h>
#else
#include <main.h>
#endif

#include <Time_control.h>

// Список всех типов устройств
enum Type_device {
	Signal_PWM = 0,
	Signal_digital,
	Fan_PWM,
	Pumping_system,
	Phytolamp_digital,
	Phytolamp_PWM,
    Time_Channel,
    GrowTimer,
    // Количество типов устройств
    AMT_DEVICES,
};
// Количество всех типов устройств
#define COUNT_TYPE_DEVICE AMT_DEVICES

enum class Work_mode {
    Auto = 0,
    Manual,
};
enum class Manual_work_state {
    On = 0,
    Off,
};

class Grow_device_component {
private:
    enum Type_device type_device_; // тип устройства
    uint8_t id_;  // номер устройства данного типа
    uint16_t value_; // состояние устройства

    // (-) ----- (!) ----- \/ \/ \/ КОСТЫЛЬ
    std::vector<dtc::Grow_timer> time_channel;
    bool time_change{false};
    uint16_t send_server_value{0xFFFF};
    bool server_value_change{false};
    Work_mode work_mode{Work_mode::Auto};
    Manual_work_state manual_work_state{Manual_work_state::On};
    bool work_state_change{false};
    // (-) ----- (!) ----- /\ /\ /\ КОСТЫЛЬ
public:
    Grow_device_component(Type_device type_device=Signal_digital, uint8_t id=0);
    ~Grow_device_component() = default;

    // получение типа датчика
    enum Type_device get_type() const;

    // получение id
    uint8_t get_id() const;
    // установка id
    void set_id(uint8_t id);

    // Получить значение считанного показателя
    uint16_t get_value() const;
    // Установить значение считанного показателя
    bool set_value(uint16_t value);

    // Проверка совпадения неизменяемых программно частей модуля
    bool filter(Grow_device_component &component);

    // (-) ----- (!) ----- \/ \/ \/ КОСТЫЛЬ

    /// Контроль периодов
    // Установить значение периодов
    void set_timer(std::vector<dtc::Grow_timer> set_time_channel);
    // Получить значение периодов
    const std::vector<dtc::Grow_timer>& get_timer() const;
    // Получить флаг изменения периодов
    bool get_timer_change() const;
    // Очистить флаг изменения периодов
    bool reset_timer_change();

    // Получить флаг изменения установленного значения
    bool set_send_server_value(uint16_t val);
    uint16_t get_send_server_value() const;
    void clear_send_server_value();
    // Получить флаг изменения установленного значения
    bool get_server_value_change() const;
    // Очистить флаг изменения установленного значения
    bool reset_server_value_change();

    /// Режим работы
    void set_work_mode(Work_mode val);
    Work_mode get_work_mode() const;
    void set_manual_work_state(Manual_work_state val);
    Manual_work_state get_manual_work_state() const;
    // Получить флаг изменения режима работы
    bool get_work_state_change() const;
    // Очистить флаг изменения режима работы
    bool reset_work_state_change();

    // (-) ----- (!) ----- /\ /\ /\ КОСТЫЛЬ

    /// Функции превращения в байтовую строку (-) ----- доработать под код Дениса
    // Получение размера строки
    size_t get_size();
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(uint8_t *data, size_t available_size);
};

#endif // __GROW_DEVICE_COMPONENT_H__
