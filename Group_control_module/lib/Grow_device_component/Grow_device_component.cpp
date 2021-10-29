#include "Grow_device_component.h"

Grow_device_component::Grow_device_component(Type_device type_device, uint8_t id) {
    type_device_ = type_device;
    id_ = id;
    value_ = 0;
}

enum Type_device Grow_device_component::get_type() const {
    return type_device_;
}
uint8_t Grow_device_component::get_id() const {
    return id_;
}
void Grow_device_component::set_id(uint8_t id) {
    id_ = id;
}
uint16_t Grow_device_component::get_value() const {
    return value_;
}
bool Grow_device_component::set_value(uint16_t value) {
    if(value_ == value)
        return false;
    value_ = value;
    return true;
}

bool Grow_device_component::filter(Grow_device_component &component) {
    return (type_device_ == component.type_device_);
}

// (-) ----- (!) ----- \/ \/ \/ КОСТЫЛЬ

/// Контроль периодов
// Установить значение периодов
void Grow_device_component::set_timer(std::vector<dtc::Grow_timer> set_time_channel) {
    time_channel = set_time_channel;
    time_change = true;
}
// Получить значение периодов
const std::vector<dtc::Grow_timer>& Grow_device_component::get_timer() const {
    return time_channel;
}
// Получить флаг изменения периодов
bool Grow_device_component::get_timer_change() const {
    for(int i = 0; i < time_channel.size(); ++i)
        if(time_channel[i].get_internal_change())
            return true;
    return false;
}
// Очистить флаг изменения периодов
bool Grow_device_component::reset_timer_change() {
    bool result = time_change;
    time_change = false;
    for(int i = 0; i < time_channel.size(); ++i)
        result = result || time_channel[i].reset_internal_change();
    return result;
}


/// Контроль значения
bool Grow_device_component::set_send_server_value(uint16_t val) {
    if(4095 < val)
        return true;
    server_value_change = true;
    send_server_value = val;
    return false;
}
uint16_t Grow_device_component::get_send_server_value() const {
    return send_server_value;
}
void Grow_device_component::clear_send_server_value() {
    send_server_value = 0xFFFF;
    server_value_change = true;
}

// Получить флаг изменения периодов
bool Grow_device_component::get_server_value_change() const {
    return server_value_change;
}
// Очистить флаг изменения периодов
bool Grow_device_component::reset_server_value_change() {
    if(!server_value_change)
        return false;
    server_value_change = false;
    return true;
}




/// Режим работы
void Grow_device_component::set_work_mode(Work_mode val) {
    work_mode = val;
    work_state_change = true;
}
Work_mode Grow_device_component::get_work_mode() const {
    return work_mode;
}
void Grow_device_component::set_manual_work_state(Manual_work_state val) {
    manual_work_state = val;
    work_state_change = true;
}
Manual_work_state Grow_device_component::get_manual_work_state() const {
    return manual_work_state;
}
// Получить флаг изменения периодов
bool Grow_device_component::get_work_state_change() const {
    return work_state_change;
}
// Очистить флаг изменения периодов
bool Grow_device_component::reset_work_state_change() {
    if(!work_state_change)
        return false;
    work_state_change = false;
    return true;
}

// (-) ----- (!) ----- /\ /\ /\ КОСТЫЛЬ


size_t Grow_device_component::get_size() {
    // enum Type_device _type_device; << 255
    return (1);
}
size_t Grow_device_component::get_data(uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    data[size++] = (uint8_t)(type_device_ & 0xFF);
    return size;
}
size_t Grow_device_component::set_data(uint8_t *data, size_t available_size) {
    if((data == nullptr) || (available_size < get_size()))
        return 0;
    size_t size = 0;
    type_device_ = (enum Type_device)(data[size++]);
    return size;
}
