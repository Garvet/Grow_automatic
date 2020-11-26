#include "Exchange_packet.h"

Exchange_packet::Exchange_packet() {
    packet = nullptr;
    packet_data = nullptr;
    len = 0;
    creat_packet(1,0);
}

Exchange_packet::Exchange_packet(const Exchange_packet &exchange_packet) {
    packet = nullptr;
    packet_data = nullptr;
    len = 0;
    *this = exchange_packet;
}

Exchange_packet::Exchange_packet(Exchange_packet &&exchange_packet) {
    *this = std::move(exchange_packet);
}

Exchange_packet::~Exchange_packet() {
    creat_packet(0, 0xFF);
}


void Exchange_packet::creat_packet(uint8_t new_len, uint8_t new_type) {
    bool change = false;
    if (len != new_len){
        len = new_len;
        if(packet_data != nullptr) {
            delete[] packet_data;
            packet_data = nullptr;
        }
        if(len != 0) {
            packet_data = new uint8_t[len];
            change = true;
        }
    }
    if (type_packet != new_type) {
        type_packet = new_type;
        if(packet != nullptr) {
            delete[] packet;
            packet = nullptr;
        }
        if((len != 0) && (new_type != 0xFF)) {
            switch (new_type) {
            case PACKET_CONNECTION:
                packet = new class Packet_Connection;
                break;
            case PACKET_SENSOR:
                packet = new class Packet_Sensor;
                break;
            case PACKET_DEVICE:
                packet = new class Packet_Device;
                break;
            case PACKET_SYSTEM:
                packet = new class Packet_System;
                break;
            case 0x04: // (-) -----
                // packet = new class Packet_Error;
                packet = new class Packet_analyzer;
                break;
            default:
                packet = new class Packet_analyzer;
                break;
            }
            change = true;
        }
    }
    if(change && (packet_data != nullptr) && (packet != nullptr)) {
        packet->select_packet(packet_data, new_len);
        packet->set_setting(setting_);
    }
}

void Exchange_packet::clear_packet() {
    creat_packet(1,0);
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
    for(int i = 0; i < len; ++i)
        pack.push_back(packet_data[i]);
    return pack;
}
void Exchange_packet::set_packet(std::vector<uint8_t> pack) {
    creat_packet(pack.size(), 0);
    for (int i = 0; i < pack.size(); ++i)
        packet_data[i] = pack[i];
    creat_packet(pack.size(), packet_data[6]);
}

class Exchange_packet& Exchange_packet::operator=(const class Exchange_packet& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Копирование значений
    set_setting(right.setting_);
    creat_packet(right.len, right.type_packet);
    for (int i = 0; i < len; ++i)
        packet_data[i] = right.packet_data[i];
    return *this;
}
class Exchange_packet& Exchange_packet::operator=(class Exchange_packet&& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Перемещение значений
    setting_ = right.setting_;
    packet = right.packet;
    packet_data = right.packet_data;
    len = right.len;
    type_packet = right.type_packet;
    // Удаление старых значений
    right.setting_ = 0;
    right.packet = nullptr;
    right.packet_data;
    right.len = 0;
    right.type_packet = 0xFF;
    return *this;
}
