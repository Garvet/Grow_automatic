#include "Grow_device_component.h"

Grow_device_component::Grow_device_component(enum Type_device type_device, uint8_t id, Time_control time_control) {
    type_device_ = type_device;
    id_ = id;
    pwm_value_ = 4095; // max
    state_ = false;
    time_control_ = time_control;
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

uint16_t Grow_device_component::get_pwm_value() {
    return pwm_value_;
}
bool Grow_device_component::set_pwm_value(uint16_t pwm_value) {
    if(pwm_value > 4095)
        return true;
    pwm_value_ = pwm_value;
    return false;
}

bool Grow_device_component::get_state() {
    return state_;
}
void Grow_device_component::set_state(bool state) {
    state_ = state;
}

Time_control Grow_device_component::get_time_control() {
    return time_control_;
}
void Grow_device_component::set_time_control(Time_control time_control) {
    time_control_ = time_control;
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
    // uint8_t frequency, period, bias;
    // data[size++] = (uint8_t)(type_device_ & 0xFF); // << 255
    // data[size++] = (uint8_t)(period_type_ & 0xFF); // << 255
    // bias = period_.get_dawn();
    // period = period_.get_number();
    // frequency = period_.get_max_period_range() / period_.get_period_range();
    // data[size++] = frequency;
    // data[size++] = period;
    // data[size++] = bias;
    return size;
}
size_t Grow_device_component::set_data(uint8_t *data, size_t available_size) {
    if((data == nullptr) || (available_size < get_size()))
        return 0;
    size_t size = 0;
    // uint8_t max_period, frequency, period, bias;
    // type_device_ = (enum Type_device)(data[size++]);
    // period_type_ = (enum Period_type)(data[size++]);
    // frequency = data[size++];
    // period = data[size++];
    // bias = data[size++];
    // if(period_type_ == HOUR)
    //     max_period = 24;
    // else
    //     max_period = 60;
    // period_ = Period_adjuster(period, bias, max_period, (max_period / frequency));
    return size;
}
