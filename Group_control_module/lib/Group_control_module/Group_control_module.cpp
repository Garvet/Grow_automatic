#include "Group_control_module.h"

#define NUM_SENSOR 0x01
#define NUM_DEVICE 0x02

#define PRINT_RESULT_TRANSFER

uint32_t amt_error_connect_s = 0;
uint32_t amt_error_connect_num_s[6] = {0, 0, 0, 0, 0, 0};
uint32_t amt_error_connect_d = 0;
uint32_t amt_error_connect_num_d[6] = {0, 0, 0, 0, 0, 0};
// - 2021.06.11 - перезапуск при отсутствии соединения -
#if defined (ERROR_REBOOT)
static const uint16_t MAX_ERRORS_ONE_COMPONENT = 3;
uint32_t amt_error_connect_last_num_s[6] = {0, 0, 0, 0, 0, 0};
uint32_t amt_error_connect_last_num_d[6] = {0, 0, 0, 0, 0, 0};
#endif
// -----------------------------------------------------

#if defined (BUILD_TESTING_CODE_409)
#define VERSION_MODULE 1
extern uint32_t led1_time;
extern uint32_t led2_time;
extern uint32_t led3_time;
extern bool led1_state;
extern bool led2_state;
extern bool led3_state;
#endif

// - 2021.06.11 - перезапуск при отсутствии соединения -
#if defined (ERROR_REBOOT)
#ifndef MAX_ERRORS_IN_ROW
static const uint16_t MAX_ERRORS_IN_ROW = 9; // MAX_... >= amt_component
#endif
static uint16_t number_errors_row = 0;
#endif
// -----------------------------------------------------


enum class Purpose_contact {
    noname = 0,
    get_data,
    set_state,
    set_sync_rtc,

    set_server_value // (-) ----- (!) ----- КОСТЫЛЬ

    /* data */
};
Purpose_contact last_purpose_contact = Purpose_contact::noname;


volatile DRAM_ATTR bool lora_interrupt_flag = false;
// (!) ----- Переделать в нормальный вид (pin -> тот или иной GT)
static Group_control_module* group_control_module_interrupt;
void IRAM_ATTR lora_interrupt() {
    lora_interrupt_flag = true;
    // group_control_module_interrupt->LoRa_interrupt();
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
    date_time_ = 0;
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
    static bool in_work_system = false;
    if(in_work_system)
        return 0;
    in_work_system = true;
    switch (mode_) {
    case GT_SETTING:
        // (?) ----- а есть ли здесь хоть что-то?
        break;
    case GT_PROCESSING:
        // if(contact_data_.get_signal_complete()) {
        if(contact_data_.get_state_contact() == SC_DOWNTIME) {
#if defined (BUILD_TESTING_CODE_409)
                                                            // Если тестовый модуль, то не начинаем контакт
                                                            break;
#endif
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
    in_work_system = false;
    return 0;
}
static float value = 0.0;
static float* p_value = nullptr;
void Group_control_module::LoRa_interrupt() {
    // (контролирует каждый шаг передачи, по завершению запускает work_system, контролирует трансляцию, заносит запросы регистрации)
    // (-) -----
    bool add_err = false;
    static bool in_interrupt = false;
    if(in_interrupt)
        return;
    in_interrupt = true;
    uint32_t error = contact_data_.work_contact_system();
    if(error != 0) {
        if(mode_ == GT_SETTING) {
            LoRa_address adr = contact_data_.get_connect_adr();
            for(int i = 0; i < devices_.size(); ++i) {
                if(devices_[i].get_address() == adr) { // (?) ----- adr.branch
                    devices_[i].set_active(0);
                    Serial.print("Add contact error = ");
                    Serial.println(error);
                    add_err = true;
                    break;
                }
            }
            for(int i = 0; i < sensors_.size(); ++i) {
                if(sensors_[i].get_address() == adr) { // adr.branch
                    sensors_[i].set_active(0);
                    Serial.print("Add contact error = ");
                    Serial.println(error);
                    add_err = true;
                    break;
                }
            }
        }
        else {
            bool this_device = false;
            LoRa_address adr = contact_data_.get_connect_adr();

            for(int i = 0; i < devices_.size(); ++i) {
                if(devices_[i].get_address() == adr) { // adr.branch
                    ++amt_error_connect_d; // (?) -----

#if defined (BUILD_TESTING_CODE_409)
                                                            // Если тестовый модуль, то включаем диод
                                                            led2_state = false;
                                                            led3_state = true;
                                                            led3_time = millis();
#endif
                    amt_error_connect_num_d[i] += 1;
                    // - 2021.06.11 - перезапуск при отсутствии соединения -
                    #if defined (ERROR_REBOOT)
                    amt_error_connect_last_num_d[i] += 1;
                    if(MAX_ERRORS_ONE_COMPONENT < amt_error_connect_last_num_d[i]) {
                        RtcDateTime rdt = get_date_time();
                        uint16_t data;
                        Serial.println("\n| ----- -----");
                        Serial.println("| ------- ----- -----");
                        Serial.print("| ----- ");
                        data = rdt.Year(); Serial.print(data); Serial.print("/");
                        data = rdt.Month(); Serial.print(data); Serial.print("/");
                        data = rdt.Day(); Serial.print(data); Serial.print("  ");
                        data = rdt.Hour(); Serial.print(data); Serial.print(":");
                        data = rdt.Minute(); Serial.print(data); Serial.print(":");
                        data = rdt.Second(); Serial.print(data); Serial.print(" - {amt error device[");
                        Serial.print(i); Serial.println("] reboot}");
                        Serial.println("| ------- ----- -----");
                        Serial.println("| ----- -----\n");
                        esp_restart();
                    }
                    #endif
                    // -----------------------------------------------------
                    this_device = true;
                    break;
                }
            }
            if(!this_device) {
                for(int i = 0; i < sensors_.size(); ++i) {
                    if(sensors_[i].get_address() == adr) { // adr.branch
                        ++amt_error_connect_s; // (?) -----
                        amt_error_connect_num_s[i] += 1;
                        // - 2021.06.11 - перезапуск при отсутствии соединения -
                        #if defined (ERROR_REBOOT)
                        amt_error_connect_last_num_s[i] += 1;
                        if(MAX_ERRORS_ONE_COMPONENT < amt_error_connect_last_num_s[i]) {
                            RtcDateTime rdt = get_date_time();
                            uint16_t data;
                            Serial.println("\n| ----- -----");
                            Serial.println("| ------- ----- -----");
                            Serial.print("| ----- ");
                            data = rdt.Year(); Serial.print(data); Serial.print("/");
                            data = rdt.Month(); Serial.print(data); Serial.print("/");
                            data = rdt.Day(); Serial.print(data); Serial.print("  ");
                            data = rdt.Hour(); Serial.print(data); Serial.print(":");
                            data = rdt.Minute(); Serial.print(data); Serial.print(":");
                            data = rdt.Second(); Serial.print(data); Serial.print(" - {amt error sensor[");
                            Serial.print(i); Serial.println("] reboot}");
                            Serial.println("| ------- ----- -----");
                            Serial.println("| ----- -----\n");
                            esp_restart();
                        }
                        #endif
                        // -----------------------------------------------------
                        break;
                    }
                }
            }

            // - 2021.06.11 - перезапуск при отсутствии соединения -
            #if defined (ERROR_REBOOT)
            if(++number_errors_row == MAX_ERRORS_IN_ROW) {
                RtcDateTime rdt = get_date_time();
                uint16_t data;
                Serial.println("\n| ----- -----");
                Serial.println("| ------- ----- -----");
                Serial.print("| ----- ");
                data = rdt.Year(); Serial.print(data); Serial.print("/");
                data = rdt.Month(); Serial.print(data); Serial.print("/");
                data = rdt.Day(); Serial.print(data); Serial.print("  ");
                data = rdt.Hour(); Serial.print(data); Serial.print(":");
                data = rdt.Minute(); Serial.print(data); Serial.print(":");
                data = rdt.Second(); Serial.print(data); Serial.println(" - {LoRa error reboot}");
                Serial.println("| ------- ----- -----");
                Serial.println("| ----- -----\n");
                esp_restart();
            }
            #endif
            // -----------------------------------------------------

#if defined( PRINT_RESULT_TRANSFER )
                                                                                                                    LoRa_address print_adr = contact_data_.get_connect_adr();
                                                                                                                    Serial.print("complete check = {");
                                                                                                                    Serial.print(print_adr.group);
                                                                                                                    Serial.print(",");
                                                                                                                    Serial.print(print_adr.branch);
                                                                                                                    Serial.println("}");
                                                                                                                    Serial.print("Err: ");
                                                                                                                    Serial.println(error);
                                                                                                                    Serial.println();
#endif
        }
        // (-) ----- обработка ошибок, сохранение их для дальнейшей выдачи
    }
    if((error == 0) || add_err) {
        switch (mode_) {
        case GT_SETTING: {
            // (-) ----- (?) -----
            if(permission_regist_interrupt_) {
                if(contact_data_.get_state_contact() == SC_PACKET_ACCEPTED) {
                                                                                                    Serial.println("XXX");
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
                                                                                                    Serial.println("YYY");
                    LoRa_address adr = contact_data_.get_connect_adr();
                    for(int i = 0; i < devices_.size(); ++i) {
                        if(devices_[i].get_address() == adr) // adr.branch
                            devices_[i].set_active(2);
                    }
                    for(int i = 0; i < sensors_.size(); ++i) {
                        if(sensors_[i].get_address() == adr) // adr.branch
                            sensors_[i].set_active(2);
                    }

                    // всё зарегистрировано
                    bool not_all_reg = false;
                    for(int i = 0; i < devices_.size(); ++i) {
                        if(devices_[i].get_active() != 2) {
                            not_all_reg = true;
                            break;
                        }
                    }
                    if(!not_all_reg)
                        for(int i = 0; i < sensors_.size(); ++i) {
                            if(sensors_[i].get_active() != 2) {
                                not_all_reg = true;
                                break;
                            }
                        }
                    if (!not_all_reg) {
                        contact_data_.end_contact();
                        set_mode(Mode::GT_PROCESSING);
                    }

                }

                if(contact_data_.get_signal_complete() || contact_data_.get_state_contact() == SC_WAITING || contact_data_.get_state_contact() == SC_DOWNTIME) {
                    bool check_reg = false;
                    LoRa_address regist_adr = {contact_data_.get_my_adr().group, 0};
                    for(int i = 0; i < devices_.size(); ++i) {
                        if(devices_[i].get_active() == 1) {
                            regist_adr.branch = devices_[i].get_address().branch; // adr.branch
                            check_reg = true;
                            break;
                        }
                    }
                    if(!check_reg) {
                        for(int i = 0; i < sensors_.size(); ++i) {
                            if(sensors_[i].get_active() == 1) {
                                                                                                    Serial.println("ZZZ");
                                regist_adr.branch = sensors_[i].get_address().branch; // adr.branch
                                check_reg = true;
                                break;
                            }
                        }
                    }
                    if(check_reg) {
                                                                                                    Serial.println("ZZZ");
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
                                                            uint8_t com = 0x02;
                                                            uint8_t len = 2;
                                                            packet_system.get_size_by_data(&com, &len, size);
                                                            // static_cast<Packet_System*>(packet.packet)->get_size_by_data(&size, &com, &len);
                                                            uint8_t data[2];
                                                            // packet.creat_packet(size + 10, Packet_Type::SYSTEM);
                                                            packet_system.set_dest_adr(packet, regist_adr);
                                                            packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                                                            packet_system.set_packet_type(packet, Packet_Type::SYSTEM);

                                                            data[0] = (contact_data_.get_channel() >> 8) & 0xFF;
                                                            data[1] = contact_data_.get_channel() & 0xFF;
                                                            // packet.packet->set_dest_adr(regist_adr);
                                                            // packet.packet->set_sour_adr(contact_data_.get_my_adr());
                                                            // packet.packet->set_packet_type(Packet_Type::SYSTEM);
                                                            // static_cast<Packet_System*>(packet.packet)->set_packet_data(&com, data, &len);
                                                            packet_system.set_packet_data(packet, &com, data, &len);

                                                            contact_data_.init_contact(regist_adr);
                                                            contact_data_.add_packet(packet);
                                                            contact_data_.start_transfer();
                                                        }
                // data[num_byte++] = (contact_data_.get_channel() >> 8) & 0xFF; // (-) ----- channel со стороны STM!!!
                // data[num_byte++] = (contact_data_.get_channel()     ) & 0xFF;

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
                // - 2021.06.11 - перезапуск при отсутствии соединения -
                #if defined (ERROR_REBOOT)
                number_errors_row = 0;
                #endif
                // -----------------------------------------------------
                                                                                                    {
#if defined( PRINT_RESULT_TRANSFER )
                                                                                                        LoRa_address print_adr = contact_data_.get_connect_adr();
                                                                                                        Serial.print("complete check = {");
                                                                                                        Serial.print(print_adr.group);
                                                                                                        Serial.print(",");
                                                                                                        Serial.print(print_adr.branch);
                                                                                                        Serial.println("}");
                                                                                                        Serial.print("check sensor complete (amt error = ");
                                                                                                        Serial.print(amt_error_connect_s);
                                                                                                        Serial.print(") [");
                                                                                                        for(int i = 0; i < sensors_.size(); ++i) {
                                                                                                            Serial.print("err№");
                                                                                                            Serial.print(i+1);
                                                                                                            Serial.print(" = ");
                                                                                                            Serial.print(amt_error_connect_num_s[i]);
                                                                                                            if(i < sensors_.size() - 1)
                                                                                                                Serial.print(";  ");
                                                                                                        }

                                                                                                        Serial.println("]");
                                                                                                        Serial.print("check device complete (amt error = ");
                                                                                                        Serial.print(amt_error_connect_d);
                                                                                                        Serial.print(") [");
                                                                                                        for(int i = 0; i < devices_.size(); ++i) {
                                                                                                            Serial.print("err№");
                                                                                                            Serial.print(i+1);
                                                                                                            Serial.print(" = ");
                                                                                                            Serial.print(amt_error_connect_num_d[i]);
                                                                                                            if(i < devices_.size() - 1)
                                                                                                                Serial.print(";  ");
                                                                                                        }
                                                                                                        Serial.println("]");
                                                                                                        Serial.println();
#endif
                                                                                                        // - 2021.06.11 - перезапуск при отсутствии соединения -
                                                                                                        #if defined (ERROR_REBOOT)
                                                                                                        Serial.print("check last sensor complete            [");
                                                                                                        for(int i = 0; i < sensors_.size(); ++i) {
                                                                                                            Serial.print("err№");
                                                                                                            Serial.print(i+1);
                                                                                                            Serial.print(" = ");
                                                                                                            Serial.print(amt_error_connect_last_num_s[i]);
                                                                                                            if(i < sensors_.size() - 1)
                                                                                                                Serial.print(";  ");
                                                                                                        }

                                                                                                        Serial.println("]");
                                                                                                        Serial.print("check last device complete            [");
                                                                                                        for(int i = 0; i < devices_.size(); ++i) {
                                                                                                            Serial.print("err№");
                                                                                                            Serial.print(i+1);
                                                                                                            Serial.print(" = ");
                                                                                                            Serial.print(amt_error_connect_last_num_d[i]);
                                                                                                            if(i < devices_.size() - 1)
                                                                                                                Serial.print(";  ");
                                                                                                        }
                                                                                                        Serial.println("]");
                                                                                                        #endif
                                                                                                    }
                                                                                                    // -----------------------------------------------------
#if defined (BUILD_TESTING_CODE_409)
                                                            // Если тестовый модуль, то включаем диод
                                                            led2_state = false;
                                                            led1_state = true;
                                                            led1_time = millis();
                                                            // и выключаемся
                                                            contact_data_.end_contact();
                                                            in_interrupt = false;
                                                            return;
#endif
                // (-) -----
                // Обработать показания


                bool this_device = false;
                bool this_sensor = false;
                LoRa_address adr = contact_data_.get_connect_adr();
                LoRa_packet packet_processing;
                std::vector<LoRa_packet> packets = contact_data_.get_all_packet();
                size_t num_component = 0;
                for(int i = 0; i < devices_.size(); ++i) {
                    if(devices_[i].get_address() == adr) { // adr.branch
                        this_device = true;
                        num_component = i;
                        // - 2021.06.11 - перезапуск при отсутствии соединения -
                        #if defined (ERROR_REBOOT)
                        amt_error_connect_last_num_d[i] = 0;
                        #endif
                        // -----------------------------------------------------
                        break;
                    }
                }
                if(!this_device) {
                    for(int i = 0; i < sensors_.size(); ++i) {
                        if(sensors_[i].get_address() == adr) { // adr.branch
                            this_sensor = true;
                            num_component = i;
                            // - 2021.06.11 - перезапуск при отсутствии соединения -
                            #if defined (ERROR_REBOOT)
                            amt_error_connect_last_num_s[i] = 0;
                            #endif
                            // -----------------------------------------------------
                            break;
                        }
                    }
                }
                if(this_device) {

                    if(last_purpose_contact == Purpose_contact::set_state) {
                        // (packets.size() == 0) && (devices_[num_component].get_state___() == scs::State::from_work_to_stop || devices_[num_component].get_state___() == scs::State::from_stop_to_work)) {
                        
                        // set state
                        Serial.print("Device {");
                        Serial.print(adr.group, 16);
                        Serial.print("|");
                        Serial.print(adr.branch, 16);
                        Serial.print("} set mode ");

                        if(devices_[num_component].get_state___() == scs::State::from_work_to_stop) {
                            devices_[num_component].set_state___(scs::State::stop);
                            Serial.println("'stop'");
                        }
                        else if (devices_[num_component].get_state___() == scs::State::from_stop_to_work) {
                            devices_[num_component].set_state___(scs::State::work);
                            Serial.println("'work'");
                        }
                        else {
                            devices_[num_component].set_state___(scs::State::from_work_to_stop);
                            Serial.println("'set stop' (error set state → stop device)");
                        }
                        last_purpose_contact = Purpose_contact::noname;
                    }
                    else if(last_purpose_contact == Purpose_contact::set_server_value){
                        devices_[num_component].clear_send_server_value();
                        last_purpose_contact = Purpose_contact::noname;
                    }
                    else if(last_purpose_contact == Purpose_contact::set_sync_rtc){
                        devices_[num_component].set_rtc_sync(false);
                        last_purpose_contact = Purpose_contact::noname;
                    }
                    else {
                        if(last_purpose_contact == Purpose_contact::get_data) {
                            // Serial.println("Read devices data:"); // (?) ----- WTF?
                        }
                        for(int i = 0; i < packets.size(); ++i) {
                            // packet_processing.set_packet(packets[i]);
                            // if(packet_processing.packet->get_sour_adr() != contact_data_.get_connect_adr())
                            if(packet_analyzer.get_sour_adr(packets[i]) != contact_data_.get_connect_adr())
                                continue; // (-) ----- неправильная обработка (?) ----- возврат пакета обратно, можно запомнить i

                            //                           (!) ----- (-) ----- (-) ----- (-) ----- (!) ----- interface
                            if(packet_analyzer.get_packet_type(packets[i]) == Packet_Type::DEVICE) {
                                uint8_t size = 0;
                                uint8_t id = 0;
                                uint8_t com = 0;
                                uint8_t obj = 0;
                                uint16_t data = 0;
                                uint8_t len = 0;

                                packet_device.set_setting(devices_[num_component].get_setting());
                                packet_device.get_size_by_packet(packets[i], &obj, size);

                                packet_device.get_packet_data(packets[i], &obj, &id, &com, (uint8_t*)&data, &len);
                                if(obj == 0x06) { // rtc(!) -----
                                    if(com == 0x00) // get rtc time (!) -----
                                        devices_[num_component].set_rtc_sync(true);
                                    else {
                                        Serial.print("Error, recieve device[] packet: rtc type = ");
                                        Serial.print(num_component);
                                        Serial.print("] packet: rtc com = ");
                                        Serial.println(com);
                                    }
                                }
                                else {
                                    for(int k = 0; k < devices_[num_component].get_count_component(); ++k) {
                                        if(devices_[num_component].get_component(k).get_type() == obj)
                                            if(devices_[num_component].get_component(k).get_id() == id) {
                                                if((obj == Signal_digital) || (obj == Pumping_system) || (obj == Phytolamp_digital)) {
                                                    devices_[num_component].set_value(k, data);
                                                                                                                        // Serial.print("Device data T[");
                                                                                                                        // Serial.print(obj);
                                                                                                                        // Serial.print("] id№");
                                                                                                                        // Serial.print(id);
                                                                                                                        // Serial.print(" : ");
                                                                                                                        // Serial.println(data);
                                                }
                                                else if((obj == Signal_PWM) || (obj == Fan_PWM) || (obj == Phytolamp_PWM)){
                                                    devices_[num_component].set_value(k, data);
                                                                                                                        // Serial.print("Device data T[");
                                                                                                                        // Serial.print(obj);
                                                                                                                        // Serial.print("] id№");
                                                                                                                        // Serial.print(id);
                                                                                                                        // Serial.print(" : ");
                                                                                                                        // Serial.println(data);
                                                }
                                                break;
                                            }
                                    }
                                }
                                // packet_sensor.set_setting(sensors_[num_component].get_setting());
                                // packet_sensor.get_size_by_packet(packets[i], &amt, nullptr, size);
                                // if(amt == 0)
                                //     continue;
                                



                                // {-} ----- прочитать пакеты от устройств ----- {-}
                                // uint8_t amt = 0;
                                // uint8_t size = 0;
                                // uint8_t id[3];
                                // uint8_t param[3];
                                // uint32_t data[3];
                                // for(int j = 0; j < 3; ++j)
                                //     id[j] = param[j] = data[j] = 0;
                                // packet_sensor.set_setting(sensors_[num_component].get_setting());
                                // packet_sensor.get_size_by_packet(packets[i], &amt, nullptr, size);
                                // if(amt == 0)
                                //     continue;
                                // packet_sensor.get_packet_data(packets[i], &amt, param, id, data);
                                // for(int j = 0; j < amt; ++j) {
                                //     for(int k = 0; k < sensors_[num_component].get_count_component(); ++k) {
                                //         if(sensors_[num_component].get_component(k).get_type() == param[j])
                                //             if(sensors_[num_component].get_component(k).get_id() == id[j]) {
                                //                 // у 0x00 тип uint16_t = 0-4095, у 0x01 тип bool
                                //                 if(param[j] == 0x00 || (param[j] == 0x01)) {
                                //                     value = data[j];
                                //                     sensors_[num_component].set_value(k, value);
                                //                 }
                                //                 else {
                                //                     sensors_[num_component].set_value(k, data[j]);
                                //                                                                                         // float g_value;
                                //                                                                                         // Serial.print("Sensor data T[");
                                //                                                                                         // Serial.print(param[j]);
                                //                                                                                         // Serial.print("] id№");
                                //                                                                                         // Serial.print(id[j]);
                                //                                                                                         // Serial.print(" : ");
                                //                                                                                         // sensors_[num_sensor].get_value(k, g_value);
                                //                                                                                         // Serial.println(g_value);
                                //                 }
                                //                 break;
                                //             }
                                //     }
                                // }

                            }
                            else {
                                // packet not devices?
                            }
                        }
#if defined( PRINT_RESULT_TRANSFER )
                        if(last_purpose_contact == Purpose_contact::get_data) {
                            Serial.println();
                        }
#endif
                    }
                } 
                else if(this_sensor) {
                    for(int i = 0; i < packets.size(); ++i) {
                        // packet_processing.set_packet(packets[i]);
                        // if(packet_processing.packet->get_sour_adr() != contact_data_.get_connect_adr())
                        if(packet_analyzer.get_sour_adr(packets[i]) != contact_data_.get_connect_adr())
                            continue; // (-) ----- неправильная обработка (?) ----- возврат пакета обратно, можно запомнить i

                        //                           (!) ----- (-) ----- (-) ----- (-) ----- (!) ----- interface
                        if(packet_analyzer.get_packet_type(packets[i]) == Packet_Type::SENSOR) {
                            uint8_t amt = 0;
                            uint8_t size = 0;
                            uint8_t id[10];
                            uint8_t param[10];
                            uint32_t data[10];
                            for(int j = 0; j < 10; ++j)
                                id[j] = param[j] = data[j] = 0;

                            packet_sensor.set_setting(sensors_[num_component].get_setting());
                            packet_sensor.get_size_by_packet(packets[i], &amt, nullptr, size);
                            if(amt == 0)
                                continue;


                            packet_sensor.get_packet_data(packets[i], &amt, param, id, data);
                            for(int j = 0; j < amt; ++j) {
                                for(int k = 0; k < sensors_[num_component].get_count_component(); ++k) {
                                    if(sensors_[num_component].get_component(k).get_type() == param[j])
                                        if(sensors_[num_component].get_component(k).get_id() == id[j]) {
                                            // у 0x00 тип uint16_t = 0-4095, у 0x01 тип bool
                                            if(param[j] == 0x00 || (param[j] == 0x01)) {
                                                value = data[j];
                                                sensors_[num_component].set_value(k, value);
#if defined( PRINT_RESULT_TRANSFER )
                                                                                                                    Serial.print("Sensor data T[");
                                                                                                                    Serial.print(param[j]);
                                                                                                                    Serial.print("] id№");
                                                                                                                    Serial.print(id[j]);
                                                                                                                    Serial.print(" : ");
                                                                                                                    Serial.println(value);
#endif
                                            }
                                            else {
                                                sensors_[num_component].set_value(k, data[j]);
#if defined( PRINT_RESULT_TRANSFER )
                                                                                                                    float g_value;
                                                                                                                    Serial.print("Sensor data T[");
                                                                                                                    Serial.print(param[j]);
                                                                                                                    Serial.print("] id№");
                                                                                                                    Serial.print(id[j]);
                                                                                                                    Serial.print(" : ");
                                                                                                                    sensors_[num_component].get_value(k, g_value);
                                                                                                                    Serial.println(g_value);
#endif
                                            }
                                            break;
                                        }
                                }
                            }

                        }
                        else {
                            // packet not sensors?
                        }

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
                                                                        Serial.println("!this_HeZeChTO!");
                    // Не устройство или датчик, а ... ?
                }
            }
            // Запустить проверку датчиков/фомул при успешном завершении контакта (-) -----
            work_system(); // проверка встроена
            break;
        }
        default:
            break;
        }
    }
    else {
        contact_data_.end_contact();
    }
    in_interrupt = false;
}

// Обработка времени
void Group_control_module::set_date_time(RtcDateTime date_time) {
    date_time_ = date_time;
    if (rtc_begin_)
        rtc_.SetDateTime(date_time_);
}
RtcDateTime Group_control_module::get_date_time() {
    if (rtc_begin_)
        date_time_ = rtc_.GetDateTime();
    return date_time_;
}

// Поиск номера устройства/датчика по параметру
int Group_control_module::search_device(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> search_id) {
    auto it = std::find_if(devices_.begin(), devices_.end(),
        [&search_id] (const Grow_device& check) {if(check.get_system_id() == search_id) return true; return false;});
    if (it == devices_.end())
        return -1;
    return (int)std::distance( devices_.begin(), it );
}
int Group_control_module::search_sensor(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> search_id) {
    auto it = std::find_if(sensors_.begin(), sensors_.end(),
        [&search_id] (const Grow_sensor& check) {if(check.get_system_id() == search_id) return true; return false;});
    if (it == sensors_.end())
        return -1;
    return (int)std::distance( sensors_.begin(), it );
}
int Group_control_module::search_device(uint16_t address) {
    auto it = std::find_if(devices_.begin(), devices_.end(),
        [address] (const Grow_device& check) {if(check.get_address().branch == address) return true; return false;}); // adr.branch
    if (it == devices_.end())
        return -1;
    return (int)std::distance( devices_.begin(), it );
}
int Group_control_module::search_sensor(uint16_t address) {
    auto it = std::find_if(sensors_.begin(), sensors_.end(),
        [&address](const Grow_sensor& check) {if(check.get_address().branch == address) return true; return false; }); // adr.branch
    if (it == sensors_.end())
        return -1;
    return (int)std::distance( sensors_.begin(), it );
}

int Group_control_module::search_module(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> search_id) {
    int result = search_device(search_id);
    if(result == -1) {
        result = search_sensor(search_id);
        if(result != -1)
            result += 1000;
    }
    return result;
}
int Group_control_module::search_module(uint16_t address) {
    int result = search_device(address);
    if(result == -1) {
        result = search_sensor(address);
        if(result != -1)
            result += 1000;
    }
    return result;
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
        size += grow_device_interface.get_size(devices_[i]);
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
        size += grow_device_interface.get_data(devices_[i], data+size);
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
    address_ = {value, 0};

    value  = ((uint16_t)data[size++]) << 8;
    value |= ((uint16_t)data[size++]);
    contact_data_.set_channel(value);

    len_name = data[size++];
    amt_sensors  = ((uint16_t)data[size++]) << 8;
    amt_sensors |= ((uint16_t)data[size++]);
    amt_devices  = ((uint16_t)data[size++]) << 8;
    amt_devices |= ((uint16_t)data[size++]);

    if(len_name != 0) {
        name_.reserve(len_name);
        name_.resize(len_name);
        for(int i = 0; i < len_name; ++i)
            name_[i] = data[size++];
    }

    if(amt_sensors != 0) {
        sensors_.reserve(amt_sensors);
        sensors_.resize(amt_sensors);
        for(int i = 0; i < amt_sensors; ++i) {
            size += grow_sensor_interface.set_data(sensors_[i], data+size, available_size-size);
            sensors_[i].set_address({address_.group, address++}); // adr.branch
        }
    }

    if(amt_devices != 0) {
        devices_.reserve(amt_devices);
        devices_.resize(amt_devices);
        for(int i = 0; i < amt_devices; ++i) {
            size += grow_device_interface.set_data(devices_[i], data+size, available_size-size);
            devices_[i].set_address({address_.group, address++}); // adr.branch
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
            filter_adr_.push_back(reg_devices_[i].get_system_id());
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
            filter_adr_.push_back(reg_sensors_[i].get_system_id());
            result = true;
        }
    }
    enable_regist_interrupt();
    return result;
}
// [T] Регистрация модуля(ей)
bool Group_control_module::regist_device(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> device_id, uint16_t new_adr) {
    bool err = true;
    if(mode_ != GT_SETTING)
        return err;
    // Поиск в конфигурации
    uint16_t num_device;
    for(int i = 0; i < devices_.size(); ++i) {
        if(new_adr == devices_[i].get_address().branch) { // adr.branch
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
        if(device_id == filter_adr_[i]) {
            filter_adr_.erase(filter_adr_.begin() + i);
            for(int j = 0; j < reg_devices_.size(); ++j) {
                if(device_id == reg_devices_[j].get_system_id()) {
                    reg_devices_.erase(reg_devices_.begin() + j);
                    err = false;
                    break;
                }
            }
            break;
        }
    }
    if(!err) {
        // (-) ----- (!) -----
        // Произвести обмен информацией
                    // LoRa_packet packet;
                    // uint8_t size = 0;
                    // uint8_t com = 0x03;
                    // uint8_t len = 0;
                    // packet_system.get_size_by_data(&com, &len, size);
                    // uint8_t *data = new uint8_t[size];

                    // packet_system.set_dest_adr(packet, {0, old_adr});
                    // packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                    // packet_system.set_packet_type(packet, Packet_Type::SYSTEM);

                    // data[0] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                    // data[1] = (contact_data_.get_my_adr().group << 7) & 0x80;
                    // data[1] |= (new_adr >> 8) & 0x7F;
                    // data[2] = (new_adr) & 0xFF;

                    // packet_system.set_packet_data(packet, &com, data, &len);
                    // delete[] data;
                    // contact_data_.add_packet(packet);
                    // contact_data_.broadcast_send(true);

        devices_[num_device].set_active(1);
    }
    enable_regist_interrupt();
    // save (-) -----
    return err;
}
bool Group_control_module::regist_sensor(std::array<uint8_t, AMT_BYTES_SYSTEM_ID> sensor_id, uint16_t new_adr) {
    bool err = true;
    if(mode_ != GT_SETTING)
        return err;
    // Поиск в конфигурации
    uint16_t num_sensor;
    for(int i = 0; i < sensors_.size(); ++i) {
        if(new_adr == sensors_[i].get_address().branch) { // adr.branch
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
        if(sensor_id == filter_adr_[i]) {
            filter_adr_.erase(filter_adr_.begin() + i);
            for(int j = 0; j < reg_sensors_.size(); ++j) {
                if(sensor_id == reg_sensors_[j].get_system_id()) {
                    reg_sensors_.erase(reg_sensors_.begin() + j);
                    err = false;
                    break;
                }
            }
            break;
        }
    }
    if(!err) {
        // (+) -----
        // Произвести обмен информацией

                LoRa_packet packet;

                uint8_t err = 0x00;
                uint8_t com = 0x01;
                uint8_t len = 0;
                uint8_t data[3 + AMT_BYTES_SYSTEM_ID];
                uint8_t num_byte = 0;

                for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
                    data[num_byte++] = sensor_id[i];
                // data[num_byte++] = (sensor_id >> 24) & 0xFF;
                // data[num_byte++] = (sensor_id >> 16) & 0xFF;
                // data[num_byte++] = (sensor_id >>  8) & 0xFF;
                // data[num_byte++] = (sensor_id)       & 0xFF;

                data[num_byte++] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                data[num_byte++] = (contact_data_.get_my_adr().group << 7) & 0x80
                                 | (new_adr >> 8) & 0x7F;
                data[num_byte++] = (new_adr) & 0xFF;


                err = packet_system.set_dest_adr(packet, LORA_GLOBAL_ADDRESS);
                err = packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                err = packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
                err = packet_system.set_packet_data(packet, &com, data, &len);

                // uint8_t size = 0;
                // uint8_t com = 0x03;
                // uint8_t len = 0;
                // packet_system.get_size_by_data(&com, &len, size);
                // uint8_t *data = new uint8_t[size];
                // packet_system.set_dest_adr(packet, LORA_GLOBAL_ADDRESS);
                // packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                // packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
                // data[0] = (contact_data_.get_my_adr().group >> 1) & 0xFF;
                // data[1] = (contact_data_.get_my_adr().group << 7) & 0x80;
                // data[1] |= (new_adr >> 8) & 0x7F;
                // data[2] = (new_adr) & 0xFF;
                // packet_system.set_packet_data(packet, &com, data, &len);
                // delete[] data;

                contact_data_.add_packet(std::move(packet));
                                            contact_data_.broadcast_send(true);
                                                                    // contact_data_.broadcast_send(false); // true!!!
                                                                    // static bool test_2_sensor = false;
                                                                    // if(test_2_sensor)
                                                                    //     set_mode(Mode::GT_PROCESSING);
                                                                    // else
                                                                    //     test_2_sensor = true;

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
// получение индивидуального номера платы
std::array<uint8_t, AMT_BYTES_SYSTEM_ID> Group_control_module::get_system_id() const {
    return system_id_;
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

    if(packet_system.get_size_by_packet(reg_packet, size) || (size < AMT_BYTES_SYSTEM_ID))
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
    std::array<uint8_t, AMT_BYTES_SYSTEM_ID> module_id;
    for(int i = 0; i < AMT_BYTES_SYSTEM_ID; ++i)
        module_id[i] = data[symbol++];
    // module_id = (module_id << 8) | data[symbol++];
    // module_id = (module_id << 8) | data[symbol++];
    // module_id = (module_id << 8) | data[symbol++];
    // if(module_id == 0)
    //     err = true;

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
        delete[] data;
        return true;
    }
    len = data[symbol++];
    switch (data[symbol++]) {
    case NUM_SENSOR: {
        // Добавление датчика
                                                    Serial.println("!S!");
        Grow_sensor new_sensor(len, &data[symbol++]);
        new_sensor.set_system_id(module_id);
        reg_sensors_.push_back(new_sensor);
        break;
    }
    case NUM_DEVICE: {
        // Добавление устройства
                                                    Serial.println("!D!");
        // Grow_device new_device(len-1, &data[symbol++]);
        Grow_device new_device(len, &data[symbol++]);
        new_device.set_system_id(module_id);
        reg_devices_.push_back(new_device);
        break;
    }
    default:
        // Неизвестный модуль
        delete[] data;
        return true;
    }
    // Успешное добавление модуля
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
    get_date_time();
    for(int i = 0; i < devices_.size(); ++i) {
        if(devices_[i].check_time(date_time_))
            check_device_ = true;
    }
    // save_v2 (-) -----
    return check_device_;
}
bool Group_control_module::check_device_read() {
    unsigned long time = millis();
    check_device_ = false;
    for(int i = 0; i < devices_.size(); ++i) {
        if (devices_[i].check_time(time) && devices_[i].get_state___() == scs::State::work) {
            check_device_ = true;
            devices_[i].update();
        }
    }
    // save_v2 (-) -----
    return check_device_;
}
bool Group_control_module::check_sensor_read() {
    unsigned long time = millis();
    check_sensor_ = false;
    for(int i = 0; i < sensors_.size(); ++i) {
        if (sensors_[i].check_time(time) && sensors_[i].get_state___() == scs::State::work) {
            check_sensor_ = true;
            sensors_[i].update();
        }
    }
    // save_v2 (-) -----
    return check_sensor_;
}

bool Group_control_module::handler_devices() {
                                                                                                                                                                                        // (*)-(*)-(*)-(*)-(*)
                                                                                                                                                                                        // Serial.print("!_! handler_devices: ");
                                                                                                                                                                                        // (*)-(*)-(*)-(*)-(*)
    // Смена режима работы устройства
    {
        // Поиск среди устройств
        int send_state_devices = -1;
        scs::State dev_st;
        for(int i = 0; i < devices_.size(); ++i) {
            if (devices_[i].get_state___() == scs::State::from_work_to_stop ||
                devices_[i].get_state___() == scs::State::from_stop_to_work) {
                send_state_devices = i;
                dev_st = devices_[i].get_state___();
                break;
            }
        }
        // Если устройство найдено
        if(send_state_devices != -1) {
            last_purpose_contact = Purpose_contact::set_state;
            LoRa_address connect_adr = devices_[send_state_devices].get_address(); // not adr.branch
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)
                                                                                                                                                                                            // Serial.println("send_state_devices");
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)

            // (-) ----- создание пакета установки режима
            LoRa_packet packet;
            uint8_t size = 0;
            uint8_t com = 0x05;
            packet_system.get_size_by_data(&com, nullptr, size);
            uint8_t data[1];
            if(dev_st == scs::State::from_work_to_stop) {
                data[0] = static_cast<uint8_t>(scs::Packet_State::stop);
            }
            else if (dev_st == scs::State::from_stop_to_work) {
                data[0] = static_cast<uint8_t>(scs::Packet_State::work);
            }
            else {
                Serial.println("unknown state");
                delay(1500);
                return 1;
            }
            packet_system.set_dest_adr(packet, connect_adr);
            packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
            packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
            packet_system.set_packet_data(packet, &com, data, nullptr);
            contact_data_.init_contact(connect_adr);
            contact_data_.add_packet(packet);
            if(contact_data_.start_transfer())
                Serial.println("Error start transfer set devices state");
                                                                // LoRa_packet packet;
                                                                // uint8_t size = 0;
                                                                // uint8_t com = 0x05;
                                                                // uint8_t len = 2;
                                                                // packet_system.get_size_by_data(&com, &len, size);
                                                                // // static_cast<Packet_System*>(packet.packet)->get_size_by_data(&size, &com, &len);
                                                                // uint8_t data[2];
                                                                // // packet.creat_packet(size + 10, Packet_Type::SYSTEM);
                                                                // packet_system.set_dest_adr(packet, regist_adr);
                                                                // packet_system.set_sour_adr(packet, contact_data_.get_my_adr());
                                                                // packet_system.set_packet_type(packet, Packet_Type::SYSTEM);
                                                                // data[0] = (contact_data_.get_channel() >> 8) & 0xFF;
                                                                // data[1] = contact_data_.get_channel() & 0xFF;
                                                                // // packet.packet->set_dest_adr(regist_adr);
                                                                // // packet.packet->set_sour_adr(contact_data_.get_my_adr());
                                                                // // packet.packet->set_packet_type(Packet_Type::SYSTEM);
                                                                // // static_cast<Packet_System*>(packet.packet)->set_packet_data(&com, data, &len);
                                                                // packet_system.set_packet_data(packet, &com, data, &len);
                                                                // contact_data_.init_contact(regist_adr);
                                                                // contact_data_.add_packet(packet);
                                                                // contact_data_.start_transfer();

            // () ----- создание пакета установки режима

            return false; // false - был контакт
        }
    }

    // Передача значения на устройство
    {
        // Поиск среди устройств
        static bool last_val = false;
        int send_value_devices = -1;
        uint16_t value = 0xFFFF;
        if(last_val) {
            last_val = false;
        }
        else {
            last_val = true;
            for(int i = 0; i < devices_.size(); ++i) {
                if (devices_[i].get_send_server_value() != 0xFFFF) {
                    send_value_devices = i;
                    value = devices_[i].get_send_server_value();
                    break;
                }
            }
        }
        // Если устройство найдено
        if(send_value_devices != -1) {
            last_purpose_contact = Purpose_contact::set_server_value;
            LoRa_address connect_adr = devices_[send_value_devices].get_address(); // not adr.branch

            // (-) ----- создание пакета данных с сервера
            LoRa_packet packet;
            uint8_t size = 0;
            uint8_t obj = 0x00; // PWM signal
            uint8_t com = 0x01; // set value
            uint8_t num = 0;
            packet_device.get_size_by_data(&obj, &com, size);
            uint8_t data[2];
            data[0] = (value >> 8) & 0x0F;
            data[1] = value & 0xFF;

            packet_device.set_setting(devices_[send_value_devices].get_setting());

            packet_device.set_dest_adr(packet, connect_adr);
            packet_device.set_sour_adr(packet, contact_data_.get_my_adr());
            packet_device.set_packet_type(packet, Packet_Type::DEVICE);
            packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
            contact_data_.init_contact(connect_adr);
            contact_data_.add_packet(packet);
            if(contact_data_.start_transfer())
                Serial.println("Error start transfer value on devices");
            // () ----- создание пакета данных с сервера
            return false; // false - был контакт
        }
    }

    // Установка времени на устройстве
    {
        // Поиск среди устройств
        int send_rtc_sync = -1;
        for(int i = 0; i < devices_.size(); ++i) {
            if (devices_[i].get_rtc_sync()) {
                send_rtc_sync = i;
                // dev_st = devices_[i].get_state___();
                break;
            }
        }
    #if defined (BUILD_TESTING_CODE_409)
        send_rtc_sync = -1; // Для тестирования нет необходимости в установке времени
    #endif
        // Если устройство найдено
        if(send_rtc_sync != -1) {
            last_purpose_contact = Purpose_contact::set_sync_rtc;
            LoRa_address connect_adr = devices_[send_rtc_sync].get_address(); // not adr.branch
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)
                                                                                                                                                                                            // Serial.println("send_time_devices");
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)

            // (-) ----- создание пакета установки времени
            LoRa_packet packet;
            uint8_t size = 0;
            uint8_t obj = 0x06; // RTC
            uint8_t com = 0x02; // set time (0x03 set date)
            uint8_t num = 0;
            packet_device.get_size_by_data(&obj, &com, size);
            uint8_t data[3];
            get_date_time();
            data[0] = date_time_.Second();
            data[1] = date_time_.Minute();
            data[2] = date_time_.Hour();

            packet_device.set_setting(devices_[send_rtc_sync].get_setting());

            packet_device.set_dest_adr(packet, connect_adr);
            packet_device.set_sour_adr(packet, contact_data_.get_my_adr());
            packet_device.set_packet_type(packet, Packet_Type::DEVICE);
            packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
            contact_data_.init_contact(connect_adr);
            contact_data_.add_packet(packet);
            if(contact_data_.start_transfer())
                Serial.println("Error start transfer sync devices time");
            // () ----- создание пакета установки времени
            return false; // false - был контакт
        }
    }

    // Установка контроля периодов
    {
        // Поиск среди устройств
        int send_setting_period = -1;
        for(int i = 0; i < devices_.size(); ++i) {
            if (devices_[i].get_setting_change_period()) {
                send_setting_period = i;
                // dev_st = devices_[i].get_state___();
                break;
            }
        }
    #if defined (BUILD_TESTING_CODE_409)
        send_setting_period = -1; // Для тестирования нет необходимости в установке времени
    #endif
        // Если устройство найдено
        send_setting_period = -1; // Для тестирования нет необходимости в установке времени
        if(send_setting_period != -1) {
            last_purpose_contact = Purpose_contact::set_sync_rtc;
            LoRa_address connect_adr = devices_[send_setting_period].get_address(); // not adr.branch
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)
                                                                                                                                                                                            // Serial.println("send_time_devices");
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)

            // (-) ----- создание пакетов установки периодов
            static LoRa_packet packet;
            static uint8_t size, obj, com, num, data[10];
            static uint8_t amount_period, number_period;
            static uint8_t amount_timer, number_timer;
            static uint32_t buf_value;

            contact_data_.init_contact(connect_adr);
            packet_device.set_setting(devices_[send_setting_period].get_setting());
            packet_device.set_dest_adr(packet, connect_adr);
            packet_device.set_sour_adr(packet, contact_data_.get_my_adr());
            packet_device.set_packet_type(packet, Packet_Type::DEVICE);
            // ----- Установка количества периодов -----
            {
                size = 0;
                obj = 0x08;
                com = 0x01;
                num = 0;
                packet_device.get_size_by_data(&obj, &com, size);

                amount_period = 0;
                for(int i = 0; i < devices_[send_setting_period].component_.size(); ++i)
                    amount_period += devices_[send_setting_period].component_[i].get_timer().size();
                data[0] = amount_period;
                // data[0] = X    { X = sum по i (device.component_[i].get_timer().size()) }
                packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
                contact_data_.add_packet(packet);
            }
            // ----- Установка количества каналов времени -----
            {
                size = 0;
                obj = 0x07;
                com = 0x01;
                num = 0;
                packet_device.get_size_by_data(&obj, &com, size);

                amount_timer = 0;
                for(int i = 0; i < devices_[send_setting_period].component_.size(); ++i)
                    amount_timer += devices_[send_setting_period].component_[i].get_timer().size();
                data[0] = amount_timer;
                // data[0] = X    { X = sum по i (device.component_[i].get_timer().size()) }
                packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
                contact_data_.add_packet(packet);
            }
            // ----- Настройка периодов -----
            {
                size = 0;
                obj = 0x08;
                com = 0x03;
                num = 0;
                packet_device.get_size_by_data(&obj, &com, size);
                // for(X)
                //     data[] = ?
                number_period = 0;
                for(int i = 0; i < devices_[send_setting_period].component_.size(); ++i) {
                    // очистка буфера
                    for(int k = 0; k < size; ++k)
                        data[k] = 0;
                    // заполнение новыми данными
                    for(int j = 0; j < devices_[send_setting_period].component_[i].get_timer().size(); ++j) {
                        data[0] = number_period++; // смещаем период, после заполнения
                        data[1] = devices_[send_setting_period].component_[i].get_timer()[j].get_start_seconds();
                        data[2] = devices_[send_setting_period].component_[i].get_timer()[j].get_start_minutes();
                        data[3] = devices_[send_setting_period].component_[i].get_timer()[j].get_start_hours();
                        data[4] = devices_[send_setting_period].component_[i].get_timer()[j].get_end_seconds();
                        data[5] = devices_[send_setting_period].component_[i].get_timer()[j].get_end_minutes();
                        data[6] = devices_[send_setting_period].component_[i].get_timer()[j].get_end_hours();
                        // добавление пакета
                        packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
                        contact_data_.add_packet(packet);
                    }
                }
            }
            // ----- Настройка каналов времени -----
            {
                size = 0;
                obj = 0x07;
                com = 0x03;
                num = 0;
                packet_device.get_size_by_data(&obj, &com, size);
                // for(X)
                //     data[] = ?
                number_timer = 0;
                for(int i = 0; i < devices_[send_setting_period].component_.size(); ++i) {
                    // очистка буфера
                    for(int k = 0; k < size; ++k)
                        data[k] = 0;
                    // заполнение новыми данными
                    for(int j = 0; j < devices_[send_setting_period].component_[i].get_timer().size(); ++j) {
                        data[0] = number_timer++; // смещаем период, после заполнения
                        buf_value = devices_[send_setting_period].component_[i].get_timer()[j].get_channel().get_duration_on();
                        data[1] = (buf_value >> 24) & 0xFF;
                        data[2] = (buf_value >> 16) & 0xFF;
                        data[3] = (buf_value >>  8) & 0xFF;
                        data[4] = (buf_value >>  0) & 0xFF;
                        buf_value = devices_[send_setting_period].component_[i].get_timer()[j].get_channel().get_duration_off();
                        data[5] = (buf_value >> 24) & 0xFF;
                        data[6] = (buf_value >> 16) & 0xFF;
                        data[7] = (buf_value >>  8) & 0xFF;
                        data[8] = (buf_value >>  0) & 0xFF;
                        // добавление пакета
                        packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
                        contact_data_.add_packet(packet);
                    }
                }
            }
            // ----- Установка связей периодов-каналов -----
            // for(X)
            //     data[] = ?
            //     В 0x08 (GrowTimer) - 0x06 (привязка) - data[2] = {i[0,X], i[0,X]}
            // ----- Установка связей каналов-устройств -----
            // for(X)
            //     data[] = ?

            // devices_[i].set_setting_change_period()


            packet_device.get_size_by_data(&obj, &com, size);
            // uint8_t data[3];
            get_date_time();
            data[0] = date_time_.Second();
            data[1] = date_time_.Minute();
            data[2] = date_time_.Hour();

            // packet_device.set_setting(devices_[send_rtc_sync].get_setting());

            packet_device.set_dest_adr(packet, connect_adr);
            packet_device.set_sour_adr(packet, contact_data_.get_my_adr());
            packet_device.set_packet_type(packet, Packet_Type::DEVICE);
            packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
            contact_data_.init_contact(connect_adr);
            contact_data_.add_packet(packet);


            if(contact_data_.start_transfer())
                Serial.println("Error start transfer sync devices time");
            // () ----- создание пакетов установки периодов





            // // (-) ----- создание пакета установки времени
            // LoRa_packet packet;
            // uint8_t size = 0;
            // uint8_t obj = 0x06; // RTC
            // uint8_t com = 0x02; // set time (0x03 set date)
            // uint8_t num = 0;
            // packet_device.get_size_by_data(&obj, &com, size);
            // uint8_t data[3];
            // get_date_time();
            // data[0] = date_time_.Second();
            // data[1] = date_time_.Minute();
            // data[2] = date_time_.Hour();

            // packet_device.set_setting(devices_[send_rtc_sync].get_setting());

            // packet_device.set_dest_adr(packet, connect_adr);
            // packet_device.set_sour_adr(packet, contact_data_.get_my_adr());
            // packet_device.set_packet_type(packet, Packet_Type::DEVICE);
            // packet_device.set_packet_data(packet, &obj, &num, &com, data, &size);
            // contact_data_.init_contact(connect_adr);
            // contact_data_.add_packet(packet);
            // if(contact_data_.start_transfer())
            //     Serial.println("Error start transfer sync devices time");
            // // () ----- создание пакета установки времени
            return false; // false - был контакт
        }
    }


    // Проверка периода считывания устройства
    {
        // Поиск среди устройств
        if(!check_device_read())
            return true; // true - небыло контакта
        // Если устройство найдено
        last_purpose_contact = Purpose_contact::get_data;
        // Произвести чтение с устройств
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)
                                                                                                                                                                                            // Serial.println("read_data_device");
                                                                                                                                                                                            // (*)-(*)-(*)-(*)-(*)
        for(int i = 0; i < devices_.size(); ++i) {
            if (devices_[i].read_signal(true)) {
                LoRa_address connect_adr = contact_data_.get_my_adr();
                connect_adr.branch = devices_[i].get_address().branch; // adr.branch
                contact_data_.init_contact(connect_adr);
                if(contact_data_.start_transfer())
                    Serial.println("Error start devices transfer");
                break;
            }
        }
    }
    return false; // false - был контакт

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
                                                    // Serial.println("check sensor = true");
    // Произвести чтение с датчиков
    for(int i = 0; i < sensors_.size(); ++i) {
                                                                                            // Serial.println("1");
        if (sensors_[i].read_signal(true)) {
                                                                                            // Serial.println("2");
            LoRa_address connect_adr = contact_data_.get_my_adr();
            connect_adr.branch = sensors_[i].get_address().branch; // adr.branch
                                                                                            // Serial.println("3");
            contact_data_.init_contact(connect_adr);
                                                                                            // Serial.println("4");
            if(contact_data_.start_transfer()) {
                                                                                            // Serial.println("5");
                Serial.println("Error start sensor transfer");
            }
                                                                                            // Serial.println("6");
            break;
        }
    }
    return false;
}