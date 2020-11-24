#include "Grow_sensor.h"

extern const uint16_t LORA_ADDRESS_BRANCH;

static uint8_t id_mas[COUNT_TYPE_SENSOR];

Grow_sensor::Grow_sensor(uint8_t amt_component, enum Type_sensor* type_sensor) {
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_sensor_component(type_sensor[i], (id_mas[type_sensor[i]]++)));
}
Grow_sensor::Grow_sensor(uint8_t amt_component, uint8_t* type_sensor) {
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_sensor_component((enum Type_sensor)(type_sensor[i]), (id_mas[type_sensor[i]]++)));
}

Grow_sensor::Grow_sensor(std::vector<enum Type_sensor> type_sensor) {
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < type_sensor.size(); ++i)
        component_.push_back(Grow_sensor_component(type_sensor[i], (id_mas[type_sensor[i]]++)));
}

void Grow_sensor::set_system_id(uint32_t system_id) {
    system_id_ = system_id;
}
uint32_t Grow_sensor::get_system_id() {
    return system_id_;
}

void Grow_sensor::set_active(uint8_t active) {
    if(active < 3)
        active_ = active;
}

uint8_t Grow_sensor::get_active() {
    return active_;
}

bool Grow_sensor::get_change_value() {
    return change_value_;
}
void Grow_sensor::clear_change_value() {
    change_value_ = false;
}

bool Grow_sensor::set_address(uint16_t address) {
    if((address >= (1 << LORA_ADDRESS_BRANCH)) || (address == 0))
        return true;
    address_ = address;
    return false;
}
uint16_t Grow_sensor::get_address() {
    return address_;
}

void Grow_sensor::set_setting(uint8_t setting) {
    setting_ = setting;
}
uint8_t Grow_sensor::get_setting() {
    return setting_;
}

bool Grow_sensor::get_type(uint8_t num, enum Type_sensor &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
std::vector<enum Type_sensor> Grow_sensor::get_type() {
    std::vector<enum Type_sensor> type_sensor;
    for(int i = 0; i < get_count_component(); ++i)
        type_sensor.push_back(component_[i].get_type());
    return type_sensor;
}
bool Grow_sensor::get_id(uint8_t num, uint8_t &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_id();
    return false;
}
std::vector<uint8_t> Grow_sensor::get_id() {
    std::vector<uint8_t> id;
    for(int i = 0; i < get_count_component(); ++i)
        id.push_back(component_[i].get_id());
    return id;
}

void Grow_sensor::set_period(unsigned long period) {
    period_ = period;
}
unsigned long Grow_sensor::get_period() {
    return period_;
}
bool Grow_sensor::check_time(unsigned long time) {
    end_time_ = time;
    if ((end_time_ - read_time_) > period_)
        readout_signal_ = true;
    return readout_signal_;
}
void Grow_sensor::update() {
    read_time_ = end_time_; 
}
bool Grow_sensor::read_signal(bool clear) {
    if (!clear)
        return readout_signal_;
    clear = readout_signal_;
    readout_signal_ = false;
    return clear;
}

bool Grow_sensor::set_value(uint8_t num, float value) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_value(value);
    change_value_ = true;
    return false;
}
bool Grow_sensor::get_value(uint8_t num, float &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_value();
    return false;
}
bool Grow_sensor::set_value(std::vector<float> value) {
    if(get_count_component() != value.size())
        return true;
    for(int i = 0; i < get_count_component(); ++i)
        component_[i].set_value(value[i]);
    return false;
}
std::vector<float> Grow_sensor::get_value() {
    std::vector<float> value;
    for(int i = 0; i < get_count_component(); ++i)
        value.push_back(component_[i].get_value());
    return value;
}


uint8_t Grow_sensor::get_count_component() {
    return component_.size();
}

Grow_sensor_component Grow_sensor::get_component(uint8_t num) {
    return component_[num];
}
std::vector<Grow_sensor_component> Grow_sensor::get_component() {
    return component_;
} 

bool Grow_sensor::filter(Grow_sensor &sensor) {
    if(component_.size() != sensor.component_.size())
        return false;
    for(int i = 0; i < component_.size(); ++i) {
        if(!component_[i].filter(sensor.component_[i]))
            return false;
    }
    return true;
}


size_t Grow_sensor::get_size() {
    // unsigned long _period; 4
    // uint8_t _amt_component; 1
    // Config_grow_sensor_component* _component; not save
    size_t size = 5;
    for(int i = 0; i < component_.size(); ++i)
        size += component_[i].get_size();
    return size;
}
size_t Grow_sensor::get_data(uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    data[size++] = (uint8_t)((period_ >> 24) & 0xFF);
    data[size++] = (uint8_t)((period_ >> 16) & 0xFF);
    data[size++] = (uint8_t)((period_ >> 8) & 0xFF);
    data[size++] = (uint8_t)(period_ & 0xFF);
    data[size++] = component_.size();
    for(int i = 0; i < component_.size(); ++i)
        size += component_[i].get_data(data+size);
    return size;
}
size_t Grow_sensor::set_data(uint8_t *data, size_t available_size) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    component_.clear();
    period_  = ((ulong)data[size++]) << 24;
    period_ |= ((ulong)data[size++]) << 16;
    period_ |= ((ulong)data[size++]) << 8;
    period_ |= ((ulong)data[size++]);
    uint8_t amt_component = data[size++];
    if(amt_component != 0) {
        component_.resize(amt_component);
        for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
            id_mas[i] = 0;
        for(int i = 0; i < amt_component; ++i) {
            size += component_[i].set_data(data+size, available_size-size);
            component_[i].set_id(id_mas[component_[i].get_type()]++);            
        }
    }
    return size;
}

#if defined (SERIAL_LOG_OUTPUT)
const char *sensor_component_name[] = 
    {"Analog_signal", "Discrete_signal", "Battery_charge", "Air_humidity", "Air_temperature", 
    "Water_temperature", "Illumination_level", "Lamp_power", "Pump_power", "Indicator_pH",
    "Indicator_EC", "Indicator_eCO2", "Indicator_nYVOC"};

void Grow_sensor::print() {
    Serial.print("System ID: ");
    for(int i = 0; i < 4; ++i) {
        uint8_t data = (system_id_ >> ((3 - i) * 8)) & 0xFF;
        if(data < 16)
            Serial.print("0");
        Serial.print(data, 16);
        if(i < 3)
            Serial.print(".");
    }
    Serial.println();
    Serial.print("Address: ");
    if((address_ >> 8) < 16)
        Serial.print("0");
    Serial.print((address_ >> 8), 16);
    Serial.print(" ");
    if((address_ & 0xFF) < 16)
        Serial.print("0");
    Serial.print((address_ & 0xFF), 16);
    if(active_ == 0)
        Serial.println(" (-)");
    else if(active_ == 1)
        Serial.println(" (?)");
    else
        Serial.println(" (+)");
    
    Serial.print("  Period: ");
    Serial.println(get_period());

    Serial.println("  Components:");
    enum Type_sensor type_component;
    for(int j = 0; j < component_.size(); ++j) {
        Serial.print("    [");
        Serial.print(j);
        Serial.print("] ");
        get_type(j, type_component);
        Serial.print(sensor_component_name[type_component]);
        Serial.print(" (№");
        Serial.print((uint)type_component);
        Serial.println(")");
    }
}
#endif