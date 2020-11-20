#include "Grow_device_component.h"

Grow_device_component::Grow_device_component(enum Type_device type_device, uint8_t id) {
    type_device_ = type_device;
    id_ = id;
    state_change_ = 0;
    value_ = 4095;
    signal_ = false;
    // Period_adjuster(длина, начало, максимальный период, текущий период)
    set_period(Period_adjuster(16, 7, 24, 24), HOUR);
}

Grow_device_component::Grow_device_component(enum Type_device type_device, uint8_t id, Period_adjuster period, enum Period_type period_type) {
    type_device_ = type_device;
    id_ = id;
    state_change_ = 0;
    value_ = 4095;
    signal_ = false;
    set_period(period, period_type);
}

enum Type_device Grow_device_component::get_type() {
    return type_device_;
}
uint8_t Grow_device_component::get_id() {
    return id_;
}
void Grow_device_component::set_id(uint8_t id) {
    id_ = id;
}

void Grow_device_component::set_period(Period_adjuster period, enum Period_type period_type) {
    period_ = period;
    period_type_ = period_type;
}
Period_adjuster Grow_device_component::get_period() {
    return period_;
}
enum Period_type Grow_device_component::get_period_type() {
    return period_type_;
}

int8_t Grow_device_component::check_time(bool clear) {
    signal_ = period_.check_work();
    state_change_ = period_.state_change(clear);
    return state_change_;
}
int8_t Grow_device_component::check_time(uint8_t time, bool clear) {
    signal_ = period_.check_work(time);
    state_change_ = period_.state_change(clear);
    return state_change_;
}

bool Grow_device_component::set_value(uint16_t value) {
    bool change_value = false;
    if(value_ != value) {
        change_value = true;
        signal_ = true;
    }
    value_ = value;
    return change_value;
} 

bool Grow_device_component::get_signal() {
    return signal_;
}
int8_t Grow_device_component::get_state_change() {
    return state_change_;
}
uint16_t Grow_device_component::get_value() {
    return value_;
}

bool Grow_device_component::filter(Grow_device_component &component) {
    return (type_device_ == component.type_device_);
}



size_t Grow_device_component::get_size() {
    // enum Type_device _type_device; << 255
    // enum Period_type _period_type; << 255
    // uint8_t _frequency;
    // uint8_t _period;
    // uint8_t _bias; 
    return (5);
}
size_t Grow_device_component::get_data(uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    uint8_t frequency, period, bias;
    data[size++] = (uint8_t)(type_device_ & 0xFF); // << 255
    data[size++] = (uint8_t)(period_type_ & 0xFF); // << 255
    bias = period_.get_dawn();
    period = period_.get_number();
    frequency = period_.get_max_period_range() / period_.get_period_range();
    data[size++] = frequency;
    data[size++] = period;
    data[size++] = bias;
    return size;
}
size_t Grow_device_component::set_data(uint8_t *data, size_t available_size) {
    if((data == nullptr) || (available_size < get_size()))
        return 0;
    size_t size = 0;
    uint8_t max_period, frequency, period, bias;
    type_device_ = (enum Type_device)(data[size++]);
    period_type_ = (enum Period_type)(data[size++]);
    frequency = data[size++];
    period = data[size++];
    bias = data[size++];
    if(period_type_ == HOUR)
        max_period = 24;
    else
        max_period = 60;
    period_ = Period_adjuster(period, bias, max_period, (max_period / frequency));
    return size;
}


