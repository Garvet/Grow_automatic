#include "Group_control_module.h"

#define NUM_SENSOR 0x01
#define NUM_DEVICE 0x02

// (!) ----- Переделать в нормальный вид (pin -> тот или иной GT)
static Group_control_module* group_control_module_interrupt;
void lora_interrupt() {
    group_control_module_interrupt->LoRa_interrupt();
}

//   ----- ----- ----- ---- ----- ----- -----
// ----- ----- -----  Public  ----- ----- -----
//   ----- ----- ----- ---- ----- ----- -----

// (-) ----- удалить, вынести за пределы .cpp с настройкой извне
#define RTC_DS1302_DAT GPIO_NUM_4 // DS1302 DAT/IO --> 4
#define RTC_DS1302_CLK GPIO_NUM_5 // DS1302 CLK/SCLK --> 5
#define RTC_DS1302_RST GPIO_NUM_2 // DS1302 RST/CE --> 2
static ThreeWire rtc_wire(RTC_DS1302_DAT, RTC_DS1302_CLK, RTC_DS1302_RST);
// !-! -----

Group_control_module::Group_control_module(/* args */): rtc_(rtc_wire) {
    // (-) ----- зделать загрузку при включении, потом инициализацию если нет файла
    mode_ = GT_SETTING;
    check_sensor_ = false;
    check_device_ = false;
    end_adr_ = 0; // (--) ----- (исключается)
    rtc_begin_ = false;
    data_time_ = 0;
    permission_regist_interrupt_ = true; 
}

Group_control_module::Group_control_module(uint8_t pin_reset, uint8_t spi_bus, uint8_t spi_nss, uint8_t pin_dio0, 
                       uint8_t pin_dio1, uint8_t pin_dio3): rtc_(rtc_wire) {
    // (-) ----- (обычный конструктор)
    LoRa_init(pin_reset, spi_bus, spi_nss, pin_dio0, pin_dio1, pin_dio3);
}

Group_control_module::~Group_control_module() {

}


// Инициализация параметров LoRa модуля (частота задаётся конфигурацией)
bool Group_control_module::LoRa_init(uint8_t pin_reset, uint8_t spi_bus, uint8_t spi_nss, 
                           uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3) {
    group_control_module_interrupt = this;
    bool result = contact_data_.init_lora_module(pin_reset, spi_bus, spi_nss, pin_dio0, pin_dio1, pin_dio3);
    interrupt_init(pin_dio0, pin_dio1, pin_dio3);
    return result;
}
// Запуск 
void Group_control_module::rtc_begin() {
    rtc_.Begin();
    rtc_begin_ = true;
}

uint8_t Group_control_module::LoRa_begin() {
    if(mode_ != GT_SETTING)
        ; // contact_data_.begin_lora_module([channel]);
    return contact_data_.begin_lora_module(REIST_BAND);
}
uint8_t Group_control_module::begin() {
    rtc_begin();
    return LoRa_begin();
}
// Работа класса
uint8_t Group_control_module::work_system() { 
    // (проверяет систему, обрабатывает формулы (У|Д|Ф), гинерит пакеты, запускает передачу)
    switch (mode_) {
    case GT_SETTING:
        // (?) ----- а есть ли здесь хоть что-то?
        break;
    case GT_PROCESSING:
        // if(contact_data_.get_signal_complete()) {
        if(contact_data_.get_state_contact() == SC_DOWNTIME) {
            if(!handler_devices())
                break;
            if(!handler_sensors())
                break;
            // формулы()   // --- if(формулы()) ---
            //             // ---     break;    ---
            // опустить все флаги датчиков (они не должны опускаться если из-за формул меняются устройства) (нет, т.к. 
            //     я пройдусь по всем формулам)

            // в формулы() 
            //          false при отсутствии change_value_ в датчиках
            //          пройтись по всем формулам
            //          выдать true, если 

            // (-) -----
            // 2. Обработка устройств (проверить то, что было в старом GT и параметр change_value_)
            // 3. Обработка датчиков (при приёме установить значение переменных с данными, чтобы изменился change_value_)
            // 4. Обработка формул при изменении показателей датчиков (проверить все формулы с датчиками, у которых 
            //      есть change_value_, внося в вектор все проверенные датчики, после - их опустить, [?] в sensors сделать 
            //      режимы "в общении", и "ошибка", при 1-м формулы опускать, при 2-м - выключить/включить устройства)
            //      в результате формул посылать управление ШИМом и сигналом
        }
        break;
    default:
        break;
    }
    return false;
}
void Group_control_module::LoRa_interrupt() { 
    // (контролирует каждый шаг передачи, по завершению запускает work_system, контролирует трансляцию, заносит запросы регистрации)
    // (-) -----
    static bool in_interrupt = false;
    if(in_interrupt)
        return;
    in_interrupt = true;
    uint16_t err = contact_data_.work_contact_system();
    bool add_err = false;
    if(err != 0) {
        if(mode_ == GT_SETTING) {
            LoRa_address adr = contact_data_.get_connect_adr();
            for(int i = 0; i < devices_.size(); ++i) {
                if(devices_[i].get_address() == adr.branch) {
                    devices_[i].set_active(0);    
                    Serial.print("Add contact error = ");
                    Serial.println(err);
                    add_err = true;
                    break;
                }
            }
            for(int i = 0; i < sensors_.size(); ++i) {
                if(sensors_[i].get_address() == adr.branch) {
                    sensors_[i].set_active(0);    
                    Serial.print("Add contact error = ");
                    Serial.println(err);
                    add_err = true;
                    break;
                }
            }
        }
        else {
            Serial.print("Err: ");
            Serial.println(err);
        }
        // (-) ----- обработка ошибок, сохранение их для дальнейшей выдачи
    }
    if((err == 0) || add_err) {
        switch (mode_) {
        case GT_SETTING: {
            // (-) ----- (?) -----
            if(permission_regist_interrupt_) {
                if(contact_data_.get_state_contact() == SC_PACKET_ACCEPTED) {
                    std::vector<LoRa_packet> packets;
                    LoRa_packet reg_packet;
                    // Exchange_packet reg_packet;

                    packets = contact_data_.get_all_packet();

                    for (int i = 0; i < packets.size(); ++i) {
                        reg_packet = std::move(packets[i]);
                        if(add_reg_module(reg_packet))
                            Serial.println("Add error");
                        // reg_packet.set_packet(packets[i]);
                        // if(add_reg_module(reg_packet))
                        //     Serial.println("Add error");
                    }
                }
                else if (contact_data_.get_signal_complete()) {
                    LoRa_address adr = contact_data_.get_connect_adr();
                    for(int i = 0; i < devices_.size(); ++i) {
                        if(devices_[i].get_address() == adr.branch)
                            devices_[i].set_active(2);    
                    }
                    for(int i = 0; i < sensors_.size(); ++i) {
                        if(sensors_[i].get_address() == adr.branch)
                            sensors_[i].set_active(2);    
                    }
                }

                if(contact_data_.get_signal_complete() || contact_data_.get_state_contact() == SC_WAITING || contact_data_.get_state_contact() == SC_DOWNTIME) {
                    bool check_reg = false;
                    LoRa_address regist_adr = {contact_data_.get_my_adr().group, 0};
                    for(int i = 0; i < devices_.size(); ++i) {
                        if(devices_[i].get_active() == 1) {
                            regist_adr.branch = devices_[i].get_address();
                            check_reg = true;
                            break;
                        }
                    }
                    if(!check_reg) {
                        for(int i = 0; i < sensors_.size(); ++i) {
                            if(sensors_[i].get_active() == 1) {
                                regist_adr.branch = sensors_[i].get_address();
                                check_reg = true;
                                break;
                            }
                        }
                    }
                    if(check_reg) {
                                                        // (?) ----- (-) ----- проверка (должна выглядеть не так?, - settings), и не забудь settings в contact_data включить, пока там только датчики, устройства и системные пакеты
                                                        {
                                                            // Exchange_packet packet;
                                                            // uint8_t size = 0;
                                                            // uint8_t com = 0x0D;
                                                            // uint8_t len = 0;
                                                            // static_cast<Packet_System*>(packet.packet)->get_size_by_data(&size, &com, &len);
                                                            // uint8_t *data = new uint8_t[size];
                                                            // packet.creat_packet(size + 10, Packet_Type::SYSTEM);
                                                            // packet.packet->set_dest_adr(regist_adr);
                                                            // packet.packet->set_sour_adr(contact_data_.get_my_adr());
                                                            // packet.packet->set_packet_type(Packet_Type::SYSTEM);
                                                            // static_cast<Packet_System*>(packet.packet)->set_packet_data(&com, data, &len);
                                                            // delete[] data;
                                                            // contact_data_.init_contact(regist_adr);
                                                            // contact_data_.add_packet(packet);
                                                            // contact_data_.start_transfer();
                                                            LoRa_packet packet;
                                                            uint8_t size = 0;
                                                            uint8_t com = 0x0D;
                                                            uint8_t len = 0;
                                                            packet_system.get_size_by_data(&com, &len, size);
                                                            // static_cast<Packet_System*>(packet.packet)->get_size_by_data(&size, &com, &len);
                                                            uint8_t *data = new uint8_t[size];
                                                            // packet.creat_packet(size + 10, Packet_Type::SYSTEM);
                                                            packet_system.set_dest_adr(packet, regist_adr);
                                                            packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                                                            packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
                                                            // packet.packet->set_dest_adr(regist_adr);
                                                            // packet.packet->set_sour_adr(contact_data_.get_my_adr());
                                                            // packet.packet->set_packet_type(Packet_Type::SYSTEM);
                                                            // static_cast<Packet_System*>(packet.packet)->set_packet_data(&com, data, &len);
                                                            packet_system.set_packet_data(packet, &com, data, &len);
                                                            delete[] data;
                                                            contact_data_.init_contact(regist_adr);
                                                            contact_data_.add_packet(packet);
                                                            contact_data_.start_transfer();
                                                        }

                    }
                    else if(contact_data_.get_signal_complete() || add_err) {
                        contact_data_.broadcast_receive();
                    }
                }
            }
            break;
        }
        case GT_PROCESSING: {
            if(contact_data_.get_signal_complete()) {
                // (-) -----
                // Обработать показания с датчиков 
                LoRa_packet packet_processing;
                std::vector<LoRa_packet> packets = contact_data_.get_all_packet();
                uint16_t num_sensor;
                for(num_sensor = 0; num_sensor < sensors_.size(); ++num_sensor) {
                    if(sensors_[num_sensor].get_address() == contact_data_.get_connect_adr())
                        break;
                }
                if(num_sensor < sensors_.size()) {
                    for(int i = 0; i < packets.size(); ++i) {
                        // packet_processing.set_packet(packets[i]);
                        // if(packet_processing.packet->get_sour_adr() != contact_data_.get_connect_adr())
                        if(packet_analyzer.get_sour_adr(packets[i]) != contact_data_.get_connect_adr())
                            continue; // (-) ----- неправильная обработка (?) ----- возврат пакета обратно, можно запомнить i
                        //                           (!) ----- (-) ----- (-) ----- (-) ----- (!) ----- interface
                        // if(packet_processing.type_packet == PACKET_SENSOR) {
                        //     float value = 0.0;
                        //     uint8_t amt = 0;
                        //     uint8_t size = 0;
                        //     uint8_t *id  = nullptr;
                        //     uint8_t *param = nullptr;
                        //     uint32_t *data = nullptr; 
                        //     static_cast<Packet_Sensor*>(packet_processing.packet)->set_setting(sensors_[num_sensor].get_setting());
                        //     static_cast<Packet_Sensor*>(packet_processing.packet)->get_size_by_packet(&amt, nullptr, &size);
                        //     id = new uint8_t[amt];
                        //     param = new uint8_t[amt];
                        //     data = new uint32_t[amt];
                        //     static_cast<Packet_Sensor*>(packet_processing.packet)->get_packet_data(&amt, param, id, data);
                        //     for(int j = 0; j < amt; ++j) {
                        //         for(int k = 0; k < sensors_[num_sensor].get_count_component(); ++k) {
                        //             if(sensors_[num_sensor].get_component(k).get_type() == param[j])
                        //                 if(sensors_[num_sensor].get_component(k).get_id() == id[j]) {
                        //                     // у 0x00 тип uint16_t = 0-4095, у 0x01 тип bool
                        //                     if(param[j] == 0x00 || (param[j] == 0x01)) { 
                        //                         value = data[j];
                        //                     }
                        //                     else {
                        //                         value = *((float*)&(data[j]));
                        //                     }
                        //                     sensors_[num_sensor].set_value(k, value);
                        //                     break;
                        //                 }
                        //         }
                        //     }
                        // }
                    }
                }
                else {
                    // Не датчик, а ... ?
                }
            }
            // Запустить проверку датчиков/фомул при успешном завершении контакта
            work_system(); // проверка встроена
            break;
        }
        default:
            break;
        }
    }
    in_interrupt = false;
}

// Обработка времени
void Group_control_module::set_data_time(RtcDateTime data_time) {
    data_time_ = data_time;
    if (rtc_begin_)
        rtc_.SetDateTime(data_time_);
}
RtcDateTime Group_control_module::get_data_time() {
    if (rtc_begin_)
        data_time_ = rtc_.GetDateTime();
    return data_time_;
}


// --- Настройка системы ---

// Установка конфигурации
// bool Group_control_module::set_configuration(std::vector<uint8_t> data) {
//     if(mode_ != GT_SETTING)
//         return true;
//     // (-+-) -----
//     // save (-+-) -----
//     return false;
// }

size_t Group_control_module::get_size() {
    // uint16_t _address; 2
    // uint16_t _channel; 2
    // uint8_t  _len_name; 1
    // uint16_t _amt_sensors; 2
    // uint16_t _amt_devices; 2
    // char* _name; not save
    // class Config_grow_sensor* _sensors; not save
    // class Config_grow_device* _devices; not save
    size_t size = 9 + (name_.size() & 0xFF);
    for(int i = 0; i < sensors_.size(); ++i)
        size += grow_sensor_interface.get_size(sensors_[i]);
    for(int i = 0; i < devices_.size(); ++i)
        size += devices_[i].get_size();
    return size;
}
size_t Group_control_module::get_data(uint8_t *data) {
    if(data == nullptr)
        return 0;
    size_t size = 0;
    uint16_t value;
    value = contact_data_.get_my_adr().group;
    data[size++] = (uint8_t)((value >> 8) & 0xFF);
    data[size++] = (uint8_t)(value & 0xFF);
    value = contact_data_.get_channel();
    data[size++] = (uint8_t)((value >> 8) & 0xFF);
    data[size++] = (uint8_t)(value & 0xFF);

    data[size++] = name_.size() & 0xFF;
    data[size++] = (uint8_t)((sensors_.size() >> 8) & 0xFF);
    data[size++] = (uint8_t)(sensors_.size() & 0xFF);
    data[size++] = (uint8_t)((devices_.size() >> 8) & 0xFF);
    data[size++] = (uint8_t)(devices_.size() & 0xFF);
    for(int i = 0; i < name_.size(); ++i)
        data[size++] = name_[i];
    for(int i = 0; i < sensors_.size(); ++i)
        size += grow_sensor_interface.get_data(sensors_[i], data+size);
    for(int i = 0; i < devices_.size(); ++i)
        size += devices_[i].get_data(data+size);
    return size;
}
size_t Group_control_module::set_data(uint8_t *data, size_t available_size) {
    if(mode_ != GT_SETTING)
        return 0;
    size_t size = 0;
    uint16_t value, len_name, amt_sensors, amt_devices, address = 1;
    name_.clear();
    sensors_.clear();
    devices_.clear();

    value  = ((uint16_t)data[size++]) << 8;
    value |= ((uint16_t)data[size++]);
    contact_data_.set_my_adr({value, 0});

    value  = ((uint16_t)data[size++]) << 8;
    value |= ((uint16_t)data[size++]);
    contact_data_.set_channel(value);

    len_name = data[size++];
    amt_sensors  = ((uint16_t)data[size++]) << 8;
    amt_sensors |= ((uint16_t)data[size++]);
    amt_devices  = ((uint16_t)data[size++]) << 8;
    amt_devices |= ((uint16_t)data[size++]);

    if(len_name != 0) {
        name_.resize(len_name);
        for(int i = 0; i < len_name; ++i)
            name_[i] = data[size++];
    }

    if(amt_sensors != 0) {
        sensors_.resize(amt_sensors);
        for(int i = 0; i < amt_sensors; ++i) {
            size += grow_sensor_interface.set_data(sensors_[i], data+size, available_size-size);
            sensors_[i].set_address(address++);
        }
    }

    if(amt_devices != 0) {
        devices_.resize(amt_devices);
        for(int i = 0; i < amt_devices; ++i) {
            size += devices_[i].set_data(data+size, available_size-size);
            devices_[i].set_address(address++);
        }
    }

// (?) ----- старт приёма для регистрации
    contact_data_.broadcast_receive();
// save (-) -----
    return size;
    
    // resize
}


// Фильтрация регистрируемых модулей 
bool Group_control_module::filter_devices(Grow_device &devices) {
    if(mode_ != GT_SETTING)
        return true;
    bool result = false;
    filter_adr_.clear();
    disable_regist_interrupt();
    for(int i = 0; i < reg_devices_.size(); ++i) {
        if(reg_devices_[i].filter(devices)) {
            filter_adr_.push_back(reg_devices_[i].get_address());
            result = true;
        }
    }
    enable_regist_interrupt();
    return result;
}
bool Group_control_module::filter_sensors(Grow_sensor &sensor) {
    if(mode_ != GT_SETTING)
        return true;
    bool result = false;
    filter_adr_.clear();
    disable_regist_interrupt();
    for(int i = 0; i < reg_sensors_.size(); ++i) {
        if(reg_sensors_[i].filter(sensor)) {
            filter_adr_.push_back(reg_sensors_[i].get_address());
            result = true;
        }
    }
    enable_regist_interrupt();
    return result;
}
// [T] Регистрация модуля(ей)
bool Group_control_module::regist_device(uint16_t old_adr, uint16_t new_adr) {
    bool err = true;
    if(mode_ != GT_SETTING)
        return err;
    // Поиск в конфигурации
    uint16_t num_device;
    for(int i = 0; i < devices_.size(); ++i) {
        if(new_adr == devices_[i].get_address()) {
            if(devices_[i].get_active() == 0) // удалять если занят (?) -----
                err = false;
            num_device = i;
            break;
        }
    }
    if(err)
        return err;
    // Процесс регистрации
    err = true;
    disable_regist_interrupt();
    for(int i = 0; i < filter_adr_.size(); ++i) {
        if(old_adr == filter_adr_[i]) {
            err = false;
            filter_adr_.erase(filter_adr_.begin() + i);
            for(int j = 0; j < reg_devices_.size(); ++j) {
                if(old_adr == reg_devices_[j].get_address()) {
                    reg_devices_.erase(reg_devices_.begin() + j);
                    break;
                }
            }
            break;
        }
    }
    if(!err) {
        // (+) -----
        // Произвести обмен информацией
                    // Exchange_packet packet;
                    // uint8_t size = 0;
                    // uint8_t com = 0x03;
                    // uint8_t len = 0;
                    // static_cast<Packet_System*>(packet.packet)->get_size_by_data(&size, &com, &len);
                    // uint8_t *data = new uint8_t[size];
                    // packet.creat_packet(size + 10, Packet_Type::SYSTEM);

                    // packet.packet->set_dest_adr({0, old_adr});
                    // packet.packet->set_sour_adr(contact_data_.get_my_adr());
                    // packet.packet->set_packet_type(Packet_Type::SYSTEM);

                    // data[0] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                    // data[1] = (contact_data_.get_my_adr().group << 7) & 0x80;
                    // data[1] |= (new_adr >> 8) & 0x7F;
                    // data[2] = (new_adr) & 0xFF;

                    // static_cast<Packet_System*>(packet.packet)->set_packet_data(&com, data, &len);
                    // delete[] data;
                    // contact_data_.add_packet(packet);
                    // contact_data_.broadcast_send(true);
                    LoRa_packet packet;
                    uint8_t size = 0;
                    uint8_t com = 0x03;
                    uint8_t len = 0;
                    packet_system.get_size_by_data(&com, &len, size);
                    uint8_t *data = new uint8_t[size];

                    packet_system.set_dest_adr(packet, {0, old_adr});
                    packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                    packet_system.set_packet_type(packet, Packet_Type::SYSTEM);

                    data[0] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                    data[1] = (contact_data_.get_my_adr().group << 7) & 0x80;
                    data[1] |= (new_adr >> 8) & 0x7F;
                    data[2] = (new_adr) & 0xFF;

                    packet_system.set_packet_data(packet, &com, data, &len);
                    delete[] data;
                    contact_data_.add_packet(packet);
                    contact_data_.broadcast_send(true);
                    
        devices_[num_device].set_active(1);
    }   
    enable_regist_interrupt();
    // save (-) -----
    return err;
}
bool Group_control_module::regist_sensor(uint16_t old_adr, uint16_t new_adr) {
    bool err = true;
    if(mode_ != GT_SETTING)
        return err;
    // Поиск в конфигурации
    uint16_t num_sensor;
    for(int i = 0; i < sensors_.size(); ++i) {
        if(new_adr == sensors_[i].get_address()) {
            if(sensors_[i].get_active() == 0) // удалять если занят (?) -----
                err = false;
            num_sensor = i;
            break;
        }
    }
    if(err)
        return err;
    // Процесс регистрации
    disable_regist_interrupt();
    err = true;
    for(int i = 0; i < filter_adr_.size(); ++i) {
        if(old_adr == filter_adr_[i]) {
            err = false;
            filter_adr_.erase(filter_adr_.begin() + i);
            for(int j = 0; j < reg_sensors_.size(); ++j) {
                if(old_adr == reg_sensors_[j].get_address()) {
                    reg_sensors_.erase(reg_sensors_.begin() + j);
                    break;
                }
            }
            break;
        }
    }
    if(!err) {
        // (+) -----
        // Произвести обмен информацией
                // Exchange_packet packet;
                // uint8_t size = 0;
                // uint8_t com = 0x03;
                // uint8_t len = 0;
                // static_cast<Packet_System*>(packet.packet)->get_size_by_data(&size, &com, &len);
                // uint8_t *data = new uint8_t[size];
                // packet.creat_packet(size + 10, Packet_Type::SYSTEM);
                // packet.packet->set_dest_adr({0, old_adr});
                // packet.packet->set_sour_adr(contact_data_.get_my_adr());
                // packet.packet->set_packet_type(Packet_Type::SYSTEM);
                // data[0] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                // data[1] = (contact_data_.get_my_adr().group << 7) & 0x80;
                // data[1] |= (new_adr >> 8) & 0x7F;
                // data[2] = (new_adr) & 0xFF;
                // static_cast<Packet_System*>(packet.packet)->set_packet_data(&com, data, &len);
                // delete[] data;
                // contact_data_.add_packet(packet);
                // contact_data_.broadcast_send(true);
                LoRa_packet packet;
                uint8_t size = 0;
                uint8_t com = 0x03;
                uint8_t len = 0;
                packet_system.get_size_by_data(&com, &len, size);
                uint8_t *data = new uint8_t[size];
                packet_system.set_dest_adr(packet, {0, old_adr});
                packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
                data[0] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                data[1] = (contact_data_.get_my_adr().group << 7) & 0x80;
                data[1] |= (new_adr >> 8) & 0x7F;
                data[2] = (new_adr) & 0xFF;
                packet_system.set_packet_data(packet, &com, data, &len);
                delete[] data;
                contact_data_.add_packet(packet);
                contact_data_.broadcast_send(true);
        sensors_[num_sensor].set_active(1);
    }
    enable_regist_interrupt();
    // save (-) -----
    return err;
}
// [T] Сигнал модуля (?) ----- 
bool Group_control_module::module_set_signal(uint16_t adr) {
    if(mode_ != GT_SETTING)
        return true;
    // (-) -----
    return false;
}
// [C] Удаление модуля (?) -----
bool Group_control_module::remove_module(uint16_t adr) {
    if(mode_ != GT_SETTING)
        return true;
    // удаление (-) -----
    // save (-) -----
    return false;
}
// Установка режима 
bool Group_control_module::set_mode(Group_control_module::Mode mode) {
    switch (mode)
    {
    case GT_SETTING:
        mode_ = GT_SETTING; // Остановка обработок системы (-) ----- 
        return false;
        break;
    case GT_PROCESSING:
        mode_ = GT_PROCESSING; // Включение обработок системы, если КФ есть и заполнен полностью (-) -----
        return false;
        break;
    default:
        break;
    }
    // save (-) -----
    return true;
}



//   ----- ----- ----- ----- ----- ----- -----
// ----- ----- -----  Private  ----- ----- -----
//   ----- ----- ----- ----- ----- ----- -----



// --- Обработка модулей ---
void Group_control_module::enable_regist_interrupt() {
    permission_regist_interrupt_ = true;
    // Ф проверить и добавить модули // (-) -----
}
void Group_control_module::disable_regist_interrupt() {
    permission_regist_interrupt_ = false; // прерывания происходят, чтение пакетов идёт, но обработка их - остановлена
}
void Group_control_module::interrupt_init(uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3) {
    if(pin_dio0 != 0) {
        pinMode(pin_dio0, INPUT);
        attachInterrupt(digitalPinToInterrupt(pin_dio0), lora_interrupt, RISING);
    }
    if(pin_dio1 != 0) {
        pinMode(pin_dio1, INPUT);
        attachInterrupt(digitalPinToInterrupt(pin_dio1), lora_interrupt, RISING);
    }
    if(pin_dio3 != 0) {
        pinMode(pin_dio3, INPUT);
        attachInterrupt(digitalPinToInterrupt(pin_dio3), lora_interrupt, RISING);
    }
#if defined( WIFI_LoRa_32_V2 ) 
#define PIN_IRQ   GPIO_NUM_26
    pinMode(PIN_IRQ, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_IRQ),  lora_interrupt, RISING);
#endif // WIFI_LoRa_32_V2
}

bool Group_control_module::add_reg_module(const LoRa_packet &reg_packet) {
    uint8_t size = 0;
    // Обработка пакета
    if(packet_analyzer.get_packet_type(reg_packet) != Packet_Type::SYSTEM)
        return true;

    if(packet_system.get_size_by_packet(reg_packet, size) || (size < 4))
        return true;


    uint8_t *data = new uint8_t[size];
    uint8_t com = 0xFF;
    uint8_t len = 0xFF;

    packet_system.get_packet_data(reg_packet, &com, data, &len);
    if (com != 0x00) {
        delete[] data;
        return true;
    }
    bool err = false;
    uint8_t symbol = 0;
    // Проверка ID
    // (-) ----- превратить в get_id()
    uint32_t module_id = data[symbol++];
    module_id = (module_id << 8) | data[symbol++];
    module_id = (module_id << 8) | data[symbol++];
    module_id = (module_id << 8) | data[symbol++];
    if(module_id == 0)  
        err = true;
    
    if(!err) {
        for(int i = 0; i < reg_devices_.size(); ++i) {
            if(module_id == reg_devices_[i].get_system_id()) {
                err = true;
                break;
            }
        }
    }
    if(!err) {
        for(int i = 0; i < reg_sensors_.size(); ++i) {
            if(module_id == reg_sensors_[i].get_system_id()) {
                err = true;
                break;
            }
        }
    }
    if(err) {
        // Модуль не добавляется (или имеется или ошибка ID)
        if(data != nullptr)
            delete[] data;
        return true;
    }

    switch (data[symbol++]) {
    case NUM_SENSOR: {
        // Добавление датчика
        Grow_sensor new_sensor(len-1, &data[symbol++]);
        new_sensor.set_system_id(module_id);
        reg_sensors_.push_back(new_sensor);
        break;
    }
    case NUM_DEVICE: {
        // Добавление устройства
        Grow_device new_device(len-1, &data[symbol++]);
        new_device.set_system_id(module_id);
        reg_devices_.push_back(new_device);
        break;
    }
    default:
        // Неизвестный модуль
        if(data != nullptr)
            delete[] data;
        return true;
    }
    // Успешное добавление модуля
    if(data != nullptr)
        delete[] data;
    return false;

    // uint8_t size = 0;
    // // Обработка пакета
    // if(reg_packet.type_packet != Packet_Type::SYSTEM)
    //     return true;
    // static_cast<Packet_System*>(reg_packet.packet)->get_size_by_packet(&size);
    // uint8_t *data = nullptr;
    // if(size != 0)
    //     data = new uint8_t[size];
    // uint8_t com = 0xFF;
    // uint8_t len = 0xFF;
    // static_cast<Packet_System*>(reg_packet.packet)->get_packet_data(&com, data, &len);
    // if ((com != 0x00) && ((Packet_Type)data[0] != Packet_Type::SYSTEM)){
    //     if(data != nullptr)
    //         delete[] data;
    //     return true;
    // }
    // bool err = false;
    // uint8_t symbol = 0;
    // // Проверка ID
    // // (-) ----- превратить в get_id()
    // uint32_t module_id = data[symbol++];
    // module_id = (module_id << 8) | data[symbol++];
    // module_id = (module_id << 8) | data[symbol++];
    // module_id = (module_id << 8) | data[symbol++];
    // if(module_id == 0)  
    //     err = true;
    // // Проверка наличия ID в списке запросивших
    // if(!err) {
    //     for(int i = 0; i < reg_devices_.size(); ++i) {
    //         if(module_id == reg_devices_[i].get_system_id()) {
    //             err = true;
    //             break;
    //         }
    //     }
    // }
    // if(!err) {
    //     for(int i = 0; i < reg_sensors_.size(); ++i) {
    //         if(module_id == reg_sensors_[i].get_system_id()) {
    //             err = true;
    //             break;
    //         }
    //     }
    // }
    // if(err) {
    //     // Модуль не добавляется (или имеется или ошибка ID)
    //     if(data != nullptr)
    //         delete[] data;
    //     return true;
    // }
    // switch (data[symbol++]) {
    // case NUM_SENSOR: {
    //     // Добавление датчика
    //     Grow_sensor new_sensor(len-1, &data[symbol++]);
    //     new_sensor.set_system_id(module_id);
    //     reg_sensors_.push_back(new_sensor);
    //     break;
    // }
    // case NUM_DEVICE: {
    //     // Добавление устройства
    //     Grow_device new_device(len-1, &data[symbol++]);
    //     new_device.set_system_id(module_id);
    //     reg_devices_.push_back(new_device);
    //     break;
    // }
    // default:
    //     // Неизвестный модуль
    //     if(data != nullptr)
    //         delete[] data;
    //     return true;
    // }
    // // Успешное добавление модуля
    // if(data != nullptr)
    //     delete[] data;
    // return false;
}


void Group_control_module::clear_regist_data() {
    reg_devices_.clear();
    reg_sensors_.clear();
    filter_adr_.clear();
}

// Обработка модулей
bool Group_control_module::check_device_period() {
    check_device_ = false;
    get_data_time();
    for(int i = 0; i < devices_.size(); ++i) {
        if(devices_[i].check_time(data_time_))
            check_device_ = true;
    }
    // save_v2 (-) -----
    return check_device_;
}
bool Group_control_module::check_sensor_read() {
    unsigned long time = millis();
    check_sensor_ = false;
    for(int i = 0; i < sensors_.size(); ++i) {
        if (sensors_[i].check_time(time)) {
            check_sensor_ = true;
            sensors_[i].update();
        }
    }
    // save_v2 (-) -----
    return check_sensor_;
}

bool Group_control_module::handler_devices() {
    if(!check_sensor_read())
        return true;
    // Произвести чтение с устройств
    for(int i = 0; i < devices_.size(); ++i) {
        if (devices_[i].read_signal(true)) {
            LoRa_address connect_adr = devices_[i].get_address();
            contact_data_.init_contact(connect_adr);
            if(contact_data_.start_transfer())
                Serial.println("Error start sensor transfer");
            break;
        }
    }
    return false;
    // if(!check_device_period())
    //     return true;
    // // Произвести переключение исполнительных устройств
    // for(int i = 0; i < devices_.size(); ++i) {
    //     bool result = devices_[i].get_state_change();
    //     if (result) {
    //         LoRa_address connect_adr = devices_[i].get_address();
    //         // инициализация
    //         contact_data_.init_contact(connect_adr);
    //         // создание пакетов обмена
    //         for(int j = 0; j < devices_[i].get_count_component(); ++j) {
    //             int8_t change_state;
    //             uint16_t device_value;
    //             Exchange_packet packet;
    //             uint8_t size = 0;
    //             uint8_t com = 1; // установить значение
    //             if (devices_[i].get_state_change(j, change_state)) {
    //                 if(change_state == -1) {
    //                     device_value = 0;
    //                 }
    //                 else {
    //                     devices_[i].get_value(j, device_value);
    //                 }

    //                 uint8_t obj = devices_[i].get_component(j).get_type();
    //                 uint8_t id = devices_[i].get_component(j).get_id();
    //                 static_cast<Packet_Device*>(packet.packet)->get_size_by_data(&obj, &com, &size);
    //                 uint8_t *data = new uint8_t[size];
    //                 if(size == 1) { 
    //                     data[0] = device_value & 0xFF;
    //                 }
    //                 else {
    //                     data[0] = (device_value >> 8) & 0xFF;
    //                     data[1] = device_value & 0xFF;
    //                 }
    //                 packet.creat_packet(size + 11, PACKET_DEVICE);
    //                 packet.packet->set_dest_adr(connect_adr);
    //                 packet.packet->set_sour_adr(contact_data_.get_my_adr());
    //                 packet.packet->set_packet_type(PACKET_DEVICE);
    //                 static_cast<Packet_Device*>(packet.packet)->set_setting(devices_[i].get_setting());
    //                 static_cast<Packet_Device*>(packet.packet)->set_packet_data(&obj, &id, &com, data, nullptr);
    //                 contact_data_.add_packet(packet);
    //                 delete[] data;
    //             }
    //         }
    //         // старт передачи
    //         if(contact_data_.start_transfer())
    //             Serial.println("Error start device transfer");
    //         devices_[i].check_time(true); // (!) ----- Обнуляем вне зависимости от того, дошло ли сообщение, возвращаем при ошибке
    //         break;
    //     }
    // }
    // return false;
}
bool Group_control_module::handler_sensors() {
    if(!check_sensor_read())
        return true;
    // Произвести чтение с датчиков
    for(int i = 0; i < sensors_.size(); ++i) {
        if (sensors_[i].read_signal(true)) {
            LoRa_address connect_adr = sensors_[i].get_address();
            contact_data_.init_contact(connect_adr);
            if(contact_data_.start_transfer())
                Serial.println("Error start sensor transfer");
            break;
        }
    }
    return false;
}