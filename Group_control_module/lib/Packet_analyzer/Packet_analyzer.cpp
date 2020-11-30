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

Packet_analyzer::Packet_analyzer() {
    field_packet_ = nullptr;
    amt_field_packet_ = 0;
    field_header_ = field_packet_header;
    amt_field_header_ = count_field_packet_header;
}

// Адрес группы адресанта
uint16_t Packet_analyzer::get_dest_adr_group(LoRa_packet& packet) {
    return field_header_[num_dest_adr_group]->get_value(packet);
    // return field_header_[num_dest_adr_group]->get_value(&(packet[0]), packet.get_len());
}
// Адрес ветви  адресанта
uint16_t Packet_analyzer::get_dest_adr_branch(LoRa_packet& packet) {
    return field_header_[num_dest_adr_branch]->get_value(packet);
    // return field_header_[num_dest_adr_branch]->get_value(&(packet[0]), packet.get_len());
}
// Адрес адресанта
LoRa_address Packet_analyzer::get_dest_adr(LoRa_packet& packet) {
    return {get_dest_adr_group(packet), get_dest_adr_branch(packet)};
}
// Адрес группы отправителя
uint16_t Packet_analyzer::get_sour_adr_group(LoRa_packet& packet) {
    return field_header_[num_sour_adr_group]->get_value(packet);
    // return field_header_[num_sour_adr_group]->get_value(&(packet[0]), packet.get_len());
}
// Адрес ветви  отправителя
uint16_t Packet_analyzer::get_sour_adr_branch(LoRa_packet& packet) {
    return field_header_[num_sour_adr_branch]->get_value(packet);
    // return field_header_[num_sour_adr_branch]->get_value(&(packet[0]), packet.get_len());
}
// Адрес отправителя
LoRa_address Packet_analyzer::get_sour_adr(LoRa_packet& packet) {
    return {get_sour_adr_group(packet), get_sour_adr_branch(packet)};    
}
// Тип пакета
uint8_t  Packet_analyzer::get_packet_type(LoRa_packet& packet) {
    return field_header_[num_type]->get_value(packet);
    // return field_header_[num_type]->get_value(&(packet[0]), packet.get_len());
}
// Номер пакета
uint16_t Packet_analyzer::get_packet_number(LoRa_packet& packet) {
    return field_header_[num_number]->get_value(packet);
    // return field_header_[num_number]->get_value(&(packet[0]), packet.get_len());
}

// Адрес группы адресанта
bool Packet_analyzer::set_dest_adr_group(LoRa_packet& packet, uint16_t adr) {
    return field_header_[num_dest_adr_group]->set_value(adr, packet);
    // return field_header_[num_dest_adr_group]->set_value(adr, &(packet[0]));
}
// Адрес ветви  адресанта
bool Packet_analyzer::set_dest_adr_branch(LoRa_packet& packet, uint16_t adr) {
    return field_header_[num_dest_adr_branch]->set_value(adr, packet);
    // return field_header_[num_dest_adr_branch]->set_value(adr, &(packet[0]));
}
// Адрес адресанта
bool Packet_analyzer::set_dest_adr(LoRa_packet& packet, LoRa_address adr) {
    uint16_t buffer = get_dest_adr_group(packet);
    if(set_dest_adr_group(packet, adr.group))
        return true;
    if(set_dest_adr_branch(packet, adr.branch)) {
        set_dest_adr_group(packet, buffer);
        return true;
    }
    return false;
}
// Адрес группы отправителя
bool Packet_analyzer::set_sour_adr_group(LoRa_packet& packet, uint16_t adr) {
    return field_header_[num_sour_adr_group]->set_value(adr, packet);
    // return field_header_[num_sour_adr_group]->set_value(adr, &(packet[0]));
}
// Адрес ветви  отправителя
bool Packet_analyzer::set_sour_adr_branch(LoRa_packet& packet, uint16_t adr) {
    return field_header_[num_sour_adr_branch]->set_value(adr, packet);
    // return field_header_[num_sour_adr_branch]->set_value(adr, &(packet[0]));
}
// Адрес отправителя
bool Packet_analyzer::set_sour_adr(LoRa_packet& packet, LoRa_address adr) {
    uint16_t buffer = get_sour_adr_group(packet);
    if(set_sour_adr_group(packet, adr.group))
        return true;
    if(set_sour_adr_branch(packet, adr.branch)) {
        set_sour_adr_group(packet, buffer);
        return true;
    }
    return false;
}
// Тип пакета
bool Packet_analyzer::set_packet_type(LoRa_packet& packet, uint8_t pac_type) {
    return field_header_[num_type]->set_value(pac_type, packet);
    // return field_header_[num_type]->set_value(pac_type, &(packet[0]));
}
// Номер пакета
bool Packet_analyzer::set_packet_number(LoRa_packet& packet, uint16_t num) {
    return field_header_[num_number]->set_value(num, packet);
    // return field_header_[num_number]->set_value(num, &(packet[0]));
}

bool Packet_analyzer::set_setting(uint8_t setting) {
    return false; // настройки индивидуальны, у общего класса их нет
}
uint8_t Packet_analyzer::get_setting() {
    return setting_;
}



//   ----- ----- ----- ----- -----  ----- -----  ----- ----- ----- ----- -----
// ----- ----- ----- ----- ----- Packet_Connection ----- ----- ----- ----- -----
//   ----- ----- ----- ----- -----  ----- -----  ----- ----- ----- ----- -----

bool Packet_Connection::set_setting(uint8_t setting) {
    return false; // не зависит от настроек
}

/// --- Запись в пакет ---

// Установить команду
uint8_t Packet_Connection::set_command(LoRa_packet& packet, uint8_t com) {
    // Проверка корректнгости команды
    if (!(com < CONNECT_COMMAND_DATA))
        return 1;
    command_ = com;
    // Заполение поля
    bool error = false;
    last_filled_byte = PACKET_HEADER_SIZE; // Команда всегда находится сразу после шапки
    error = field_byte.set_value(command_, packet, last_filled_byte);
    // error = field_byte.set_value(command_, &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
    if (error)
        return 2;
    ++last_filled_byte;
    return 0;
}
// Установить данные
uint8_t Packet_Connection::set_data(LoRa_packet& packet, uint8_t *data, uint8_t len) {
    uint8_t len_data;
    uint8_t error = 0;

    error = get_size_by_data(&command_, &len, len_data);
    if(error)
        return error;


    // Получение длины в соответствии с командой
    if (connect_com_data[command_] == 0xFF) {
        // Расчётное соответствие команды длинне
        switch (command_) {
        case 0x01:
        case 0x09:
        case 0x0A: {
            break;
        }
        case 0x06:
        case 0x0C: {
            error = field_byte.set_value(len, packet, last_filled_byte);
            // error = field_byte.set_value((len), &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
            ++last_filled_byte;
            if (error)
                return 4;
            break;
        }
        default:
            return 6;
        }
    }
    // Заполение данных
    for (int i = 0; i < len_data; ++i) {
        error = field_byte.set_value(data[i], packet, last_filled_byte);
        // error = field_byte.set_value(data[i], &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
        ++last_filled_byte;
        if (error)
            return 8;
    }
    // (!) ----- packet.len = last_filled_byte
    return 0;
}
// Занести в пакет параметры и данные
uint8_t Packet_Connection::set_packet_data(LoRa_packet& packet, uint8_t *com, uint8_t *data, uint8_t *len) {
    uint8_t error;
    error = set_command(packet, *com);
    if(error)
        return error;
    error = set_data(packet, data, *len);
    return error;
}

/// --- Чтение из пакета ---

// Получить команду
uint8_t Packet_Connection::get_command(LoRa_packet& packet, uint8_t *com) {
    last_read_byte = PACKET_HEADER_SIZE; // Команда всегда находится сразу после шапки
    // Проверка некорректного аргумента
    if(com == nullptr)
        return 1;
    // Чтение комманды
    command_ = field_byte.get_value(packet, last_read_byte);
    // command_ = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
    ++last_read_byte;
    // Проверка команды
    if (!(command_ < CONNECT_COMMAND_DATA)) {
        command_ = 0xFF;
        return 2;
    }
    *com = command_;
    return 0;
}
// Получить данные
uint8_t Packet_Connection::get_data(LoRa_packet& packet, uint8_t *data, uint8_t *len) {
    uint8_t len_data;
    uint8_t error = 0;
    // Проверка некорректного аргумента
    if(len == nullptr)
        return 1;
    // Получение длины в соответствии с командой
    error = get_size_by_data(&command_, nullptr, len_data);
    if(error == 2) {
        *len = field_byte.get_value(packet, last_read_byte);
        ++last_read_byte;
        // *len = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
        error = get_size_by_data(&command_, len, len_data);
        if (command_ == 0x06) {
            *len = len_data/2;
        }
        else {
            *len = len_data;
        }
        
    }
    else if(error == 3) {
        if (((packet.get_len() - last_read_byte) == 1) || ((packet.get_len() - last_read_byte) == 2)) {
            len_data = packet.get_len() - last_read_byte;
            *len = len_data;
            error = 0;
        }
    }
    else
        *len = len_data;
    if(error)
        return error;


    // if (connect_com_data[command_] != 0xFF) {
    //     len_data = connect_com_data[command_];
    //     *len = len_data;
    // }
    // else {
    //     switch (command_)
    //     {
    //     case 0x01:
    //         if ((*len != 1) && (*len != 2))
    //             return 4;
    //         len_data = *len;
    //         break;
    //     case 0x06:
    //         *len = field_byte.get_value(packet, last_read_byte);
    //         // *len = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
    //         ++last_read_byte;
    //         len_data = *len * 2;
    //         break;
    //     case 0x09:
    //         len_data = *len = 0; // (-) ----- (!) -----
    //         break;
    //     case 0x0A:
    //         len_data = *len = 0; // (-) ----- (!) -----
    //         break;
    //     case 0x0C:
    //         len_data = field_byte.get_value(packet, last_read_byte);
    //         // len_data = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
    //         ++last_read_byte;
    //         *len = len_data;
    //         break;
    //     default:
    //         return 5;
    //     }
    // }



    if(len_data > 0) {
        // Проверка некорректного аргумента
        if(data == nullptr)
            return 6;
        // Получение данных
        for (int i = 0; i < len_data; ++i) {
            data[i] = field_byte.get_value(packet, last_read_byte);
            // data[i] = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
            ++last_read_byte;
        }
    }
    return 0;
}
// Получить из пакета параметры и данные
uint8_t Packet_Connection::get_packet_data(LoRa_packet& packet, uint8_t *com, uint8_t *data, uint8_t *len) {
    uint8_t error;
    error = get_command(packet, com);
    if(error)
        return error;
    error = get_data(packet, data, len);
    return error;
}

/// --- Расчёты ---

// Узнать объём поля данных по параметрам
uint8_t Packet_Connection::get_size_by_data(uint8_t *com, uint8_t *len, uint8_t &size_data) {
    size_data = 0;
    // Комманда, проверка на выход за границы 
    if (!(*com < CONNECT_COMMAND_DATA)) {
        return 1;
    }
    // Получение объёма
    if (connect_com_data[*com] != 0xFF) {
        size_data = connect_com_data[*com];
    }
    else {
        // Проверка некорректного аргумента
        if(len == nullptr)
            return 2;
        switch (*com)
        {
        case 0x01:
            if ((*len != 1) && (*len != 2))
                return 3;
            size_data = *len;
            break;
        case 0x06:
            size_data = *len * 2;
            break;
        case 0x09:
            size_data = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0A:
            size_data = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0C:
            size_data = *len;
            break;
        default:
            return 4;
        }
    }
    return 0;
}
// Узнать объём поля данных по содержимому пакета
uint8_t Packet_Connection::get_size_by_packet(LoRa_packet& packet, uint8_t &size_data) {
    uint8_t error;
    error = get_command(packet, &command_);
    if(error)
        return error;
    error = get_size_by_data(&command_, nullptr, size_data);
    if(error == 2) {
        uint8_t len = field_byte.get_value(packet, last_read_byte);
        // uint8_t len = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
        error = get_size_by_data(&command_, &len, size_data);
        last_read_byte = PACKET_HEADER_SIZE + 1;
    }
    if(error == 3) {
        if (((packet.get_len() - last_read_byte) == 1) || ((packet.get_len() - last_read_byte) == 2)) {
            size_data = packet.get_len() - last_read_byte;
            error = 0;
        }
    }
    return error;
}



//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// ----- ----- ----- ----- -----  Packet_Sensor  ----- ----- ----- ----- -----
//   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----

bool Packet_Sensor::set_setting(uint8_t setting) {
    // if по типам
    setting_ = setting;
    return false;
}

/// --- Запись в пакет ---
// занести в пакет параметры и данные
uint8_t Packet_Sensor::set_packet_data(LoRa_packet& packet, uint8_t *amt, uint8_t *param, uint8_t *num, uint32_t *data) {
    uint8_t error = 0;
    uint8_t count = 1;
    last_filled_byte = PACKET_HEADER_SIZE;
    // проверка наличия поля количества
    if (field_bit[0].get_value(&setting_) != 0) {
        error = field_byte.set_value(*amt, packet, last_filled_byte);
        // error = field_byte.set_value(*amt, &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
        if(error)
            return 1;
        count = *amt;
        ++last_filled_byte;
    }
    for (int i = 0; i < count; ++i) {
        // параметр
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            return 2;
        }
        if (field_bit[1].get_value(&setting_) != 1) {
            error = field_byte.set_value(param[i], packet, last_filled_byte);
            // error = field_byte.set_value(param[i], &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
            if (error)
                return 2;
            ++last_filled_byte;
        }
        // индивидуальный номер
        if (field_bit[2].get_value(&setting_) != 0) {
            error = field_byte.set_value(num[i], packet, last_filled_byte);
            // error = field_byte.set_value(num[i], &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
            if (error)
                return 3;
            ++last_filled_byte;
        }
        // данные
        switch (sensor_param_data[param[i]]) {
        case 1:
            error = field_byte.set_value(*(data + i), packet, last_filled_byte);
            // error = field_byte.set_value(*(data + i), &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
            break;
        case 2:
            error = field_PWM.set_value(*(data + i), packet, last_filled_byte);
            // error = field_PWM.set_value(*(data + i), &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
            break;
        case 4:
            error = field_float.set_value(*(data + i), packet, last_filled_byte);
            // error = field_float.set_value(*(data + i), &(packet[last_filled_byte]), (SIZE_LORA_PACKET_MAX_LEN - last_filled_byte));
            break;
        default:
            return 4;
            break;
        }
        if (error)
            return 5;
        last_filled_byte += sensor_param_data[param[i]];
    }
    return 0;
}

/// --- Чтение из пакета ---
// получить из пакета параметры и данные
uint8_t Packet_Sensor::get_packet_data(LoRa_packet& packet, uint8_t *amt, uint8_t *param, uint8_t *num, uint32_t *data) {
    uint8_t count = 1;
    last_read_byte = PACKET_HEADER_SIZE;
    // количество параметров
    if(!get_count(packet, count))
        *amt = count;
    // получение параметров
    for (int i = 0; i < count; ++i) {
        // параметр
        if (field_bit[1].get_value(&setting_) != 1) {
            param[i] = field_byte.get_value(packet, last_read_byte);
            // param[i] = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
            ++last_read_byte;
        }
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            return 2;
        }
        // индивидуальный номер
        if (field_bit[2].get_value(&setting_) != 0) {
            num[i] = field_byte.get_value(packet, last_read_byte);
            // num[i] = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
            ++last_read_byte;
        }
        // данные
        switch (sensor_param_data[param[i]]) {
        case 1:
            data[i] = field_byte.get_value(packet, last_read_byte); // & 0xFF;
            // data[i] = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte)); // & 0xFF;
            break;
        case 2:
            data[i] = field_PWM.get_value(packet, last_read_byte); // & 0xFFFF; (?)
            // data[i] = field_PWM.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte)); // & 0xFFFF; (?)
            break;
        case 4:
            data[i] = field_float.get_value(packet, last_read_byte);
            // data[i] = field_float.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
            break;
        default:
            return 3;
        }
        last_read_byte += sensor_param_data[param[i]];
    }
    return 0;
}
bool Packet_Sensor::get_count(LoRa_packet& packet, uint8_t &amt) {
    amt = 1;
    // проверка наличия поля
    if (field_bit[0].get_value(&setting_) == 0) 
        return true;
    // Количество всегда находится сразу после шапки
    last_filled_byte = PACKET_HEADER_SIZE;
    amt = field_byte.get_value(packet, last_read_byte);
    // amt = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
    ++last_read_byte;
    return false;
}
// bool Packet_Sensor::get_parametr(LoRa_packet& packet, uint8_t &param, uint8_t *num) {
//     // проверка наличия поля параметра
//     if (field_bit[1].get_value(&setting_) != 1) {
//         param = 0xFF;
//         return true;
//     }
//     uint8_t processed_param;
//     last_filled_byte = PACKET_HEADER_SIZE;
//     // проверка наличия поля количества
//     if (field_bit[0].get_value(&setting_) != 0)
//         ++last_filled_byte; 
//     for (int i = 0; i <= *num; ++i) {
//         // параметр
//         processed_param = field_byte.get_value(packet, last_read_byte);
//         // processed_param = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
//         if (!(processed_param < SENSOR_PARAM_DATA)) {
//             param = 0xFF;
//             return true;
//         }
//         param = processed_param;
//         // индивидуальный номер
//         if (field_bit[2].get_value(&setting_) != 0) {
//             last_read_byte += 1;
//         }
//         // данные
//         last_read_byte += sensor_param_data[processed_param];
//         // switch (sensor_param_data[processed_param]) {
//         // case 2:
//         //     last_read_byte += 2;
//         //     // _send_length += 2;
//         //     break;
//         // case 1:
//         //     last_read_byte += 1;
//         //     // _send_length += 1;
//         //     break;
//         // case 4:
//         //     last_read_byte += 4;
//         //     // _send_length += 4;
//         //     break;
//         // default:
//         //     return true;
//         //     break;
//         // }
//     }
//     return false;
// }
// bool Packet_Sensor::get_id_component(LoRa_packet& packet, uint8_t &id, uint8_t *num, uint8_t *param) {
//     id = 0;
//     if (field_bit[2].get_value(&setting_) != 0) {
//         return true; // (!) -----
//         bool static_param = (field_bit[1].get_value(&setting_) == 1);
//         last_filled_byte = PACKET_HEADER_SIZE;
//         // проверка наличия поля количества
//         if (field_bit[0].get_value(&setting_) != 0)
//             ++last_filled_byte;
//         if(static_param) { // не бывает static_param, бывает лишь заранее известное положение этих param
//             // (!) -----
//         }
//         else {
//             // (!) -----
//         }
//     }
//     return false;
// }
// bool Packet_Sensor::get_data(LoRa_packet& packet, uint32_t &data, uint8_t *num, uint8_t *param) {
//     last_filled_byte = PACKET_HEADER_SIZE;
//     // проверка наличия поля количества
//     if (field_bit[0].get_value(&setting_) != 0)
//         ++last_filled_byte;
//     bool static_param = (field_bit[1].get_value(&setting_) == 1);
//     if(static_param) { // не бывает static_param, бывает лишь заранее известное положение этих param
//     }
//     else {
//     }
// }

/// --- Расчёты ---
// узнать объём поля данных по параметрам
uint8_t Packet_Sensor::get_size_by_data(uint8_t *amt, uint8_t *param, uint8_t &size_data) {
    size_data = 0;
    uint8_t count = 1;
    // количество параметров
    if (field_bit[0].get_value(&setting_) != 0) {
        count = *amt;
    }
    // Цикл по параметрам
    for (int i = 0; i < count; ++i) {
        // параметр
        if (!(param[i] < SENSOR_PARAM_DATA))
            return 2;
        // данные
        size_data += sensor_param_data[param[i]];
    }
    // поле параметр
    if (field_bit[1].get_value(&setting_) != 1) // есть поле param
        size_data += count;
    // индивидуальный номер
    if (field_bit[2].get_value(&setting_) != 0) // есть поле id
        size_data += count;
    return 0;
}
// узнать объём поля данных по содержимому пакета
uint8_t Packet_Sensor::get_size_by_packet(LoRa_packet& packet, uint8_t *amt, uint8_t *param, uint8_t &size_data) {
    uint8_t count = 1;
    last_read_byte = PACKET_HEADER_SIZE;
    // количество параметров
    if(!get_count(packet, count))
        *amt = count;
    size_data = 0;
    for (int i = 0; i < count; ++i) {
        if (field_bit[1].get_value(&setting_) != 1) {
            param[i] = field_byte.get_value(packet, last_read_byte);
            // param[i] = field_byte.get_value(&(packet[last_read_byte]), (packet.get_len() - last_read_byte));
            ++last_read_byte;
        }
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            return 2;
        }
        // индивидуальный номер
        if (field_bit[2].get_value(&setting_) != 0) {
            ++last_read_byte;
        }
        // данные
        last_read_byte = sensor_param_data[param[i]];
    }
    return 0;
}





// uint8_t Packet_Sensor::get_size_by_data(uint8_t *amt, uint8_t *param, uint8_t *size_data) {
//     _send_length = PACKET_HEADER_SIZE;
//     *size_data = 0;
//     uint8_t count = 1;
//     // количество параметров
//     if (field_bit[0].get_value(&_setting) != 0) {
//         count = *amt;
//         ++_send_length;
//     }
//     for (int i = 0; i < count; ++i) {
//         // параметр
//         if (!(param[i] < SENSOR_PARAM_DATA)) {
//             _param = 0xFF;
//             return 2;
//         }
//         _param = param[i];
//         if (field_bit[1].get_value(&_setting) != 1) {
//             ++_send_length;
//         }
//         // индивидуальный номер
//         if (field_bit[2].get_value(&_setting) != 0) {
//             ++_send_length;
//         }
//         // данные
//         switch (sensor_param_data[_param]) {
//         case 0:
//             *size_data += 2;
//             _send_length += 2;
//             break;
//         case 1:
//             *size_data += 1;
//             _send_length += 1;
//             break;
//         case 2:
//             *size_data += 4;
//             _send_length += 4;
//             break;
//         default:
//             return 5;
//             break;
//         }
//     }
//     return 0;
// }
// uint8_t Packet_Sensor::get_size_by_packet(uint8_t *amt, uint8_t *param, uint8_t *size_data) {
//     if (_packet == nullptr)
//         return 1;
//     *amt = 1;
//     *size_data = 0;
//     _send_length = PACKET_HEADER_SIZE;
//     // // количество параметров
//     // if (field_bit[0].get_value(&_setting) != 0) {
//     //     *amt = field_byte.get_value((_packet + _send_length), (_length - _send_length));
//     //     ++_send_length;
//     // }
//     for (int i = 0; i < *amt; ++i) {
//         if (field_bit[1].get_value(&_setting) != 1) {
//             _param = field_byte.get_value((_packet + _send_length), (_length - _send_length));
//             ++_send_length;
//         }
//         else {
//             _param = param[i];
//         }
//         if (!(_param < SENSOR_PARAM_DATA)) {
//             _param = 0xFF;
//             return 2;
//         }   
//         // индивидуальный номер
//         if (field_bit[2].get_value(&_setting) != 0) {
//             ++_send_length;
//         }
//         // данные
//         switch (sensor_param_data[_param]) {
//         case 0:
//             *size_data += 2;
//             _send_length += 2;
//             break;
//         case 1:
//             *size_data += 1;
//             _send_length += 1;
//             break;
//         case 2:
//             *size_data += 4;
//             _send_length += 4;
//             break;
//         default:
//             return 3;
//         }
//     }
//     return 0;
// }


















































#ifndef __PACKET_ANALYZER_H__OLD

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
bool Packet_analyzer::select_packet(LoRa_packet& packet) {
    _creat_packet = false;
    _packet = &packet[0];
    _length = SIZE_LORA_PACKET_MAX_LEN;
    _receive_length = _max_address + 1;
    _send_length = _max_address + 1;
    return false;
}
bool Packet_analyzer::select_packet(uint8_t* packet, uint8_t length) {
    if ((length < _max_address) || (packet == nullptr))
        return true;
    _creat_packet = false;
    _packet = packet;
    _length = length;
    _receive_length = _max_address + 1;
    _send_length = _max_address + 1;
    return false;
}
uint8_t Packet_analyzer::get_receive_length() {
    return _receive_length;
}
uint8_t Packet_analyzer::get_send_length() {
    return _send_length;
}

bool Packet_analyzer::set_setting(uint8_t setting) {
    return true; // настройки индивидуальны
}

// Адрес группы адресанта
uint16_t Packet_analyzer::get_dest_adr_group() {
    return _field[num_dest_adr_group]->get_value(_packet, _length);
}  
// Адрес ветви адресанта
uint16_t Packet_analyzer::get_dest_adr_branch() {
    return _field[num_dest_adr_branch]->get_value(_packet, _length);
}
// Адрес адресанта
LoRa_address Packet_analyzer::get_dest_adr() {
    return {get_dest_adr_group(), get_dest_adr_branch()};
}
// Адрес группы отправителя
uint16_t Packet_analyzer::get_sour_adr_group() {
    return _field[num_sour_adr_group]->get_value(_packet, _length);
}
// Адрес ветви отправителя
uint16_t Packet_analyzer::get_sour_adr_branch() {
    return _field[num_sour_adr_branch]->get_value(_packet, _length);
}
// Адрес отправителя
LoRa_address Packet_analyzer::get_sour_adr() {
    return {get_sour_adr_group(), get_sour_adr_branch()};
}
// Тип пакета
uint8_t  Packet_analyzer::get_packet_type() {
    return _field[num_type]->get_value(_packet, _length);
}
// Номер пакета
uint16_t Packet_analyzer::get_packet_number() {
    return _field[num_number]->get_value(_packet, _length);
}          
// Адрес группы адресанта
bool Packet_analyzer::set_dest_adr_group(uint16_t adr) {
    return _field[num_dest_adr_group]->set_value(adr, _packet, _length);
}  
// Адрес ветви  адресанта
bool Packet_analyzer::set_dest_adr_branch(uint16_t adr) {
    return _field[num_dest_adr_branch]->set_value(adr, _packet, _length);
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
    return _field[num_sour_adr_group]->set_value(adr, _packet, _length);
}  
// Адрес ветви  отправителя
bool Packet_analyzer::set_sour_adr_branch(uint16_t adr) {
    return _field[num_sour_adr_branch]->set_value(adr, _packet, _length);
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
    return _field[num_type]->set_value(pac_type, _packet, _length);
} 
// Номер пакета
bool Packet_analyzer::set_packet_number(uint16_t num) {
    return _field[num_number]->set_value(num, _packet, _length);
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
    _receive_length = MINIMAL_PACKET_SIZE;


    // комманда
    if (!(*com < CONNECT_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    _command = *com;
    error = field_byte.set_value(_command, (_packet + _receive_length), (_length - _receive_length));
    ++_receive_length;
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
            error = field_byte.set_value(_len, (_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
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
            error = field_byte.set_value(_len, (_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
            if (error)
                return 7;
            break;
        default:
            return 8;
        }
    }
    
    for (int i = 0; i < _len; ++i) {
        error = field_byte.set_value(data[i], (_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
        if (error)
            return 9;
    }

    return 0;
}

uint8_t Packet_Connection::get_packet_data(uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    _receive_length = MINIMAL_PACKET_SIZE;

    // комманда
    _command = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
    ++_receive_length;
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
            _len = (field_byte.get_value((_packet + _receive_length), _length)) * 2;
            ++_receive_length;
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
            _len = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
            *len = _len;
            break;
        default:
            return 4;
        }
    }

    for (int i = 0; i < _len; ++i) {
        data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
    }

    return 0;
}



uint8_t Packet_Connection::get_size_by_data(uint8_t *com, uint8_t *size_data, uint8_t *len) {
    _send_length = MINIMAL_PACKET_SIZE;
    *size_data = 0;

    // комманда
    if (!(*com < CONNECT_COMMAND_DATA)) {
        _command = 0xFF;
        return 1;
    }
    _command = *com;
    ++_send_length;
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
            ++_send_length;
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
            ++_send_length;
            _len = *len;
            break;
        default:
            return 4;
        }
    }
    *size_data = _len;
    _send_length += _len;
    return 0;
}
uint8_t Packet_Connection::get_size_by_packet(uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;
    _send_length = MINIMAL_PACKET_SIZE;
    // комманда
    _command = field_byte.get_value((_packet + _send_length), (_length - _send_length));
    ++_send_length;
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
            if (((_length - _send_length) == 1) || ((_length - _send_length) == 2))
                *size_data = _length - _send_length;
            else
                *size_data = 0xFF;
            return 3;
            break;
        case 0x06:
            *size_data = (field_byte.get_value((_packet + _send_length), (_length - _send_length))) * 2;
            ++_send_length;
            break;
        case 0x09:
            *size_data = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0A:
            *size_data = 0; // (!) ----- ----- ----- ----- -----
            break;
        case 0x0C:
            *size_data = field_byte.get_value((_packet + _send_length), (_length - _send_length));
            ++_send_length;
            break;
        default:
            return 4;
        }
    }
    _send_length += *size_data;
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
    _receive_length = MINIMAL_PACKET_SIZE;
    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        field_byte.set_value(*amt, (_packet + _receive_length), (_length - _receive_length));
        count = *amt;
        ++_receive_length;
    }

    for (int i = 0; i < count; ++i) {
        // параметр
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return 2;
        }
        _param = param[i];
        if (field_bit[1].get_value(&_setting) != 1) {
            error = field_byte.set_value(_param, (_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
        }
        if (error)
            return 3;
        
        // индивидуальный номер
        if (field_bit[2].get_value(&_setting) != 0) {
            error = field_byte.set_value(num[i], (_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
        }
        if (error)
            return 4;

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            error = field_PWM.set_value(*(data + i), (_packet + _receive_length), (_length - _receive_length));
            _receive_length += 2;
            break;
        case 1:
            error = field_byte.set_value(*(data + i), (_packet + _receive_length), (_length - _receive_length));
            _receive_length += 1;
            break;
        case 2:
            error = field_float.set_value(*(data + i), (_packet + _receive_length), (_length - _receive_length));
            _receive_length += 4;
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
    _receive_length = MINIMAL_PACKET_SIZE;
    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        count = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        *amt = count;
        ++_receive_length;
    }

    for (int i = 0; i < count; ++i) {
        // параметр
        if (field_bit[1].get_value(&_setting) != 1) {
            _param = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
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
            num[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
        }

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            data[i] = field_PWM.get_value((_packet + _receive_length), (_length - _receive_length)); // & 0xFFFF; (?)
            _receive_length += 2;
            break;
        case 1:
            data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length)); // & 0xFF;
            _receive_length += 1;
            break;
        case 2:
            data[i] = field_float.get_value((_packet + _receive_length), (_length - _receive_length));
            _receive_length += 4;
            break;
        default:
            return 3;
        }
    }
    return 0;
}



uint8_t Packet_Sensor::get_size_by_data(uint8_t *amt, uint8_t *param, uint8_t *size_data) {
    _send_length = MINIMAL_PACKET_SIZE;
    *size_data = 0;
    uint8_t count = 1;
    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        count = *amt;
        ++_send_length;
    }

    for (int i = 0; i < count; ++i) {
        // параметр
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return 2;
        }
        _param = param[i];

        if (field_bit[1].get_value(&_setting) != 1) {
            ++_send_length;
        }
        
        // индивидуальный номер
        if (field_bit[2].get_value(&_setting) != 0) {
            ++_send_length;
        }

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            *size_data += 2;
            _send_length += 2;
            break;
        case 1:
            *size_data += 1;
            _send_length += 1;
            break;
        case 2:
            *size_data += 4;
            _send_length += 4;
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
    _send_length = MINIMAL_PACKET_SIZE;

    // количество параметров
    if (field_bit[0].get_value(&_setting) != 0) {
        *amt = field_byte.get_value((_packet + _send_length), (_length - _send_length));
        ++_send_length;
    }

    for (int i = 0; i < *amt; ++i) {
        if (field_bit[1].get_value(&_setting) != 1) {
            _param = field_byte.get_value((_packet + _send_length), (_length - _send_length));
            ++_send_length;
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
            ++_send_length;
        }

        // данные
        switch (sensor_param_data[_param]) {
        case 0:
            *size_data += 2;
            _send_length += 2;
            break;
        case 1:
            *size_data += 1;
            _send_length += 1;
            break;
        case 2:
            *size_data += 4;
            _send_length += 4;
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
        _receive_length = MINIMAL_PACKET_SIZE;
    uint8_t bias = _receive_length; // смещение на шапку

    // (!) ----- поддержка в текущей версии отсутствует -----
    // if (field_bit[0].get_value(_setting))
    //     count = ; // поле set_count
    //     bias = 10; // смещение на шапку + байт со значением count
    for (int i = 0; i < count; ++i) {
        if (!(param[i] < SENSOR_PARAM_DATA)) {
            _param = 0xFF;
            return true;
        }
        field_byte.set_value(_param, (_packet + bias), _length);
        _param = param[i];
        ++_receive_length;
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
    uint8_t bias = _receive_length; // смещение на шапку
    // uint8_t data_bias = 0;
    bool error = false;
    // _receive_length = bias;
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
            error = field_PWM.set_value(*(data + i), (_packet + bias), _length);
            bias += 2;
            break;
        case 1:
            error = field_byte.set_value(*(data + i), (_packet + bias), _length);
            bias += 1;
            break;
        case 2:
            error = field_float.set_value(*(data + i), (_packet + bias), _length);
            bias += 4;
            break;
        default:
            error = true;
            break;
        }
        _receive_length = bias;

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
    _receive_length = MINIMAL_PACKET_SIZE;


    // объект
    if (!(*obj < DEVICE_OBJECT)) {
        _object = 0xFF;
        return 2;
    }
    _object = *obj;
    if (field_bit[0].get_value(&_setting) != 1) {
        error = field_byte.set_value(_object, (_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
    }
    if (error)
        return 3;


    // индивидуальный номер
    if (field_bit[1].get_value(&_setting) != 0) {
        error = field_byte.set_value(*num, (_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
    }
    if (error)
        return 4;

    // комманда
    if (!(*com < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 5;
    }
    _command = *com;
    error = field_byte.set_value(_command, (_packet + _receive_length), (_length - _receive_length));
    ++_receive_length;
    if (error)
        return 6;

    // данные
    for (int i = 0; i < device_object_data[_object][_command]; ++i) {
        error = field_byte.set_value(data[i], (_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;

        if (error)
            return 7;
    }

    return 0;
}

uint8_t Packet_Device::get_packet_data(uint8_t *obj, uint8_t *num, uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    _receive_length = MINIMAL_PACKET_SIZE;

    // объект
    if (field_bit[0].get_value(&_setting) != 1) {
        _object = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
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
        *num = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
    }

    // комманда
    _command = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
    ++_receive_length;
    if (!(_command < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 3;
    }
    *com = _command;

    // данные
    if(len != nullptr)
        *len = 0;
    for (int i = 0; i < device_object_data[_object][_command]; ++i) {
        data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        if(len != nullptr)
            ++*len;
        ++_receive_length;
    }
    
    return 0;
}



    
uint8_t Packet_Device::get_size_by_data(uint8_t *obj, uint8_t *com, uint8_t *size_data) {
    _send_length = MINIMAL_PACKET_SIZE;
    *size_data = 0;

    // объект
    if (!(*obj < DEVICE_OBJECT)) {
        _object = 0xFF;
        return 1;
    }
    _object = *obj;
    if (field_bit[0].get_value(&_setting) != 1) {
        ++_send_length;
    }
    
    // индивидуальный номер
    if (field_bit[1].get_value(&_setting) != 0) {
        ++_send_length;
    }

    // комманда
    if (!(*com < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 2;
    }
    _command = *com;
    ++_send_length;

    // данные
    _send_length += device_object_data[_object][_command];
    *size_data += device_object_data[_object][_command];
    
    return 0;
}

uint8_t Packet_Device::get_size_by_packet(uint8_t *obj, uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;

    *size_data = 0;
    _send_length = MINIMAL_PACKET_SIZE;

    // объект
    if (field_bit[0].get_value(&_setting) != 1) {
        _object = field_byte.get_value((_packet + _send_length), (_length - _send_length));
        ++_send_length;
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
        ++_send_length;
    }

    // комманда
    _command = field_byte.get_value((_packet + _send_length), (_length - _send_length));
    ++_send_length;
    if (!(_command < DEVICE_COMMAND[_object])) {
        _command = 0xFF;
        return 3;
    }

    // данные
    *size_data += device_object_data[_object][_command];
    _send_length += device_object_data[_object][_command];

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
    _receive_length = MINIMAL_PACKET_SIZE;


    // комманда
    if (!(*com < SYSTEM_COMMAND_DATA)) {
        _command = 0xFF;
        return 2;
    }
    _command = *com;
    error = field_byte.set_value(_command, (_packet + _receive_length), (_length - _receive_length));
    ++_receive_length;
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
                error = field_byte.set_value(data[i], (_packet + _receive_length), (_length - _receive_length));
                ++_receive_length;
                if (error)
                    return 4 + i;
            }
            data = data + 4;

            _len = *len - 1;
            error = field_byte.set_value(_len, (_packet + _receive_length), (_length - _receive_length));
            ++_receive_length;
            if (error)
                return 5;
            _len += 1;
            break;

        // case 0x02:
        //     for(int i = 0; i < 2; ++i) {
        //         error = field_byte.set_value(data[i], (_packet + _receive_length), (_length - _receive_length));
        //         ++_receive_length;
        //         if (error)
        //             return 6 + i;
        //     }
        //     data = data + 2;
            
        //     if ((*len % 2) != 0)
        //         return 7;
        //     _len = *len / 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     error = field_byte.set_value(_len, (_packet + _receive_length), (_length - _receive_length));
        //     ++_receive_length;
        //     if (error)
        //         return 8;
        //     _len *= 2;
        //     break;

        // case 0x09:
        //     for(int i = 0; i < 3; ++i) {
        //         error = field_byte.set_value(data[i], (_packet + _receive_length), (_length - _receive_length));
        //         ++_receive_length;
        //         if (error)
        //             return 9 + i;
        //     }
        //     data = data + 3;
            
        //     if ((*len % 2) != 0)
        //         return 10;
        //     _len = *len / 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     error = field_byte.set_value(_len, (_packet + _receive_length), (_length - _receive_length));
        //     ++_receive_length;
        //     if (error)
        //         return 11;
        //     _len *= 2;
        //     break;
        default:
            return 12;
        }
    }

    for (int i = 0; i < _len; ++i) {
        error = field_byte.set_value(data[i], (_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
        if (error)
            return 13;
    }

    return 0;
}

uint8_t Packet_System::get_packet_data(uint8_t *com, uint8_t *data, uint8_t *len) {
    if (_packet == nullptr)
        return 1;
    _receive_length = MINIMAL_PACKET_SIZE;

    // комманда
    _command = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
    ++_receive_length;
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
                data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
                ++_receive_length;
            }
            data = data + 4;

            _len = (field_byte.get_value((_packet + _receive_length), _length)) + 1;
            ++_receive_length;
            *len = _len;
            break;
        // case 0x02:
        //     for (int i = 0; i < 2; ++i) {
        //         data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        //         ++_receive_length;
        //     }
        //     data = data + 2;
        //     _len = (field_byte.get_value((_packet + _receive_length), _length)) * 2;
        //     ++_receive_length;
        //     *len = _len;
        //     break;
        // case 0x09:
        //     for (int i = 0; i < 3; ++i) {
        //         data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        //         ++_receive_length;
        //     }
        //     data = data + 3;
        //     _len = (field_byte.get_value((_packet + _receive_length), _length)) * 2;
        //     ++_receive_length;
        //     *len = _len;
        //     break;
        default:
            return 4;
        }
    }

    for (int i = 0; i < _len; ++i) {
        data[i] = field_byte.get_value((_packet + _receive_length), (_length - _receive_length));
        ++_receive_length;
    }
    return 0;
}


uint8_t Packet_System::get_size_by_data(uint8_t *size_data, uint8_t *com, uint8_t *len) {
    _send_length = MINIMAL_PACKET_SIZE;
    *size_data = 0;

    // комманда
    if (!(*com < SYSTEM_COMMAND_DATA)) {
        _command = 0xFF;
        return 1;
    }
    _command = *com;
    ++_send_length;
    // данные
    int _len;
    if (system_com_data[_command] != 0xFF) {
        _len = system_com_data[_command];
    }
    else {
        switch (_command)
        {
        case 0x00:
            _send_length += 4; // смещение на ID
            ++_send_length;
            _len = *len + 1;
            size_data += 4;
            break;

        // case 0x02:
        //     if ((*len % 2) != 0)
        //         return 3;
        //     _send_length += 3;
        //     _len = *len; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     // _len = *len * 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     break;
        // case 0x09:
        //     if ((*len % 2) != 0)
        //         return 4;
        //     _send_length += 4;
        //     _len = *len; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     // _len = *len * 2; // делить на 2 или не делить (число номеров по 8 или по 16)
        //     break;
        default:
            return 5;
        }
    }
    *size_data += _len;
    _send_length += _len;
    return 0;
}
uint8_t Packet_System::get_size_by_packet(uint8_t *size_data) {
    if (_packet == nullptr)
        return 1;
    _send_length = MINIMAL_PACKET_SIZE;
    // комманда
    _command = field_byte.get_value((_packet + _send_length), (_length - _send_length));
    ++_send_length;
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
            _send_length += 4; // смещение на ID
            *size_data = 4; // смещение на ID
            *size_data += (field_byte.get_value((_packet + _send_length), (_length - _send_length))) + 1;
            ++_send_length;
            break;
        // case 0x02:
        //     _send_length += 2;
        //     *size_data = (field_byte.get_value((_packet + _send_length), (_length - _send_length))) * 2;
        //     ++_send_length;
        //     break;
        // case 0x09:
        //     _send_length += 3;
        //     *size_data = (field_byte.get_value((_packet + _send_length), (_length - _send_length))) * 2;
        //     ++_send_length;
        //     break;
        default:
            return 4;
        }
    }
    _send_length += *size_data;
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

#endif 