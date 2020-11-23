#include "Packet_analyzer.h"
#include "Packet_field.h"

LoRa_address::LoRa_address(const uint16_t group, const uint16_t branch):group(group),branch(branch){}

LoRa_address::LoRa_address(const uint32_t adr):group((uint16_t)(adr >> 16)),branch((uint16_t)(adr)){}

bool LoRa_address::global() {
    return ((group == LORA_GLOBAL_ADR_GROUP) && (branch == LORA_GLOBAL_ADR_BRANCH));
}
bool operator==(const LoRa_address& left, const LoRa_address& right) {
    return ((left.group == right.group) && (left.branch == right.branch));
}
bool operator!=(const LoRa_address& left, const LoRa_address& right) {
    return ((left.group != right.group) || (left.branch != right.branch));
}
bool operator==(const LoRa_address& left, const uint32_t& right) {
    return ((left.group == ((right >> 16) & 0xFFFF)) && (left.branch == (right & 0xFFFF)));
}
bool operator!=(const LoRa_address& left, const uint32_t& right) {
    return ((left.group != ((right >> 16) & 0xFFFF)) || (left.branch != (right & 0xFFFF)));
}
bool operator==(const uint32_t& left, const LoRa_address& right) {
    return ((((left >> 16) & 0xFFFF) == right.group) && ((left & 0xFFFF) == right.branch));
}
bool operator!=(const uint32_t& left, const LoRa_address& right) {
    return ((((left >> 16) & 0xFFFF) != right.group) || ((left & 0xFFFF) != right.branch));
}

//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ----- ----- ----- ----- ----- Packet_analyzer ----- ----- ----- ----- ----- 
//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----   

bool Packet_analyzer::set_field(Address_field **field, uint16_t count_field) {
    if ((field == nullptr) || (count_field == 0))
        return true;
    _field = field;
    _count_field = count_field;
    _max_address = 0;
    _setting = 0;
    for (int i = 0; i < count_field; ++i) {
        if (_max_address < _field[i]->get_max_address())
            _max_address = _field[i]->get_max_address();
    }
    return false;
}
Packet_analyzer::Packet_analyzer() {
    set_field(field_packet_connection, count_field_packet_connection);
}
// Packet_analyzer::Packet_analyzer(Address_field *field, uint16_t count_field) {
//     _field = field;
//     _count_field = count_field;
// }
bool Packet_analyzer::select_packet(uint8_t* packet, uint8_t lenght) {
    if ((lenght < _max_address) || (packet == nullptr))
        return true;
    _creat_packet = false;
    _packet = packet;
    _lenght = lenght;
    _receiv_lenght = _max_address + 1;
    _send_lenght = _max_address + 1;
    return false;
}
uint8_t Packet_analyzer::get_receiv_lenght() {
    return _receiv_lenght;
}
uint8_t Packet_analyzer::get_send_lenght() {
    return _send_lenght;
}

bool Packet_analyzer::set_setting(uint8_t setting) {
    return true; // настройки индивидуальны
}

// Адрес группы адресанта
uint16_t Packet_analyzer::get_dest_adr_group() {
    return _field[num_dest_adr_group]->get_value(_packet, _lenght);
}  
// Адрес ветви адресанта
uint16_t Packet_analyzer::get_dest_adr_branch() {
    return _field[num_dest_adr_branch]->get_value(_packet, _lenght);
}
// Адрес адресанта
LoRa_address Packet_analyzer::get_dest_adr() {
    return {get_dest_adr_group(), get_dest_adr_branch()};
}
// Адрес группы отправителя
uint16_t Packet_analyzer::get_sour_adr_group() {
    return _field[num_sour_adr_group]->get_value(_packet, _lenght);
}
// Адрес ветви отправителя
uint16_t Packet_analyzer::get_sour_adr_branch() {
    return _field[num_sour_adr_branch]->get_value(_packet, _lenght);
}
// Адрес отправителя
LoRa_address Packet_analyzer::get_sour_adr() {
    return {get_sour_adr_group(), get_sour_adr_branch()};
}
// Тип пакета
uint8_t  Packet_analyzer::get_packet_type() {
    return _field[num_type]->get_value(_packet, _lenght);
}
// Номер пакета
uint16_t Packet_analyzer::get_packet_number() {
    return _field[num_number]->get_value(_packet, _lenght);
}          
// Адрес группы адресанта
bool Packet_analyzer::set_dest_adr_group(uint16_t adr) {
    return _field[num_dest_adr_group]->set_value(adr, _packet, _lenght);
}  
// Адрес ветви  адресанта
bool Packet_analyzer::set_dest_adr_branch(uint16_t adr) {
    return _field[num_dest_adr_branch]->set_value(adr, _packet, _lenght);
} 
// Адрес адресанта
bool Packet_analyzer::set_dest_adr(LoRa_address adr) {
    uint16_t buffer = get_dest_adr_group();
    if(set_dest_adr_group(adr.group))
        return true;
    if(set_dest_adr_branch(adr.branch)) {
        set_dest_adr_group(buffer);
        return true;
    }
    return false;
}
// Адрес группы отправителя
bool Packet_analyzer::set_sour_adr_group(uint16_t adr) {
    return _field[num_sour_adr_group]->set_value(adr, _packet, _lenght);
}  
// Адрес ветви  отправителя
bool Packet_analyzer::set_sour_adr_branch(uint16_t adr) {
    return _field[num_sour_adr_branch]->set_value(adr, _packet, _lenght);
} 
// Адрес отправителя
bool Packet_analyzer::set_sour_adr(LoRa_address adr) {
    uint16_t buffer = get_sour_adr_group();
    if(set_sour_adr_group(adr.group))
        return true;
    if(set_sour_adr_branch(adr.branch)) {
        set_sour_adr_group(buffer);
        return true;
    }
    return false;
}
// Тип пакета
bool Packet_analyzer::set_packet_type(uint8_t pac_type) {
    return _field[num_type]->set_value(pac_type, _packet, _lenght);
} 
// Номер пакета
bool Packet_analyzer::set_packet_number(uint16_t num) {
    return _field[num_number]->set_value(num, _packet, _lenght);
}          



//   ----- ----- ----- ----- -----  ----- -----  ----- ----- ----- ----- -----
// ----- ----- ----- ----- ----- Packet_Connection ----- ----- ----- ----- -----
//   ----- ----- ----- ----- -----  ----- -----  ----- ----- ----- ----- -----

bool Packet_Connection::set_setting(uint8_t setting) {
    return false; // не зависит от настроек
}



uint8_t Packet_Connection::set_packet_data(uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    
    bool error = false;
    _receiv_lenght = MINIMAL_PACKET_SIZE;


    // комманда
    if (!(*com < CONNECT_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    _command = *com;
    error = field_byte.set_value(_command, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
    ++_receiv_lenght;
    if (error)
        return 3;

    // данные
    int _len;
    if (connect_com_data[_command] != 0xFF) {
        _len = connect_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x01:
            if ((*len != 1) && (*len != 2))
                return 4;
            _len = *len;
            break;

        case 0x06:
            if ((*len % 2) != 0)
                return 5;
            _len = *len / 2; // делить на 2 или не делить (число номеров по 8 или по 16)
            error = field_byte.set_value(_len, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
            if (error)
                return 6;
            _len *= 2;
            break;
        case 0x09:
            _len = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0A:
            _len = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0C:
            _len = *len;
            error = field_byte.set_value(_len, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
            if (error)
                return 7;
            break;
        default:
            return 8;
        }
    }
    
    for (int i = 0; i < _len; ++i) {
        error = field_byte.set_value(data[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
        if (error)
            return 9;
    }

    return 0;
}

uint8_t Packet_Connection::get_packet_data(uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    _receiv_lenght = MINIMAL_PACKET_SIZE;

    // комманда
    _command = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
    ++_receiv_lenght;
    if (!(_command < CONNECT_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    *com = _command;


    // данные
    int _len;
    if (connect_com_data[_command] != 0xFF) {
        _len = connect_com_data[_command];
        *len = _len;
    }
    else {
        switch (_command)
        {
        case 0x01:
            if ((*len != 1) && (*len != 2))
                return 3;
            _len = *len;
            break;
        case 0x06:
            _len = (field_byte.get_value((_packet + _receiv_lenght), _lenght)) * 2;
            ++_receiv_lenght;
            *len = _len;
            break;
        case 0x09:
            _len = 0; // (!) ----- ----- ----- ----- -----
            *len = 0;
            break;
        case 0x0A:
            _len = 0; // (!) ----- ----- ----- ----- -----
            *len = 0;
            break;
        case 0x0C:
            _len = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
            *len = _len;
            break;
        default:
            return 4;
        }
    }

    for (int i = 0; i < _len; ++i) {
        data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
    }

    return 0;
}



uint8_t Packet_Connection::get_size_by_data(uint8_t *com, uint8_t *size_data, uint8_t *len) {
    _send_lenght = MINIMAL_PACKET_SIZE;
    *size_data = 0;

    // комманда
    if (!(*com < CONNECT_COMMAND_DATA)) {
        _command = 0xFF;
        return 1;
    }
    _command = *com;
    ++_send_lenght;
    // данные
    int _len;
    if (connect_com_data[_command] != 0xFF) {
        _len = connect_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x01:
            if ((*len != 1) && (*len != 2))
                return 2;
            _len = *len;
            break;

        case 0x06:
            if ((*len % 2) != 0)
                return 3;
            ++_send_lenght;
            _len = *len; // делить на 2 или не делить (число номеров по 8 или по 16)
            // _len = *len * 2; // делить на 2 или не делить (число номеров по 8 или по 16)
            break;
        case 0x09:
            _len = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0A:
            _len = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0C:
            ++_send_lenght;
            _len = *len;
            break;
        default:
            return 4;
        }
    }
    *size_data = _len;
    _send_lenght += _len;
    return 0;
}
uint8_t Packet_Connection::get_size_by_packet(uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;
    _send_lenght = MINIMAL_PACKET_SIZE;
    // комманда
    _command = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
    ++_send_lenght;
    if (!(_command < CONNECT_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    // данные
    if (connect_com_data[_command] != 0xFF) {
        *size_data = connect_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x01:
            if (((_lenght - _send_lenght) == 1) || ((_lenght - _send_lenght) == 2))
                *size_data = _lenght - _send_lenght;
            else
                *size_data = 0xFF;
            return 3;
            break;
        case 0x06:
            *size_data = (field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght))) * 2;
            ++_send_lenght;
            break;
        case 0x09:
            *size_data = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0A:
            *size_data = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0C:
            *size_data = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
            ++_send_lenght;
            break;
        default:
            return 4;
        }
    }
    _send_lenght += *size_data;
    return 0;
}


bool Packet_Connection::set_command(uint8_t com) {
    // (!) ----- поддержка в текущей версии отсутствует -----
    return true;
}
bool Packet_Connection::set_data(uint8_t *data, uint8_t len) {
    // (!) ----- поддержка в текущей версии отсутствует -----
    return true;
}



//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ----- ----- ----- ----- -----  Packet_Sensor  ----- ----- ----- ----- -----
//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----


// uint8_t param; // при setting[1] == 1 в пакет не запишется, но нужен для определения размера data

bool Packet_Sensor::set_setting(uint8_t setting) {
    // if по типам
    _setting = setting;
    return false;
}


uint8_t Packet_Sensor::set_packet_data(uint8_t* amt, uint8_t* param, uint8_t* num, uint32_t* data) {
    if (_packet == nullptr)
        return 1;
    uint8_t count = 1;
    bool error = false;
    _receiv_lenght = MINIMAL_PACKET_SIZE;
    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        field_byte.set_value(*amt, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        count = *amt;
        ++_receiv_lenght;
    }

    for (int i = 0; i < count; ++i) {
        // параметр
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return 2;
        }
        _param = param[i];
        if (field_bit[1].get_value(&_setting) != 1) {
            error = field_byte.set_value(_param, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
        }
        if (error)
            return 3;
        
        // индивидуальный номер
        if (field_bit[2].get_value(&_setting) != 0) {
            error = field_byte.set_value(num[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
        }
        if (error)
            return 4;

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            error = field_PWM.set_value(*(data + i), (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            _receiv_lenght += 2;
            break;
        case 1:
            error = field_byte.set_value(*(data + i), (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            _receiv_lenght += 1;
            break;
        case 2:
            error = field_float.set_value(*(data + i), (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            _receiv_lenght += 4;
            break;
        default:
            return 5;
            break;
        }
        if (error)
            return 6;
    }
    return 0;
}

uint8_t Packet_Sensor::get_packet_data(uint8_t* amt, uint8_t* param, uint8_t* num, uint32_t* data) {
    if (_packet == nullptr)
        return 1;
    uint8_t count = 1;
    _receiv_lenght = MINIMAL_PACKET_SIZE;
    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        count = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        *amt = count;
        ++_receiv_lenght;
    }

    for (int i = 0; i < count; ++i) {
        // параметр
        if (field_bit[1].get_value(&_setting) != 1) {
            _param = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
        }
        else {
            _param = param[i];
        }
        
        if (!(_param < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return 2;
        }
        param[i] = _param;
        
        
        // индивидуальный номер
        if (field_bit[2].get_value(&_setting) != 0) {
            num[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
        }

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            data[i] = field_PWM.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght)); // & 0xFFFF; (?)
            _receiv_lenght += 2;
            break;
        case 1:
            data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght)); // & 0xFF;
            _receiv_lenght += 1;
            break;
        case 2:
            data[i] = field_float.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            _receiv_lenght += 4;
            break;
        default:
            return 3;
        }
    }
    return 0;
}



uint8_t Packet_Sensor::get_size_by_data(uint8_t *amt, uint8_t *param, uint8_t *size_data) {
    _send_lenght = MINIMAL_PACKET_SIZE;
    *size_data = 0;
    uint8_t count = 1;
    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        count = *amt;
        ++_send_lenght;
    }

    for (int i = 0; i < count; ++i) {
        // параметр
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return 2;
        }
        _param = param[i];

        if (field_bit[1].get_value(&_setting) != 1) {
            ++_send_lenght;
        }
        
        // индивидуальный номер
        if (field_bit[2].get_value(&_setting) != 0) {
            ++_send_lenght;
        }

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            *size_data += 2;
            _send_lenght += 2;
            break;
        case 1:
            *size_data += 1;
            _send_lenght += 1;
            break;
        case 2:
            *size_data += 4;
            _send_lenght += 4;
            break;
        default:
            return 5;
            break;
        }
    }
    return 0;
}
uint8_t Packet_Sensor::get_size_by_packet(uint8_t *amt, uint8_t *param, uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;
    *amt = 1;
    *size_data = 0;
    _send_lenght = MINIMAL_PACKET_SIZE;

    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        *amt = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
        ++_send_lenght;
    }

    for (int i = 0; i < *amt; ++i) {
        if (field_bit[1].get_value(&_setting) != 1) {
            _param = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
            ++_send_lenght;
        }
        else {
            _param = param[i];
        }
        if (!(_param < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return 2;
        }
        
        
        // индивидуальный номер
        if (field_bit[2].get_value(&_setting) != 0) {
            ++_send_lenght;
        }

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            *size_data += 2;
            _send_lenght += 2;
            break;
        case 1:
            *size_data += 1;
            _send_lenght += 1;
            break;
        case 2:
            *size_data += 4;
            _send_lenght += 4;
            break;
        default:
            return 3;
        }
    }
    return 0;
}




// setting[0] == 1
bool Packet_Sensor::set_count(uint8_t amt) {
    return true;
    // (!) ----- поддержка в текущей версии отсутствует -----
    if ((_packet == nullptr) || (field_bit[0].get_value(&_setting)))
        return true;
    return false;
}
bool Packet_Sensor::set_parametr(uint8_t *param) {
    if (_packet == nullptr)
        return true;
    uint8_t count = 1;

    if (field_bit[0].get_value(&_setting) == 0)
        _receiv_lenght = MINIMAL_PACKET_SIZE;
    uint8_t bias = _receiv_lenght; // смещение на шапку

    // (!) ----- поддержка в текущей версии отсутствует -----
    // if (field_bit[0].get_value(_setting))
    //     count = ; // поле set_count
    //     bias = 10; // смещение на шапку + байт со значением count
    for (int i = 0; i < count; ++i) {
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return true;
        }
        field_byte.set_value(_param, (_packet + bias), _lenght);
        _param = param[i];
        ++_receiv_lenght;
        // bias += X, где X = 1, 2 или 4 в зависимости от параметра
    }

    return false;
}
// setting[2] == 1
bool Packet_Sensor::set_number(uint8_t num) {
    return true;
    // (!) ----- поддержка в текущей версии отсутствует -----
    if ((_packet == nullptr) || (field_bit[2].get_value(&_setting)))
        return true;
    return false;
}
// len = {2, 1, 4} 2 - PWM, 1 - On|Off, 4 - float
bool Packet_Sensor::set_data(uint32_t *data) {
    if (_packet == nullptr)
        return true;
    uint8_t count = 1;
    uint8_t bias = _receiv_lenght; // смещение на шапку
    // uint8_t data_bias = 0;
    bool error = false;
    // _receiv_lenght = bias;
    // (!) ----- поддержка в текущей версии отсутствует -----
    // if (field_bit[0].get_value(_setting))
    //     count = ; // поле set_count
    //     bias = 10; // смещение на шапку + байт со значением count

    for (int i = 0; i < count; ++i) {
        uint8_t param = _param;
        // if (!(field_bit[1].get_value(&_setting)))
        //     bias += 1;

        switch (sensor_param_data[param]) {
        case 0:
            error = field_PWM.set_value(*(data + i), (_packet + bias), _lenght);
            bias += 2;
            break;
        case 1:
            error = field_byte.set_value(*(data + i), (_packet + bias), _lenght);
            bias += 1;
            break;
        case 2:
            error = field_float.set_value(*(data + i), (_packet + bias), _lenght);
            bias += 4;
            break;
        default:
            error = true;
            break;
        }
        _receiv_lenght = bias;

        if (error)
            return error;
        // (!) ----- поддержка в текущей версии отсутствует -----
        // bias += X для следующих параметров
    }

    return false;
}




//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ----- ----- ----- ----- -----  Packet_Device  ----- ----- ----- ----- -----
//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----


bool Packet_Device::set_setting(uint8_t setting) {
    // if по типам
    _setting = setting;
    return false;
}



uint8_t Packet_Device::set_packet_data(uint8_t *obj, uint8_t *num, uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    
    bool error = false;
    _receiv_lenght = MINIMAL_PACKET_SIZE;


    // объект
    if (!(*obj < DEVICE_OBJECT)) {
        _object = 0xFF;
        return 2;
    }
    _object = *obj;
    if (field_bit[0].get_value(&_setting) != 1) {
        error = field_byte.set_value(_object, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
    }
    if (error)
        return 3;


    // индивидуальный номер
    if (field_bit[1].get_value(&_setting) != 0) {
        error = field_byte.set_value(*num, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
    }
    if (error)
        return 4;

    // комманда
    if (!(*com < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 5;
    }
    _command = *com;
    error = field_byte.set_value(_command, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
    ++_receiv_lenght;
    if (error)
        return 6;

    // данные
    for (int i = 0; i < device_object_data[_object][_command]; ++i) {
        error = field_byte.set_value(data[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;

        if (error)
            return 7;
    }

    return 0;
}

uint8_t Packet_Device::get_packet_data(uint8_t *obj, uint8_t *num, uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    _receiv_lenght = MINIMAL_PACKET_SIZE;

    // объект
    if (field_bit[0].get_value(&_setting) != 1) {
        _object = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
    }
    else {
        _object = *obj;
    }
    if (!(_object < DEVICE_OBJECT)) {
        _object = 0xFF;
        return 2;
    }
    *obj = _object;

    // индивидуальный номер
    if (field_bit[1].get_value(&_setting) != 0) {
        *num = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
    }

    // комманда
    _command = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
    ++_receiv_lenght;
    if (!(_command < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 3;
    }
    *com = _command;

    // данные
    if(len != nullptr)
        *len = 0;
    for (int i = 0; i < device_object_data[_object][_command]; ++i) {
        data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        if(len != nullptr)
            ++*len;
        ++_receiv_lenght;
    }
    
    return 0;
}



    
uint8_t Packet_Device::get_size_by_data(uint8_t *obj, uint8_t *com, uint8_t *size_data) {
    _send_lenght = MINIMAL_PACKET_SIZE;
    *size_data = 0;

    // объект
    if (!(*obj < DEVICE_OBJECT)) {
        _object = 0xFF;
        return 1;
    }
    _object = *obj;
    if (field_bit[0].get_value(&_setting) != 1) {
        ++_send_lenght;
    }
    
    // индивидуальный номер
    if (field_bit[1].get_value(&_setting) != 0) {
        ++_send_lenght;
    }

    // комманда
    if (!(*com < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 2;
    }
    _command = *com;
    ++_send_lenght;

    // данные
    _send_lenght += device_object_data[_object][_command];
    *size_data += device_object_data[_object][_command];
    
    return 0;
}

uint8_t Packet_Device::get_size_by_packet(uint8_t *obj, uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;

    *size_data = 0;
    _send_lenght = MINIMAL_PACKET_SIZE;

    // объект
    if (field_bit[0].get_value(&_setting) != 1) {
        _object = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
        ++_send_lenght;
    }
    else {
        _object = *obj;
    }
    if (!(_object < DEVICE_OBJECT)) {
        _object = 0xFF;
        return 2;
    }

    // индивидуальный номер
    if (field_bit[1].get_value(&_setting) != 0) {
        ++_send_lenght;
    }

    // комманда
    _command = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
    ++_send_lenght;
    if (!(_command < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 3;
    }

    // данные
    *size_data += device_object_data[_object][_command];
    _send_lenght += device_object_data[_object][_command];

    return 0;
}


bool Packet_Device::set_object(uint8_t obj) {
    return true;
}
// setting[1] == 1
bool Packet_Device::set_number(uint8_t num) {
    return true;
}
bool Packet_Device::set_command(uint8_t com) {
    return true;
}
// len учитываетсятолько при комбинации obj = 0x06 и com = 0x05
bool Packet_Device::set_data(uint8_t *data, uint8_t len) {
    return true;
}




//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ----- ----- ----- ----- -----  Packet_System  ----- ----- ----- ----- -----
//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----



bool Packet_System::set_setting(uint8_t setting) {
    // if по типам
    return true; // не реализованы
}



uint8_t Packet_System::set_packet_data(uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    
    bool error = false;
    _receiv_lenght = MINIMAL_PACKET_SIZE;


    // комманда
    if (!(*com < SYSTEM_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    _command = *com;
    error = field_byte.set_value(_command, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
    ++_receiv_lenght;
    if (error)
        return 3;

    // данные
    int _len;
    if (system_com_data[_command] != 0xFF) {
        _len = system_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x00:
            // запись ID до длины поля с данными
            for(int i = 0; i < 4; ++i) {
                error = field_byte.set_value(data[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
                ++_receiv_lenght;
                if (error)
                    return 4 + i;
            }
            data = data + 4;

            _len = *len - 1;
            error = field_byte.set_value(_len, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
            ++_receiv_lenght;
            if (error)
                return 5;
            _len += 1;
            break;

        // case 0x02:
        //     for(int i = 0; i < 2; ++i) {
        //         error = field_byte.set_value(data[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        //         ++_receiv_lenght;
        //         if (error)
        //             return 6 + i;
        //     }
        //     data = data + 2;
            
        //     if ((*len % 2) != 0)
        //         return 7;
        //     _len = *len / 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     error = field_byte.set_value(_len, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        //     ++_receiv_lenght;
        //     if (error)
        //         return 8;
        //     _len *= 2;
        //     break;

        // case 0x09:
        //     for(int i = 0; i < 3; ++i) {
        //         error = field_byte.set_value(data[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        //         ++_receiv_lenght;
        //         if (error)
        //             return 9 + i;
        //     }
        //     data = data + 3;
            
        //     if ((*len % 2) != 0)
        //         return 10;
        //     _len = *len / 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     error = field_byte.set_value(_len, (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        //     ++_receiv_lenght;
        //     if (error)
        //         return 11;
        //     _len *= 2;
        //     break;
        default:
            return 12;
        }
    }

    for (int i = 0; i < _len; ++i) {
        error = field_byte.set_value(data[i], (_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
        if (error)
            return 13;
    }

    return 0;
}

uint8_t Packet_System::get_packet_data(uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    _receiv_lenght = MINIMAL_PACKET_SIZE;

    // комманда
    _command = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
    ++_receiv_lenght;
    if (!(_command < SYSTEM_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    *com = _command;


    // данные
    int _len;
    if (system_com_data[_command] != 0xFF) {
        _len = system_com_data[_command];
        *len = _len;
    }
    else {
        switch (_command)
        {
        case 0x00:
            // получение ID до длины поля с данными
            for (int i = 0; i < 4; ++i) {
                data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
                ++_receiv_lenght;
            }
            data = data + 4;

            _len = (field_byte.get_value((_packet + _receiv_lenght), _lenght)) + 1;
            ++_receiv_lenght;
            *len = _len;
            break;
        // case 0x02:
        //     for (int i = 0; i < 2; ++i) {
        //         data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        //         ++_receiv_lenght;
        //     }
        //     data = data + 2;
        //     _len = (field_byte.get_value((_packet + _receiv_lenght), _lenght)) * 2;
        //     ++_receiv_lenght;
        //     *len = _len;
        //     break;
        // case 0x09:
        //     for (int i = 0; i < 3; ++i) {
        //         data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        //         ++_receiv_lenght;
        //     }
        //     data = data + 3;
        //     _len = (field_byte.get_value((_packet + _receiv_lenght), _lenght)) * 2;
        //     ++_receiv_lenght;
        //     *len = _len;
        //     break;
        default:
            return 4;
        }
    }

    for (int i = 0; i < _len; ++i) {
        data[i] = field_byte.get_value((_packet + _receiv_lenght), (_lenght - _receiv_lenght));
        ++_receiv_lenght;
    }
    return 0;
}


uint8_t Packet_System::get_size_by_data(uint8_t *size_data, uint8_t *com, uint8_t *len) {
    _send_lenght = MINIMAL_PACKET_SIZE;
    *size_data = 0;

    // комманда
    if (!(*com < SYSTEM_COMMAND_DATA)) {
        _command = 0xFF;
        return 1;
    }
    _command = *com;
    ++_send_lenght;
    // данные
    int _len;
    if (system_com_data[_command] != 0xFF) {
        _len = system_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x00:
            _send_lenght += 4; // смещение на ID
            ++_send_lenght;
            _len = *len + 1;
            size_data += 4;
            break;

        // case 0x02:
        //     if ((*len % 2) != 0)
        //         return 3;
        //     _send_lenght += 3;
        //     _len = *len; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     // _len = *len * 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     break;
        // case 0x09:
        //     if ((*len % 2) != 0)
        //         return 4;
        //     _send_lenght += 4;
        //     _len = *len; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     // _len = *len * 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     break;
        default:
            return 5;
        }
    }
    *size_data += _len;
    _send_lenght += _len;
    return 0;
}
uint8_t Packet_System::get_size_by_packet(uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;
    _send_lenght = MINIMAL_PACKET_SIZE;
    // комманда
    _command = field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght));
    ++_send_lenght;
    if (!(_command < SYSTEM_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    // данные
    if (system_com_data[_command] != 0xFF) {
        *size_data = system_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x00:
            _send_lenght += 4; // смещение на ID
            *size_data = 4; // смещение на ID
            *size_data += (field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght))) + 1;
            ++_send_lenght;
            break;
        // case 0x02:
        //     _send_lenght += 2;
        //     *size_data = (field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght))) * 2;
        //     ++_send_lenght;
        //     break;
        // case 0x09:
        //     _send_lenght += 3;
        //     *size_data = (field_byte.get_value((_packet + _send_lenght), (_lenght - _send_lenght))) * 2;
        //     ++_send_lenght;
        //     break;
        default:
            return 4;
        }
    }
    _send_lenght += *size_data;
    return 0;
}



bool Packet_System::set_command(uint8_t com) {
    return true;
}
bool Packet_System::set_data(uint8_t *data, uint8_t len) {
    return true;
}



//   ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ----- ----- ----- ----- ----- Err ----- ----- ----- ----- -----
//   ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
