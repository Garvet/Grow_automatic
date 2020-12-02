#include "LoRa_register.h"


LoRa_register::LoRa_register(/* args */) {
    // _max_adr = 1;
    _send = false;
    clear();
}

LoRa_register::LoRa_register(SPIClass* set_spi, SPISettings* set_setting, uint8_t nss) {
    // _max_adr = 1;
    _send = false;
    clear();
    init(set_spi, set_setting, nss);
}

LoRa_register::~LoRa_register() {
}

void LoRa_register::init(SPIClass* set_spi, SPISettings* set_setting, uint8_t nss) {
    _spi = set_spi;
    _setting = set_setting;
    _nss = nss;
    pinMode(_nss, OUTPUT);
    digitalWrite(_nss, HIGH);
}





void LoRa_register::clear() {
    for(int i = 0; i < LORA_DATA_SIZE; ++i) {
        _registers_data[i] = 0;
        _registers_state[i] = false;
    }
    _send = false;
}
bool LoRa_register::get_send() {
    return _send;
}
// uint8_t check_error();



// // Выдаёт адреса регистров, относящихся к данному полю
// std::vector<uint8_t> LoRa_register::field_registers(Address_field field) {
//     std::vector<uint8_t> registers;
//     for(int i = 0; i < field.get_reg_count(); ++i) {
//         registers.push_back((uint8_t)(field.get_registers()[i].address() & 0xFF));
//     }
//     return registers;
// }
// std::vector<uint8_t> LoRa_register::field_registers(Address_field* fields, uint8_t amt_fields) {
//     std::vector<uint8_t> registers;
//     bool reg_add[LORA_DATA_SIZE];
//     uint8_t adr;
//     if((fields == nullptr) || (amt_fields == 0))
//         return registers;
//     for(int i = 0; i < LORA_DATA_SIZE; ++i) {
//         reg_add[i] = false;
//     }
//     for(int i = 0; i < amt_fields; ++i) {
//         for(int j = 0; j < fields[i].get_reg_count(); ++j) {
//             adr = (uint8_t)(fields[i].get_registers()[j].address() & 0xFF);
//             if(reg_add[adr] == false) {
//                 registers.push_back(adr);
//                 reg_add[adr] = true;
//             }
//         }
//     }
//     return registers;
// }
// std::vector<uint8_t> LoRa_register::field_registers(const std::vector<Address_field>& fields) {
//     std::vector<uint8_t> registers;
//     bool reg_add[LORA_DATA_SIZE];
//     uint8_t adr;
//     if(fields.size() == 0)
//         return registers;
//     for(int i = 0; i < LORA_DATA_SIZE; ++i) {
//         reg_add[i] = false;
//     }
//     for(int i = 0; i < fields.size(); ++i) {
//         for(int j = 0; j < fields[i].get_reg_count(); ++j) {
//             adr = (uint8_t)(fields[i].get_registers()[j].address() & 0xFF);
//             if(reg_add[adr] == false) {
//                 registers.push_back(adr);
//                 reg_add[adr] = true;
//             }
//         }
//     }
//     return registers;
// }
// // Проверка получения номеров регистров, которые нужно считать для данного списка полей
// std::vector<uint8_t> LoRa_register::check_missing_register(Address_field field) {
//     std::vector<uint8_t> registers;
//     registers = field_registers(field);
//     for(int i = registers.size() - 1; i >= 0; --i) {
//         if(_registers_state[registers[i]] != false) {
//             registers.erase(registers.begin() + i);
//         }
//     }
//     return registers;
// }
// std::vector<uint8_t> LoRa_register::check_missing_register(Address_field* fields, uint8_t amt_fields) {
//     std::vector<uint8_t> registers;
//     if((fields == nullptr) || (amt_fields == 0))
//         return registers;
//     registers = field_registers(fields, amt_fields);
//     for(int i = registers.size() - 1; i >= 0; --i) {
//         if(_registers_state[registers[i]] != false) {
//             registers.erase(registers.begin() + i);
//         }
//     }
//     return registers;
// }
// std::vector<uint8_t> LoRa_register::check_missing_register(const std::vector<Address_field>& fields) {
//     std::vector<uint8_t> registers;
//     if(fields.size() == 0)
//         return registers;
//     registers = field_registers(fields);
//     for(int i = registers.size() - 1; i >= 0; --i) {
//         if(_registers_state[registers[i]] != false) {
//             registers.erase(registers.begin() + i);
//         }
//     }
//     return registers;
// }




// Выдаёт адреса регистров, относящихся к данному полю
uint8_t LoRa_register::field_registers(Address_field field) {
    result_field_registers_len = 0;
    for(int i = 0; i < field.get_reg_count(); ++i) {
        result_field_registers_data[result_field_registers_len++] = ((uint8_t)(field.get_registers()[i].address() & 0xFF));
    }
    return result_field_registers_len;
}
uint8_t LoRa_register::field_registers(Address_field* fields, uint8_t amt_fields) {
    // std::vector<uint8_t> registers;
    result_field_registers_len = 0;
    bool reg_add[LORA_DATA_SIZE];
    uint8_t adr;
    if((fields == nullptr) || (amt_fields == 0))
        return result_field_registers_len;
    for(int i = 0; i < LORA_DATA_SIZE; ++i) {
        reg_add[i] = false;
    }
    for(int i = 0; i < amt_fields; ++i) {
        for(int j = 0; j < fields[i].get_reg_count(); ++j) {
            adr = (uint8_t)(fields[i].get_registers()[j].address() & 0xFF);
            if(reg_add[adr] == false) {
                result_field_registers_data[result_field_registers_len++] = adr;
                reg_add[adr] = true;
            }
        }
    }
    return result_field_registers_len;
}
// uint8_t LoRa_register::field_registers(const std::vector<Address_field>& fields) {
//     // std::vector<uint8_t> registers;
//     result_field_registers_len = 0;
//     bool reg_add[LORA_DATA_SIZE];
//     uint8_t adr;
//     if(fields.size() == 0)
//         return result_field_registers_len;
//     for(int i = 0; i < LORA_DATA_SIZE; ++i) {
//         reg_add[i] = false;
//     }
//     for(int i = 0; i < fields.size(); ++i) {
//         for(int j = 0; j < fields[i].get_reg_count(); ++j) {
//             adr = (uint8_t)(fields[i].get_registers()[j].address() & 0xFF);
//             if(reg_add[adr] == false) {
//                 result_field_registers_data[result_field_registers_len++] = adr;
//                 reg_add[adr] = true;
//             }
//         }
//     }
//     return result_field_registers_len;
// }
// Проверка получения номеров регистров, которые нужно считать для данного списка полей
uint8_t LoRa_register::check_missing_register(Address_field field) {
    // std::vector<uint8_t> registers;
    result_check_missing_register_len = 0;
    field_registers(field);
    for(int i = result_field_registers_len - 1; i >= 0; --i) {
        if(_registers_state[result_field_registers_data[i]] == false) {
            result_check_missing_register_data[result_check_missing_register_len++] = result_field_registers_data[i];
        }
        // if(_registers_state[registers[i]] != false) {
        //     registers.erase(registers.begin() + i);
        // }
    }
    return result_check_missing_register_len;
}
uint8_t LoRa_register::check_missing_register(Address_field* fields, uint8_t amt_fields) {
    // std::vector<uint8_t> registers;
    result_check_missing_register_len = 0;
    if((fields == nullptr) || (amt_fields == 0))
        return result_check_missing_register_len;
    field_registers(fields, amt_fields);
    for(int i = result_field_registers_len - 1; i >= 0; --i) {
        if(_registers_state[result_field_registers_data[i]] == false) {
            result_check_missing_register_data[result_check_missing_register_len++] = result_field_registers_data[i];
        }
        // if(_registers_state[registers[i]] != false) {
        //     registers.erase(registers.begin() + i);
        // }
    }
    return result_check_missing_register_len;
}
// uint8_t LoRa_register::check_missing_register(const std::vector<Address_field>& fields) {
//     // std::vector<uint8_t> registers;
//     result_check_missing_register_len = 0;
//     if(fields.size() == 0)
//         return result_check_missing_register_len;
//     field_registers(fields);
//     for(int i = result_field_registers_len - 1; i >= 0; --i) {
//         if(_registers_state[result_field_registers_data[i]] == false) {
//             result_check_missing_register_data[result_check_missing_register_len++] = result_field_registers_data[i];
//         }
//         // if(_registers_state[registers[i]] != false) {
//         //     registers.erase(registers.begin() + i);
//         // }
//     }
//     return result_check_missing_register_len;
// }






// Проверка на необходимость считывания перед записью новых значений (т.е. занимают ли поля весь объём данных регистра)
bool LoRa_register::check_read(Address_field field) { 
    for(int i = 0; i < field.get_reg_count(); ++i) {
        if(_registers_state[field.get_registers()[i].address()] == false)
            if(field.get_registers()[i].mask() != 0xFF)
                return true;
    }
    return false;
}
bool LoRa_register::check_read(Address_field* fields, uint8_t amt_fields) { 
    bool check_reg_read = false;
    uint8_t reg_bit[LORA_DATA_SIZE];
    for(int i = 0; i < LORA_DATA_SIZE; ++i) {
        reg_bit[i] = 0;
    }
    // Достаём из полей значения битов регистров
    uint8_t adr;//, val;
    for(int i = 0; i < amt_fields; ++i) {
        for(int j = 0; j < fields[i].get_reg_count(); ++j) {
            adr = fields[i].get_registers()[j].address();
            reg_bit[adr] |= fields[i].get_registers()[j].mask();
        }
    }

    reg_read_len = 0;
    reg_not_read_len = 0;

    for(int i = 0; i < LORA_DATA_SIZE; ++i) {
        if (reg_bit[i] != 0x00) {
            if ((reg_bit[i] == 0xFF) || (_registers_state[i] == true)) {
                // Если биты занимают всё пространство регистра, то можно не считывать, или если он был считан ранее
                reg_not_read[reg_not_read_len++] = i;
                // if(reg_not_read != nullptr)
                //     reg_not_read->push_back(i);
            }
            else {
                // Если хоть один бит не занят, то без считывания его можно будет затереть => считывать необходимо
                reg_read[reg_read_len++] = i;
                check_reg_read = true;
                // if(reg_read != nullptr)
                //     reg_read->push_back(i);
                // check_reg_read = true;
            }
        }
    }
    return check_reg_read; // Наличие хоть одного регистра, который нужно будет считать

}
// bool LoRa_register::check_read(const std::vector<Address_field>& fields, std::vector<uint8_t>* reg_read, std::vector<uint8_t>* reg_not_read) { 
//     bool check_reg_read = false;
//     uint8_t reg_bit[LORA_DATA_SIZE];
//     for(int i = 0; i < LORA_DATA_SIZE; ++i) {
//         reg_bit[i] = 0;
//     }
//     // Достаём из полей значения битов регистров
//     uint8_t adr;//, val;
//     for(int i = 0; i < fields.size(); ++i) {
//         for(int j = 0; j < fields[i].get_reg_count(); ++j) {
//             adr = fields[i].get_registers()[j].address();
//             reg_bit[adr] |= fields[i].get_registers()[j].mask();
//         }
//     }
//     for(int i = 0; i < LORA_DATA_SIZE; ++i) {
//         if (reg_bit[i] != 0x00) {
//             if ((reg_bit[i] == 0xFF) || (_registers_state[i] == true)) {
//                 // Если биты занимают всё пространство регистра, то можно не считывать, или если он был считан ранее
//                 if(reg_not_read != nullptr)
//                     reg_not_read->push_back(i);
//             }
//             else {
//                 // Если хоть один бит не занят, то без считывания его можно будет затереть => считывать необходимо
//                 if(reg_read != nullptr)
//                     reg_read->push_back(i);
//                 check_reg_read = true;
//             }
//         }
//     }
//     return check_reg_read; // Наличие хоть одного регистра, который нужно будет считать
// }

// считывает регистры относящиеся к полю(ям) (+) ----- -----------------------------------------------------------------------------------------------------------
uint8_t LoRa_register::register_read(Address_field field, bool update) {
    return register_read(&field, 1, update);
}
uint8_t LoRa_register::register_read(Address_field* fields, uint8_t amt_fields, bool update) {
    if (_send) // (+) ----- -----------------------------------------------------------------------------------------------------------
        clear(); // (+) ----- -----------------------------------------------------------------------------------------------------------
    std::array<uint8_t, 80>* registers;
    uint8_t len = 0;
    if (update) {
        field_registers(fields, amt_fields);
        registers = &result_field_registers_data;
        len = result_field_registers_len;
    }
    else {
        check_missing_register(fields, amt_fields);
        registers = &result_check_missing_register_data;
        len = result_check_missing_register_len;
    }
    uint8_t adr;
    uint8_t amt_read = len;
    for(int i = 0; i < amt_read; ++i) {
        adr = (*registers)[i];
        _registers_data[adr] = _read_register(adr);
        _registers_state[adr] = true;
    }
    return amt_read;
}
// записывает регистры относящиеся к полю(ям)
uint8_t LoRa_register::register_write(bool fl_clear) {
    uint8_t amt_send = 0;
    for(int i = 0; i < LORA_DATA_SIZE; ++i) {
        if(_registers_state[i] == true) {
            _write_register(i, _registers_data[i]);
            ++amt_send;
        }
    }
    _send = true;
    if (fl_clear) { 
        clear();
    }
    return amt_send;
}
uint8_t LoRa_register::register_write(Address_field field, bool fl_clear, bool error_clear) {
    return register_write(&field, 1, fl_clear, error_clear);
}
uint8_t LoRa_register::register_write(Address_field* fields, uint8_t amt_fields, bool fl_clear, bool error_clear) {
    // std::vector<uint8_t> write_adr = check_missing_register(fields, amt_fields);
    if(check_missing_register(fields, amt_fields) != 0) {
        if(error_clear)
            clear();
        return 0;
    }
    field_registers(fields, amt_fields);
    // uint8_t amt_write = write_adr.size();
    for(int i = 0; i < result_field_registers_len; ++i) {
        _write_register(result_field_registers_data[i], _registers_data[result_field_registers_data[i]]);
    }
    _send = true;
    if (fl_clear) { 
        clear();
    }
    return result_field_registers_len;
}
// Установка значения(ий) полю(ям)
uint8_t LoRa_register::set_field_value(Address_field field, uint32_t value) {
    return set_field_value(&field, &value, 1);
}
uint8_t LoRa_register::set_field_value(Address_field* fields, uint32_t* values, uint8_t amt) {
    // Проверяем и считываем значения всех отсутствующих регистров
    uint8_t amt_set_value = 0;
    // std::vector<uint8_t> missing_register = check_missing_register(fields, amt);
    check_missing_register(fields, amt);
    if(result_check_missing_register_len > 0) {
        bool read_fields;
        // std::vector<uint8_t> reg_read;
        // std::vector<uint8_t> reg_not_read;
        // read_fields = check_read(fields, amt, &reg_read, &reg_not_read);
        // for(int i = 0; i < reg_not_read.size(); ++i) {
        // std::vector<uint8_t> reg_read;
        // std::vector<uint8_t> reg_not_read;
        read_fields = check_read(fields, amt);
        for(int i = 0; i < reg_not_read_len; ++i) {
            _registers_state[reg_not_read[i]] = true;
        }
        if(read_fields) {
            register_read(fields, amt, false);
        }
    }
    // Заполняем все регистры, если ошибка в заполнении выходим
    bool result;
    for(int i = 0; i < amt; ++i) {
        result = fields[i].set_value(values[i], _registers_data, LORA_DATA_SIZE);
        if(result) 
            break;
        ++amt_set_value;
    }
    return amt_set_value;
}
// Получение значения(ий) поля(ей)
uint8_t LoRa_register::get_field_value(Address_field field, uint32_t* value, bool read) {
    return get_field_value(&field, value, 1, read);
}
uint8_t LoRa_register::get_field_value(Address_field* fields, uint32_t* values, uint8_t amt, bool read) {
    // std::vector<uint8_t> check = check_missing_register(fields, amt);
    check_missing_register(fields, amt);
    if((result_check_missing_register_len != 0) || read) {
        register_read(fields, amt, read);
    }
    for(int i = 0; i < amt; ++i)
        values[i] = fields[i].get_value(_registers_data, LORA_DATA_SIZE);
    return amt;
}
// Сброс значения(ий) флага(ов)
uint8_t LoRa_register::clear_flags(Address_field flag, bool back_value) {
    return clear_flags(&flag, 1, back_value);
}
static uint32_t _values_clear_flags_[150];
uint8_t LoRa_register::clear_flags(Address_field* flags, uint8_t amt_flags, bool back_value) {
    if(_registers_state[REG_IRQ_FLAGS] == false) {
        return 0; // флаги не считаны
    }
    for(int i = 0; i < amt_flags; ++i) {
        bool err_flag = true;
        for(int j = 0; j < AMT_FLAGS; ++j) {
            if(flags[i] == _flags[j]) {
                err_flag = false;
                break;
            }
        }
        if(err_flag) {
            return 0; // хоть одно поле не флаг
        }
    }
    // Запоминаем текущие состояние флагов
    uint8_t data = _registers_data[REG_IRQ_FLAGS];
    _registers_data[REG_IRQ_FLAGS] = 0;
    // Запоминаем текущие состояние флагов
    // uint32_t* values = new uint32_t[amt_flags];
    for(int i = 0; i < amt_flags; ++i)
        _values_clear_flags_[i] = 1;
    set_field_value(flags, _values_clear_flags_, amt_flags);
    _write_register(REG_IRQ_FLAGS, _registers_data[REG_IRQ_FLAGS]);
    // Возврат значений
    if(back_value) {
        // Возвращаем значение всех флагов или неопущенных флагов
        _registers_data[REG_IRQ_FLAGS] = data;
    }
    else {
        // Возвращаем значение неопущенных флагов
        _registers_data[REG_IRQ_FLAGS] = data | ~_registers_data[REG_IRQ_FLAGS];
    }
    return amt_flags;
}





// (+) ----- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t LoRa_register::_read_register(uint8_t address) {
    return _single_transfer(address & 0x7f, 0x00);
}
// (+) ----- -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void LoRa_register::_write_register(uint8_t address, uint8_t value) {
    _single_transfer(address | 0x80, value);
}
uint8_t LoRa_register::_single_transfer(uint8_t address, uint8_t value) {
    uint8_t response;
    digitalWrite(_nss, LOW);
    _spi->beginTransaction(*_setting);
    _spi->transfer(address);
    response = _spi->transfer(value);
    _spi->endTransaction();
    digitalWrite(_nss, HIGH);
    return response;
}