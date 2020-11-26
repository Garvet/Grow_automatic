#include "Grow_sensor_interface.h"

Grow_sensor_interface grow_sensor_interface;

extern const uint16_t LORA_ADDRESS_BRANCH;
extern uint8_t id_mas_sensors[COUNT_TYPE_SENSOR];

// --- Сохранение в энергонезависимую память ---

size_t Grow_sensor_interface::get_size(Grow_sensor &grow_sensor) {
    // unsigned long _period; 4
    // uint8_t _amt_component; 1
    // Config_grow_sensor_component* _component; not save
    size_t size = 5;
    for(int i = 0; i < grow_sensor.component_.size(); ++i)
        size += grow_sensor.component_[i].get_size();
    return size;
}
size_t Grow_sensor_interface::get_data(Grow_sensor &grow_sensor, uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    data[size++] = (uint8_t)((grow_sensor.period_ >> 24) & 0xFF);
    data[size++] = (uint8_t)((grow_sensor.period_ >> 16) & 0xFF);
    data[size++] = (uint8_t)((grow_sensor.period_ >> 8) & 0xFF);
    data[size++] = (uint8_t)(grow_sensor.period_ & 0xFF);
    data[size++] = grow_sensor.component_.size();
    for(int i = 0; i < grow_sensor.component_.size(); ++i)
        size += grow_sensor.component_[i].get_data(data+size);
    return size;
}
size_t Grow_sensor_interface::set_data(Grow_sensor &grow_sensor, uint8_t *data, size_t available_size) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    grow_sensor.component_.clear();
    grow_sensor.period_  = ((ulong)data[size++]) << 24;
    grow_sensor.period_ |= ((ulong)data[size++]) << 16;
    grow_sensor.period_ |= ((ulong)data[size++]) << 8;
    grow_sensor.period_ |= ((ulong)data[size++]);
    uint8_t amt_component = data[size++];
    if(amt_component != 0) {
        grow_sensor.component_.resize(amt_component);
        for(int i = 0; i < COUNT_TYPE_SENSOR; ++i)
            id_mas_sensors[i] = 0;
        for(int i = 0; i < amt_component; ++i) {
            size += grow_sensor.component_[i].set_data(data+size, available_size-size);
            grow_sensor.component_[i].set_id(id_mas_sensors[grow_sensor.component_[i].get_type()]++);            
        }
    }
    return size;
}

// --- Обмен с телефоном ---

// --- LoRa-соединение ---
Exchange_packet Grow_sensor_interface::creat_regist_packet(Grow_sensor &grow_sensor) {
    Exchange_packet packet;
    packet.creat_packet(16 + grow_sensor.get_count_component(), PACKET_SYSTEM); 
    packet.packet->set_dest_adr(LORA_GLOBAL_ADDRESS);
    packet.packet->set_sour_adr(LORA_GLOBAL_ADDRESS);
    packet.packet->set_packet_type(PACKET_SYSTEM);

    uint8_t com = 0;
    uint8_t len = grow_sensor.get_count_component();
    uint8_t* data = new uint8_t[len + 6]; // ID, Type, Length

    data[0] = (grow_sensor.get_system_id() >> 24) & 0xFF;
    data[1] = (grow_sensor.get_system_id() >> 16) & 0xFF;
    data[2] = (grow_sensor.get_system_id() >> 8) & 0xFF;
    data[3] = grow_sensor.get_system_id() & 0xFF;

    data[4] = 0x01; // sensors

}
Grow_sensor Grow_sensor_interface::read_regist_packet(Exchange_packet& Exchange_packet) {
    // Exchange_packet packet;
    // packet.creat_packet(16 + grow_sensor.get_count_component(), PACKET_SYSTEM); 
    // packet.packet->set_dest_adr(LORA_GLOBAL_ADDRESS);
    // packet.packet->set_sour_adr(LORA_GLOBAL_ADDRESS);
    // packet.packet->set_packet_type(PACKET_SYSTEM);

    // uint8_t com = 0;
    // uint8_t len = grow_sensor.get_count_component();
    // uint8_t* data = new uint8_t[len + 6]; // ID и Type

    // data[0] = (grow_sensor.get_system_id() >> 24) & 0xFF;
    // data[1] = (grow_sensor.get_system_id() >> 16) & 0xFF;
    // data[2] = (grow_sensor.get_system_id() >> 8) & 0xFF;
    // data[3] = grow_sensor.get_system_id() & 0xFF;

    // data[4] = 0x01; // sensors

}
std::vector<std::vector<uint8_t>> creat_send_data_packet(Grow_sensor &grow_sensor) {
    
}

// --- Отчётность ---
uint16_t Grow_sensor_interface::report_to_server(Grow_sensor &grow_sensor, uint8_t *buf, uint16_t size) {
    uint16_t save_size = 0;
    if(size < 5 + (6 * grow_sensor.get_count_component()))
        return save_size;
    uint8_t save_byte = 0;
    float save_value = 0;
    uint32_t save_component = grow_sensor.get_system_id(); 
    // ID
    buf[save_size++] = (save_component >> 24) & 0xFF;
    buf[save_size++] = (save_component >> 16) & 0xFF;
    buf[save_size++] = (save_component >>  8) & 0xFF;
    buf[save_size++] =  save_component        & 0xFF;
    // количество компонентов
    buf[save_size++] = grow_sensor.get_count_component();
    // состав компонентов
    for(int i = 0; i < grow_sensor.get_count_component(); ++i) {
        // тип
        grow_sensor.get_type(i, save_byte);
        buf[save_size++] = save_byte;
        // номер
        grow_sensor.get_id(i, save_byte);
        buf[save_size++] = save_byte;
        // значение
        grow_sensor.get_value(i, save_value);
        save_component = *reinterpret_cast<uint32_t*>(&save_value);
        buf[save_size++] = (save_component >> 24) & 0xFF;
        buf[save_size++] = (save_component >> 16) & 0xFF;
        buf[save_size++] = (save_component >>  8) & 0xFF;
        buf[save_size++] =  save_component        & 0xFF;
    }
    return save_size;
}