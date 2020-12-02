#ifndef __GROW_SENSOR_INTERFACE_H__
#define __GROW_SENSOR_INTERFACE_H__

#include <Arduino.h>
#include <Grow_sensor.h>
#include <Packet_analyzer.h>
#include <LoRa_packet.h>

#define GROUP_CONTROL_MODULE

class Grow_sensor_interface {
private:
public:
    Grow_sensor_interface() = default;
    ~Grow_sensor_interface() = default;

    /// --- Сохранение в энергонезависимую память ---
    // Получение размера строки
    size_t get_size(Grow_sensor &grow_sensor);
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(Grow_sensor &grow_sensor, uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(Grow_sensor &grow_sensor, uint8_t *data, size_t available_size);

    /// --- Обмен с телефоном ---
    // фильтрация
    // кодовое представление (2 вида)

    /// --- LoRa-соединение ---
    // Регистрация (представиться) кодирование и декодирование
    LoRa_packet creat_regist_packet(Grow_sensor &grow_sensor);
    Grow_sensor read_regist_packet(LoRa_packet& packet);
    // sitting (period) // device setting work 
    // Отправка и приём данных
    uint8_t creat_send_data_packet(Grow_sensor &grow_sensor, LoRa_packet* packet);
    uint8_t read_send_data_packet(Grow_sensor &grow_sensor, LoRa_packet* packet, uint8_t amt);

    /// --- Отчётность ---
    // Регистрация
    // Отправка данных (отчитывающийся объект, адрес массива, в size максимум | возврат - количество байт)
    uint16_t report_to_server(Grow_sensor &grow_sensor, uint8_t *buf, uint16_t size=(uint16_t)0xFFFFFFFF);

};

extern Grow_sensor_interface grow_sensor_interface;

#endif // __GROW_SENSOR_INTERFACE_H__