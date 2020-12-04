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
LoRa_packet Grow_sensor_interface::creat_regist_packet(const Grow_sensor &grow_sensor, LoRa_contact_data& contact_data) {
    // Exchange_packet packet;
    // // packet.creat_packet(16 + grow_sensor.get_count_component(), Packet_Type::SYSTEM); 
    // // packet.packet->set_dest_adr(LORA_GLOBAL_ADDRESS);
    // // packet.packet->set_sour_adr(LORA_GLOBAL_ADDRESS);
    // // packet.packet->set_packet_type(Packet_Type::SYSTEM);
    // packet_system.set_dest_adr(packet.packet_data, LORA_GLOBAL_ADDRESS);
    // packet_system.set_sour_adr(packet.packet_data, LORA_GLOBAL_ADDRESS);
    // packet_system.set_packet_type(packet.packet_data, Packet_Type::SYSTEM);
    LoRa_packet packet;
    packet_system.set_dest_adr(packet, LORA_GLOBAL_ADDRESS);
    packet_system.set_sour_adr(packet, LORA_GLOBAL_ADDRESS);
    packet_system.set_packet_type(packet, Packet_Type::SYSTEM);

    uint8_t com = 0;
    // uint8_t sensor_type = 0;
    uint8_t len = grow_sensor.get_count_component();
    uint8_t num_byte = 0;
    uint8_t* data = new uint8_t[len + 6]; // ID, Type, Length

    data[num_byte++] = (grow_sensor.get_system_id() >> 24) & 0xFF;
    data[num_byte++] = (grow_sensor.get_system_id() >> 16) & 0xFF;
    data[num_byte++] = (grow_sensor.get_system_id() >> 8) & 0xFF;
    data[num_byte++] = grow_sensor.get_system_id() & 0xFF;

    data[num_byte++] = 0x01; // Type = sensors

    data[num_byte++] = len; // Length = grow_sensor.get_count_component();
    for(int i = 0; i < len; ++i)
    // {
        grow_sensor.get_type(i, data[num_byte++]);
    //     grow_sensor.get_type(i, sensor_type);
    //     data[num_byte++] = sensor_type;
    // }
    packet_system.set_packet_data(packet, &com, data, &len);
    delete[] data;
    return packet;
}
Grow_sensor Grow_sensor_interface::read_regist_packet(LoRa_packet& packet) {
    // Получение длины
    uint8_t size;
    uint8_t err = 0;
    err = packet_system.get_size_by_packet(packet, size);
    if((err != 0) || (size == 0)) 
        return Grow_sensor();
    //Получение содержимого
    uint8_t com = 0;
    uint8_t len = 0;
    uint8_t* data = nullptr;
    data = new uint8_t[size]; // ID, Type, Length
    err = packet_system.get_packet_data(packet, &com, data, &len);
    if((err != 0) || (com != 0)) {
        return Grow_sensor();
    }
    // Расшифровка содержимого
    uint8_t num_byte = 0;
    uint32_t sensor_id = data[num_byte++];
    sensor_id = (sensor_id << 8) | data[num_byte++];
    sensor_id = (sensor_id << 8) | data[num_byte++];
    sensor_id = (sensor_id << 8) | data[num_byte++];
    // Генерация объекта
    Grow_sensor sensors(len, &(data[num_byte]));
    sensors.set_system_id(sensor_id);
    delete[] data;
    return sensors;
}
bool Grow_sensor_interface::check_regist_packet(LoRa_contact_data& contact_data) {

}
uint8_t Grow_sensor_interface::creat_send_data_packet(Grow_sensor &grow_sensor, LoRa_contact_data& contact_data) {
    if(contact_data.get_signal_start_connect())
        return 0;
    LoRa_packet packet;
    uint8_t err = 0;
    uint8_t amt = grow_sensor.get_count_component();
    uint8_t param, id;
    uint32_t value;
    packet_sensor.set_setting(grow_sensor.get_setting());
    contact_data.clear_send_packet();
    for(int i = 0; i < amt; ++i) {
    // last_send_packet_.packet->set_dest_adr(connect_adr_);
    // last_send_packet_.packet->set_sour_adr(my_adr_);
        packet_sensor.set_dest_adr(packet, contact_data.get_connect_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
        packet_sensor.set_sour_adr(packet, contact_data.get_my_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
        err = grow_sensor.get_type(i, param);
        if(err) return i;
        err = grow_sensor.get_id(i, id);
        if(err) return i;
        err = grow_sensor.get_value(i, *reinterpret_cast<float*>(&value));
        if(err) return i;
        packet_sensor.set_packet_type(packet, Packet_Type::SENSOR);
        err = packet_sensor.set_packet_data(packet, nullptr, &param, &id, &value);
        if(err) return i;
        contact_data.add_packet(std::move(packet));
    }
    contact_data.wait_recipient();
    return amt;
}
uint8_t Grow_sensor_interface::read_send_data_packet(Grow_sensor &grow_sensor, LoRa_packet* packet, uint8_t amt) {
    return 0; // (-) ----- (!) -----
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