#include "LoRa_contact_data.h"

// #define SERIAL_PRINT_ON // вывод стадий при каждом их прохождении (при каждой отправке/приёме)
#define SERIAL_PRINT_ON_1 // вывод только пакетов по мере отправки/приёма
#define ALL_SAVE_PACKET // сохраняет в массив all_packet все принятые и отправленные пакеты
#if defined ( ALL_SAVE_PACKET )
class pack {
public:
    std::vector<uint8_t> packet_data_;
    bool send_;
    bool err_ = false;
    pack(std::vector<uint8_t> packet_data, bool send, bool err = false): packet_data_(packet_data), send_(send), err_(err) {}
    void print() {
        if(send_) {       Serial.print("(O) --> ");
        } else { if(err_) Serial.print("(I|-) < ");
            else          Serial.print("(I|+) < "); }
        Serial.print("[");
        for(int i = 0; i < packet_data_.size(); ++i) {
            if(packet_data_[i] < 16) Serial.print("0");
            Serial.print(packet_data_[i], 16);
            if(i < (packet_data_.size() - 1)) Serial.print(", ");
        }
        Serial.print("] - ");
        Serial.print(packet_data_.size());
    }
    void println() { print(); Serial.println(); }
};
std::vector<pack> all_packet;
void print_all_packet() {
    Serial.println("All packet:");
    for(int i = 0; i < all_packet.size(); ++i) {
        Serial.print("{");
        Serial.print(i);
        Serial.print("} ");
        all_packet[i].println();
    }
}
#endif

//   ----- ----- ----- ---- ----- ----- -----
// ----- ----- -----  Public  ----- ----- -----
//   ----- ----- ----- ---- ----- ----- -----

LoRa_contact_data::LoRa_contact_data() {
    clear();
    // Преобразовать к нормальному виду (!) ----- (-) -----
    /// Время
    // Инициатор
    time_constraints_.I_connect  = 1500;    // Ответа на запрос об установке соединения
    time_constraints_.I_wait_connect = -1;  // Отложенного соединения
    time_constraints_.I_amt_pack = 1500;    // Ответа с количеством принятых пакетов 
    time_constraints_.I_num_pack = 1000;    // Ответа с номерами принятых пакетов
    // Получатель
    time_constraints_.R_connect    = 2500; // Запроса после сброса ожидания (отложенного соединения)
    time_constraints_.R_data_pack  = 2500; // Пакетов данных (после отправляется пакет с количеством пришедших пакетов)
    time_constraints_.R_correct_pack = 2500;// Результатов подтверждения
    time_constraints_.R_disconnect = 1500;  // Разрыва соединения
    /// Количество
    // Инициатор
    packet_constraints_.I_connect  = 10;     // Запрос на установку соединения
    packet_constraints_.I_amt_pack = 10;     // Запрос о количестве принятых пакетов
    packet_constraints_.I_num_pack = 5;     // Запрос номеров принятых пакетов
    // Получатель
    packet_constraints_.R_wait_connect = 10; // Сбросов ожидания (отложенного соединения)
    packet_constraints_.R_correct_pack = 10; // Ответов с количеством принятых пакетов
    
}
LoRa_contact_data::LoRa_contact_data(LoRa_address adr) {
    // (-) ----- добавить стандартный конструктор
    clear();
    set_my_adr(adr);
}
LoRa_contact_data::~LoRa_contact_data() {

}

// Функция инициализации модуля после создания объекта 
bool LoRa_contact_data::init_lora_module(uint8_t pin_reset, uint8_t spi_bus, uint8_t spi_nss, 
                                      uint8_t pin_dio0, uint8_t pin_dio1, uint8_t pin_dio3) {
    if(!lora_.init(pin_reset, spi_bus, spi_nss, pin_dio0, pin_dio1, pin_dio3)) {
        lora_init_ = true;
        return false;
    }
    return true;
}
// Функция запуска работы системы и LoRa-модуля
uint8_t LoRa_contact_data::begin_lora_module(ulong frequency, bool paboost, 
          uint8_t signal_power, uint8_t SF, ulong SBW, uint8_t sync_word) {
    uint8_t err = lora_.begin(frequency, paboost, signal_power, SF, SBW, sync_word);
    if(err == 0) {
        lora_begin_ = true;
    }
    return err;
}

// Установка адреса этого модуля
bool LoRa_contact_data::set_my_adr(LoRa_address adr) {
    // (-) ----- проверка на ошибку
    my_adr_ = adr;
    return false;
}
// Получаение адреса этого модуля
LoRa_address LoRa_contact_data::get_my_adr() {
    return my_adr_;
}
    // Получаение адреса устройства соединения
LoRa_address LoRa_contact_data::get_connect_adr() {
    return connect_adr_;
}

// Установка канала связи
bool LoRa_contact_data::set_channel(uint16_t channel) {
    // (-) ----- скорректировать проверку канала и добавить настройку LoRa-модуля в зависимости от канала
    if((channel < 1) || (channel > 240))
        return true;
    channel_ = channel;
    return false;
}
// Получаение канала связи
uint16_t LoRa_contact_data::get_channel() {
    return channel_;
}


// --- Создание пакетов для обмена информацией ---

bool LoRa_contact_data::add_packet(uint8_t len, uint8_t* packet) {
    // std::vector<uint8_t> v_packet;
    // for(int i = 0; i < len; ++i)
    //     v_packet.push_back(packet[i]);
    return add_packet(LoRa_packet(packet, len));
}
bool LoRa_contact_data::add_packet(const std::vector<uint8_t>& packet) {
    // class Exchange_packet e_packet;
    // e_packet.set_packet(packet);
    // e_packet.set_setting(setting);
    // return add_packet(e_packet);
    return add_packet(LoRa_packet(packet)); 
    // return add_packet(LoRa_packet(packet, len));
}
uint8_t LoRa_contact_data::add_packet(const std::vector<std::vector<uint8_t>>& packet) {
    // if(packet.size() != setting.size())
    //     return 0;
    // class Exchange_packet e_packet;
    for(int i = 0; i < packet.size(); ++i) {
        // e_packet.set_packet(packet[i]);
        // e_packet.set_setting(setting[i]);
        if(add_packet(LoRa_packet(packet[i])))
            return i;
    }
    return packet.size();
}
bool LoRa_contact_data::add_packet(LoRa_packet packet) {

    // if((!_init) || (packet.len < MINIMAL_PACKET_SIZE) || (send_packet_.size() >= 250)) // проверка на инициализацию только в 1 сторону (?) ----- 

    if(init_) {
        // if((packet_analyzer.get_dest_adr_group(packet) != connect_adr_.group) ||
        //    (packet_analyzer.get_dest_adr_branch(packet) != connect_adr_.branch))
        if(packet_analyzer.get_dest_adr(packet)!= connect_adr_)
            return true;
    }
    packet_analyzer.set_packet_number(packet,num_end_packet_++);
    // packet.packet->set_packet_number(num_end_packet_++);

    // packet.packet->set_dest_adr(connect_adr_); (?) -----
    // packet.packet->set_sour_adr(my_adr_);      (?) -----

    send_packet_.push_back(std::move(packet));
    send_flag_.push_back(true); 
    amt_packet_ = send_packet_.size();
    return false;
}
bool LoRa_contact_data::clear_send_packet() {
    if(send_packet_.size() == 0)
        return true;
    num_end_packet_ -= send_packet_.size();
    send_packet_.clear();
    return false;
}



// --- Соединение через установление контакта ---

// Инициализатор
// clear + creat init_packet
bool LoRa_contact_data::init_contact(uint16_t adr_group, uint16_t adr_branch) {
    return init_contact({adr_group, adr_branch});
}
// clear + creat init_packet
bool LoRa_contact_data::init_contact(LoRa_address address) {
    if((address == LORA_GLOBAL_ADDRESS) || (my_adr_ == LORA_GLOBAL_ADDRESS) ||
      (!lora_init_) || (!lora_begin_))
        return true;
    connect_adr_ = address;
    clear();
    create_connect_packet();
    init_ = true;
    return false;
}
bool LoRa_contact_data::start_transfer() {
    if (!init_)
       return true;
    amt_packet_ = 0xFF;
    if(send_packet_.size() != 0) {
        amt_packet_ = send_packet_.size();
        // for(int i = 0; i < amt_packet_; ++i)
        //     send_flag_.push_back(true); (?) ----- почему убрал и куда перенёс?
    }
    uint8_t com = 0x00;
    // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, &amt_packet_, nullptr);
    packet_connection.set_packet_data(last_send_packet_.packet_data, &com, &amt_packet_, nullptr);
    stade_communication_ = SC_CONNECTION;
    set_LoRa_mode_send();
    return false;
}

// Получатель
bool LoRa_contact_data::wait_recipient(LoRa_address address) {
    wait_adr_ = address;
    type_communication_ = TC_RECIPIENT;
    stade_communication_ = SC_CONNECTION;
    connection_ = C_AWAITING_REQUEST;
    set_LoRa_mode_receive();
    // (?) ----- что хотел возвращать и при каких условиях?
    return false;
}



// --- Соединение через трансляции пакетов в сети ---
uint16_t LoRa_contact_data::broadcast_send(bool reply) {
    // (-) ----- Запуск трансляции в сеть (переход в SC_BROADCASTING при завершении в SC_DOWNTIME)
    // clear(); (+) ----- убран т.к. удалял пакеты, готовые к отправке
    if(send_packet_.size() == 0)
        return 1;
    set_LoRa_mode_sleep();
    type_communication_ = TC_BROADCAST;
    if(reply)
        stade_communication_ = SC_REPLY_BROADCAST;
    else
        stade_communication_ = SC_BROADCASTING;
    create_data_packet();
    set_LoRa_mode_send();
    return 0;
}
uint16_t LoRa_contact_data::broadcast_receive(LoRa_address address) {
    // (-) ----- Запуск приёма транслируемых пакетов в сеть (переход в SC_WAITING)
    clear();
    wait_adr_ = address;
    type_communication_ = TC_BROADCAST;
    stade_communication_ = SC_WAITING;
    set_LoRa_mode_receive();
    return 0;
}
uint16_t LoRa_contact_data::complete_broadcast_receive() {
    // (-) ----- Завершение приёма транслируемых пакетов в сеть (переход в SC_COMPLETE с прекращением приёма)
    if(stade_communication_ == SC_WAITING)
        clear();
    else if(stade_communication_ == SC_PACKET_ACCEPTED)
        stade_communication_ = SC_COMPLETE;
    else 
        return 1;
    set_LoRa_mode_sleep();
    return 0;
}


// Получить количество отправляемых/принимаемых пакетов
uint8_t LoRa_contact_data::get_amt_packet() {
    return amt_packet_;
}

// Получить текущую стадию контакта/трансляции
int8_t LoRa_contact_data::get_state_contact() {
    return (int8_t) stade_communication_;
} 

// Изъять пакет из списка принятых (удаляется)
LoRa_packet LoRa_contact_data::get_packet() {
    if((!get_signal_complete()) && (stade_communication_ != SC_PACKET_ACCEPTED))
        return {};
    if(reciev_packet_.size() < 2) {
        if (stade_communication_ == SC_PACKET_ACCEPTED) {
            stade_communication_ = SC_WAITING;
        }
        else {
            stade_communication_ = SC_DOWNTIME;
        }
    }
    if(reciev_packet_.size() == 0)
        return {};
    LoRa_packet packet;
    packet = std::move(reciev_packet_[0]);
    reciev_packet_.erase(reciev_packet_.begin());
    return packet;
}
// Изъять все пришедшие пакеты
std::vector<LoRa_packet> LoRa_contact_data::get_all_packet() {
    if((!get_signal_complete()) && (stade_communication_ != SC_PACKET_ACCEPTED))
        return {};
    if (stade_communication_ == SC_PACKET_ACCEPTED) {
        stade_communication_ = SC_WAITING;
    }
    else {
        stade_communication_ = SC_DOWNTIME;
    }
    return std::move(reciev_packet_);
}

// Функция работы системы (запускается прерываниями на DIO, 
//   или в цикличной функции с предварительной проверкой DIO)
uint16_t LoRa_contact_data::work_contact_system() {
    if(in_processing_ || (!lora_init_) || (!lora_begin_) || (stade_communication_ == SC_DOWNTIME) || (stade_communication_ == SC_COMPLETE))
        return 0;
    in_processing_ = true;
    uint16_t error = 0;
    // Начало обработки

    // проверить DIO 0 и 1 // (-) ----- 

    // проверить тип обмена данными (работа системы/регистрация) // (+) ----- 

    // при приёме проверить адрес, отреагировать только если отложением запроса, (не факт, пока оставить)
    //    не опускать флаги, остальное всё расписано внутри // (+-) -----

    if(module_state_ == MS_LoRa_RECIEVE) { 
        // read packet -> _receive_packet
        class LoRa_packet read_packet = lora_.receiver_packet(0, 0);
        if(!((read_packet.get_len() < MINIMAL_PACKET_SIZE) || (read_packet.get_crc_error()))) {
            last_receive_packet_.set_packet(read_packet.get_data());

                                                            #if defined( SERIAL_PRINT_ON ) || defined( SERIAL_PRINT_ON_1)
                                                            std::vector<uint8_t> in;
                                                            in = last_receive_packet_.get_packet();
                                                            Serial.print("\t\t\t(+) [");
                                                            for(int i = 0; i < in.size(); ++i) {
                                                                if(in[i] < 16)
                                                                    Serial.print("0");
                                                                Serial.print(in[i], 16);
                                                                if(i < in.size() - 1)
                                                                    Serial.print(" ");
                                                            }
                                                            Serial.println("] <--");
                                                            #endif // SERIAL_PRINT_ON


                                                            #if defined ( ALL_SAVE_PACKET )
                                                            all_packet.push_back(pack(read_packet.get_data(), false, false));
                                                            #endif
        }
        else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.print("\t\t\t(-) ");
                                                            std::vector<uint8_t> in;
                                                            in = read_packet.get_data();
                                                            Serial.print("[");
                                                            for(int i = 0; i < in.size(); ++i) {
                                                                if(in[i] < 16)
                                                                    Serial.print("0");
                                                                Serial.print(in[i], 16);
                                                                if(i < in.size() - 1)
                                                                    Serial.print(" ");
                                                            }
                                                            Serial.println("] <--");
                                                            #endif // SERIAL_PRINT_ON


                                                            #if defined ( ALL_SAVE_PACKET )
                                                            all_packet.push_back(pack(read_packet.get_data(), false, true));
                                                            #endif
            last_receive_packet_.clear_packet();
        }
    }

    // Проверка типа участника соединения
    switch (type_communication_) {
    // Инициатор
    case TC_INITIATOR: {
        // Стадии соединения
        switch (stade_communication_) {
        // Установка соединения
        case SC_CONNECTION: {
            error = init_connection();
            break;
        }
        // Обмен пакетами 
        case SC_EXCHANGE: {
            error = init_exchange();
            break;
        }
        // Разрыв соединения
        case SC_DISCONNECT: {
            error = init_disconnect();
            break;
        }
        // Неизвестная стадия
        default:
            error = 100;
            delay(1000);
            break;
        }
        break;
    }
    // Получатель
    case TC_RECIPIENT: {
        // Стадии соединения
        switch (stade_communication_) {
        // Установка соединения
        case SC_CONNECTION: {
            error = recip_connection();
            break;
        }
        // Обмен пакетами
        case SC_EXCHANGE: {
            error = recip_exchange();
            break;
        }
        // Разрыв соединения
        case SC_DISCONNECT: {
            error = recip_disconnect();
            break;
        }
        // Неизвестная стадия
        default:
            error = 200;
            delay(1000);
            break;
        }
        break;
    }
    // Регистрирующий модуль
    case TC_BROADCAST: {
        // Стадии трансляции
        switch (stade_communication_) {
        // Ожидание пакетов
        case SC_WAITING:
        case SC_PACKET_ACCEPTED: {
            error = broadcast_wait_packet();
            break;
        }
        // Отправка пакетов
        case SC_BROADCASTING: 
        case SC_REPLY_BROADCAST:{
            error = broadcast_send_packet();
            break;
        }
        // Неизвестная стадия
        default:
            error = 300;
            delay(1000);
            break;
        }
        break;
    }
    // Неизвестный тип участника
    default:
        error = 400;
            delay(1000);
        break;
    }
    // Конец обработки
    in_processing_ = false;
    if(error != 0)
        init_ = false;
    return error;
}

// Выдаёт сигнал, если текущая стадия SC_COMPLETE 
bool LoRa_contact_data::get_signal_complete() {
    return (stade_communication_ == SC_COMPLETE);
}



//   ----- ----- ----- ----- ----- ----- -----
// ----- ----- -----  Private  ----- ----- -----
//   ----- ----- ----- ----- ----- ----- -----

void LoRa_contact_data::clear() {
    send_packet_.clear();
    send_flag_.clear();
    reciev_packet_.clear();
    type_communication_ = TC_INITIATOR;
    stade_communication_ = SC_DOWNTIME;
    connection_ = C_EXPECTATION;
    exchange_ = E_BOARDCAST;
    disconnect_ = D_WAITING_CONNECTION_BREAK;
}



// --- функции создания пакетов ---

// создание основы пакета
void LoRa_contact_data::create_packet(uint8_t size, Packet_Type type_packet) {
    // last_send_packet_.creat_packet(size, type_packet);
    // last_send_packet_.packet->set_dest_adr(connect_adr_);
    // last_send_packet_.packet->set_sour_adr(my_adr_);
    // last_send_packet_.packet->set_packet_type(type_packet);
    // last_send_packet_.packet->set_packet_number(num_end_packet_++);
    packet_analyzer.set_dest_adr(last_send_packet_.packet_data, connect_adr_);
    packet_analyzer.set_sour_adr(last_send_packet_.packet_data, my_adr_);
    packet_analyzer.set_packet_type(last_send_packet_.packet_data, type_packet);
    packet_analyzer.set_packet_number(last_send_packet_.packet_data, (num_end_packet_++));
}
// создание пакета установки соединения (num = count_pack) или ответа на 
//   запрос соединения (3 реакции +, +num и -wait) (и при замене инициативы)
bool LoRa_contact_data::create_connect_packet(uint8_t amt_packet, bool swap_type) {
    bool err = false;
    uint8_t size = 11;
    uint8_t com = 0x01; 
    uint8_t len; 
    uint8_t data[2] = {0, 0}; 
    switch (type_communication_) {
    // Инициатор
    case TC_INITIATOR: {
        size = 11;
        com = 0x00; 
        data[0] = amt_packet; 
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, nullptr);
        packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, nullptr);
        break;
    }
    // Получатель
    case TC_RECIPIENT: {
        // 255 - отложен, 0-254 без swap_type - принят, с swap_type - принят с заменой, 
        // connect_adr_ = last_receive_packet_.packet->get_sour_adr();
        connect_adr_ = packet_analyzer.get_sour_adr(last_send_packet_.packet_data);
        if(amt_packet != 255) {
            if(!swap_type) {
                size = 11;
                com = 0x01; 
                data[1] = 0; 
                len = 1;
                create_packet(size, Packet_Type::CONNECTION);
                // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, &len);
                packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, &len);
            }
            else {
                size = 12;
                com = 0x01; 
                data[0] = 0;
                data[1] = amt_packet;
                len = 2;
                create_packet(size, Packet_Type::CONNECTION);
                // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, &len);
                packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, &len);
                
            }
        }
        else {
            size = 11 + (uint8_t)swap_type;
            com = 0x01; 
            data[0] = 1; 
            create_packet(size, Packet_Type::CONNECTION);
            // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, nullptr);
            packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, nullptr);
        }
        break;
    }
    default: 
        err = true; 
        break;
    }
    return err;
}
// создание пакета на основе номера из списка
bool LoRa_contact_data::create_data_packet() {
    bool err = true;
    switch (type_communication_) {
    // Инициатор
    case TC_INITIATOR: 
    case TC_BROADCAST: {
        last_send_packet_.clear_packet();
        for(int i = 0; i < send_packet_.size(); ++i) {
            if(send_flag_[i]) {
                // last_send_packet_.set_packet(send_packet_[i]);
                last_send_packet_.packet_data = send_packet_[i];

                send_flag_[i] = false;
                err = false;
                break;
            }
        }
        break;
    }
    default: break;
    }
    return err;
}
// создание пакета количества пакетов
bool LoRa_contact_data::create_amt_packet() {
    bool err = false;
    switch (type_communication_) {
    // Инициатор
    case TC_INITIATOR: {
        // создание запроса
        uint8_t size = 10;
        uint8_t com = 0x03;
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, nullptr, nullptr);
        packet_connection.set_packet_data(last_send_packet_.packet_data, &com, nullptr, nullptr);
        break;
    }
    // Получатель
    case TC_RECIPIENT: {
        // Изъятие количества пакетов из _receive и создание запроса
        uint8_t size = 11;
        uint8_t com = 0x04;
        uint8_t data[1] = {amt_packet_}; 
        // сделать проверку повторной передачи (?) -----
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, nullptr);
        packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, nullptr);
        break;
    }
    default: 
        err = true; 
        break;
    }
    return err;
}
// Создание пакета номеров пришедших пакетов
bool LoRa_contact_data::create_number_packet() {
    bool err = false;
    switch (type_communication_) {
    // Инициатор
    case TC_INITIATOR: {
        // Создание запроса
        uint8_t size = 10;
        uint8_t com = 0x05;
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, nullptr, nullptr);
        packet_connection.set_packet_data(last_send_packet_.packet_data, &com, nullptr, nullptr);
        break;
    }
    // Получатель
    case TC_RECIPIENT: {
        // Изъятие количества пакетов и их номеров из _receive и создание запроса
        Exchange_packet packet;
        uint8_t com = 0x06; 
        uint8_t len = reciev_packet_.size() * 2;
        uint8_t size = 11 + len;
        uint8_t *data = nullptr;
        if(len != 0)
            data = new uint8_t[len]; 
        for(int i = 0; i < (len / 2); ++i) {
            uint16_t number;

            packet.packet_data = reciev_packet_[i];
            // packet.set_packet(reciev_packet_[i]);

            // number = packet.packet->get_packet_number();
            number = packet_analyzer.get_packet_number(packet.packet_data);
            data[i * 2] = (number >> 8) & 0xFF;
            data[(i * 2) + 1] = number & 0xFF;
        }
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, &len);
        packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, &len);
        if(len != 0)
            delete[] data;
        break;
    }
    default:
        err = true; 
        break;
    }
    return err;
}
// Создание пакета сброса ожидания установки соединения
bool LoRa_contact_data::create_reset_wait_packet() {
    bool err = false;
    switch (type_communication_) {
    // Получатель
    case TC_RECIPIENT: {
        // Создание пакета
        uint8_t size = 10;
        uint8_t com = 0x02;
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, nullptr, nullptr);
                packet_connection.set_packet_data(last_send_packet_.packet_data, &com, nullptr, nullptr);
        break;
    }
    default: 
        err = true; 
        break;
    }
    return err;
}
// Создание пакета разрыва соединения // err переменная класса? 
bool LoRa_contact_data::create_disconnet_packet(bool error) {
    bool err = false;
    init_ = false;
    switch (type_communication_) {
    // Инициатор
    case TC_INITIATOR:
    // Получатель
    case TC_RECIPIENT: {
        // Создание пакета 
        uint8_t size = 11;
        uint8_t com = 0x08;
        uint8_t data[1] = {0}; 
        if(error)
            data[0] = 2;
        create_packet(size, Packet_Type::CONNECTION);
        // static_cast<Packet_Connection*>(last_send_packet_.packet)->set_packet_data(&com, data, nullptr);
        packet_connection.set_packet_data(last_send_packet_.packet_data, &com, data, nullptr);
        break;
    }
    default: 
        err = true; 
        break;
    }
    return err;
} 



// --- функции обработки пакетов ---

// Проверка пакета на "пришёл" и "мой|глобальный", и от того, кого
//   я жду | от всех (при глобальном адресе ожидания - тоже все)
bool LoRa_contact_data::packet_dont_correct(bool global_adr, bool all_adr_sendler) {
    // Проверка размера пакета
    if(last_receive_packet_.packet_data.get_len() < MINIMAL_PACKET_SIZE)
        return false;
    // Проверка "пакет отправлен мне"
    // if(last_receive_packet_.packet->get_dest_adr() != my_adr_) {
    if(packet_analyzer.get_dest_adr(last_receive_packet_.packet_data) != my_adr_) {
        // Проверка "пакет отправлен всем"
        // if(!(global_adr && (last_receive_packet_.packet->get_dest_adr().global()))) {
        if(!(global_adr && (packet_analyzer.get_dest_adr(last_receive_packet_.packet_data).global()))) {
            return false;
        }
    }
    // Проверка "мне не важно кто отправлял"
    if(!((connect_adr_.global()) || all_adr_sendler))
        // Проверка "мне важно кто отправил"
        // if(last_receive_packet_.packet->get_sour_adr() != connect_adr_)
        if(packet_analyzer.get_sour_adr(last_receive_packet_.packet_data) != connect_adr_)
            return false;
    return true;
}
// Проверка соответствия типа пакета
bool LoRa_contact_data::check_packet_type(Packet_Type type_packet, const std::vector<uint8_t>& subtype_packet) {
    if(last_receive_packet_.type_packet != type_packet)
        return false;
    if(subtype_packet.size() == 0)
        return true;
    // (-!) -----
    for(int i = 0; i < subtype_packet.size(); ++i)
        if(last_receive_packet_.packet_data[9] == subtype_packet[i])
            return true;
    return false;
}

// поиск номера в send_packet_
int16_t LoRa_contact_data::search_num_packet(uint16_t number) {
    // class Exchange_packet packet;
    for(int16_t i = 0; i < send_packet_.size(); ++i) {
        // packet.set_packet(send_packet_[i]);
        if(number == packet_analyzer.get_packet_number(send_packet_[i]))
            return i;
        // packet.set_packet(send_packet_[i]);
        // if(packet.packet->get_packet_number() == number)
        //     return i;

    }
    return -1;
}
// удаление номеров в send_packet_ и обновление send_flag_
uint8_t LoRa_contact_data::pop_num_packet(const std::vector<uint16_t>& number) {
    int i;
    for(i = 0; i < number.size(); ++i) {
        int16_t search = search_num_packet(number[i]);
        if(search == -1)
            break;
        send_packet_.erase(send_packet_.begin() + search);
    }
    send_flag_.clear();
    amt_packet_ = send_packet_.size();
    for(int j = 0; j < amt_packet_; ++j)
        send_flag_.push_back(true);
    return i;
}
// сортировка reciev_packet_
void LoRa_contact_data::sort_num_packet() {
    // class Exchange_packet packet;
    // LoRa_packet buffer;


    uint16_t number1, number2;
    bool swap;
    for(int i = 0; i < (reciev_packet_.size() - 1);) {
        swap = false;

        // packet.set_packet(reciev_packet_[i]);
        // number1 = packet.packet->get_packet_number();
        // packet.set_packet(reciev_packet_[i+1]);
        // number2 = packet.packet->get_packet_number();

        number1 = packet_analyzer.get_packet_number(reciev_packet_[i]);
        number2 = packet_analyzer.get_packet_number(reciev_packet_[i+1]);

        if(number1 == 0) {
            if(0xFF00 <= number2)
                swap = true;
        }
        else if (number2 == 0) {
            if(number1 <= 0x00FF)
                swap = true; 
        } 
        else {
            if(number2 < number1)
                swap = true;
        }
        if(swap) {

            std::swap(reciev_packet_[i], reciev_packet_[i+1]);
            // buffer = reciev_packet_[i];
            // reciev_packet_[i] = reciev_packet_[i+1];
            // reciev_packet_[i+1] = buffer;

            if(i != 0)
                --i;
        }
        else
            ++i;
    }
}



// --- функции контроля и обработки состояний ---

// Инициализатор
uint16_t LoRa_contact_data::init_connection() {
    uint16_t error = 0;
    // Подстадии соединения "Установка соединения"
    switch (connection_) {
    // Ожидание реакции на запрос
    case C_EXPECTATION: { 
        error = init_connection_expect();
        break;
    }
    // Ожидание сброса пакета
    case C_WAIT_RESET: { 
        error = init_connection_wait();
        break;
    }
    default:
        error = 110;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::init_connection_expect() {
    uint16_t error = 0;
    //// ----- Пути -----
    // - Прошло время, ответа нет => повтор отправки пакета
    // - Ответ свидетельствующий о том, что соединение было отложено
    //   (стадия _connection = C_WAIT_RESET)
    // - Ответ об установке соединения
    //   (стадия _exchange = E_BOARDCAST)
    //   (стадия stade_communication_ = SC_EXCHANGE)
    //// ----- ---- -----

    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета - перейти в режим приёма пакета
    case MS_LoRa_SEND: {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.C.E.!S!");
                                                            #endif // SERIAL_PRINT_ON
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета -
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct()) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.C.E.!R!");
                                                            #endif // SERIAL_PRINT_ON
            // Превышено время ожидания, или чужой пакет
            if(millis() - time_last_packet_ < time_constraints_.I_connect) {
                set_LoRa_mode_receive();
            }
            else {
                if(send_packet_amt_ < packet_constraints_.I_connect) {
                    ++send_packet_amt_;
                    set_LoRa_mode_send();
                }
                else {
                    error = 112;
                    // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                    stade_communication_ = SC_DOWNTIME;
                    create_disconnet_packet(true);
                    set_LoRa_mode_send(true);
                }
            }
        }
        else {
            // Пришёл ответ на запрос
            if(check_packet_type(Packet_Type::CONNECTION, {0x01})) {
                uint8_t data[2] = {0, 0}; 
                uint8_t com = 0x01; 
                uint8_t len = 1;
                if(amt_packet_ == 0xFF)
                    len = 2;
                // if (static_cast<Packet_Connection*>(last_receive_packet_.packet)->get_packet_data(&com, data, &len) != 0) {
                if (packet_connection.get_packet_data(last_receive_packet_.packet_data, &com, data, &len) != 0) {
                    Serial.print("err!!!");
                }
                if(data[0] == 0) {
                    if(amt_packet_ == 0xFF) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.C.E.!R!.C.R");
                                                            #endif // SERIAL_PRINT_ON
                        // set adr (не нужно)
                        type_communication_ = TC_RECIPIENT;
                        stade_communication_ = SC_EXCHANGE;
                        exchange_ = E_BOARDCAST;

                        expected_amt_packet_ = data[0];
                        amt_packet_ = 0;
                        // подготовка пакета
                        create_connect_packet(); // уже не инициатор 
                        set_LoRa_mode_send(true);
                    }
                    else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.C.E.!R!.C.S");
                                                            #endif // SERIAL_PRINT_ON
                        exchange_ = E_BOARDCAST;
                        stade_communication_ = SC_EXCHANGE;
                        // режим отправки массива пакетов
                        create_data_packet();
                        set_LoRa_mode_send(true);
                    }
                }
                else { 
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.C.E.!R!.C.W");
                                                            #endif // SERIAL_PRINT_ON
                    // контакт отложен 
                    connection_ = C_WAIT_RESET;
                    set_LoRa_mode_receive();
                }
            }
            else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.C.E.!R!.E");
                                                            #endif // SERIAL_PRINT_ON
                set_LoRa_mode_receive(); // ??? (?) -----
            }
        }
        break;
    }
    default:
        error = 111;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::init_connection_wait() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Пакет о сбросе ожидания соединения => повтор отправки пакета
    //   (стадия _connection = C_EXPECTATION)
    //-// - Прошло длительное время (~часы), ответа нет => повтор отправки пакета
    //-//   (стадия _connection = C_EXPECTATION)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета - перейти в режим приёма пакета
    case MS_LoRa_SEND: {
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета -
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct()) {
            // .. превышено время ожидания, или чужой пакет
            if(millis() - time_last_packet_ < time_constraints_.I_wait_connect) {
                set_LoRa_mode_receive();
            }
            else {
                error = 114;
                // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                stade_communication_ = SC_DOWNTIME;
                create_disconnet_packet(true);
                set_LoRa_mode_send(true);
            }
        }
        else {
            if(check_packet_type(Packet_Type::CONNECTION, {0x02})) {
                connection_ = C_EXPECTATION; // ??? (?) -----
            }
            else {
                set_LoRa_mode_receive();
            }
        }
        break;
    }
    default:
        error = 113;
        break;
    }
    return error;
}

uint16_t LoRa_contact_data::init_exchange() {
    uint16_t error = 0;
    // Подстадии соединения "Обмен пакетами"
    switch (exchange_) {
    // Передача пакетов
    case E_BOARDCAST: { 
        error = init_exchange_boardcast();
        break;
    }
    // Ожидание подтверждения получения пакетов
    case E_WAITING_CONFIRMATION: { 
        error = init_exchange_wait_confirmation();
        break;
    }
    // Ожидание номеров полученных пакетов
    case E_WAITING_NUMBERS: {
        error = init_exchange_wait_numbers();
        break;
    }
    default:
        error = 120;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::init_exchange_boardcast() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Завершена отправка пакета, но не всех пакетов => отправка следующего пакета
    // - Завершена отправка последнего пакета => отправка запроса подтверждения
    //   (стадия _exchange = E_WAITING_CONFIRMATION)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
        bool all_send = true;
        for(int i = 0; i < send_flag_.size(); ++i) {
            if(send_flag_[i] == true) {
                all_send = false;
                break;
            }
        }
        if(all_send) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.B.!S!.E");
                                                            #endif // SERIAL_PRINT_ON
            exchange_ = E_WAITING_CONFIRMATION;
            // подготовка пакета запроса
            create_amt_packet(); 
            set_LoRa_mode_send();
        }
        else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.B.!S!.S");
                                                            #endif // SERIAL_PRINT_ON
            // режим отправки массива пакетов
            create_data_packet();
            set_LoRa_mode_send();
        }
        break;
    }
    default:
        error = 121;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::init_exchange_wait_confirmation() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Прошло время, ответа нет => повтор отправки запроса подтверждения
    // - Прошло время, ответа нет, и отправлено было слишком много запросов => отправить пакет о разрыве соединения с ошибкой
    //   (стадия stade_communication_ = SC_DISCONNECT)
    // - Есть ответ, количество принятых пакетов = отправленных => отправить пакет о разрыве соединения
    //   (стадия stade_communication_ = SC_DISCONNECT)
    // - Есть ответ, количество принятых пакетов < отправленных => отправить зпрос полученных номеров
    //   (стадия _exchange = E_WAITING_NUMBERS)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.C.!S!");
                                                            #endif // SERIAL_PRINT_ON
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct()) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.C.!R!.E");
                                                            #endif // SERIAL_PRINT_ON
            // .. превышено время ожидания, или чужой пакет
            if(millis() - time_last_packet_ < time_constraints_.I_amt_pack) {
                set_LoRa_mode_receive();
            }
            else {
                if(send_packet_amt_ < packet_constraints_.I_amt_pack) {
                    ++send_packet_amt_;
                    set_LoRa_mode_send();
                }
                else {
                    error = 123;
                    // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                    stade_communication_ = SC_DOWNTIME;
                    create_disconnet_packet(true);
                    set_LoRa_mode_send(true);
                }
            }
        }
        else {
            // .. пришёл ответ на запрос
            if(check_packet_type(Packet_Type::CONNECTION, {0x04})) {

                uint8_t data[1] = {0}; 
                uint8_t com = 0x04; 
                uint8_t len = 1;
                // if (static_cast<Packet_Connection*>(last_receive_packet_.packet)->get_packet_data(&com, data, &len) != 0) {
                if (packet_connection.get_packet_data(last_receive_packet_.packet_data, &com, data, &len) != 0) {
                    Serial.print("err!!!");
                }
                if(data[0] == amt_packet_) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.C.!R!.C.+");
                                                            #endif // SERIAL_PRINT_ON
                    // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                    contact_complete();
                    create_disconnet_packet();
                    set_LoRa_mode_send(true);
                }
                else { 
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.C.!R!.C.-");
                                                            #endif // SERIAL_PRINT_ON
                    // количество принятых пакетов < отправленных
                    exchange_ = E_WAITING_NUMBERS;
                    // подготовка пакета запроса
                    create_number_packet();
                    set_LoRa_mode_send();
                }
            }
            else {
                set_LoRa_mode_receive(); // ??? (?) -----
            }
        }
        break;
    }
    default:
        error = 122;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::init_exchange_wait_numbers() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Прошло время, ответа нет => повтор отправки запроса полученных номеров
    // - Прошло время, ответа нет, и отправлено было слишком много запросов => отправить пакет о разрыве соединения с ошибкой
    //   (стадия stade_communication_ = SC_DISCONNECT)
    // - Есть ответ => выбрать пакеты для отправки и отправить пакет 
    //   (стадия _exchange = E_BOARDCAST)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.W.!S!");
                                                            #endif // SERIAL_PRINT_ON
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct()) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.W.!R!.E");
                                                            #endif // SERIAL_PRINT_ON
            // .. превышено время ожидания, или чужой пакет
            if(millis() - time_last_packet_ < time_constraints_.I_num_pack) {
                set_LoRa_mode_receive();
            }
            else {
                if(send_packet_amt_ < packet_constraints_.I_num_pack) {
                    ++send_packet_amt_;
                    set_LoRa_mode_send();
                }
                else {
                    error = 125;
                    // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                    stade_communication_ = SC_DOWNTIME;
                    create_disconnet_packet(true);
                    set_LoRa_mode_send(true);
                }
            }
        }
        else {
            // .. пришёл ответ на запрос
            if(check_packet_type(Packet_Type::CONNECTION, {0x06})) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.W.!R!.N");
                                                            #endif // SERIAL_PRINT_ON
                exchange_ = E_BOARDCAST;
                // выбор номеров в data на основе принятых пакетов (?) -----
                // и исключение старых из списка, у оставшихся поднять флаги
                uint8_t size = 0;
                // static_cast<Packet_Connection*>(last_receive_packet_.packet)->get_size_by_packet(&size);
                packet_connection.get_size_by_packet(last_receive_packet_.packet_data, size);
                uint8_t *data = new uint8_t[size];
                uint8_t com = 0xFF;
                uint8_t len = 0xFF;
                // static_cast<Packet_Connection*>(last_receive_packet_.packet)->get_packet_data(&com, data, &len);
                packet_connection.get_packet_data(last_receive_packet_.packet_data, &com, data, &len);
                if (com != 0x06) {
                    if(data != nullptr)
                        delete[] data;
                    data = nullptr;
                    error = 126;
                    // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                    stade_communication_ = SC_DOWNTIME;
                    create_disconnet_packet(true);
                    set_LoRa_mode_send(true);
                }
                else {
                    std::vector<uint16_t> pack_num;
                    for(int i = 0; i < len/2; ++i) {
                        pack_num.push_back((((uint16_t)data[i*2]) << 8) | (data[i*2+1]));
                    }
                    pop_num_packet(pack_num);
                    if(data != nullptr)
                        delete[] data;
                    // подготовка пакета (режим обмена)
                    create_data_packet();
                    set_LoRa_mode_send(true);
                }
            }
            else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("I.E.W.!R!.N.E");
                                                            #endif // SERIAL_PRINT_ON
                set_LoRa_mode_receive(); // ??? (?) -----
            }
        }
        break;
    }
    default:
        error = 124;
        break;
    }
    return error;
}

uint16_t LoRa_contact_data::init_disconnect() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Отправить пакет о разрыве соединения
    //   (стадия stade_communication_ = SC_DOWNTIME)
    // - Отправить пакет о разрыве соединения с ошибкой
    //   (стадия stade_communication_ = SC_DOWNTIME)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
        //   _стадия stade_communication_ = SC_DOWNTIME|SC_COMPLETE;  // (!) -----
        //   contact_complete();
        break;
    }
    default:
        error = 131;
        break;
    }
    return error;
}

// Получатель
uint16_t LoRa_contact_data::recip_connection() {
    uint16_t error = 0;
    // Подстадии соединения "Установка соединения"
    switch (connection_) {
    // Проверка очереди на соединение
    case C_QUEUE_CHECK: {
        error = recip_connection_queue_check();
        break;
    }
    // Ожидание запроса на соединение
    case C_AWAITING_REQUEST: {
        error = recip_connection_wait_request();
        break;
    }
    default:
        error = 210;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::recip_connection_queue_check() {
    uint16_t error = 0;
    // ----- Пути -----
    // - При пустой очереди просто меняем стадию 
    //   (стадия _connection = C_AWAITING_REQUEST)
    // - Сброс ожидания определённого модуля => отправить пакет сброса
    //   (стадия _connection = C_AWAITING_REQUEST)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
        if(wait_connect_.size() != 0) {
            connect_adr_ = wait_connect_[0];
            wait_connect_.erase(wait_connect_.begin());
            // подготовка пакета
            send_wait_reset_packet_ = true;
            create_reset_wait_packet();
            set_LoRa_mode_send();
        }
        else {
            connect_adr_ = wait_adr_;
        }
        connection_ = C_AWAITING_REQUEST;
        break;
    }
    default:
        error = 211;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::recip_connection_wait_request() {
    uint16_t error = 0;
    // ----- Пути -----
    // - пропуск (зацикливание бесконечного ожидания)
    // - Получение ппакета запроса => отправка ответа на запрос
    //   (стадия _exchange = E_EXPECTATION)
    //   (стадия stade_communication_ = SC_EXCHANGE)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.C.A.!S!");
                                                            #endif // SERIAL_PRINT_ON
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct(true)) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.C.A.!R!");
                                                            #endif // SERIAL_PRINT_ON
            // .. превышено время ожидания, или чужой пакет
            if(!packet_dont_correct(true, true)) {
                if (send_wait_reset_packet_)
                {
                    if(millis() - time_last_packet_ < time_constraints_.R_connect) {
                        set_LoRa_mode_receive();
                    }
                    else {
                        if(send_packet_amt_ < packet_constraints_.R_wait_connect) {
                            ++send_packet_amt_;
                            set_LoRa_mode_send();
                        }
                        else {
                            error = 213;
                            connection_ = C_QUEUE_CHECK;
                        }
                    }
                }
                else {
                    set_LoRa_mode_receive();
                }
            }
            else {
                if(check_packet_type(Packet_Type::CONNECTION, {0x00}) && wait_adr_.global()) {
                    // добавить в очередь _wait_connect (?) ----- ----- -----
                    // wait_connect_.push_back(last_receive_packet_.packet->get_dest_adr());
                    wait_connect_.push_back(packet_analyzer.get_dest_adr(last_receive_packet_.packet_data));
                    // подготовка пакета
                    create_connect_packet(0xFF); // отложен
                    set_LoRa_mode_send();
                }
                else {
                    set_LoRa_mode_receive();
                }
            }
        }
        else {
            // .. пришёл запрос
            if(check_packet_type(Packet_Type::CONNECTION, {0x00})) {
                send_wait_reset_packet_ = false;
                uint8_t data[1] = {0}; 
                uint8_t com = 0x00; 
                uint8_t len = 1;
                // if (static_cast<Packet_Connection*>(last_receive_packet_.packet)->get_packet_data(&com, data, &len) != 0) {
                if (packet_connection.get_packet_data(last_receive_packet_.packet_data, &com, data, &len) != 0) {
                    Serial.print("err!!!");
                }
                if(data[0] == 0xFF) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.C.A.!R!.S");
                                                            #endif // SERIAL_PRINT_ON
                    // creat init packet (другой, в качестве ответа,
                    //   устройство должно ждать такого запроса и 
                    //   подготовить пакеты/реакцию на него)
                    // подготовка пакета
                    connect_adr_ = packet_analyzer.get_sour_adr(last_receive_packet_.packet_data); // (?) -----
                    // connect_adr_ = last_receive_packet_.packet->get_sour_adr(); // (?) -----
                    create_connect_packet(send_packet_.size(), true);
                    set_LoRa_mode_send();
                    type_communication_ = TC_INITIATOR;
                    stade_communication_ = SC_CONNECTION;
                    connection_ = C_EXPECTATION;
                }
                else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.C.A.!R!.C");
                                                            #endif // SERIAL_PRINT_ON
                    // _exchange = E_BOARDCAST;
                    exchange_ = E_EXPECTATION;
                    stade_communication_ = SC_EXCHANGE;

                    expected_amt_packet_ = data[0];
                    amt_packet_ = 0;
                    // подготовка пакета
                    create_connect_packet();
                    set_LoRa_mode_send();
                }
            }
        }
        break;
    }
    default:
        error = 212;
        break;
    }
    return error;
}

uint16_t LoRa_contact_data::recip_exchange() {
    uint16_t error = 0;
    // Подстадии соединения "Обмен пакетами"
    switch (exchange_) {
    // Ожидание пакетов
    case E_EXPECTATION: { 
        error = recip_exchange_expect();
        break;
    }
    // Ожидание реакции на подтверждение
    case E_WAITING_REACTION: {
        error = recip_exchange_wait_reaction();
        break;
    }
    default:
        error = 220;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::recip_exchange_expect() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Получен пакет => запомнить, ждать следующего
    // - Получен запрос на подтверждение => отправить подтверждение
    //   (стадия _exchange = E_WAITING_REACTION)
    // - Прошло много времени, после последего приёма пакета => отправить подтверждение
    //   (стадия _exchange = E_WAITING_REACTION)
    // ----- ---- -----
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.E.!S!");
                                                            #endif // SERIAL_PRINT_ON
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct()) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.E.!R!.E");
                                                            #endif // SERIAL_PRINT_ON
            if(millis() - time_last_packet_ < time_constraints_.R_data_pack) {
                set_LoRa_mode_receive();
            }
            else {
                // подготовка пакета подтверждения о принятии N пакетов
                create_amt_packet(); 
                set_LoRa_mode_send();
                exchange_ = E_WAITING_REACTION;
            }
        }
        else {
            if(check_packet_type(Packet_Type::CONNECTION, {0x03})) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.E.!R!.C.3");
                                                            #endif // SERIAL_PRINT_ON
                // подготовка пакета подтверждения о принятии N пакетов
                create_amt_packet(); 
                set_LoRa_mode_send();
                exchange_ = E_WAITING_REACTION;
            }
            // (!) ----- заменить if ниже на "не пакеты контакта", кроме настроек и т.п.
            // (-) ----- пока только датчики, устройства и системные пакеты, а нужно сделать не пакеты контакта
            else if(check_packet_type(Packet_Type::DEVICE) ||
                    check_packet_type(Packet_Type::SENSOR) ||
                    check_packet_type(Packet_Type::SYSTEM)) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.E.!R!.C.D");
                                                            #endif // SERIAL_PRINT_ON
                // запомнить  // (+) -----
                reciev_packet_.push_back(last_receive_packet_.get_packet());
                ++amt_packet_;
                set_LoRa_mode_receive();
            }
            else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.E.!R!.C.X");
                                                            #endif // SERIAL_PRINT_ON
                set_LoRa_mode_receive(); // ??? (?) -----
            }
        }
        break;
    }
    default:
        error = 221;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::recip_exchange_wait_reaction() {
    uint16_t error = 0;
    // ----- Пути -----
    // - Прошло время, ответа нет => ожидать запроса подтверждения
    // - Прошло время, ответа нет, и прошло слишком много времени => отправить пакет о разрыве соединения с ошибкой
    //   (стадия _disconnect = D_WAITING_CONNECTION_BREAK)
    //   (стадия stade_communication_ = SC_DISCONNECT)
    // - Есть ответ, зпрос полученных номеров => отправить ответ с номерами полученных пакетов
    //   (стадия _exchange = E_EXPECTATION)
    // - Есть ответ, разрыв соединения
    //   (стадия _disconnect = D_WAITING_CONNECTION_BREAK)
    //   (стадия stade_communication_ = SC_DISCONNECT)
    // ----- ---- ----- 
    // Проверка состояние модуля
    switch (module_state_) {
    // отправка пакета
    case MS_LoRa_SEND: {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.W.!S!");
                                                            #endif // SERIAL_PRINT_ON
        set_LoRa_mode_receive();
        break;
    }
    // приём пакета
    case MS_LoRa_RECIEVE: {
        if(!packet_dont_correct()) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.W.!R!.E");
                                                            #endif // SERIAL_PRINT_ON
            // .. превышено время ожидания, или чужой пакет
            if(millis() - time_last_packet_ < time_constraints_.R_correct_pack) {
                set_LoRa_mode_receive();
            }
            else {
                if(send_packet_amt_ < packet_constraints_.R_correct_pack) {
                    ++send_packet_amt_;
                    set_LoRa_mode_send();
                }
                else {
                    if(expected_amt_packet_ != reciev_packet_.size()) {
                        error = 223;
                        // _стадия _disconnect = D_WAITING_CONNECTION_BREAK;
                        // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                        stade_communication_ = SC_DOWNTIME;
                        create_disconnet_packet(true);
                        set_LoRa_mode_send(true);
                    }
                    else {
                        sort_num_packet();
                        contact_complete();
                    }
                }
            }
        }
        else {
            if(check_packet_type(Packet_Type::CONNECTION, {0x05})) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.W.!R!.C.1");
                                                            #endif // SERIAL_PRINT_ON
                // подготовка пакета с номерами полученных пакетов
                create_number_packet();
                set_LoRa_mode_send();
                amt_packet_ = 0;
                exchange_ = E_EXPECTATION;
            }
            else if(check_packet_type(Packet_Type::CONNECTION, {0x08})) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.W.!R!.C.2");
                                                            #endif // SERIAL_PRINT_ON
                // _стадия _disconnect = D_WAITING_CONNECTION_BREAK;
                // _стадия stade_communication_ = SC_DISCONNECT; // (!) -----
                sort_num_packet();
                contact_complete();
                //correct end
            }
            else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("R.E.W.!R!.C.3");
                                                            #endif // SERIAL_PRINT_ON
                set_LoRa_mode_receive(); // ??? (?) -----
            }
        }
        break;
    }
    default:
        error = 222;
        break;
    }
    return error;
}

uint16_t LoRa_contact_data::recip_disconnect() {
    uint16_t error = 0;
    // Подстадии соединения "Разрыв соединения"
    switch (disconnect_) {
    // Ожидание разрыва соединения
    case D_WAITING_CONNECTION_BREAK: { 
        // ----- Пути -----
        // - Получен пакет о разрыве соединения
        //   (стадия stade_communication_ = SC_COMPLETE)
        // ----- ---- -----
        // Проверка состояние модуля
        switch (module_state_) {
        // отправка пакета
        case MS_LoRa_SEND: {
            set_LoRa_mode_receive();
            break;
        }
        // приём пакета
        case MS_LoRa_RECIEVE: {
            if(!packet_dont_correct()) {
                if(millis() - time_last_packet_ < time_constraints_.R_disconnect) {
                    set_LoRa_mode_receive();
                }
                else {
                    // (!) ----- error connect
                    stade_communication_ = SC_DOWNTIME;
                    create_disconnet_packet(true);
                    set_LoRa_mode_send(true);
                }
            }
            else {
                if(check_packet_type(Packet_Type::CONNECTION, {0x08})) {
                    //???//  // (!) ----- complete connect
                    sort_num_packet();
                    contact_complete();
                }
            }
            break;
        }
        default:
            error = 231;
            break;
        }
        break;
    }
    default:
        error = 230;
        break;
    }
    return error;
}



// --- функции контроля и обработки состояний при трансляции ---

uint16_t LoRa_contact_data::broadcast_wait_packet() {
    uint16_t error = 0;
    switch (module_state_) {
    case MS_LoRa_SEND: {
        set_LoRa_mode_receive();
        break;
    }
    case MS_LoRa_RECIEVE: {
        // получить пакет, если не свой - игнорь, если свой - запомни и перейди в режим SC_PACKET_ACCEPTED, продолжай принимать
        if(!packet_dont_correct(true, true)) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("T.W.!R!");
                                                            #endif // SERIAL_PRINT_ON
            set_LoRa_mode_receive();
        }
        else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("T.W.!R!");
                                                            #endif // SERIAL_PRINT_ON
            reciev_packet_.push_back(last_receive_packet_.get_packet());
            stade_communication_ = SC_PACKET_ACCEPTED;
            set_LoRa_mode_receive();
        }
        break;
    }
    default:
        error = 310;
        break;
    }
    return error;
}
uint16_t LoRa_contact_data::broadcast_send_packet() {
    uint16_t error = 0;
    switch (module_state_) {
    case MS_LoRa_SEND: {
        // отправлять, если кончились перейти в режим complete
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("T.S.!S!");
                                                            #endif // SERIAL_PRINT_ON
        bool all_send = true;
        for(int i = 0; i < send_flag_.size(); ++i) {
            if(send_flag_[i] == true) {
                all_send = false;
                break;
            }
        }
        if(all_send) {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("T.S.!S!.E");
                                                            #endif // SERIAL_PRINT_ON
            send_packet_.clear();
            send_flag_.clear();
            // (?) ----- Возможность транслировать пакеты, во время приёма
            if(stade_communication_ == SC_BROADCASTING) {
                stade_communication_ = SC_COMPLETE;
                set_LoRa_mode_sleep();
            }
            else {
                if(reciev_packet_.size() == 0) {
                    stade_communication_ = SC_WAITING;
                }
                else {
                    stade_communication_ = SC_PACKET_ACCEPTED;
                }
                set_LoRa_mode_receive();
            }
        }
        else {
                                                            #ifdef SERIAL_PRINT_ON
                                                            Serial.println("T.S.!S!.S");
                                                            #endif // SERIAL_PRINT_ON
            create_data_packet();
            set_LoRa_mode_send();
        }
        break;
    }
    default:
        error = 320;
        break;
    }
    return error;
}



// --- Функции общения с LoRa ---

void LoRa_contact_data::contact_complete() {
    stade_communication_ = SC_COMPLETE;
    set_LoRa_mode_sleep();
}
void LoRa_contact_data::set_LoRa_mode_receive() {
    module_state_ = MS_LoRa_RECIEVE;
    lora_.mode_sleep();
    delay(5);
    lora_.receiver_packet(1, 0);
}
void LoRa_contact_data::set_LoRa_mode_send(bool first) {
    module_state_ = MS_LoRa_SEND;
    time_last_packet_ = millis();
    if(first)
        time_first_packet_ = time_last_packet_;

    lora_.mode_sleep();
    delay(7);
    lora_.mode_FSTX();
    delay(7);
                                                            #if defined( SERIAL_PRINT_ON ) || defined( SERIAL_PRINT_ON_1)
                                                            std::vector<uint8_t> out;
                                                            out = last_send_packet_.get_packet();
                                                            lora_.sender_packet(out, false);
                                                            Serial.print("\t\t\t--> [");
                                                            for(int i = 0; i < out.size(); ++i) {
                                                                if(out[i] < 16)
                                                                    Serial.print("0");
                                                                Serial.print(out[i], 16);
                                                                if(i < out.size() - 1)
                                                                    Serial.print(" ");
                                                            }
                                                            Serial.println("]");
                                                            #else
    lora_.sender_packet(last_send_packet_.get_packet(), false);
    #endif // - SERIAL_PRINT_ON


                                                            #if defined ( ALL_SAVE_PACKET )
                                                            all_packet.push_back(pack(last_send_packet_.get_packet(), true));
                                                            #endif
}
void LoRa_contact_data::set_LoRa_mode_sleep() {
    module_state_ = MS_LoRa_SLEEP;
    // lora mode
    lora_.mode_sleep();
}
