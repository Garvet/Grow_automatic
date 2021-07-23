#ifndef __GROW_DEVICE_INTERFACE_H__
#define __GROW_DEVICE_INTERFACE_H__

#include <Arduino.h>
#include <Grow_device.h>
#include <Packet_analyzer.h>
#include <LoRa_packet.h>
#include <LoRa_contact_data.h>

#define GROUP_CONTROL_MODULE

class Grow_device_interface {
private:
public:
    Grow_device_interface() = default;
    ~Grow_device_interface() = default;

    /// --- Сохранение в энергонезависимую память ---
    // Получение размера строки
    size_t get_size(Grow_device &grow_device);
    // Заполение массива байтов, возврат количество байт (должен совпадать с размером строки)
    size_t get_data(Grow_device &grow_device, uint8_t *data);
    // Заполнение объекта класса по байтовой строке, возврат количество использованных байт
    size_t set_data(Grow_device &grow_device, uint8_t *data, size_t available_size);

    /// --- Обмен с телефоном ---
    // фильтрация
    // кодовое представление (2 вида)

    /// --- LoRa-соединение ---
    // Регистрация (представиться) кодирование и декодирование
    LoRa_packet creat_regist_packet(const Grow_device &grow_device, LoRa_contact_data& contact_data);
    Grow_device read_regist_packet(LoRa_packet& packet);
    bool check_regist_packet(LoRa_contact_data& contact_data);
    // sitting (period) // device setting work 
    // Отправка и приём данных
    uint8_t creat_send_data_packet(Grow_device &grow_device, LoRa_contact_data& contact_data);
    uint8_t read_send_data_packet(Grow_device &grow_device, LoRa_packet* packet, uint8_t amt);

    /// --- Отчётность ---
    // Регистрация
    // Отправка данных (отчитывающийся объект, адрес массива, в size максимум | возврат - количество байт)
    uint16_t report_to_server(Grow_device &grow_device, uint8_t *buf, uint16_t size=(uint16_t)0xFFFFFFFF);

};

extern Grow_device_interface grow_device_interface;

#endif // __GROW_DEVICE_INTERFACE_H__