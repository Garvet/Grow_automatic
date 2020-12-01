#include "Exchange_packet.h"

Exchange_packet::Exchange_packet() {
    packet = &packet_analyzer;
    // packet_data = new LoRa_packet();
    // len = 0;
    // creat_packet(1,0);
}

Exchange_packet::Exchange_packet(const Exchange_packet &exchange_packet) {
    packet = &packet_analyzer;
    // packet_data = new LoRa_packet();
    // len = 0;
    *this = exchange_packet;
}

Exchange_packet::Exchange_packet(Exchange_packet &&exchange_packet) {
    *this = std::move(exchange_packet);
}

Exchange_packet::~Exchange_packet() {
    // if(packet_data != nullptr)
    //     delete packet_data;
    // creat_packet(0, 0xFF);
    // if(packet != nullptr)
    //     delete[] packet;
}

void Exchange_packet::creat_packet(uint8_t new_len, uint8_t new_type) {
    creat_packet(new_len, (Packet_Type)new_type);
}
void Exchange_packet::creat_packet(uint8_t new_len, Packet_Type new_type) {
    bool change = false;
    // if (len != new_len){
    //     len = new_len;
    //     if(packet_data != nullptr) {
    //         delete[] packet_data;
    //         packet_data = nullptr;
    //     }
    //     if(len != 0) {
    //         packet_data = new uint8_t[len];
    //         change = true;
    //     }
    // }
    // packet_data.clear_packet();
    if (type_packet != new_type) {
        type_packet = new_type;
        // if(packet != nullptr) {
        //     // delete packet;
        //     delete[] packet;
        //     packet = nullptr;
        // }
        if(new_len != 0) {
            switch (new_type) {
            case Packet_Type::CONNECTION:
                packet = &packet_connection;
                break;
            case Packet_Type::SENSOR:
                packet = &packet_sensor;
                break;
            case Packet_Type::DEVICE:
                packet = &packet_device;
                break;
            case Packet_Type::SYSTEM:
                packet = &packet_system;
                break;
            case (Packet_Type)0x04: // (-) -----
                packet = &packet_analyzer;  //&packet_error
                break;
            default:
                packet = &packet_analyzer;
                break;
            }
            // packet->select_packet(packet_data);
            change = true;
        }
    }
    if(change && (packet != nullptr)) {
        // packet->select_packet(packet_data);
        packet->set_setting(setting_);
    }
}

void Exchange_packet::clear_packet() {
    packet_data.clear_packet();
    // creat_packet(1,0);
}

void Exchange_packet::set_setting(uint8_t setting) {
    setting_ = setting;
    if(packet != nullptr) 
        packet->set_setting(setting_);
}
uint8_t Exchange_packet::get_setting() const {
    return setting_;
}

std::vector<uint8_t> Exchange_packet::get_packet() const {
    std::vector<uint8_t> pack;
    for(int i = 0; i < packet_data.get_len(); ++i)
        pack.push_back(packet_data[i]);
    return pack;
}
void Exchange_packet::set_packet(const std::vector<uint8_t>& pack) {
    creat_packet(pack.size(), 0);
    for (int i = 0; i < pack.size(); ++i)
        packet_data.add_packet_data(pack[i]);
        // packet_data[i] = pack[i];
    creat_packet(pack.size(), packet_data[6]);
}

class Exchange_packet& Exchange_packet::operator=(const class Exchange_packet& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Копирование значений
    set_setting(right.setting_);
    packet_data = right.packet_data;
    // if(right.packet_data != nullptr) {
    //     creat_packet(right.len, right.type_packet);
    //     for (int i = 0; i < len; ++i)
    //         packet_data[i] = right.packet_data[i];
    // }

    return *this;
}
class Exchange_packet& Exchange_packet::operator=(class Exchange_packet&& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Перемещение значений
    setting_ = right.setting_;
    packet = right.packet;
    packet_data = std::move(right.packet_data);
    // len = right.len;
    type_packet = right.type_packet;
    // Удаление старых значений
    right.setting_ = 0;
    // right.packet = nullptr;
    // right.packet_data = nullptr;
    // right.len = 0;
    // right.type_packet = 0xFF;
    return *this;
}
