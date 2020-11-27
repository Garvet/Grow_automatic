#include "Grow_device.h"

extern const uint16_t LORA_ADDRESS_BRANCH;

static uint8_t id_mas[COUNT_TYPE_DEVICE];

Grow_device::Grow_device(uint8_t amt_component, enum Type_device* type_device) {
    system_id_ = 0;
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_device_component(type_device[i], (id_mas[type_device[i]]++)));
    
    for (int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        if(id_mas[i] > 1) {
            setting_ = 0x02; // 0000.0010 - бит индивидуального номера, в случае наличия повторов
        }
}
Grow_device::Grow_device(uint8_t amt_component, uint8_t* type_device) {
    system_id_ = 0;
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < amt_component; ++i)
        component_.push_back(Grow_device_component((enum Type_device)(type_device[i]), (id_mas[type_device[i]]++)));
    
    for (int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        if(id_mas[i] > 1) {
            setting_ = 0x02;
        }
}
Grow_device::Grow_device(const std::vector<enum Type_device>& type_device) {
    system_id_ = 0;
    address_ = 0xFFFF;
    setting_ = 0;
    active_ = false;
    change_value_ = false;
    for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        id_mas[i] = 0;
    for(int i = 0; i < type_device.size(); ++i)
        component_.push_back(Grow_device_component(type_device[i], (id_mas[type_device[i]]++)));
    
    for (int i = 0; i < COUNT_TYPE_DEVICE; ++i)
        if(id_mas[i] > 1) {
            setting_ = 0x02;
        }
}

// --- Поля класса-платы ---

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

// --- Обработка времени ---

void Grow_device::set_period(unsigned long period) {
    period_ = period;
}
unsigned long Grow_device::get_period() {
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

bool Grow_device::get_type(uint8_t num, enum Type_device &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_type();
    return false;
}
std::vector<enum Type_device> Grow_device::get_type() {
    std::vector<enum Type_device> type_device;
    for(int i = 0; i < get_count_component(); ++i)
        type_device.push_back(component_[i].get_type());
    return type_device;
}
bool Grow_device::get_id(uint8_t num, uint8_t &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_id();
    return false;
}
std::vector<uint8_t> Grow_device::get_id() {
    std::vector<uint8_t> id;
    for(int i = 0; i < get_count_component(); ++i)
        id.push_back(component_[i].get_id());
    return id;
}

bool Grow_device::set_pwm_value(uint8_t num, uint16_t pwm_value) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_pwm_value(pwm_value);
    change_value_ = true;
    return false;
}
bool Grow_device::get_pwm_value(uint8_t num, uint16_t &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_pwm_value();
    return false;
}
bool Grow_device::set_pwm_value(const std::vector<uint16_t>& pwm_value) {
    if(get_count_component() != pwm_value.size())
        return true;
    for(int i = 0; i < get_count_component(); ++i)
        component_[i].set_pwm_value(pwm_value[i]);
    change_value_ = true;
    return false;
}
std::vector<uint16_t> Grow_device::get_pwm_value() {
    std::vector<uint16_t> pwm_value;
    for(int i = 0; i < get_count_component(); ++i)
        pwm_value.push_back(component_[i].get_pwm_value());
    return pwm_value;
}

bool Grow_device::set_state(uint8_t num, bool state) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_state(state);
    change_value_ = true;
    return false;
}
bool Grow_device::get_state(uint8_t num, bool &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_state();
    return false;
}
bool Grow_device::set_state(std::vector<bool> state) {
    if(get_count_component() != state.size())
        return true;
    for(int i = 0; i < get_count_component(); ++i)
        component_[i].set_state(state[i]);
    change_value_ = true;
    return false;
}
std::vector<bool> Grow_device::get_state() {
    std::vector<bool> state;
    for(int i = 0; i < get_count_component(); ++i)
        state.push_back(component_[i].get_state());
    return state;
}

bool Grow_device::set_time_control(uint8_t num, Time_control time_control) {
    if(get_count_component() <= num)
        return true;
    component_[num].set_time_control(time_control);
    change_value_ = true;
    return false;
}
bool Grow_device::get_time_control(uint8_t num, Time_control &result) {
    if(get_count_component() <= num)
        return true;
    result = component_[num].get_time_control();
    return false;
}
bool Grow_device::set_time_control(const std::vector<Time_control>& time_controls) {
    if(get_count_component() != time_controls.size())
        return true;
    for(int i = 0; i < get_count_component(); ++i)
        component_[i].set_time_control(time_controls[i]);
    change_value_ = true;
    return false;
}
std::vector<Time_control> Grow_device::get_time_control() {
    std::vector<Time_control> state;
    for(int i = 0; i < get_count_component(); ++i)
        state.push_back(component_[i].get_time_control());
    return state;
}

// --- Информации о компонентах ---

uint8_t Grow_device::get_count_component() {
    return component_.size();
}

Grow_device_component Grow_device::get_component(uint8_t num) {
    return component_[num];
}
std::vector<Grow_device_component> Grow_device::get_component() {
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


#if defined(SERIAL_LOG_OUTPUT)
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
    
    // Serial.println("  Components:");
    // enum Type_device type_component;
    // enum Period_type period_type; 
    // Period_adjuster period_adjuster;
    // uint8_t frequency, period, bias;
    // for(int j = 0; j < component_.size(); ++j) {
    //     Serial.print("   [");
    //     Serial.print(j);
    //     Serial.print("] ");
    //     get_type(j, type_component);
    //     Serial.print(device_component_name[type_component]);
    //     Serial.print(" (№");
    //     Serial.print((uint)type_component);
    //     Serial.println(")");
    //     get_period_type(j, period_type);
    //     Serial.print("       Period_type: ");
    //     get_period(j, period_adjuster);
    //     bias = period_adjuster.get_dawn();
    //     period = period_adjuster.get_number();
    //     frequency = period_adjuster.get_max_period_range() / period_adjuster.get_period_range();
    //     Serial.println(device_period_type_name[period_type]);
    //     Serial.print("       Frequency: ");
    //     Serial.println(frequency);
    //     Serial.print("       Period: ");
    //     Serial.println(period);
    //     Serial.print("       Bias: ");
    //     Serial.println(bias);
    // }
}
#endif