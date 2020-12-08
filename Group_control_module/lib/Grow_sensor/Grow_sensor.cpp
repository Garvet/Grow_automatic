#include "Grow_sensor.h"

extern const uint16_t LORA_ADDRESS_BRANCH;

uint8_t id_mas_sensors[COUNT_TYPE_SENSOR];

Grow_sensor::Grow_sensor(uint8_t amt_component, enum Type_sensor* type_sensor) {
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        system_id_[i] = 0;
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        id_mas_sensors[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_sensor_component(type_sensor[i], (id_mas_sensors[type_sensor[i]]++)));

    for (int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        if(id_mas_sensors[i] > 1) {
            setting_ = 0x04; // 0000.0100 - бит индивидуального номера, в случае наличия повторов
        }
}
Grow_sensor::Grow_sensor(uint8_t amt_component, uint8_t* type_sensor) {
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        system_id_[i] = 0;
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        id_mas_sensors[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_sensor_component((enum Type_sensor)(type_sensor[i]), (id_mas_sensors[type_sensor[i]]++)));

    for (int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        if(id_mas_sensors[i] > 1) {
            setting_ = 0x04;
        }
}

Grow_sensor::Grow_sensor(const std::vector<enum Type_sensor>& type_sensor) {
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        system_id_[i] = 0;
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        id_mas_sensors[i] = 0;
    for(int i = 0; i < type_sensor.size(); ++i)
        component_.push_back(Grow_sensor_component(type_sensor[i], (id_mas_sensors[type_sensor[i]]++)));

    for (int i = 0; i < COUNT_TYPE_SENSOR; ++i)
        if(id_mas_sensors[i] > 1) {
            setting_ = 0x04;
        }
}

// --- Поля класса-платы ---

void Grow_sensor::set_system_id(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> system_id) {
    system_id_ = system_id;
}
std::array<uint8_t, AMT_BYTES_SYSTEM_ID> Grow_sensor::get_system_id() const {
    return system_id_;
}

void Grow_sensor::set_active(uint8_t active) {
    if(active < 3)
        active_ = active;
}

uint8_t Grow_sensor::get_active() const {
    return active_;
}

bool Grow_sensor::get_change_value() const {
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
uint16_t Grow_sensor::get_address() const {
    return address_;
}

void Grow_sensor::set_setting(uint8_t setting) {
    setting_ = setting;
}
uint8_t Grow_sensor::get_setting() const {
    return setting_;
}

// --- Обработка времени ---

void Grow_sensor::set_period(unsigned long period) {
    period_ = period;
}
unsigned long Grow_sensor::get_period() const {
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

// --- Поля компонентов ---

bool Grow_sensor::get_type(uint8_t num, enum Type_sensor &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
bool Grow_sensor::get_type(uint8_t num, uint8_t &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
std::vector<enum Type_sensor> Grow_sensor::get_type() const {
    std::vector<enum Type_sensor> type_sensor;
    for(int i = 0; i < get_count_component(); ++i)
        type_sensor.push_back(component_[i].get_type());
    return type_sensor;
}
bool Grow_sensor::get_id(uint8_t num, uint8_t &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_id();
    return false;
}
std::vector<uint8_t> Grow_sensor::get_id() const {
    std::vector<uint8_t> id;
    for(int i = 0; i < get_count_component(); ++i)
        id.push_back(component_[i].get_id());
    return id;
}

bool Grow_sensor::set_value(uint8_t num, float value) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_value(value);
    change_value_ = true;
    return false;
}
bool Grow_sensor::get_value(uint8_t num, float &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_value();
    return false;
}
bool Grow_sensor::set_value(const std::vector<float>& value) {
    if(get_count_component() != value.size())
        return true;
    for(int i = 0; i < get_count_component(); ++i)
        component_[i].set_value(value[i]);
    change_value_ = true;
    return false;
}
std::vector<float> Grow_sensor::get_value() const {
    std::vector<float> value;
    for(int i = 0; i < get_count_component(); ++i)
        value.push_back(component_[i].get_value());
    return value;
}

// --- Информации о компонентах ---

uint8_t Grow_sensor::get_count_component() const {
    return component_.size();
}

Grow_sensor_component Grow_sensor::get_component(uint8_t num) const {
    return component_[num];
}
std::vector<Grow_sensor_component> Grow_sensor::get_component() const {
    return component_;
}

// --- Внешняя связь ---

bool Grow_sensor::filter(Grow_sensor &sensor) {
    if(component_.size() != sensor.component_.size())
        return false;
    for(int i = 0; i < component_.size(); ++i) {
        if(!component_[i].filter(sensor.component_[i]))
            return false;
    }
    return true;
}

#if defined(SERIAL_LOG_OUTPUT)
const char *sensor_component_name[] =
    {"Analog_signal", "Discrete_signal", "Battery_charge", "Air_humidity", "Air_temperature",
    "Water_temperature", "Illumination_level", "Lamp_power", "Pump_power", "Indicator_pH",
    "Indicator_EC", "Indicator_eCO2", "Indicator_nYVOC"};

void Grow_sensor::print() {
    Serial.print("System ID: ");
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i) {
        uint8_t data = system_id_[i];
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