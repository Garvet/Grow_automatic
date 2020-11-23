#include "Grow_device.h"

extern const uint16_t LORA_ADDRESS_BRANCH;

static uint8_t id_mas[COUNT_TYPE_DEVICE];

Grow_device::Grow_device(uint8_t amt_component, enum Type_device* type_device) {
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_device_component(type_device[i], (id_mas[type_device[i]]++)));
}
Grow_device::Grow_device(uint8_t amt_component, uint8_t* type_device) {
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_device_component((enum Type_device)(type_device[i]), (id_mas[type_device[i]]++)));
}
Grow_device::Grow_device(std::vector<enum Type_device> type_device) {
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < type_device.size(); ++i)
        component_.push_back(Grow_device_component(type_device[i], (id_mas[type_device[i]]++)));
}

void Grow_device::set_system_id(uint32_t system_id) {
    system_id_ = system_id;
}
uint32_t Grow_device::get_system_id() {
    return system_id_;
}

void Grow_device::set_active(uint8_t active) {
    if(active < 3)
        active_ = active;
}

uint8_t Grow_device::get_active() {
    return active_;
}

bool Grow_device::get_change_value() {
    return change_value_;
}
void Grow_device::clear_change_value() {
    change_value_ = false;
}

bool Grow_device::set_address(uint16_t address) {
    if((address >= (1 << LORA_ADDRESS_BRANCH)) || (address == 0))
        return true;
    address_ = address;
    return false;
}
uint16_t Grow_device::get_address() {
    return address_;
}

void Grow_device::set_setting(uint8_t setting) {
    setting_ = setting;
}
uint8_t Grow_device::get_setting() {
    return setting_;
}

bool Grow_device::get_type(uint8_t num, enum Type_device &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
bool Grow_device::get_id(uint8_t num, uint8_t &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_id();
    return false;
}

bool Grow_device::set_period(uint8_t num, Period_adjuster period, enum Period_type period_type) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_period(period, period_type);
    return false;
}

bool Grow_device::get_period(uint8_t num, Period_adjuster &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_period();
    return false;
}
bool Grow_device::get_period_type(uint8_t num, enum Period_type &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_period_type();
    return false;
}
bool Grow_device::set_period(std::vector<Period_adjuster> period, std::vector<enum Period_type> period_type) {
    if((get_count_component() != period.size()) || (get_count_component() != period_type.size()))
        return true;
    for(int i = 0; i < get_count_component(); ++i) {
        component_[i].set_period(period[i], period_type[i]);
    }
    return false;
}
std::vector<Period_adjuster> Grow_device::get_period() {
    std::vector<Period_adjuster> period;
    for(int i = 0; i < get_count_component(); ++i) {
        period.push_back(component_[i].get_period());
    }
    return period;
}
std::vector<enum Period_type> Grow_device::get_period_type() {
    std::vector<enum Period_type> period_type;
    for(int i = 0; i < get_count_component(); ++i) {
        period_type.push_back(component_[i].get_period_type());
    }
    return period_type;
}

int8_t Grow_device::check_time(bool clear) {
    int8_t state_change = 0;
    int8_t result = 0;
    for(int i = 0; i < get_count_component(); ++i) {
        result = component_[i].check_time(clear);
        if(result != 0)
            state_change = result;
    }
    return state_change;
}
int8_t Grow_device::check_time(uint8_t time, bool clear) {
    int8_t state_change = 0;
    int8_t result = 0;
    for(int i = 0; i < get_count_component(); ++i) {
        result = component_[i].check_time(time, clear);
        if(result != 0)
            state_change = result;
    }
    return state_change;
}
int8_t Grow_device::check_time(RtcDateTime date_time, bool clear) {
    int8_t state_change = 0;
    int8_t result = 0;
    uint8_t time = 0;
    for(int i = 0; i < get_count_component(); ++i) {
        switch (component_[i].get_period_type()) {
        case SEC: time = date_time.Second(); break;
        case MIN: time = date_time.Minute(); break;
        case HOUR: time = date_time.Hour(); break;
        default: continue;
        }
        result = component_[i].check_time(time, clear);
        if(result != 0)
            state_change = result;
    }
    return state_change;
}

bool Grow_device::set_value(uint8_t num, uint16_t value) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_value(value);
    change_value_ = true;
    return false;
}

bool Grow_device::get_value(uint8_t num, uint16_t &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_value();
    return false;
}
bool Grow_device::get_signal() {
    for(int i = 0; i < get_count_component(); ++i) {
        if(component_[i].get_signal())
            return true;
    }
    return false;
}
bool Grow_device::get_signal(uint8_t num) {
    if(get_count_component() <= num)
        return false;
    return component_[num].get_signal();
}
bool Grow_device::get_state_change() {
    for(int num = 0; num < component_.size(); ++num) {
        if(component_[num].get_state_change())
            return true;
    }
    return false;
}
bool Grow_device::get_state_change(uint8_t num, int8_t &result) {
    if(get_count_component() <= num)
        return false;
    result = component_[num].get_state_change();
    return true;
}

uint8_t Grow_device::get_count_component() {
    return component_.size();
}

Grow_device_component Grow_device::get_component(uint8_t num) {
    return component_[num];
}
std::vector<Grow_device_component> Grow_device::get_component() {
    return component_;
}

bool Grow_device::filter(Grow_device &device) {
    if(component_.size() != device.component_.size())
        return false;
    for(int i = 0; i < component_.size(); ++i) {
        if(!component_[i].filter(device.component_[i]))
            return false;
    }
    return true;
}


size_t Grow_device::get_size() {
    // uint8_t _amt_component;
    // Config_grow_device_component* _component; not save
    size_t size = 1;
    for(int i = 0; i < component_.size(); ++i)
        size += component_[i].get_size();
    return size;
}
size_t Grow_device::get_data(uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    data[size++] = component_.size();
    for(int i = 0; i < component_.size(); ++i)
        size += component_[i].get_data(data+size);
    return size;
}
size_t Grow_device::set_data(uint8_t *data, size_t available_size) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    component_.clear();
    uint8_t amt_component = data[size++];
    if(amt_component != 0) {
        component_.resize(amt_component);
        for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
            id_mas[i] = 0;
        for(int i = 0; i < amt_component; ++i) {
            size += component_[i].set_data(data+size, available_size-size);
            component_[i].set_id(id_mas[component_[i].get_type()]++);  
        }
    }
    return size;
}


#if defined (SERIAL_LOG_OUTPUT)
const char *device_component_name[] = 
    {"Signal_PWM", "Signal_digital", "Fan_PWM", "Pumping_system", "Phytolamp_digital", 
    "Phytolamp_PWM"};
const char *device_period_type_name[] = {"SEC", "MIN", "HOUR"};

void Grow_device::print() {
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
    if(active_ == 0)
        Serial.println(" (-)");
    else if(active_ == 1)
        Serial.println(" (?)");
    else
        Serial.println(" (+)");

    Serial.print("Address: ");
    if((address_ >> 8) < 16)
        Serial.print("0");
    Serial.print((address_ >> 8), 16);
    Serial.print(" ");
    if((address_ & 0xFF) < 16)
        Serial.print("0");
    Serial.print((address_ & 0xFF), 16);
    
    Serial.println("  Components:");
    enum Type_device type_component;
    enum Period_type period_type; 
    Period_adjuster period_adjuster;
    uint8_t frequency, period, bias;
    for(int j = 0; j < component_.size(); ++j) {
        Serial.print("   [");
        Serial.print(j);
        Serial.print("] ");
        get_type(j, type_component);
        Serial.print(device_component_name[type_component]);
        Serial.print(" (№");
        Serial.print((uint)type_component);
        Serial.println(")");
        get_period_type(j, period_type);
        Serial.print("       Period_type: ");
        get_period(j, period_adjuster);
        bias = period_adjuster.get_dawn();
        period = period_adjuster.get_number();
        frequency = period_adjuster.get_max_period_range() / period_adjuster.get_period_range();
        Serial.println(device_period_type_name[period_type]);
        Serial.print("       Frequency: ");
        Serial.println(frequency);
        Serial.print("       Period: ");
        Serial.println(period);
        Serial.print("       Bias: ");
        Serial.println(bias);
    }
}
#endif