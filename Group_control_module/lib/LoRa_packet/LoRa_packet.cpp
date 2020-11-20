#include "LoRa_packet.h"

// LoRa_packet::LoRa_packet(): {}

LoRa_packet::LoRa_packet(uint8_t* data, uint8_t len, bool crc_error, uint8_t rssi, float snr): _data(nullptr) {
    set_packet(data, len, crc_error, rssi, snr);
}

LoRa_packet::~LoRa_packet() {
    if(_data != nullptr) {
        delete[] _data;
        _data = nullptr;
        _len = 0;
    }
}

void LoRa_packet::set_packet(uint8_t* data, uint8_t len, bool crc_error, uint8_t rssi, float snr) {
    if(_data != nullptr) {
        delete[] _data;
        _data = nullptr;
        _len = 0;
    }
    if((data != nullptr) && (len != 0)){
        _len = len;
        _data = new uint8_t[_len];
        for(int i = 0; i < len; ++i)
            _data[i] = data[i];
    }
    _crc_error = crc_error;
    _rssi = rssi;
    // _snr = snr;
}
std::vector<uint8_t> LoRa_packet::get_data() {
    std::vector<uint8_t> data;
    for(int i = 0; i < _len; ++i)
        data.push_back(_data[i]);
    return data;
}
uint8_t LoRa_packet::get_data(int num) {
    return _data[num];
}
uint8_t LoRa_packet::get_len() {
    return _len;
}
bool LoRa_packet::get_crc_error() {
    return _crc_error;
}
uint8_t LoRa_packet::get_rssi() {
    return _rssi;
}
float LoRa_packet::get_snr() {
    // return _snr;
    return 0;
}
uint8_t& LoRa_packet::operator[] (const int index) {
    return _data[index];
}
class LoRa_packet& LoRa_packet::operator=(const class LoRa_packet& right) {
    // Проверка на самоприсваивание
    if (this == &right)
        return *this;
    // Перенос значений
    _len = right._len;
    _crc_error = right._crc_error;
    _rssi = right._rssi;
    // _snr = right._snr;
    if(_len != 0)
        _data = new uint8_t[_len];
    for(int i = 0; i < _len; ++i) 
        _data[i] = right._data[i];
    return *this;
}