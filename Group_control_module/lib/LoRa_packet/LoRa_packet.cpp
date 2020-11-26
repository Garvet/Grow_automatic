#include "LoRa_packet.h"


// ----- LoRa_packet_data -----


LoRa_packet_data lora_packet_data[SIZE_LORA_PACKET_BUFFER];
// std::array<LoRa_packet_data, SIZE_LORA_PACKET_BUFFER> lora_packet_data;

bool LoRa_packet_data::set_data(uint8_t* set_data, uint8_t set_len) {
    if(set_len > SIZE_LORA_PACKET_MAX_LEN)
        return true;
    if((data != nullptr) && (set_len != 0)){
        len = set_len;
        for(int i = 0; i < set_len; ++i)
            data[i] = set_data[i];
    }
    else
        len = 0;
}
void LoRa_packet_data::set_data(const class LoRa_packet& lora_packet) {
    *this = lora_packet;
}
void LoRa_packet_data::set_data(const class LoRa_packet_data& lora_packet) {
    *this = lora_packet;
}
void LoRa_packet_data::set_data(class LoRa_packet_data&& lora_packet) {
    *this = std::move(lora_packet);
}
class LoRa_packet_data& LoRa_packet_data::operator=(const class LoRa_packet& right) {
    if((right.packet_data->data != nullptr) && (right.packet_data->len != 0)){
        len = right.packet_data->len;
        for(int i = 0; i < right.packet_data->len; ++i)
            data[i] = right.packet_data->data[i];
    }
    else
        len = 0;
    return *this;
}
class LoRa_packet_data& LoRa_packet_data::operator=(const class LoRa_packet_data& right) {
    if((right.data != nullptr) && (right.len != 0)){
        len = right.len;
        for(int i = 0; i < right.len; ++i)
            data[i] = right.data[i];
    }
    else
        len = 0;
    return *this;
}


// ----- LoRa_packet -----
bool LoRa_packet::search_data() {
    // packet_data = std::find_if(lora_packet_data.begin(), lora_packet_data.end(), 
    //                     [](LoRa_packet_data &data){return data.free_object_;} );
    // if(packet_data == lora_packet_data.end()) {
    //     packet_data = nullptr;
    //     return true;
    // }
    // packet_data->free_object_ = false;
    // packet_data->len = 0;
    // return false;



    // packet_data = nullptr;
    // for(int i = 0; i < SIZE_LORA_PACKET_BUFFER; ++i) {
    //     if(lora_packet_data[i].free_object_) {
    //         packet_data = &lora_packet_data[i];
    //         break;
    //     }
    // }
    // if(packet_data == nullptr)
    //     return true;
    // packet_data->free_object_ = false;
    // packet_data->len = 0;
    // return false;



    for(int i = 0; i < SIZE_LORA_PACKET_BUFFER; ++i) {
        if(lora_packet_data[i].free_object_) {
            lora_packet_data[i].free_object_ = false;
            lora_packet_data[i].len = 0;
            packet_data = &lora_packet_data[i];
            return false;
        }
    }
    packet_data = nullptr;
    return true;
}


LoRa_packet::LoRa_packet() {
    search_data();
}
LoRa_packet::LoRa_packet(uint8_t* data, uint8_t len, bool crc_error, uint8_t rssi, float snr) {
    search_data();
    set_packet(data, len, crc_error, rssi, snr);
}
LoRa_packet::LoRa_packet(const LoRa_packet& right) {
    *this = right;
}
LoRa_packet::LoRa_packet(LoRa_packet&& right) {
    *this = std::move(right);
}

LoRa_packet::~LoRa_packet() {
    if(packet_data != nullptr)
        packet_data->free_object_ = true;
}

bool LoRa_packet::set_packet(uint8_t* data, uint8_t len, bool crc_error, uint8_t rssi, float snr) {

    crc_error_ = crc_error;
    rssi_ = rssi;
    return false;
}
std::vector<uint8_t> LoRa_packet::get_data() {
    std::vector<uint8_t> data;
    for(int i = 0; i < packet_data->len; ++i)
        data.push_back(packet_data->data[i]);
    return data;
}
uint8_t LoRa_packet::get_data(int num) {
    return packet_data->data[num];
}
uint8_t LoRa_packet::get_len() {
    return packet_data->len;
}
bool LoRa_packet::get_crc_error() {
    return crc_error_;
}
uint8_t LoRa_packet::get_rssi() {
    return rssi_;
}
uint8_t& LoRa_packet::operator[] (const int index) {
    return packet_data->data[index];
}
class LoRa_packet& LoRa_packet::operator=(const class LoRa_packet& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Перенос значений
    packet_data->len = right.packet_data->len;
    crc_error_ = right.crc_error_;
    rssi_ = right.rssi_;
    for(int i = 0; i < packet_data->len; ++i) 
        packet_data->data[i] = right.packet_data->data[i];
    return *this;
}

class LoRa_packet& LoRa_packet::operator=(class LoRa_packet&& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Перенос значений
    packet_data->free_object_=true;
    packet_data = right.packet_data;
    crc_error_ = right.crc_error_;
    rssi_ = right.rssi_;
    right.packet_data = nullptr;
    // search_data();
    return *this;
}