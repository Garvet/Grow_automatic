#include "Grow_device.h"

extern const uint16_t LORA_ADDRESS_BRANCH;

uint8_t id_mas_devices[COUNT_TYPE_DEVICE];

Grow_device::Grow_device(uint8_t amt_component, enum Type_device* type_device) {
    for(int i = 0; i < scs::AMT_BYTES_ID; ++i)
        system_id[i] = 0;
    setting_ = 0;
    active_ = 0;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas_devices[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_device_component(type_device[i], (id_mas_devices[type_device[i]]++)));

    for (int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        if(id_mas_devices[i] > 1) {
            setting_ = 0x02; // 0000.0010 - бит индивидуального номера, в случае наличия повторов
        }
}
Grow_device::Grow_device(uint8_t amt_component, uint8_t* type_device) {
    for(int i = 0; i < scs::AMT_BYTES_ID; ++i)
        system_id[i] = 0;
    setting_ = 0;
    active_ = 0;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas_devices[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_device_component((enum Type_device)(type_device[i]), (id_mas_devices[type_device[i]]++)));

    for (int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        if(id_mas_devices[i] > 1) {
            setting_ = 0x02;
        }
}

Grow_device::Grow_device(const std::vector<enum Type_device>& type_device) {
    for(int i = 0; i < scs::AMT_BYTES_ID; ++i)
        system_id[i] = 0;
    setting_ = 0;
    active_ = 0;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas_devices[i] = 0;
    for(int i = 0; i < type_device.size(); ++i)
        component_.push_back(Grow_device_component(type_device[i], (id_mas_devices[type_device[i]]++)));

    for (int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        if(id_mas_devices[i] > 1) {
            setting_ = 0x02;
        }
}

// --- Поля класса-платы ---

void Grow_device::set_active(uint8_t active) {
    if(active < 3)
        active_ = active;
}

uint8_t Grow_device::get_active() const {
    return active_;
}

bool Grow_device::get_change_value() const {
    return change_value_;
}
void Grow_device::clear_change_value() {
    change_value_ = false;
}

// bool Grow_device::set_address(uint16_t address) {
//     if((address >= (1 << LORA_ADDRESS_BRANCH)) || (address == 0))
//         return true;
//     address_ = address;
//     return false;
// }
// uint16_t Grow_device::get_address() const {
//     return address_;
// }

void Grow_device::set_setting(uint8_t setting) {
    setting_ = setting;
}
uint8_t Grow_device::get_setting() const {
    return setting_;
}

// --- Обработка времени ---

void Grow_device::set_period(unsigned long period) {
    period_ = period;
}
unsigned long Grow_device::get_period() const {
    return period_;
}

bool Grow_device::check_time(unsigned long time) {
    end_time_ = time;
    if ((end_time_ - read_time_) > period_)
        readout_signal_ = true;
    return readout_signal_;
}
void Grow_device::update() {
    read_time_ = end_time_;
}
bool Grow_device::read_signal(bool clear) {
    if (!clear)
        return readout_signal_;
    clear = readout_signal_;
    readout_signal_ = false;
    return clear;
}

// --- Поля компонентов ---

bool Grow_device::get_type(uint8_t num, enum Type_device &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
bool Grow_device::get_type(uint8_t num, uint8_t &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
std::vector<enum Type_device> Grow_device::get_type() const {
    std::vector<enum Type_device> type_device;
    for(int i = 0; i < get_count_component(); ++i)
        type_device.push_back(component_[i].get_type());
    return type_device;
}
bool Grow_device::get_id(uint8_t num, uint8_t &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_id();
    return false;
}
std::vector<uint8_t> Grow_device::get_id() const {
    std::vector<uint8_t> id;
    for(int i = 0; i < get_count_component(); ++i)
        id.push_back(component_[i].get_id());
    return id;
}

bool Grow_device::set_value(uint8_t num, uint16_t value) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_value(value);
    change_value_ = true;
    return false;
}
bool Grow_device::get_value(uint8_t num, uint16_t &result) const {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_value();
    return false;
}
bool Grow_device::set_value(const std::vector<uint16_t>& value) {
    if(get_count_component() != value.size())
        return true;
    for(int i = 0; i < get_count_component(); ++i)
        component_[i].set_value(value[i]);
    change_value_ = true;
    return false;
}
std::vector<uint16_t> Grow_device::get_value() const {
    std::vector<uint16_t> value;
    for(int i = 0; i < get_count_component(); ++i)
        value.push_back(component_[i].get_value());
    return value;
}

// Установить флаг необходимости синхронизации времени
void Grow_device::set_rtc_sync(bool flag) {
    rtc_sync = flag;
}
// Получить флаг необходимости синхронизации времени
bool Grow_device::get_rtc_sync() const {
    return rtc_sync;
}

// --- Информации о компонентах ---

uint8_t Grow_device::get_count_component() const {
    return component_.size();
}

Grow_device_component Grow_device::get_component(uint8_t num) const {
    return component_[num];
}
std::vector<Grow_device_component> Grow_device::get_component() const {
    return component_;
}

// --- Внешняя связь ---

bool Grow_device::filter(Grow_device &device) {
    if(component_.size() != device.component_.size())
        return false;
    for(int i = 0; i < component_.size(); ++i) {
        if(!component_[i].filter(device.component_[i]))
            return false;
    }
    return true;
}

// --- \/ \/ \/ Костыль \/ \/ \/ ---
bool Grow_device::get_setting_change_period() const {
    return setting_change_period;
}
void Grow_device::set_setting_change_period(bool set_setting) {
    setting_change_period = set_setting;
}

bool Grow_device::get_setting_change_mode() const {
    return setting_change_mode;
}
void Grow_device::set_setting_change_mode(bool set_setting) {
    setting_change_mode = set_setting;
}
// --- /\ /\ /\ Костыль /\ /\ /\ ---

#if defined(SERIAL_LOG_OUTPUT)
const char *device_component_name[] =
    {"Signal_PWM", "Signal_digital", "Fan_PWM", "Pumping_system", "Phytolamp_digital", "Phytolamp_PWM"};

void Grow_device::print() {
    Serial.print("System ID: ");
    for(int i = 0; i < scs::AMT_BYTES_ID; ++i) {
        uint8_t data = system_id[i];
        if(data < 16)
            Serial.print("0");
        Serial.print(data, 16);
        if(i < scs::AMT_BYTES_ID)
            Serial.print(".");
    }
    Serial.println();
    Serial.print("Address: ");
    if((address_.branch >> 8) < 16)
        Serial.print("0");
    Serial.print((address_.branch >> 8), 16);
    Serial.print(" ");
    if((address_.branch & 0xFF) < 16)
        Serial.print("0");
    Serial.print((address_.branch & 0xFF), 16);
    if(active_ == 0)
        Serial.println(" (-)");
    else if(active_ == 1)
        Serial.println(" (?)");
    else
        Serial.println(" (+)");

    Serial.print("  Period: ");
    Serial.println(get_period());

    Serial.println("  Components:");
    enum Type_device type_component;
    for(int j = 0; j < component_.size(); ++j) {
        Serial.print("    [");
        Serial.print(j);
        Serial.print("] ");
        get_type(j, type_component);
        Serial.print(device_component_name[type_component]);
        Serial.print(" (№");
        Serial.print((uint)type_component);
        Serial.println(")");
    }
}
#endif