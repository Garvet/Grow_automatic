#include "Grow_sensor_component.h"

Grow_sensor_component::Grow_sensor_component(Type_sensor type_sensor, uint8_t id) {
    type_sensor_ = type_sensor;
    id_ = id;
    value_ = 0;
}

enum Type_sensor Grow_sensor_component::get_type() {
    return type_sensor_;
}
uint8_t Grow_sensor_component::get_id() {
    return id_;
}
void Grow_sensor_component::set_id(uint8_t id) {
    id_ = id;
}
float Grow_sensor_component::get_value() {
    return value_;
}
bool Grow_sensor_component::set_value(float value) {
    if(value_ == value)
        return false;
    value_ = value;
    return true;
}

bool Grow_sensor_component::filter(Grow_sensor_component &component) {
    return (type_sensor_ == component.type_sensor_);
}


size_t Grow_sensor_component::get_size() {
    // enum Type_sensor _type_sensor; << 255
    return (1);
}
size_t Grow_sensor_component::get_data(uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    data[size++] = (uint8_t)(type_sensor_ & 0xFF);
    return size;
}
size_t Grow_sensor_component::set_data(uint8_t *data, size_t available_size) {
    if((data == nullptr) || (available_size < get_size()))
        return 0;
    size_t size = 0;
    type_sensor_ = (enum Type_sensor)(data[size++]);
    return size;    
}