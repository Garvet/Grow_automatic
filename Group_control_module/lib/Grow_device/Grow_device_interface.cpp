#include "Grow_device_interface.h"

Grow_device_interface grow_device_interface;

extern const uint16_t LORA_ADDRESS_BRANCH;
extern uint8_t id_mas_devices[COUNT_TYPE_DEVICE];

// --- Сохранение в энергонезависимую память ---

size_t Grow_device_interface::get_size(Grow_device &grow_device) {
    // unsigned long _period; 4
    // uint8_t _amt_component; 1
    // Config_grow_device_component* _component; not save
    size_t size = 5;
    for(int i = 0; i < grow_device.component_.size(); ++i)
        size += grow_device.component_[i].get_size();
    return size;
}
size_t Grow_device_interface::get_data(Grow_device &grow_device, uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    data[size++] = (uint8_t)((grow_device.period_ >> 24) & 0xFF);
    data[size++] = (uint8_t)((grow_device.period_ >> 16) & 0xFF);
    data[size++] = (uint8_t)((grow_device.period_ >> 8) & 0xFF);
    data[size++] = (uint8_t)(grow_device.period_ & 0xFF);
    data[size++] = grow_device.component_.size();
    for(int i = 0; i < grow_device.component_.size(); ++i)
        size += grow_device.component_[i].get_data(data+size);
    return size;
}
size_t Grow_device_interface::set_data(Grow_device &grow_device, uint8_t *data, size_t available_size) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    grow_device.component_.clear();
    grow_device.period_  = ((ulong)data[size++]) << 24;
    grow_device.period_ |= ((ulong)data[size++]) << 16;
    grow_device.period_ |= ((ulong)data[size++]) << 8;
    grow_device.period_ |= ((ulong)data[size++]);
    grow_device.read_time_ = millis() - grow_device.period_;
    uint8_t amt_component = data[size++];
    if(amt_component != 0) {
        grow_device.component_.resize(amt_component);
        for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
            id_mas_devices[i] = 0;
        for(int i = 0; i < amt_component; ++i) {
            size += grow_device.component_[i].set_data(data+size, available_size-size);
            grow_device.component_[i].set_id(id_mas_devices[grow_device.component_[i].get_type()]++);
        }
        for(int i = 0; i < COUNT_TYPE_DEVICE; ++i)
            if(id_mas_devices[i] > 1) {
                grow_device.set_setting(0x02);
            }
    }
    return size;
}

// --- Обмен с телефоном ---

// --- LoRa-соединение ---
LoRa_packet Grow_device_interface::creat_regist_packet(const Grow_device &grow_device, LoRa_contact_data& contact_data) {
    // Exchange_packet packet;
    // // packet.creat_packet(16 + grow_device.get_count_component(), Packet_Type::SYSTEM);
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
    // uint8_t device_type = 0;
    uint8_t len = grow_device.get_count_component();
    uint8_t num_byte = 0;
    uint8_t* data = new uint8_t[len + AMT_BYTES_SYSTEM_ID + 2]; // ID, Type, Length

    std::array<uint8_t, AMT_BYTES_SYSTEM_ID> id = grow_device.get_system_id();
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        data[num_byte++] = id[i];

    // data[num_byte++] = (grow_device.get_system_id() >> 24) & 0xFF;
    // data[num_byte++] = (grow_device.get_system_id() >> 16) & 0xFF;
    // data[num_byte++] = (grow_device.get_system_id() >> 8) & 0xFF;
    // data[num_byte++] = grow_device.get_system_id() & 0xFF;

    data[num_byte++] = 0x01; // Type = devices

    data[num_byte++] = len; // Length = grow_device.get_count_component();
    for(int i = 0; i < len; ++i)
    // {
        grow_device.get_type(i, data[num_byte++]);
    //     grow_device.get_type(i, device_type);
    //     data[num_byte++] = device_type;
    // }
    packet_system.set_packet_data(packet, &com, data, &len);
    delete[] data;
    return packet;
}
Grow_device Grow_device_interface::read_regist_packet(LoRa_packet& packet) {
    // Получение длины
    uint8_t size;
    uint8_t err = 0;
    err = packet_system.get_size_by_packet(packet, size);
    if((err != 0) || (size == 0))
        return Grow_device();
    //Получение содержимого
    uint8_t com = 0;
    uint8_t len = 0;
    uint8_t* data = nullptr;
    data = new uint8_t[size]; // ID, Type, Length
    err = packet_system.get_packet_data(packet, &com, data, &len);
    if((err != 0) || (com != 0)) {
        return Grow_device();
    }
    // Расшифровка содержимого
    uint8_t num_byte = 0;

    std::array<uint8_t, AMT_BYTES_SYSTEM_ID> device_id;
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        device_id[i] = data[num_byte++];
    // uint32_t device_id = data[num_byte++];
    // device_id = (device_id << 8) | data[num_byte++];
    // device_id = (device_id << 8) | data[num_byte++];
    // device_id = (device_id << 8) | data[num_byte++];
    // Генерация объекта
    Grow_device devices(len, &(data[num_byte]));
    devices.set_system_id(device_id);
    delete[] data;
    return devices;
}
bool Grow_device_interface::check_regist_packet(LoRa_contact_data& contact_data) {

}
uint8_t Grow_device_interface::creat_send_data_packet(Grow_device &grow_device, LoRa_contact_data& contact_data) {
    if(contact_data.get_signal_start_connect())
        return 0;
    LoRa_packet packet;
    uint8_t err = 0;
    uint8_t amt = grow_device.get_count_component();
    uint8_t obj, id, com = 0x01; // com???
    uint16_t value;
    packet_device.set_setting(grow_device.get_setting());
    contact_data.clear_send_packet();
    for(int i = 0; i < amt; ++i) {
    // last_send_packet_.packet->set_dest_adr(connect_adr_);
    // last_send_packet_.packet->set_sour_adr(my_adr_);
        packet_device.set_dest_adr(packet, contact_data.get_connect_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
        packet_device.set_sour_adr(packet, contact_data.get_my_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
        err = grow_device.get_type(i, obj);
        if(err) return i;
        err = grow_device.get_id(i, id);
        if(err) return i;
        err = grow_device.get_value(i, value);
        if(err) return i;
        packet_device.set_packet_type(packet, Packet_Type::DEVICE);

        err = packet_device.set_packet_data(packet, &obj, &id, &com, (uint8_t*)&value, nullptr);

        // err = packet_device.set_packet_data(LoRa_packet& packet, uint8_t *obj, uint8_t *num, uint8_t *com, uint8_t *data, uint8_t *len)

        // err = packet_sensor.set_packet_data(             packet,          nullptr,         &param,         &id,          &value);
        // err = packet_sensor.set_packet_data(LoRa_packet& packet, uint8_t* amt,     uint8_t* param, uint8_t* id, uint32_t* data);


        // packet_device.set_dest_adr(packet, contact_data.get_connect_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
        // packet_device.set_sour_adr(packet, contact_data.get_my_adr()); // адреса задаются при передаче в LoRa_contact_data LORA_GLOBAL_ADDRESS
        // err = grow_device.get_type(i, param);
        // if(err) return i;
        // err = grow_device.get_id(i, id);
        // if(err) return i;
        // err = grow_device.get_value(i, *reinterpret_cast<float*>(&value));
        // if(err) return i;
        // packet_device.set_packet_type(packet, Packet_Type::DEVICE);
        // err = packet_device.set_packet_data(packet, nullptr, &param, &id, &value);
        // if(err) return i;
        contact_data.add_packet(std::move(packet));


    }
    contact_data.wait_recipient();
    return amt;
}
uint8_t Grow_device_interface::read_send_data_packet(Grow_device &grow_device, LoRa_packet* packet, uint8_t amt) {
    return 0; // (-) ----- (!) -----
}

// --- Отчётность ---
uint16_t Grow_device_interface::report_to_server(Grow_device &grow_device, uint8_t *buf, uint16_t size) {
    uint16_t save_size = 0;
    if(size < 5 + (6 * grow_device.get_count_component()))
        return save_size;
    uint8_t save_byte = 0;
    uint8_t dev_type = 0;
    uint16_t save_value = 0;
    uint32_t save_component;
    // ID
    std::array<uint8_t, AMT_BYTES_SYSTEM_ID> device_id;
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        buf[save_size++] = device_id[i];

    // тип пакета
    buf[save_size++] = 2; // устройства
    // save_component = grow_device.get_system_id();
    // buf[save_size++] = (save_component >> 24) & 0xFF;
    // buf[save_size++] = (save_component >> 16) & 0xFF;
    // buf[save_size++] = (save_component >>  8) & 0xFF;
    // buf[save_size++] =  save_component        & 0xFF;
    // количество компонентов
    buf[save_size++] = grow_device.get_count_component();
    // состав компонентов
    for(int i = 0; i < grow_device.get_count_component(); ++i) {
        // тип
        grow_device.get_type(i, dev_type);
        buf[save_size++] = dev_type;
        // номер
        grow_device.get_id(i, save_byte);
        buf[save_size++] = save_byte;
        // значение
        grow_device.get_value(i, save_value);
        if(dev_type == Signal_PWM || dev_type == Fan_PWM || dev_type == Phytolamp_PWM) {
            buf[save_size++] = (save_value >>  8) & 0xFF;
            buf[save_size++] =  save_value & 0xFF;
        }
        else if (dev_type == Signal_digital || dev_type == Pumping_system || dev_type == Phytolamp_digital) {
            if(save_value)
                buf[save_size++] =  1 & 0xFF;
            else
                buf[save_size++] =  0 & 0xFF;
        }
        else {
            Serial.println("Error send type devices");
            break;
        }

        // buffer[buf_len++] = rdt.Day();
        // buffer[buf_len++] = rdt.Month();
        // buffer[buf_len++] = (rdt.Year() >> 8) & 0xFF;
        // buffer[buf_len++] = rdt.Year() & 0xFF;

        // buffer[buf_len++] = rdt.Hour();
        // buffer[buf_len++] = rdt.Minute();
        // buffer[buf_len++] = rdt.Second();

    }
    return save_size;
}